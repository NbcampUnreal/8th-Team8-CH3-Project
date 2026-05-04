#include "OblivioCharacterController.h"
#include "OblivioCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"

AOblivioCharacterController::AOblivioCharacterController()
	: DefaultMappingContext(nullptr)
	, InventoryMappingContext(nullptr)
	, MoveAction(nullptr)
	, LookAction(nullptr)
	, WheelAction(nullptr)
	, RunAction(nullptr)
	, FlashlightToggleAction(nullptr)
	, FlashbangAction(nullptr)
	, InventoryAction(nullptr)
	, CraftingAction(nullptr)
	, InteractAction(nullptr)
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AOblivioCharacterController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AOblivioCharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOblivioCharacterController::OnMove);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOblivioCharacterController::OnLook);
		EIC->BindAction(WheelAction, ETriggerEvent::Triggered, this, &AOblivioCharacterController::OnWheel);
		EIC->BindAction(RunAction, ETriggerEvent::Started, this, &AOblivioCharacterController::OnRunStarted);
		EIC->BindAction(RunAction, ETriggerEvent::Completed, this, &AOblivioCharacterController::OnRunCompleted);
		EIC->BindAction(FlashlightToggleAction, ETriggerEvent::Started, this, &AOblivioCharacterController::OnFlashlightToggle);
		EIC->BindAction(FlashbangAction, ETriggerEvent::Started, this, &AOblivioCharacterController::OnFlashbang);
		EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &AOblivioCharacterController::OnInventoryToggle);
		EIC->BindAction(CraftingAction, ETriggerEvent::Started, this, &AOblivioCharacterController::OnCraftingToggle);
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AOblivioCharacterController::OnInteract);
	}
}

void AOblivioCharacterController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateMouseRotation();
}

void AOblivioCharacterController::UpdateMouseRotation()
{
	if (APawn* MyPawn = GetPawn())
	{
		FHitResult Hit;
		if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
		{
			FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(MyPawn->GetActorLocation(), Hit.ImpactPoint);
			MyPawn->SetActorRotation(FRotator(0.f, LookRot.Yaw, 0.f));
		}
	}
}

void AOblivioCharacterController::OnMove(const FInputActionValue& Value)
{
	if (AOblivioCharacter* ObjChar = Cast<AOblivioCharacter>(GetPawn()))
		ObjChar->Move(Value.Get<FVector2D>());
}

void AOblivioCharacterController::OnLook(const FInputActionValue& Value)
{
	// Tick에서 자동 처리 중
}

void AOblivioCharacterController::OnWheel(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("OnWheel"));
	if (AOblivioCharacter* ObjChar = Cast<AOblivioCharacter>(GetPawn())) {
		UE_LOG(LogTemp, Warning, TEXT("OnWheel action"));
		ObjChar->AdjustFocus(Value.Get<float>());
	}
		
}

void AOblivioCharacterController::OnRunStarted(const FInputActionValue& Value)
{
	if (AOblivioCharacter* ObjChar = Cast<AOblivioCharacter>(GetPawn()))
		ObjChar->StartRunning();
}

void AOblivioCharacterController::OnRunCompleted(const FInputActionValue& Value)
{
	if (AOblivioCharacter* ObjChar = Cast<AOblivioCharacter>(GetPawn()))
		ObjChar->StopRunning();
}

void AOblivioCharacterController::OnFlashlightToggle(const FInputActionValue& Value)
{
	if (AOblivioCharacter* ObjChar = Cast<AOblivioCharacter>(GetPawn()))
		ObjChar->ToggleFlashlight();
}

void AOblivioCharacterController::OnFlashbang(const FInputActionValue& Value)
{
	if (AOblivioCharacter* ObjChar = Cast<AOblivioCharacter>(GetPawn()))
		ObjChar->UseFlashbang();
}

void AOblivioCharacterController::OnInventoryToggle(const FInputActionValue& Value)
{
	if (AOblivioCharacter* ObjChar = Cast<AOblivioCharacter>(GetPawn()))
		ObjChar->ToggleInventory();
}

void AOblivioCharacterController::OnCraftingToggle(const FInputActionValue& Value)
{
	if (AOblivioCharacter* ObjChar = Cast<AOblivioCharacter>(GetPawn()))
		ObjChar->ToggleCrafting();
}

void AOblivioCharacterController::OnInteract(const FInputActionValue& Value)
{
	if (AOblivioCharacter* ObjChar = Cast<AOblivioCharacter>(GetPawn()))
		ObjChar->Interact();
}