#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIEnemy/EnemyBase.h"
#include "EnemySpawner.generated.h"

USTRUCT(BlueprintType)
struct FEnemyWaveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (AllowAbstract = "false"))
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "1"))
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0.0"))
	float SpawnInterval = 1.0f;
};

USTRUCT(BlueprintType)
struct FEnemyWaveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<FEnemyWaveEntry> Entries;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEnemyWaveStartedSignature, int32, WaveIndex, int32, TotalEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyWaveCompletedSignature, int32, WaveIndex);

UCLASS(Blueprintable)
class OBLIVIO_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWave(int32 WaveIndex);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartNextWave();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StopWave();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	bool IsWaveRunning() const { return bWaveRunning; }

	UPROPERTY(BlueprintAssignable, Category = "Wave|Events")
	FEnemyWaveStartedSignature OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Wave|Events")
	FEnemyWaveCompletedSignature OnWaveCompleted;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<FEnemyWaveDefinition> Waves;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<TObjectPtr<AActor>> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	bool bAutoStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0"))
	int32 AutoStartWaveIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	bool bLoopWaves = false;

private:
	struct FPendingEnemySpawn
	{
		TSubclassOf<AEnemyBase> EnemyClass;
		float SpawnInterval = 1.0f;
	};

	UFUNCTION()
	void HandleEnemyDied(AEnemyBase* Enemy);

	void SpawnNextEnemy();
	FTransform GetSpawnTransform() const;
	void CheckWaveCompleted();

	UPROPERTY()
	TArray<TObjectPtr<AEnemyBase>> ActiveEnemies;

	TArray<FPendingEnemySpawn> PendingSpawns;
	FTimerHandle SpawnTimerHandle;
	int32 CurrentWaveIndex = INDEX_NONE;
	bool bWaveRunning = false;
};
