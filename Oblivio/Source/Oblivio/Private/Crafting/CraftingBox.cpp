#include "Crafting/CraftingBox.h"
#include "Components/StaticMeshComponent.h"

ACraftingBox::ACraftingBox()
{
	// 기본 스탯 설정
	MaxHealth = 50.0f;
	WoodCost = 2;
	IronCost = 0;

	// 내비게이션 영향을 주어 몬스터가 길을 돌아가게 만듦
	MeshComponent->SetCanEverAffectNavigation(true);
}

void ACraftingBox::BeginPlay()
{
	Super::BeginPlay();
}

void ACraftingBox::OnPlaced()
{
	Super::OnPlaced();

	// 설치시 소리 이벤트
}