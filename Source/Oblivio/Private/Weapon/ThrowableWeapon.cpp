//ThrowableWeapon.cpp


#include "Weapon/ThrowableWeapon.h"

AThrowableWeapon::AThrowableWeapon()
{
    PrimaryActorTick.bCanEverTick = true;
    ThrowOffset = 50.f;
    SecondsPerDistance = 0.001f;
    HeightPerDistance = 0.2f;
}

void AThrowableWeapon::BeginPlay()
{
    Super::BeginPlay();
    bIsFlying = false;
    TimeElapsed = 0.f;
    ThrowDuration = 0.f;
    ThrowHeight = 0.f;
    StartLocation = FVector::ZeroVector;
    TargetLocation = FVector::ZeroVector;
}
void AThrowableWeapon::StartThrow(FVector Destination) {
    StartLocation = GetActorLocation();
    TargetLocation = Destination;
    TimeElapsed = 0.f;
    ThrowDuration = (Destination - GetActorLocation()).Length() * SecondsPerDistance;
    ThrowHeight = (Destination - GetActorLocation()).Length() * HeightPerDistance;
    bIsFlying = true;

    SetActorLocation(StartLocation);
    UE_LOG(LogTemp, Warning, TEXT("ThrowDuration :%f!"), ThrowDuration);
}

void AThrowableWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsFlying) return;
    TimeElapsed += DeltaTime;
    float Alpha = FMath::Clamp(TimeElapsed / ThrowDuration, 0.f, 1.f);

    // 수평 이동
    FVector CurrentPos;
    CurrentPos.X = FMath::Lerp(StartLocation.X, TargetLocation.X, Alpha);
    CurrentPos.Y = FMath::Lerp(StartLocation.Y, TargetLocation.Y, Alpha);

    // 수직 이동
    float BaseZ = FMath::Lerp(StartLocation.Z, TargetLocation.Z, Alpha);
    CurrentPos.Z = BaseZ + FMath::Sin(Alpha * PI) * ThrowHeight;
    SetActorLocation(CurrentPos);

    //포물선 운동 끝
    if (Alpha >= 1.f)
    {
        bIsFlying = false;
        SetActorLocation(TargetLocation);
    }
}