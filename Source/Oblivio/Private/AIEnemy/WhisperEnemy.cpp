#include "AIEnemy/WhisperEnemy.h"

#include "AIController.h"
#include "Components/SpotLightComponent.h"
#include "OblivioCharacter.h"

AWhisperEnemy::AWhisperEnemy()
{
	MaxHealth = 90.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 200.0f;
	ChaseMoveSpeed = 200.0f;
	AttackDamage = 0.0f;
	AttackRange = 150.0f;
	AttackCooldown = 0.2f;
	ChaseAcceptanceRadius = 10.0f;
	ChaseProximityBuffer = 40.0f;

	// Whisper는 거리/배회에 묶이지 않고 계속 접근한다.
	AggroRadius = 0.0f;
	bAggroUseHorizontalDistance = true;
	bEnableIdleWander = false;
	bEnableLightTracking = false;
}

void AWhisperEnemy::BeginPlay()
{
	// BP에 예전 Luxeater 값이 저장돼 있어도 Whisper 컨셉을 강제한다.
	AggroRadius = 0.0f;
	bEnableIdleWander = false;
	bEnableLightTracking = false;
	WhisperRange = FMath::Max(WhisperRange, 150.0f);
	AttackRange = FMath::Max(AttackRange, WhisperRange);

	Super::BeginPlay();
}

void AWhisperEnemy::UpdateChase()
{
	AAIController* const AI = Cast<AAIController>(GetController());
	if (!AI || !IsValid(TargetActor))
	{
		return;
	}

	if (IsSelfInsideFlashlightDanger())
	{
		AvoidFlashlightCone(AI);
		return;
	}

	if (IsWithinWhisperRange())
	{
		TryCommitWhisperAttack();
	}

	ApproachTarget(AI);
}

void AWhisperEnemy::UpdateAttack()
{
	AAIController* const AI = Cast<AAIController>(GetController());
	if (!AI || !IsValid(TargetActor))
	{
		return;
	}

	if (IsSelfInsideFlashlightDanger())
	{
		AvoidFlashlightCone(AI);
		return;
	}

	if (IsWithinWhisperRange())
	{
		TryCommitWhisperAttack();
	}

	// 베이스 Attack은 StopEnemyMovement를 호출하므로 쓰지 않는다.
	// Whisper는 붙은 뒤에도 계속 압박해야 한다.
	ApproachTarget(AI);
}

bool AWhisperEnemy::IsWithinWhisperRange() const
{
	return IsValid(TargetActor) &&
		FVector::DistSquared2D(GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(WhisperRange);
}

bool AWhisperEnemy::IsPointInsideFlashlightDanger(const FVector& Point) const
{
	const AOblivioCharacter* const Player = Cast<AOblivioCharacter>(TargetActor);
	if (!Player || !Player->FlashlightComponent || !Player->bIsFlashlightOn || Player->Battery <= 0.0f)
	{
		return false;
	}

	const USpotLightComponent* const Spot = Player->FlashlightComponent;
	const FVector Origin = Spot->GetComponentLocation();
	const FVector Dir = Spot->GetForwardVector().GetSafeNormal();
	const FVector ToPoint = Point - Origin;
	const float Dist = ToPoint.Size();
	if (Dist <= KINDA_SMALL_NUMBER)
	{
		return true;
	}

	if (Dist > Spot->AttenuationRadius + DangerConeRadiusSlack)
	{
		return false;
	}

	const float HalfOuterDeg = FMath::Min(85.0f, Spot->OuterConeAngle + DangerConeAngleMarginDeg);
	const float CosCone = FMath::Cos(FMath::DegreesToRadians(HalfOuterDeg));
	return FVector::DotProduct(Dir, ToPoint / Dist) >= CosCone - KINDA_SMALL_NUMBER;
}

bool AWhisperEnemy::IsSelfInsideFlashlightDanger() const
{
	return IsPointInsideFlashlightDanger(GetActorLocation());
}

void AWhisperEnemy::ApproachTarget(AAIController* AI)
{
	if (!AI || !IsValid(TargetActor))
	{
		return;
	}

	// 캡슐 겹침으로 일찍 멈추지 않게 StopOnOverlap을 끈다.
	AI->MoveToActor(TargetActor, 5.0f, false);
}

void AWhisperEnemy::AvoidFlashlightCone(AAIController* AI)
{
	const AOblivioCharacter* const Player = Cast<AOblivioCharacter>(TargetActor);
	if (!AI || !Player || !Player->FlashlightComponent)
	{
		return;
	}

	FVector Forward = Player->FlashlightComponent->GetForwardVector().GetSafeNormal2D();
	if (Forward.IsNearlyZero())
	{
		Forward = Player->GetActorForwardVector().GetSafeNormal2D();
	}

	const FVector Origin = Player->FlashlightComponent->GetComponentLocation();
	FVector ToEnemy = GetActorLocation() - Origin;
	ToEnemy.Z = 0.0f;

	const FVector Right(-Forward.Y, Forward.X, 0.0f);
	const float Side = FVector::DotProduct(ToEnemy, Right);
	const float Sign = (Side >= 0.0f) ? 1.0f : -1.0f;

	const FVector EscapeGoal = GetActorLocation() + Right * (Sign * 700.0f) - Forward * 150.0f;
	AI->MoveToLocation(EscapeGoal, 10.0f);
}

void AWhisperEnemy::TryCommitWhisperAttack()
{
	AOblivioCharacter* const Player = Cast<AOblivioCharacter>(TargetActor);
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("Whisper: TargetActor is not AOblivioCharacter"));
		return;
	}

	if (Player->bIsFlashlightOn)
	{
		Player->bIsFlashlightOn = false;
		Player->UpdateFlashlightVisuals();
	}

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (CurrentTime < NextAttackDecisionTime)
	{
		return;
	}

	NextAttackDecisionTime = CurrentTime + AttackCooldown;

	// 공격 방식(피해, 상태이상, 연출)은 전투 담당 쪽에서 BP/C++로 구현.
	// Whisper는 공격 타이밍 판단과 손전등 OFF까지만 책임진다.
	PerformAttack(TargetActor);
	UE_LOG(LogTemp, Verbose, TEXT("Whisper %s committed attack decision"), *GetNameSafe(this));
}
