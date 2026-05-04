#include "AIEnemy/EnemyAIController.h"

// Tick 끔 — 블랙보드/행동트리 없음. 폰에 붙여 MoveTo만 쓰는 경량 컨트롤러.
AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
	bAttachToPawn = true;
}

// 디버그용 Verbose 로그
void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Verbose, TEXT("Enemy AI possessed %s"), *GetNameSafe(InPawn));
}

void AEnemyAIController::OnUnPossess()
{
	// Unpossess 전에 폰 이름 로그(Super 이후엔 Pawn nullptr)
	UE_LOG(LogTemp, Verbose, TEXT("Enemy AI unpossessed %s"), *GetNameSafe(GetPawn()));

	Super::OnUnPossess();
}
