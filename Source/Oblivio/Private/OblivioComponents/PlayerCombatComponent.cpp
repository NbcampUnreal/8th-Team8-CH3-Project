#include "OblivioComponents/PlayerCombatComponent.h"
#include "OblivioComponents/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "OblivioCharacter.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	//이벤트 구독 추가
	if (AOblivioCharacter* OwnerPlayer = Cast<AOblivioCharacter>(GetOwner()))
	{
		OwnerPlayer->OnPlayerDamaged.AddDynamic(this, &UPlayerCombatComponent::HandleOwnerDamaged);
		//OwnerPlayer->OnEnemyAttackCommitted.AddDynamic(this, &UEnemyCombatComponent::HandleOwnerAttack);
	}
}

void UPlayerCombatComponent::HandleOwnerDamaged(float DamageAmount, float CurrentHealth, float MaxHealth)
{
	ICombatInterface* MyOwner = Cast<ICombatInterface>(GetOwner());
	//체력 감소 적용
	if (MyOwner)
	{ 
		MyOwner->ApplyHealth(DamageAmount);
		//CC 호출하고싶으면 추가
	}
}
