#pragma once

#include "CoreMinimal.h"
#include "Crafting/ObstacleBase.h"
#include "CraftingWall.generated.h"

UCLASS()
class OBLIVIO_API ACraftingWall : public AObstacleBase
{
	GENERATED_BODY()

public:
	ACraftingWall();
protected:
	virtual void BeginPlay() override;

public:
	/** 설치 확정 시 호출되는 함수 */
	virtual void OnPlaced() override;
};
