#include "Items/OblivioItemBase.h"
#include "Components/StaticMeshComponent.h" // 헤더 추가 필수

AOblivioItemBase::AOblivioItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	ItemMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	ItemType = EItemType::Wood; 
}

