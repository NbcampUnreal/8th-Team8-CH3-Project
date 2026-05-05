#pragma once

#include "CoreMinimal.h"
#include "Crafting/ObstacleBase.h"
#include "CraftingFloorSpike.generated.h"

UCLASS()
class OBLIVIO_API ACraftingFloorSpike : public AObstacleBase
{
	GENERATED_BODY()
	
public:
	ACraftingFloorSpike();

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnPlaced() override;

protected:
	/** 적이 밟았는지 감지하는 콜리전 영역 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* DamageArea;

	/** 가시가 입히는 대미지 양 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spike")
	float DamageAmount;

	/** 대미지를 입히는 주기 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spike")
	float DamageInterval;

	/** 주기적인 대미지 처리를 위한 타이머 핸들 */
	FTimerHandle DamageTimerHandle;

	/** 영역에 들어온 적들을 추적하기 위한 배열 */
	UPROPERTY()
	TArray<AActor*> OverlappingEnemies;

	/** 겹침 시작 이벤트 */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 겹침 종료 이벤트 */
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** 실제로 대미지를 적용하는 함수 */
	void ApplySpikeDamage();
};
