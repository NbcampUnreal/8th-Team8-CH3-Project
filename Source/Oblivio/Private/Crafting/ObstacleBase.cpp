#include "Crafting/ObstacleBase.h"
#include "Components/StaticMeshComponent.h"

AObstacleBase::AObstacleBase()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // 기본적으로 몬스터 이동에 영향을 주도록 설정
    MeshComponent->SetCanEverAffectNavigation(true);

    CurrentHealth = MaxHealth;

}

void AObstacleBase::BeginPlay()
{
	Super::BeginPlay();
    OriginalMaterial = MeshComponent->GetMaterial(0);
}

void AObstacleBase::SetGhostMode(bool bIsGhost, bool bCanPlace)
{
    if (bIsGhost)
    {
        CurrentState = EObstacleState::Ghost;
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // 다이나믹 메테리얼 인스턴스 생성
        UMaterialInstanceDynamic* DynamicMat = MeshComponent->CreateDynamicMaterialInstance(0, GhostMaterial);

        if (DynamicMat)
        {
            // 설치 가능 여부에 따라 색상 파라미터 조절
            FLinearColor GhostColor = bCanPlace ? FLinearColor::Green : FLinearColor::Red;
            DynamicMat->SetVectorParameterValue(TEXT("Color"), GhostColor);
            DynamicMat->SetScalarParameterValue(TEXT("Opacity"), 0.5f);
        }
    }
}

void AObstacleBase::OnPlaced()
{
    CurrentState = EObstacleState::Placed;
    MeshComponent->SetMaterial(0, OriginalMaterial);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // 설치 완료 후 네비게이션 리빌드 유도
    MeshComponent->SetCanEverAffectNavigation(true);

    bIsPlaced = true;
}

float AObstacleBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (CurrentState != EObstacleState::Placed) return 0.0f;

    CurrentHealth -= DamageAmount;

    if (CurrentHealth <= 0.0f)
    {
        CurrentState = EObstacleState::Destroyed;
        Destroy();
    }

    return DamageAmount;
}
