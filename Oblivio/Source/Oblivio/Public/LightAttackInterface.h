//LightAttackInterface.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LightAttackInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULightAttackInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class OBLIVIO_API ILightAttackInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void CreateLightAttack(FVector SourceLocation, FVector LightDirection);
};
