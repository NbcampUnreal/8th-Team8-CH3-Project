#include "Crafting/CraftingFloorSpike.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"

ACraftingFloorSpike::ACraftingFloorSpike()
{
	MaxHealth = 80.0f;
	CurrentHealth = MaxHealth;

	WoodCost = 0;
	IronCost = 2;

	DamageAmount = 10.0f;
	DamageInterval = 0.5f;

	// 대미지 감지 영역 설정
	DamageArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageArea"));
	DamageArea->SetupAttachment(RootComponent);

	DamageArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageArea->SetBoxExtent(FVector(50.0f, 50.0f, 20.0f));
}

void ACraftingFloorSpike::BeginPlay()
{
	Super::BeginPlay();

	// 델리게이트 바인딩
	DamageArea->OnComponentBeginOverlap.AddDynamic(this, &ACraftingFloorSpike::OnOverlapBegin);
	DamageArea->OnComponentEndOverlap.AddDynamic(this, &ACraftingFloorSpike::OnOverlapEnd);
}

void ACraftingFloorSpike::OnPlaced()
{
	Super::OnPlaced();

	// 설치 완료 시 콜리전 활성화
	if (DamageArea)
	{
		DamageArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// 주기적으로 대미지를 입히는 타이머 시작
	GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &ACraftingFloorSpike::ApplySpikeDamage, DamageInterval, true);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Floor Spike Armed!"));
	}
}

void ACraftingFloorSpike::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && !OverlappingEnemies.Contains(OtherActor))
	{
		// 적(Pawn)인 경우에만 배열에 추가
		if (OtherActor->IsA(APawn::StaticClass()))
		{
			OverlappingEnemies.Add(OtherActor);
		}
	}
}

void ACraftingFloorSpike::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		OverlappingEnemies.Remove(OtherActor);
	}
}

void ACraftingFloorSpike::ApplySpikeDamage()
{
	// 배열을 순회하며 유효한 적들에게 대미지 적용
	for (int32 i = OverlappingEnemies.Num() - 1; i >= 0; --i)
	{
		AActor* Target = OverlappingEnemies[i];

		if (IsValid(Target))
		{
			Target->TakeDamage(DamageAmount, FDamageEvent(), GetInstigatorController(), this);
		}
		else
		{
			// 타겟이 파괴되었거나 유효하지 않으면 배열에서 제거
			OverlappingEnemies.RemoveAt(i);
		}
	}
}
