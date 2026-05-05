#pragma once

#include "CoreMinimal.h"
#include "Crafting/ObstacleBase.h"
#include "CraftingSoundDecoy.generated.h"

UCLASS()
class OBLIVIO_API ACraftingSoundDecoy : public AObstacleBase
{
	GENERATED_BODY()
	
public:
	ACraftingSoundDecoy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnPlaced() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoy")
	float NoiseInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decoy")
	float NoiseRange;

	FTimerHandle NoiseTimerHandle;

	UFUNCTION()
	void EmitNoise();
};
