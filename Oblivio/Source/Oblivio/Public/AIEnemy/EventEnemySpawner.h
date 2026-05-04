#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIEnemy/EnemyBase.h"
#include "EventEnemySpawner.generated.h"

UCLASS(Blueprintable)
class OBLIVIO_API AEventEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEventEnemySpawner();

	UFUNCTION(BlueprintCallable, Category = "Enemy|Event Spawn")
	AEnemyBase* TrySpawnLootEnemy(AActor* TriggerActor);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Event Spawn")
	AEnemyBase* TrySpawnCampingEnemy(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Event Spawn")
	AEnemyBase* SpawnEventEnemyNearActor(AActor* TargetActor, TSubclassOf<AEnemyBase> EnemyClass);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Loot Spawn")
	TSubclassOf<AEnemyBase> LootEnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Loot Spawn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LootSpawnChance = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Loot Spawn", meta = (ClampMin = "0.0"))
	float LootSpawnCooldown = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Camping Spawn")
	TSubclassOf<AEnemyBase> CampingEnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Spawn Placement", meta = (ClampMin = "0.0"))
	float MinSpawnDistance = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Spawn Placement", meta = (ClampMin = "0.0"))
	float MaxSpawnDistance = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Spawn Placement", meta = (ClampMin = "1"))
	int32 SpawnPlacementAttempts = 12;

private:
	bool FindSpawnTransformNearActor(AActor* TargetActor, FTransform& OutTransform) const;

	float LastLootSpawnTime = -BIG_NUMBER;
};
