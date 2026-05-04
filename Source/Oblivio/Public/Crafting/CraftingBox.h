#pragma once

#include "CoreMinimal.h"
#include "Crafting/ObstacleBase.h"
#include "CraftingBox.generated.h"

UCLASS()
class OBLIVIO_API ACraftingBox : public AObstacleBase
{
	GENERATED_BODY()
	
public:
	ACraftingBox();

protected:
	virtual void BeginPlay() override;

	/** 상자 고유 속성 */
	UPROPERTY(EditDefaultsOnly, Category = "Box | Stats")
	float DecoyRange = 500.0f;

public:
	/** 설치 시 효과  */
	virtual void OnPlaced() override;
};
