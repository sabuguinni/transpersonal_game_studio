#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "../SharedTypes.h"
#include "Char_PrimitiveHumanController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * Player controller specifically designed for primitive human characters in Cretaceous survival gameplay.
 * Handles input mapping, survival UI, and character-specific interactions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveHumanController : public APlayerController
{
    GENERATED_BODY()

public:
    AChar_PrimitiveHumanController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // Enhanced Input System
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* CraftAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* InventoryAction;

    // Input handlers
    void Jump();
    void StopJumping();
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Interact();
    void OpenCraftingMenu();
    void ToggleInventory();

public:
    // Survival UI management
    UFUNCTION(BlueprintCallable, Category = "Survival UI")
    void UpdateSurvivalStats(float Health, float Hunger, float Thirst, float Stamina, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Survival UI")
    void ShowInteractionPrompt(const FString& InteractionText);

    UFUNCTION(BlueprintCallable, Category = "Survival UI")
    void HideInteractionPrompt();

    // Character state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsInCraftingMode;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bInventoryOpen;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    ESurvivalState CurrentSurvivalState;

    // Camera control
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void EnableSurvivalCameraMode(bool bEnable);

private:
    // Internal state
    float BaseLookUpRate;
    float BaseTurnRate;
    bool bSurvivalCameraMode;
    
    // UI references
    UPROPERTY()
    class UUserWidget* SurvivalHUD;
    
    UPROPERTY()
    class UUserWidget* InteractionWidget;
};