#include "Crafting/CraftingReflector.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"

ACraftingReflector::ACraftingReflector()
{
	MaxHealth = 120.0f;
	CurrentHealth = MaxHealth;

	// 설치 비용 설정
	WoodCost = 0;
	IronCost = 3;

	ReflectionIntensity = 5000.0f;
	ReflectionConeAngle = 30.0f;

	// 반사광 컴포넌트 생성 및 설정
	ReflectedLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("ReflectedLight"));
	ReflectedLight->SetupAttachment(RootComponent);

	ReflectedLight->SetIntensity(0.0f);
	ReflectedLight->InnerConeAngle = ReflectionConeAngle * 0.5f;
	ReflectedLight->OuterConeAngle = ReflectionConeAngle;
}

void ACraftingReflector::BeginPlay()
{
	Super::BeginPlay();
}

void ACraftingReflector::OnPlaced()
{
	Super::OnPlaced();

	if (ReflectedLight)
	{
		ReflectedLight->SetIntensity(ReflectionIntensity);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Reflector Active: Redirecting light!"));
	}
}
