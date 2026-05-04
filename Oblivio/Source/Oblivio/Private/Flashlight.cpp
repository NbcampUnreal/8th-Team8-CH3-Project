// Fill out your copyright notice in the Description page of Project Settings.


#include "Flashlight.h"

AFlashlight::AFlashlight()
{
	AttackInterval = 0.2f;
	LightAttackComp->bIsConcentrated = true;
}

void AFlashlight::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
}

void AFlashlight::UseWeapon()
{
	if (!IsValid(LightAttackComp)) return;
	if (!GetWorld()->GetTimerManager().IsTimerActive(AttackTimerHandle)) {
		UE_LOG(LogTemp, Warning, TEXT("Setting Timer"));

		GetWorld()->GetTimerManager().SetTimer(
			AttackTimerHandle,
			[this](){
				FVector SourceLocation = LightAttackComp->GetComponentLocation();
				FVector LightDirection = LightAttackComp->GetForwardVector();
				LightAttackComp->CreateLightAttack(SourceLocation, LightDirection);}, 
			AttackInterval, 
			true);
		LightAttackComp->CreateLightAttack(GetActorLocation(), GetActorForwardVector());
	}
}

void AFlashlight::StopWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("Clearing Timer"));
	GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
	LightAttackComp->TurnOffLight();
}
void AFlashlight::ChangeWeaponAngle(float DeltaAngle)
{
	UE_LOG(LogTemp, Warning, TEXT("ChangeWeaponAngle Called"));
	LightAttackComp->ChangeLightAngle(DeltaAngle);
}
