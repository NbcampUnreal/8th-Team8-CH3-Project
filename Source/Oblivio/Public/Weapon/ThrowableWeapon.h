//ThrowableWeapon.h

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponBase.h"
#include "ThrowableWeapon.generated.h"

/**
 *
 */
UCLASS()
class OBLIVIO_API AThrowableWeapon : public AWeaponBase
{
	GENERATED_BODY()
public:
	void StartThrow(FVector Destination);
protected:
	AThrowableWeapon();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float HeightPerDistance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float SecondsPerDistance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ThrowOffset;

	bool bIsFlying;
	FVector StartLocation;
	FVector TargetLocation;
	float TimeElapsed;
	float ThrowDuration;
	float ThrowHeight;
};
