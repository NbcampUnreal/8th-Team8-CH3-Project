// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "Flashbang.generated.h"

/**
 * 
 */
UCLASS()
class OBLIVIO_API AFlashbang : public AWeaponBase
{
	GENERATED_BODY()
public:
	virtual void UseWeapon() override;
protected:
	AFlashbang();
	virtual void StopWeapon() override;
	UPROPERTY(EditAnywher, BluepritnReadOnly, Category = Weapon)
	float BangDelay;
	UPROPERTY(EditAnywher, BluepritnReadOnly, Category = Weapon)
	float FlashDuration;
	FTimerHandle BangTimerHandle;
	FTimerHandle DestroyTimerHandle;
};
