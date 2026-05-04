// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "LightAttackComponent.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	RootComponent = SceneComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	LightAttackComp = CreateDefaultSubobject<ULightAttackComponent>(TEXT("LightAttackComp"));
	LightAttackComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBase::UseWeapon()
{
	//LightAttackComp->CreateLightAttack(GetActorLocation(), GetActorForwardVector());
}

void AWeaponBase::StopWeapon()
{
	//LightAttackComp->TurnOffLight();
}

void AWeaponBase::ChangeWeaponAngle(float DeltaAngle)
{
	//UE_LOG(LogTemp, Warning, TEXT("ChangeWeaponAngle Called"));
	//LightAttackComp->ChangeLightAngle(DeltaAngle);
}

