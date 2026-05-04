#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class AEnemyBase;

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Chase UMETA(DisplayName = "Chase"),
	Attack UMETA(DisplayName = "Attack"),
	Dead UMETA(DisplayName = "Dead")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyDiedSignature, AEnemyBase*, Enemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEnemyDamagedSignature, float, DamageAmount, float, CurrentHealth, float, MaxHealth);

UCLASS(Blueprintable)
class OBLIVIO_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Enemy|State")
	EEnemyAIState GetEnemyState() const { return EnemyState; }

	UFUNCTION(BlueprintCallable, Category = "Enemy|State")
	bool IsAlive() const { return EnemyState != EEnemyAIState::Dead && CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Enemy|Light")
	virtual void OnLightHit(float Intensity, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Light")
	virtual void OnFlashbangHit(float DamageAmount = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Target")
	void SetTargetActor(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "Enemy|Stats")
	float GetCurrentHealthForUI() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Stats")
	float GetMaxHealthForUI() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Stats")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FEnemyDiedSignature OnEnemyDied;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FEnemyDamagedSignature OnEnemyDamaged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Stats")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Stats", meta = (ClampMin = "0.0"))
	float MoveSpeed = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
	float AttackDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "1.0"))
	float AttackRange = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.1"))
	float AttackCooldown = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Navigation", meta = (ClampMin = "1.0"))
	float ChaseAcceptanceRadius = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LightSlowSpeedMultiplier = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LightFreezeSpeedMultiplier = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float DefaultFlashbangDamage = 9999.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float LightSlowExposureThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float LightFreezeExposureThreshold = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float LightDeathExposureThreshold = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Light", meta = (ClampMin = "0.0"))
	float LightExposureDecayPerSecond = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Death", meta = (ClampMin = "0.0"))
	float CorpseLifeSpan = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	EEnemyAIState EnemyState = EEnemyAIState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Target")
	TObjectPtr<AActor> TargetActor;

	UFUNCTION(BlueprintNativeEvent, Category = "Enemy|Combat")
	void PerformAttack(AActor* Target);
	virtual void PerformAttack_Implementation(AActor* Target);

	virtual void FindDefaultTarget();
	virtual void UpdateState();
	virtual void UpdateChase();
	virtual void UpdateAttack();
	virtual void Die();

	void SetEnemyState(EEnemyAIState NewState);
	bool IsTargetInAttackRange() const;
	void StopEnemyMovement();
	void UpdateLightExposure(float DeltaSeconds);
	void RestoreMovementAfterLight();

private:
	float LastAttackTime = -BIG_NUMBER;
	float LightExposure = 0.0f;
	bool bIsLightFrozen = false;
	FTimerHandle LightFreezeTimerHandle;
};
