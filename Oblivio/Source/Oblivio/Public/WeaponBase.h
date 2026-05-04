// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LightAttackComponent.h"

#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class OBLIVIO_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	TObjectPtr<USceneComponent> SceneComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	TObjectPtr<UStaticMeshComponent> MeshComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	TObjectPtr<ULightAttackComponent> LightAttackComp;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void UseWeapon();

};
