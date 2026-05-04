#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OblivioGameInstance.generated.h"

UCLASS()
class OBLIVIO_API UOblivioGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Persistence")
	int32 TotalKills = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Persistence")
	int32 TotalMementos = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Persistence")
	int32 CurrentFloor = 9;

	//게임 재시작 시 데이터 초기화용
	void ResetGameData()
	{
		TotalKills = 0;
		TotalMementos = 0;
		CurrentFloor = 9;
	}
};