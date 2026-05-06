//ICombatInterface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

UINTERFACE(MinimalAPI)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

class OBLIVIO_API ICombatInterface
{
	GENERATED_BODY()

public:
	virtual void ApplyHealth(float Damage) = 0;
	virtual void ApplyCCSlow(float SpeedMultiplier, float Duration) = 0;
	virtual void ApplyCCStun(float Duration) = 0;
	virtual bool IsAlive() const = 0;
};
