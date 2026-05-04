#include "AIEnemy/StalkerEnemy.h"
#include "OblivioCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/SpotLightComponent.h"

AStalkerEnemy::AStalkerEnemy()
{
	StalkerAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("StalkerStealthAudio"));
	StalkerAudioComponent->SetupAttachment(RootComponent);
	StalkerAudioComponent->bAutoActivate = false;
	StalkerAudioComponent->bStopWhenOwnerDestroyed = true;

	// 절름발이 연출: 비추격 느림 / 추격·기습은 빠름 (기획서 스토커 프로필)
	MaxHealth = 95.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 175.0f;
	ChaseMoveSpeed = 480.0f;
	AttackDamage = 16.0f;
	AttackRange = 150.0f;
	AttackCooldown = 1.55f;
	ChaseAcceptanceRadius = 55.0f;
	ChaseProximityBuffer = 48.0f;
	AggroRadius = 1100.0f;
	CorpseLifeSpan = 3.0f;
	InvestigateStimulusTimeout = 18.0f;
}

void AStalkerEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (StalkerAudioComponent && StalkerRevealLoopSound)
	{
		StalkerAudioComponent->SetSound(StalkerRevealLoopSound);
	}

	UpdateFlashlightReveal();
}

void AStalkerEnemy::Tick(float DeltaSeconds)
{
	if (ChaseBurstTimeRemaining > 0.0f)
	{
		ChaseBurstTimeRemaining = FMath::Max(0.0f, ChaseBurstTimeRemaining - DeltaSeconds);
	}
	Super::Tick(DeltaSeconds);
	UpdateFlashlightReveal();
}

void AStalkerEnemy::Die()
{
	ApplyFlashlightReveal(true);
	Super::Die();
}

void AStalkerEnemy::NotifyEnemyStateChanged(EEnemyAIState OldState, EEnemyAIState NewState)
{
	if (NewState != EEnemyAIState::Chase && NewState != EEnemyAIState::Attack)
	{
		ChaseBurstTimeRemaining = 0.0f;
	}

	if (NewState == EEnemyAIState::Chase)
	{
		const bool bFromCombatLocomotion = (OldState == EEnemyAIState::Chase || OldState == EEnemyAIState::Attack);
		if (!bFromCombatLocomotion && ChaseBurstDuration > KINDA_SMALL_NUMBER)
		{
			if (!bChaseBurstOnlyFromBehind || ShouldTriggerChaseBurstFromBehind())
			{
				ChaseBurstTimeRemaining = ChaseBurstDuration;
			}
		}
	}
}

float AStalkerEnemy::GetLocomotionBaseSpeed() const
{
	const float Base = Super::GetLocomotionBaseSpeed();
	if (ChaseBurstTimeRemaining > KINDA_SMALL_NUMBER &&
		(EnemyState == EEnemyAIState::Chase || EnemyState == EEnemyAIState::Attack))
	{
		return Base * ChaseBurstSpeedMultiplier;
	}
	return Base;
}

bool AStalkerEnemy::ShouldTriggerChaseBurstFromBehind() const
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	const FVector PlayerLoc = TargetActor->GetActorLocation();
	const FVector ToEnemy = (GetActorLocation() - PlayerLoc).GetSafeNormal();
	if (ToEnemy.IsNearlyZero())
	{
		return false;
	}

	const FVector PlayerForward = TargetActor->GetActorForwardVector().GetSafeNormal();
	const float Dot = FVector::DotProduct(PlayerForward, ToEnemy);
	return Dot < BehindPlayerDotThreshold;
}

void AStalkerEnemy::UpdateFlashlightReveal()
{
	if (!bFlashlightStealth)
	{
		ApplyFlashlightReveal(true);
		return;
	}

	if (!IsAlive())
	{
		ApplyFlashlightReveal(true);
		return;
	}

	const bool bInCone = ComputeInPlayerFlashlightCone();
	ApplyFlashlightReveal(bInCone);
}

bool AStalkerEnemy::ComputeInPlayerFlashlightCone() const
{
	if (!IsValid(TargetActor))
	{
		return true;
	}

	const AOblivioCharacter* Player = Cast<AOblivioCharacter>(TargetActor);
	if (!Player)
	{
		return true;
	}

	const USpotLightComponent* Spot = Player->FlashlightComponent;
	if (!Spot || !Spot->IsVisible())
	{
		return false;
	}

	if (!Player->bIsFlashlightOn || Player->Battery <= 0.0f)
	{
		return false;
	}

	const FVector TestPoint = GetActorLocation() + FlashlightTestPointOffset;
	const FVector Origin = Spot->GetComponentLocation();
	const FVector Dir = Spot->GetForwardVector().GetSafeNormal();
	const FVector ToPoint = TestPoint - Origin;
	const float Dist = ToPoint.Size();
	if (Dist < KINDA_SMALL_NUMBER)
	{
		return true;
	}

	const float MaxDist = Spot->AttenuationRadius + FlashlightConeRadiusSlack;
	if (Dist > MaxDist)
	{
		return false;
	}

	const FVector ToNorm = ToPoint / Dist;
	const float HalfOuterDeg = Spot->OuterConeAngle * 0.5f;
	const float CosCone = FMath::Cos(FMath::DegreesToRadians(HalfOuterDeg));
	return FVector::DotProduct(Dir, ToNorm) >= CosCone - KINDA_SMALL_NUMBER;
}

void AStalkerEnemy::ApplyFlashlightReveal(bool bRevealed)
{
	const bool bChanged = (bRevealedByFlashlight != bRevealed);
	bRevealedByFlashlight = bRevealed;

	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->SetVisibility(bRevealed, true);
		SkelMesh->SetCastShadow(bRevealed);
	}

	if (StalkerAudioComponent)
	{
		if (bRevealed)
		{
			StalkerAudioComponent->SetVolumeMultiplier(1.0f);
			if (StalkerRevealLoopSound && !StalkerAudioComponent->IsPlaying())
			{
				StalkerAudioComponent->Play();
			}
		}
		else
		{
			StalkerAudioComponent->SetVolumeMultiplier(0.0f);
			if (bChanged && StalkerAudioComponent->IsPlaying())
			{
				StalkerAudioComponent->Stop();
			}
		}
	}

	if (bChanged)
	{
		OnFlashlightRevealChanged(bRevealed);
	}
}

void AStalkerEnemy::OnFlashlightRevealChanged_Implementation(bool bRevealed)
{
	(void)bRevealed;
}
