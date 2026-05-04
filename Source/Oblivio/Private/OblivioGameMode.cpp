#include "OblivioGameMode.h"
#include "OblivioCharacter.h"
#include "OblivioCharacterController.h"
#include "OblivioGameInstance.h"
#include "Kismet/GameplayStatics.h"

AOblivioGameMode::AOblivioGameMode()
{
	DefaultPawnClass = AOblivioCharacter::StaticClass();
	PlayerControllerClass = AOblivioCharacterController::StaticClass();
}

void AOblivioGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AOblivioGameMode::NextFloor()
{
	UOblivioGameInstance* GI = Cast<UOblivioGameInstance>(GetGameInstance());
	if (!GI) return;

	if (CollectedKeys >= RequiredKeys)
	{
		GI->CurrentFloor--;
		CollectedKeys = 0; // 이번 층의 열쇠 정보만 리셋

		if (GI->CurrentFloor <= 1)
		{
			DetermineEnding();
		}
		else
		{
			// 다음 층 레벨로 이동하는 로직 자리
			UE_LOG(LogTemp, Warning, TEXT("Floor changed to: %d"), GI->CurrentFloor);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough keys!"));
	}
}

void AOblivioGameMode::AddMonsterKill()
{
	if (UOblivioGameInstance* GI = Cast<UOblivioGameInstance>(GetGameInstance()))
	{
		GI->TotalKills++;
	}
}

void AOblivioGameMode::AddMemento()
{
	if (UOblivioGameInstance* GI = Cast<UOblivioGameInstance>(GetGameInstance()))
	{
		GI->TotalMementos++;
	}
}

EGameEndingType AOblivioGameMode::DetermineEnding()
{
	UOblivioGameInstance* GI = Cast<UOblivioGameInstance>(GetGameInstance());
	if (!GI) return EGameEndingType::None;

	EGameEndingType FinalEnding = EGameEndingType::None;

	if (GI->CurrentFloor == 0)
	{
		FinalEnding = EGameEndingType::Oblivion;
	}
	else if (GI->TotalKills > 0 && GI->TotalMementos > 0)
	{
		FinalEnding = EGameEndingType::DeathRow;
	}
	else
	{
		FinalEnding = EGameEndingType::InfiniteLoop; //기본 엔딩
	}

	//추가 로직 필요하면 이쪽에
	UE_LOG(LogTemp, Warning, TEXT("Final Ending Determined: %d"), (int32)FinalEnding);
	return FinalEnding;
}

void AOblivioGameMode::GameOver()
{
	UE_LOG(LogTemp, Warning, TEXT("Game Over!"));
}