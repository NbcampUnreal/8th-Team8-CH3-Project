#include "Crafting/CraftingReinforcedBarricade.h"
#include "Components/StaticMeshComponent.h"

ACraftingReinforcedBarricade::ACraftingReinforcedBarricade()
{
	MaxHealth = 500.0f;
	CurrentHealth = MaxHealth;

	WoodCost = 4;
	IronCost = 2;

	// 들어오는 데미지의 20%를 상시 감소
	DamageReductionRate = 0.2f;

	// 내비게이션 설정
	if (MeshComponent)
	{
		MeshComponent->SetCanEverAffectNavigation(true);
	}
}

void ACraftingReinforcedBarricade::BeginPlay()
{
	Super::BeginPlay();
}

void ACraftingReinforcedBarricade::OnPlaced()
{
	Super::OnPlaced();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Reinforced Barricade Deployed: Heavy Defense Active!"));
	}
}

float ACraftingReinforcedBarricade::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// 실제 들어오는 데미지에서 감소율만큼 차감
	float FinalDamage = DamageAmount * (1.0f - DamageReductionRate);

	return Super::TakeDamage(FinalDamage, DamageEvent, EventInstigator, DamageCauser);
}
