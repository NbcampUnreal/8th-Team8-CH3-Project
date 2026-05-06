#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OblivioItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Wood    UMETA(DisplayName = "Wood"),
	Iron    UMETA(DisplayName = "Iron"),
	Food    UMETA(DisplayName = "Food"),
	Water   UMETA(DisplayName = "Water"),
	Battery UMETA(DisplayName = "Battery")
};

UCLASS()
class OBLIVIO_API AOblivioItemBase : public AActor
{
	GENERATED_BODY()
	
public:
	AOblivioItemBase();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Settings")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Settings")
	float RestoreValue = 30.0f; // 음식/물 회복량이나 자원 개수
};
