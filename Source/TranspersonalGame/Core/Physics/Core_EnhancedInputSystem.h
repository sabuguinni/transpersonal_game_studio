#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Core_EnhancedInputSystem.generated.h"

/**
 * Core Enhanced Input System Manager
 * 
 * Manages Enhanced Input integration for the TranspersonalCharacter
 * Handles input mapping contexts, action bindings, and input validation
 * 
 * Features:
 * - Dynamic input context switching
 * - Input action validation
 * - Controller type detection
 * - Input sensitivity management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_EnhancedInputSystem : public UObject
{
    GENERATED_BODY()

public:
    UCore_EnhancedInputSystem();

    // Initialize Enhanced Input system for a character
    UFUNCTION(BlueprintCallable, Category = "Core Input")
    static bool InitializeEnhancedInput(class ATranspersonalCharacter* Character);
    
    // Validate input mapping context
    UFUNCTION(BlueprintCallable, Category = "Core Input")
    static bool ValidateInputMappingContext(UInputMappingContext* Context);
    
    // Register input actions for a character
    UFUNCTION(BlueprintCallable, Category = "Core Input")
    static void RegisterInputActions(class ATranspersonalCharacter* Character, UEnhancedInputComponent* InputComponent);
    
    // Switch input context (for different game states)
    UFUNCTION(BlueprintCallable, Category = "Core Input")
    static void SwitchInputContext(APlayerController* Controller, UInputMappingContext* NewContext, int32 Priority = 0);
    
    // Get current input device type
    UFUNCTION(BlueprintPure, Category = "Core Input")
    static FString GetCurrentInputDevice(APlayerController* Controller);
    
    // Validate Enhanced Input subsystem availability
    UFUNCTION(BlueprintPure, Category = "Core Input")
    static bool IsEnhancedInputAvailable(APlayerController* Controller);

protected:
    // Input sensitivity settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
    float MouseSensitivity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
    float GamepadSensitivity;
    
    // Default input contexts
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Contexts")
    TSoftObjectPtr<UInputMappingContext> DefaultContext;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Contexts")
    TSoftObjectPtr<UInputMappingContext> MenuContext;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Contexts")
    TSoftObjectPtr<UInputMappingContext> CraftingContext;

private:
    // Internal validation functions
    static bool ValidateInputComponent(UEnhancedInputComponent* Component);
    static bool ValidateInputSubsystem(UEnhancedInputLocalPlayerSubsystem* Subsystem);
    
    // Input action binding helpers
    static void BindMovementActions(class ATranspersonalCharacter* Character, UEnhancedInputComponent* InputComponent);
    static void BindCameraActions(class ATranspersonalCharacter* Character, UEnhancedInputComponent* InputComponent);
    static void BindInteractionActions(class ATranspersonalCharacter* Character, UEnhancedInputComponent* InputComponent);
    
    // Input validation cache
    static TMap<APlayerController*, bool> InputValidationCache;
};