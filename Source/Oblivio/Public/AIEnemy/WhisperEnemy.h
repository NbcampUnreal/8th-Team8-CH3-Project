#pragma once

#include "CoreMinimal.h"
#include "AIEnemy/EnemyBase.h"
#include "WhisperEnemy.generated.h"

class AAIController;

/**
 * AWhisperEnemy - "속삭이는 자"
 * - 배회/도주 없이 플레이어에게 계속 접근
 * - 근접하면 손전등을 강제로 끄고 공격 실행은 PerformAttack으로 위임
 * - 빛 노출에 의한 데미지/둔화/정지는 AEnemyBase 처리 그대로 적용
 */
UCLASS(Blueprintable)
class OBLIVIO_API AWhisperEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	AWhisperEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void UpdateChase() override;
	virtual void UpdateAttack() override;

	/** 손전등 OFF 및 공격 판단을 수행하는 수평 거리(cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Whisper", meta = (ClampMin = "50.0"))
	float WhisperRange = 150.0f;

	/** 손전등 콘 외각 반경에 더하는 안전 여유(cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Whisper|Avoid", meta = (ClampMin = "0.0"))
	float DangerConeRadiusSlack = 96.0f;

	/** 손전등 콘 외각 각도에 더하는 안전 마진(deg). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Whisper|Avoid", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float DangerConeAngleMarginDeg = 22.0f;

private:
	float NextAttackDecisionTime = 0.0f;

	bool IsWithinWhisperRange() const;
	bool IsPointInsideFlashlightDanger(const FVector& Point) const;
	bool IsSelfInsideFlashlightDanger() const;
	void ApproachTarget(AAIController* AI);
	void AvoidFlashlightCone(AAIController* AI);
	void TryCommitWhisperAttack();
};
