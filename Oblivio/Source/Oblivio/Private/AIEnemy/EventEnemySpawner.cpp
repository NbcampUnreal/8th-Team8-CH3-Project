#include "AIEnemy/EventEnemySpawner.h"
#include "NavigationSystem.h"
#include "Engine/World.h"

AEventEnemySpawner::AEventEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

AEnemyBase* AEventEnemySpawner::TrySpawnLootEnemy(AActor* TriggerActor)
{
	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastLootSpawnTime < LootSpawnCooldown)
	{
		return nullptr;
	}

	if (FMath::FRand() > LootSpawnChance)
	{
		return nullptr;
	}

	AEnemyBase* SpawnedEnemy = SpawnEventEnemyNearActor(TriggerActor, LootEnemyClass);
	if (SpawnedEnemy)
	{
		LastLootSpawnTime = CurrentTime;
	}

	return SpawnedEnemy;
}

AEnemyBase* AEventEnemySpawner::TrySpawnCampingEnemy(AActor* TargetActor)
{
	return SpawnEventEnemyNearActor(TargetActor, CampingEnemyClass);
}

AEnemyBase* AEventEnemySpawner::SpawnEventEnemyNearActor(AActor* TargetActor, TSubclassOf<AEnemyBase> EnemyClass)
{
	if (!IsValid(TargetActor) || !EnemyClass)
	{
		return nullptr;
	}

	FTransform SpawnTransform;
	if (!FindSpawnTransformNearActor(TargetActor, SpawnTransform))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s could not find event enemy spawn near %s"), *GetNameSafe(this), *GetNameSafe(TargetActor));
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	return GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnTransform, SpawnParameters);
}

bool AEventEnemySpawner::FindSpawnTransformNearActor(AActor* TargetActor, FTransform& OutTransform) const
{
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (!World || !NavSystem || !IsValid(TargetActor))
	{
		return false;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	for (int32 AttemptIndex = 0; AttemptIndex < SpawnPlacementAttempts; ++AttemptIndex)
	{
		const float SpawnDistance = FMath::RandRange(MinSpawnDistance, MaxSpawnDistance);
		const FVector Direction = FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f).Vector();
		const FVector CandidateLocation = TargetLocation + Direction * SpawnDistance;

		FNavLocation ProjectedLocation;
		if (NavSystem->ProjectPointToNavigation(CandidateLocation, ProjectedLocation, FVector(250.0f, 250.0f, 300.0f)))
		{
			const FVector LookDirection = TargetLocation - ProjectedLocation.Location;
			OutTransform = FTransform(LookDirection.Rotation(), ProjectedLocation.Location);
			return true;
		}
	}

	return false;
}
