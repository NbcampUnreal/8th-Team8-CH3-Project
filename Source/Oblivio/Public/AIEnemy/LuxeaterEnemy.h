#pragma once

#include "CoreMinimal.h"
#include "AIEnemy/EnemyBase.h"
#include "LuxeaterEnemy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLuxeaterPhaseChangedSignature, class ALuxeaterEnemy*, Enemy, int32, OldPhase, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLuxeaterLightAbsorbedSignature, class ALuxeaterEnemy*, Enemy, float, AbsorbedAmount, float, TotalAbsorbed);

/**
 * ALuxeaterEnemy - 6F boss, "빛을 먹는 자"
 * - 빛 피격 판단을 받으면 빛을 흡수해 이동속도·스케일만 증가 (체력 회복/상한 증가 없음)
 * - 페이즈는 체력 기준: 1페이즈 시작, 체력 50% 이하에서 2페이즈
 * - 실제 피해/CC/공격 결과는 전투 시스템에 위임하고, 보스는 강화 상태만 관리
 */
UCLASS(Blueprintable)
class OBLIVIO_API ALuxeaterEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	ALuxeaterEnemy();

	virtual void OnLightHit(float Intensity, float Duration) override;

	UFUNCTION(BlueprintPure, Category = "Enemy|Luxeater")
	int32 GetBossPhase() const { return BossPhase; }

	UFUNCTION(BlueprintPure, Category = "Enemy|Luxeater")
	float GetAbsorbedLight() const { return AbsorbedLight; }

	/**
	 * 전투 시스템이 보스 체력값을 갱신했을 때 호출.
	 * EnemyBase의 CurrentHealth/MaxHealth를 동기화하고 페이즈만 판단한다.
	 * (빛 흡수량과 무관, 빛 흡수로 체력은 오르지 않음)
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Luxeater")
	void NotifyBossHealthChanged(float NewCurrentHealth, float NewMaxHealth);

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Luxeater|Events")
	FLuxeaterPhaseChangedSignature OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Luxeater|Events")
	FLuxeaterLightAbsorbedSignature OnLightAbsorbed;

protected:
	virtual void BeginPlay() override;
	virtual void UpdateChase() override;
	virtual void UpdateAttack() override;

	/** 빛 흡수 누적량 1당 증가하는 이동속도(cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater|Light", meta = (ClampMin = "0.0"))
	float SpeedGainPerLight = 35.0f;

	/** 빛 흡수 누적량 1당 증가하는 스케일. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater|Light", meta = (ClampMin = "0.0"))
	float ScaleGainPerLight = 0.04f;

	/** 빛 흡수량으로 오를 수 있는 최대 추가 이동속도. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater|Light", meta = (ClampMin = "0.0"))
	float MaxLightSpeedBonus = 420.0f;

	/** 빛 흡수로 도달할 수 있는 최대 스케일 배율. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater|Light", meta = (ClampMin = "1.0"))
	float MaxLightScaleMultiplier = 1.6f;

	/** 2페이즈 진입 체력 비율. 기본 0.5 = 체력 절반 이하. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater|Phase", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float PhaseTwoHealthPercentThreshold = 0.5f;

private:
	float BaseMoveSpeed = 0.0f;
	float BaseChaseMoveSpeed = 0.0f;
	FVector InitialScale = FVector::OneVector;
	float AbsorbedLight = 0.0f;
	int32 BossPhase = 1;

	void ApplyLightEmpowerment();
	void UpdateHealthPhase();
};
