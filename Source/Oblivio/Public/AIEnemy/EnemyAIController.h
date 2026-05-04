#pragma once

// =============================================================================
// AEnemyAIController — 적 폰 전용 AIController. 이동/FSM 본체는 AEnemyBase::Tick.
// Possess 시 폰에 붙어 PathFollowing 등이 동작하도록 bAttachToPawn 유지.
// =============================================================================

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class OBLIVIO_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
};
