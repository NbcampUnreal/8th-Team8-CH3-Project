#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OblivioCrafting.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OBLIVIO_API UOblivioCrafting : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOblivioCrafting();

protected:
	virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** 제작할 장애물 클래스 정보 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TSubclassOf<class AObstacleBase> SelectedObstacleClass;

    /** 1~7번 키에 대응하는 장애물 클래스 맵 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TMap<int32, TSubclassOf<class AObstacleBase>> CraftingRecipes;

    /** 인덱스로 장애물 선택 */
    void SelectObstacle(int32 Index);

    /** 현재 미리보기 중인 장애물 액터 */
    UPROPERTY()
    class AObstacleBase* PreviewActor = nullptr;

    /** 크래프팅 모드 활성화 여부 */
    bool bIsCraftingModeActive;

    /** 설치 가능 거리 */
    UPROPERTY(EditAnywhere, Category = "Crafting")
    float MaxPlacementDistance = 500.0f;

    /** 크래프팅 시작/종료 */
    void ToggleCraftingMode();

    void RotatePreview();

    /** 실제 설치 실행 */
    void PlaceObstacle();

private:
    /** 설치 가능 위치 계산 및 프리뷰 업데이트 */
    void UpdatePreviewLocation();

    /** 자원 체크 로직 (인벤토리 시스템과 연동 필요) */
    bool CanAfford(class AObstacleBase* TargetObstacle);

    FRotator CurrentPreviewRotation = FRotator::ZeroRotator;
};
