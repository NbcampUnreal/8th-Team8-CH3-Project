// Fill out your copyright notice in the Description page of Project Settings.


#include "Flashbang.h"

AFlashbang::AFlashbang()
{
	LightAttackComp->Damage = 10000;
	BangDelay = 2.0f;
	FlashDuration = 0.5;
}
void AFlashbang::UseWeapon()
{
	if (!IsValid(LightAttackComp)) return;
	if (!GetWorld()->GetTimerManager().IsTimerActive(BangTimerHandle)) {
		UE_LOG(LogTemp, Warning, TEXT("Setting Timer"));
		GetWorld()->GetTimerManager().SetTimer(
			BangTimerHandle,
			[this]() {
				FVector SourceLocation = LightAttackComp->GetComponentLocation();
				FVector LightDirection = LightAttackComp->GetForwardVector();
				LightAttackComp->CreateLightAttack(SourceLocation, LightDirection); },
			BangDelay,
			false);
		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AFlashbang::StopWeapon, BangDelay + FlashDuration, false);
	}
}

void AFlashbang::StopWeapon()
{
	LightAttackComp->TurnOffLight();
	Destroy();
}
