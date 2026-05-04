#include "AIEnemy/EnemyBase.h"
#include "AIEnemy/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"

// Core movement uses AI MoveToActor; level designers must place Nav Mesh Bounds Volume and rebuild navigation.

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	FindDefaultTarget();
	SetEnemyState(TargetActor ? EEnemyAIState::Chase : EEnemyAIState::Idle);
}

void AEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsAlive() || bIsLightFrozen)
	{
		return;
	}

	UpdateLightExposure(DeltaSeconds);

	if (!IsValid(TargetActor))
	{
		FindDefaultTarget();
	}

	UpdateState();

	switch (EnemyState)
	{
	case EEnemyAIState::Idle:
		StopEnemyMovement();
		break;
	case EEnemyAIState::Chase:
		UpdateChase();
		break;
	case EEnemyAIState::Attack:
		UpdateAttack();
		break;
	case EEnemyAIState::Dead:
	default:
		break;
	}
}

float AEnemyBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!IsAlive() || AppliedDamage <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - AppliedDamage, 0.0f, MaxHealth);

	UE_LOG(LogTemp, Verbose, TEXT("%s took %.1f damage. Health: %.1f / %.1f"),
		*GetNameSafe(this), AppliedDamage, CurrentHealth, MaxHealth);

	OnEnemyDamaged.Broadcast(AppliedDamage, CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return AppliedDamage;
}

void AEnemyBase::OnLightHit(float Intensity, float Duration)
{
	if (!IsAlive())
	{
		return;
	}

	const float ClampedDuration = FMath::Max(0.0f, Duration);
	const float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	LightExposure = FMath::Max(0.0f, LightExposure + ClampedIntensity * ClampedDuration);

	if (LightDeathExposureThreshold > 0.0f && LightExposure >= LightDeathExposureThreshold)
	{
		Die();
		return;
	}

	float SpeedMultiplier = 1.0f;
	if (LightExposure >= LightFreezeExposureThreshold)
	{
		SpeedMultiplier = LightFreezeSpeedMultiplier;
	}
	else if (LightExposure >= LightSlowExposureThreshold)
	{
		SpeedMultiplier = LightSlowSpeedMultiplier;
	}

	bIsLightFrozen = ClampedDuration > 0.0f && SpeedMultiplier <= KINDA_SMALL_NUMBER;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed * SpeedMultiplier;

	if (bIsLightFrozen)
	{
		StopEnemyMovement();
	}

	if (ClampedDuration > 0.0f)
	{
		GetWorldTimerManager().ClearTimer(LightFreezeTimerHandle);
		GetWorldTimerManager().SetTimer(LightFreezeTimerHandle, this, &AEnemyBase::RestoreMovementAfterLight, ClampedDuration, false);
	}
}

void AEnemyBase::OnFlashbangHit(float DamageAmount)
{
	if (!IsAlive())
	{
		return;
	}

	const float DamageToApply = DamageAmount > 0.0f ? DamageAmount : DefaultFlashbangDamage;
	UGameplayStatics::ApplyDamage(this, DamageToApply, nullptr, nullptr, UDamageType::StaticClass());
}

void AEnemyBase::SetTargetActor(AActor* NewTarget)
{
	TargetActor = NewTarget;
	UpdateState();
}

void AEnemyBase::FindDefaultTarget()
{
	TargetActor = UGameplayStatics::GetPlayerCharacter(this, 0);
}

void AEnemyBase::UpdateState()
{
	if (!IsAlive())
	{
		SetEnemyState(EEnemyAIState::Dead);
		return;
	}

	if (!IsValid(TargetActor))
	{
		SetEnemyState(EEnemyAIState::Idle);
		return;
	}

	SetEnemyState(IsTargetInAttackRange() ? EEnemyAIState::Attack : EEnemyAIState::Chase);
}

void AEnemyBase::UpdateChase()
{
	AAIController* EnemyController = Cast<AAIController>(GetController());
	if (!EnemyController || !IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot chase: missing controller or target"), *GetNameSafe(this));
		return;
	}

	const EPathFollowingRequestResult::Type MoveResult = EnemyController->MoveToActor(TargetActor, ChaseAcceptanceRadius);
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to request path to %s"), *GetNameSafe(this), *GetNameSafe(TargetActor));
	}
}

void AEnemyBase::UpdateAttack()
{
	StopEnemyMovement();

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastAttackTime < AttackCooldown)
	{
		return;
	}

	LastAttackTime = CurrentTime;
	PerformAttack(TargetActor);
}

void AEnemyBase::PerformAttack_Implementation(AActor* Target)
{
	if (!IsValid(Target) || !IsTargetInAttackRange())
	{
		return;
	}

	UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
}

void AEnemyBase::Die()
{
	if (EnemyState == EEnemyAIState::Dead)
	{
		return;
	}

	CurrentHealth = 0.0f;
	SetEnemyState(EEnemyAIState::Dead);
	StopEnemyMovement();
	GetCharacterMovement()->DisableMovement();
	SetActorEnableCollision(false);
	OnEnemyDied.Broadcast(this);

	SetLifeSpan(FMath::Max(0.0f, CorpseLifeSpan));
}

void AEnemyBase::SetEnemyState(EEnemyAIState NewState)
{
	if (EnemyState == NewState)
	{
		return;
	}

	EnemyState = NewState;
	UE_LOG(LogTemp, Verbose, TEXT("%s state changed to %s"), *GetNameSafe(this), *UEnum::GetValueAsString(EnemyState));
}

bool AEnemyBase::IsTargetInAttackRange() const
{
	return IsValid(TargetActor) && FVector::DistSquared(GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(AttackRange);
}

void AEnemyBase::StopEnemyMovement()
{
	if (AAIController* EnemyController = Cast<AAIController>(GetController()))
	{
		EnemyController->StopMovement();
	}
}

void AEnemyBase::UpdateLightExposure(float DeltaSeconds)
{
	if (LightExposure <= 0.0f || LightExposureDecayPerSecond <= 0.0f)
	{
		return;
	}

	LightExposure = FMath::Max(0.0f, LightExposure - LightExposureDecayPerSecond * DeltaSeconds);
}

void AEnemyBase::RestoreMovementAfterLight()
{
	if (!IsAlive())
	{
		return;
	}

	bIsLightFrozen = false;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	UpdateState();
}
