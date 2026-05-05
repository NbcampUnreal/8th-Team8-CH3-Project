#pragma once

#include "CoreMinimal.h"
#include "Crafting/ObstacleBase.h"
#include "CraftingReinforcedBarricade.generated.h"

UCLASS()
class OBLIVIO_API ACraftingReinforcedBarricade : public AObstacleBase
{
	GENERATED_BODY()
	
public:
	ACraftingReinforcedBarricade();

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnPlaced() override;

	/** 데미지 저항 로직을 위해 TakeDamage 오버라이드 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	/** 데미지 감소율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade")
	float DamageReductionRate;
};
