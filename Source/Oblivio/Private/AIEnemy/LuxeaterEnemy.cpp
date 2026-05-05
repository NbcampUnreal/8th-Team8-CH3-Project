#include "AIEnemy/LuxeaterEnemy.h"

#include "AIController.h"
#include "Components/SpotLightComponent.h"
#include "NavigationSystem.h"
#include "OblivioCharacter.h"

ALuxeaterEnemy::ALuxeaterEnemy()
{
	// 기본 기획값: 느린 이동, 후방 침투 후 손전등 차단
	MaxHealth = 85.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 200.0f;
	ChaseMoveSpeed = 200.0f;
	AttackDamage = 0.0f;          // 이 적의 핵심은 데미지보다 손전등 차단
	AttackRange = 145.0f;
	AttackCooldown = 2.0f;
	ChaseAcceptanceRadius = 55.0f;
	ChaseProximityBuffer = 40.0f;
	AggroRadius = 1100.0f;
}

void ALuxeaterEnemy::Tick(float DeltaSeconds)
{
	if (bRetreating)
	{
		RetreatRemaining = FMath::Max(0.0f, RetreatRemaining - DeltaSeconds);
		if (RetreatRemaining <= KINDA_SMALL_NUMBER)
		{
			bRetreating = false;
		}
	}

	Super::Tick(DeltaSeconds);
}

void ALuxeaterEnemy::UpdateChase()
{
	AAIController* const AI = Cast<AAIController>(GetController());
	if (!AI || !IsValid(TargetActor))
	{
		return;
	}

	const FVector DesiredDestination =
		bRetreating ? ComputeRetreatPoint()
		: (IsInsidePlayerFlashlightCone() ? ComputeSideStepPoint() : ComputeRearApproachPoint());

	const FVector NavDestination = ProjectToNav(DesiredDestination);
	AI->MoveToLocation(NavDestination, ChaseAcceptanceRadius);
}

void ALuxeaterEnemy::UpdateAttack()
{
	if (!bRetreating && !IsBehindPlayer())
	{
		// 정면 접촉 대신 후방으로 재포지셔닝
		SetEnemyState(EEnemyAIState::Chase);
		return;
	}

	Super::UpdateAttack();
}

void ALuxeaterEnemy::PerformAttack_Implementation(AActor* Target)
{
	AOblivioCharacter* const Player = Cast<AOblivioCharacter>(Target);
	if (!Player)
	{
		return;
	}

	if (Player->bIsFlashlightOn)
	{
		Player->bIsFlashlightOn = false;
		Player->UpdateFlashlightVisuals();
	}

	bRetreating = true;
	RetreatRemaining = RetreatDuration;
	SetEnemyState(EEnemyAIState::Chase);
}

bool ALuxeaterEnemy::IsInsidePlayerFlashlightCone() const
{
	const AOblivioCharacter* const Player = Cast<AOblivioCharacter>(TargetActor);
	if (!Player || !Player->FlashlightComponent || !Player->bIsFlashlightOn || Player->Battery <= 0.0f)
	{
		return false;
	}

	const USpotLightComponent* const Spot = Player->FlashlightComponent;
	const FVector Origin = Spot->GetComponentLocation();
	const FVector Dir = Spot->GetForwardVector().GetSafeNormal();
	const FVector ToEnemy = GetActorLocation() - Origin;
	const float Dist = ToEnemy.Size();
	if (Dist <= KINDA_SMALL_NUMBER)
	{
		return true;
	}

	const float MaxDist = Spot->AttenuationRadius + FlashlightConeRadiusSlack;
	if (Dist > MaxDist)
	{
		return false;
	}

	const float CosHalf = FMath::Cos(FMath::DegreesToRadians(Spot->OuterConeAngle * 0.5f));
	return FVector::DotProduct(Dir, ToEnemy / Dist) >= CosHalf - KINDA_SMALL_NUMBER;
}

bool ALuxeaterEnemy::IsBehindPlayer() const
{
	if (!IsValid(TargetActor))
	{
		return false;
	}
	const FVector PlayerForward = TargetActor->GetActorForwardVector().GetSafeNormal();
	const FVector ToEnemy = (GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();
	if (PlayerForward.IsNearlyZero() || ToEnemy.IsNearlyZero())
	{
		return false;
	}
	return FVector::DotProduct(PlayerForward, ToEnemy) < BehindDotThreshold;
}

FVector ALuxeaterEnemy::ComputeRearApproachPoint() const
{
	if (!IsValid(TargetActor))
	{
		return GetActorLocation();
	}
	return TargetActor->GetActorLocation() - TargetActor->GetActorForwardVector().GetSafeNormal() * RearApproachDistance;
}

FVector ALuxeaterEnemy::ComputeRetreatPoint() const
{
	if (!IsValid(TargetActor))
	{
		return GetActorLocation();
	}

	FVector Away = GetActorLocation() - TargetActor->GetActorLocation();
	if (Away.IsNearlyZero())
	{
		Away = -TargetActor->GetActorForwardVector();
	}
	return GetActorLocation() + Away.GetSafeNormal() * RetreatDistance;
}

FVector ALuxeaterEnemy::ComputeSideStepPoint() const
{
	if (!IsValid(TargetActor))
	{
		return GetActorLocation();
	}

	const FVector ToPlayer = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	FVector Side = FVector::CrossProduct(FVector::UpVector, ToPlayer).GetSafeNormal();
	if (Side.IsNearlyZero())
	{
		Side = GetActorRightVector().GetSafeNormal2D();
	}
	const float Sign = FMath::RandBool() ? 1.0f : -1.0f;
	return GetActorLocation() + Side * Sign * 280.0f;
}

FVector ALuxeaterEnemy::ProjectToNav(const FVector& Desired) const
{
	UWorld* const World = GetWorld();
	UNavigationSystemV1* const NavSys = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (!NavSys)
	{
		return Desired;
	}

	FNavLocation Projected;
	if (NavSys->ProjectPointToNavigation(Desired, Projected, FVector(350.0f, 350.0f, 350.0f)))
	{
		return Projected.Location;
	}
	return Desired;
}

