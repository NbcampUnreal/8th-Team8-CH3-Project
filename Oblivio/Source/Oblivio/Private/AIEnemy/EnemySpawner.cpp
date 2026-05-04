#include "AIEnemy/EnemySpawner.h"
#include "Engine/World.h"
#include "TimerManager.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		StartWave(AutoStartWaveIndex);
	}
}

void AEnemySpawner::StartWave(int32 WaveIndex)
{
	if (!Waves.IsValidIndex(WaveIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot start invalid wave index %d"), *GetNameSafe(this), WaveIndex);
		return;
	}

	StopWave();

	CurrentWaveIndex = WaveIndex;
	bWaveRunning = true;

	for (const FEnemyWaveEntry& Entry : Waves[WaveIndex].Entries)
	{
		if (!Entry.EnemyClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s wave %d has an empty enemy class"), *GetNameSafe(this), WaveIndex);
			continue;
		}

		for (int32 Index = 0; Index < Entry.Count; ++Index)
		{
			FPendingEnemySpawn PendingSpawn;
			PendingSpawn.EnemyClass = Entry.EnemyClass;
			PendingSpawn.SpawnInterval = Entry.SpawnInterval;
			PendingSpawns.Add(PendingSpawn);
		}
	}

	OnWaveStarted.Broadcast(CurrentWaveIndex, PendingSpawns.Num());

	if (PendingSpawns.IsEmpty())
	{
		CheckWaveCompleted();
		return;
	}

	SpawnNextEnemy();
}

void AEnemySpawner::StartNextWave()
{
	const int32 NextWaveIndex = CurrentWaveIndex == INDEX_NONE ? 0 : CurrentWaveIndex + 1;
	if (Waves.IsValidIndex(NextWaveIndex))
	{
		StartWave(NextWaveIndex);
		return;
	}

	if (bLoopWaves && !Waves.IsEmpty())
	{
		StartWave(0);
	}
}

void AEnemySpawner::StopWave()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	PendingSpawns.Reset();

	for (TObjectPtr<AEnemyBase> Enemy : ActiveEnemies)
	{
		if (IsValid(Enemy))
		{
			Enemy->OnEnemyDied.RemoveDynamic(this, &AEnemySpawner::HandleEnemyDied);
		}
	}

	ActiveEnemies.Reset();
	bWaveRunning = false;
}

void AEnemySpawner::HandleEnemyDied(AEnemyBase* Enemy)
{
	if (Enemy)
	{
		Enemy->OnEnemyDied.RemoveDynamic(this, &AEnemySpawner::HandleEnemyDied);
		ActiveEnemies.Remove(Enemy);
	}

	CheckWaveCompleted();
}

void AEnemySpawner::SpawnNextEnemy()
{
	if (!bWaveRunning || PendingSpawns.IsEmpty())
	{
		CheckWaveCompleted();
		return;
	}

	FPendingEnemySpawn PendingSpawn = PendingSpawns[0];
	PendingSpawns.RemoveAt(0);

	UWorld* World = GetWorld();
	if (!World || !PendingSpawn.EnemyClass)
	{
		CheckWaveCompleted();
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyBase* SpawnedEnemy = World->SpawnActor<AEnemyBase>(PendingSpawn.EnemyClass, GetSpawnTransform(), SpawnParameters);
	if (SpawnedEnemy)
	{
		SpawnedEnemy->OnEnemyDied.AddDynamic(this, &AEnemySpawner::HandleEnemyDied);
		ActiveEnemies.Add(SpawnedEnemy);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to spawn enemy class %s"),
			*GetNameSafe(this), *GetNameSafe(PendingSpawn.EnemyClass.Get()));
	}

	if (!PendingSpawns.IsEmpty())
	{
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnNextEnemy, PendingSpawn.SpawnInterval, false);
	}
	else
	{
		CheckWaveCompleted();
	}
}

FTransform AEnemySpawner::GetSpawnTransform() const
{
	if (!SpawnPoints.IsEmpty())
	{
		const int32 SpawnPointIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
		if (IsValid(SpawnPoints[SpawnPointIndex]))
		{
			return SpawnPoints[SpawnPointIndex]->GetActorTransform();
		}
	}

	return GetActorTransform();
}

void AEnemySpawner::CheckWaveCompleted()
{
	if (!bWaveRunning || !PendingSpawns.IsEmpty() || !ActiveEnemies.IsEmpty())
	{
		return;
	}

	const int32 CompletedWaveIndex = CurrentWaveIndex;
	bWaveRunning = false;
	OnWaveCompleted.Broadcast(CompletedWaveIndex);
}
