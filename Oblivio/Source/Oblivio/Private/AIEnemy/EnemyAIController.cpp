#include "AIEnemy/EnemyAIController.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
	bAttachToPawn = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Verbose, TEXT("Enemy AI possessed %s"), *GetNameSafe(InPawn));
}

void AEnemyAIController::OnUnPossess()
{
	UE_LOG(LogTemp, Verbose, TEXT("Enemy AI unpossessed %s"), *GetNameSafe(GetPawn()));

	Super::OnUnPossess();
}
