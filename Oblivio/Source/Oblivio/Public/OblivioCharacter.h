#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "OblivioCharacter.generated.h"

class UOblivioCrafting;

UCLASS()
class OBLIVIO_API AOblivioCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AOblivioCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// 컨트롤러
	void Move(const FVector2D& Value);
	void StartRunning();
	void StopRunning();
	void ToggleFlashlight();
	void UseFlashbang();
	void AdjustFocus(float Value);
	void ToggleInventory();
	void ToggleCrafting();
	void PlaceObstacle();
	void Interact();

	//컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* TopDownCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
	class USpotLightComponent* FlashlightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
	UOblivioCrafting* CraftingComponent;

	//생존 스탯 및 상태 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Health = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Battery = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Hunger = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Thirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Flashlight")
	float BatteryDepletionRate = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 550.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Flashlight")
	bool bIsFlashlightOn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Upgrade")
	bool bCanAdjustFocus = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
	class UPointLightComponent* FlashbangLight;

	FTimerHandle FlashbangTimerHandle;
	float FlashbangIntensity = 0.0f;

	void FadeOutFlashbang();

	bool bIsRunning = false;
	bool bIsInventoryOpen = false;
	bool bIsCraftingOpen = false;
	float CurrentFocusAlpha = 0.5f;

	void UpdateStatus(float DeltaTime);
	void UpdateFlashlightVisuals();
};
