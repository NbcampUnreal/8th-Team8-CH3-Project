#pragma once

#include "CoreMinimal.h"
#include "AIEnemy/EnemyBase.h"
#include "LuxeaterEnemy.generated.h"

/**
 * ALuxeaterEnemy
 * - 플레이어 후방으로 접근
 * - 손전등 콘을 피해서 우회
 * - 근접 시 손전등을 꺼뜨리고 일정 시간 도주
 */
UCLASS(Blueprintable)
class OBLIVIO_API ALuxeaterEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	ALuxeaterEnemy();

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void UpdateChase() override;
	virtual void UpdateAttack() override;
	virtual void PerformAttack_Implementation(AActor* Target) override;

	/** 플레이어 뒤로 붙을 때 목표 거리(cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater", meta = (ClampMin = "50.0"))
	float RearApproachDistance = 180.0f;

	/** 플레이어로부터 도주할 거리(cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater", meta = (ClampMin = "200.0"))
	float RetreatDistance = 700.0f;

	/** 손전등을 끄고 도주하는 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater", meta = (ClampMin = "0.1"))
	float RetreatDuration = 2.0f;

	/** 손전등 콘 안으로 판정할 때 여유 반경(cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater", meta = (ClampMin = "0.0"))
	float FlashlightConeRadiusSlack = 24.0f;

	/** 플레이어 정면 내적이 이 값보다 작으면 '후방' */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Luxeater", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float BehindDotThreshold = -0.15f;

private:
	bool bRetreating = false;
	float RetreatRemaining = 0.0f;

	bool IsInsidePlayerFlashlightCone() const;
	bool IsBehindPlayer() const;
	FVector ComputeRearApproachPoint() const;
	FVector ComputeRetreatPoint() const;
	FVector ComputeSideStepPoint() const;
	FVector ProjectToNav(const FVector& Desired) const;
};

