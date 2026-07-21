#include "Core_EnhancedInputSystem.h"
#include "Core/GameFramework/TranspersonalCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogCoreInput, Log, All);

// Static member initialization
TMap<APlayerController*, bool> UCore_EnhancedInputSystem::InputValidationCache;

UCore_EnhancedInputSystem::UCore_EnhancedInputSystem()
{
    MouseSensitivity = 1.0f;
    GamepadSensitivity = 1.0f;
}

bool UCore_EnhancedInputSystem::InitializeEnhancedInput(ATranspersonalCharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogCoreInput, Error, TEXT("InitializeEnhancedInput: Character is null"));
        return false;
    }
    
    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (!PC)
    {
        UE_LOG(LogCoreInput, Error, TEXT("InitializeEnhancedInput: No PlayerController found"));
        return false;
    }
    
    // Check if Enhanced Input subsystem is available
    if (!IsEnhancedInputAvailable(PC))
    {
        UE_LOG(LogCoreInput, Error, TEXT("InitializeEnhancedInput: Enhanced Input subsystem not available"));
        return false;
    }
    
    // Get Enhanced Input component
    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(Character->InputComponent);
    if (!EnhancedInputComponent)
    {
        UE_LOG(LogCoreInput, Error, TEXT("InitializeEnhancedInput: Enhanced Input Component not found"));
        return false;
    }
    
    // Register input actions
    RegisterInputActions(Character, EnhancedInputComponent);
    
    // Cache validation result
    InputValidationCache.Add(PC, true);
    
    UE_LOG(LogCoreInput, Log, TEXT("Enhanced Input initialized successfully for %s"), *Character->GetName());
    return true;
}

bool UCore_EnhancedInputSystem::ValidateInputMappingContext(UInputMappingContext* Context)
{
    if (!Context)
    {
        UE_LOG(LogCoreInput, Warning, TEXT("ValidateInputMappingContext: Context is null"));
        return false;
    }
    
    const TArray<FEnhancedActionKeyMapping>& Mappings = Context->GetMappings();
    if (Mappings.Num() == 0)
    {
        UE_LOG(LogCoreInput, Warning, TEXT("ValidateInputMappingContext: Context has no mappings"));
        return false;
    }
    
    UE_LOG(LogCoreInput, Log, TEXT("ValidateInputMappingContext: Context %s has %d mappings"), 
           *Context->GetName(), Mappings.Num());
    return true;
}

void UCore_EnhancedInputSystem::RegisterInputActions(ATranspersonalCharacter* Character, UEnhancedInputComponent* InputComponent)
{
    if (!Character || !InputComponent)
    {
        UE_LOG(LogCoreInput, Error, TEXT("RegisterInputActions: Invalid parameters"));
        return;
    }
    
    // Bind movement actions
    BindMovementActions(Character, InputComponent);
    
    // Bind camera actions
    BindCameraActions(Character, InputComponent);
    
    // Bind interaction actions
    BindInteractionActions(Character, InputComponent);
    
    UE_LOG(LogCoreInput, Log, TEXT("Input actions registered for %s"), *Character->GetName());
}

void UCore_EnhancedInputSystem::SwitchInputContext(APlayerController* Controller, UInputMappingContext* NewContext, int32 Priority)
{
    if (!Controller || !NewContext)
    {
        UE_LOG(LogCoreInput, Error, TEXT("SwitchInputContext: Invalid parameters"));
        return;
    }
    
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer()))
    {
        // Clear existing contexts
        Subsystem->ClearAllMappings();
        
        // Add new context
        Subsystem->AddMappingContext(NewContext, Priority);
        
        UE_LOG(LogCoreInput, Log, TEXT("Switched input context to %s with priority %d"), 
               *NewContext->GetName(), Priority);
    }
}

FString UCore_EnhancedInputSystem::GetCurrentInputDevice(APlayerController* Controller)
{
    if (!Controller)
    {
        return TEXT("Unknown");
    }
    
    // Simple device detection based on last input
    if (Controller->WasInputKeyJustPressed(EKeys::Gamepad_LeftStick_Up) ||
        Controller->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom))
    {
        return TEXT("Gamepad");
    }
    
    if (Controller->WasInputKeyJustPressed(EKeys::W) ||
        Controller->WasInputKeyJustPressed(EKeys::LeftMouseButton))
    {
        return TEXT("Keyboard_Mouse");
    }
    
    return TEXT("Keyboard_Mouse"); // Default assumption
}

bool UCore_EnhancedInputSystem::IsEnhancedInputAvailable(APlayerController* Controller)
{
    if (!Controller)
    {
        return false;
    }
    
    // Check cache first
    if (bool* CachedResult = InputValidationCache.Find(Controller))
    {
        return *CachedResult;
    }
    
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer());
    bool bAvailable = (Subsystem != nullptr);
    
    // Cache result
    InputValidationCache.Add(Controller, bAvailable);
    
    return bAvailable;
}

bool UCore_EnhancedInputSystem::ValidateInputComponent(UEnhancedInputComponent* Component)
{
    return (Component != nullptr);
}

bool UCore_EnhancedInputSystem::ValidateInputSubsystem(UEnhancedInputLocalPlayerSubsystem* Subsystem)
{
    return (Subsystem != nullptr);
}

void UCore_EnhancedInputSystem::BindMovementActions(ATranspersonalCharacter* Character, UEnhancedInputComponent* InputComponent)
{
    if (!Character || !InputComponent)
    {
        return;
    }
    
    // Bind Move action
    if (Character->MoveAction)
    {
        InputComponent->BindAction(Character->MoveAction, ETriggerEvent::Triggered, Character, &ATranspersonalCharacter::Move);
        UE_LOG(LogCoreInput, Log, TEXT("Bound Move action"));
    }
    
    // Bind Jump action
    if (Character->JumpAction)
    {
        InputComponent->BindAction(Character->JumpAction, ETriggerEvent::Triggered, Character, &ACharacter::Jump);
        InputComponent->BindAction(Character->JumpAction, ETriggerEvent::Completed, Character, &ACharacter::StopJumping);
        UE_LOG(LogCoreInput, Log, TEXT("Bound Jump action"));
    }
    
    // Bind Sneak action
    if (Character->SneakAction)
    {
        InputComponent->BindAction(Character->SneakAction, ETriggerEvent::Started, Character, &ATranspersonalCharacter::StartSneaking);
        InputComponent->BindAction(Character->SneakAction, ETriggerEvent::Completed, Character, &ATranspersonalCharacter::StopSneaking);
        UE_LOG(LogCoreInput, Log, TEXT("Bound Sneak action"));
    }
    
    // Bind Run action
    if (Character->RunAction)
    {
        InputComponent->BindAction(Character->RunAction, ETriggerEvent::Started, Character, &ATranspersonalCharacter::StartRunning);
        InputComponent->BindAction(Character->RunAction, ETriggerEvent::Completed, Character, &ATranspersonalCharacter::StopRunning);
        UE_LOG(LogCoreInput, Log, TEXT("Bound Run action"));
    }
}

void UCore_EnhancedInputSystem::BindCameraActions(ATranspersonalCharacter* Character, UEnhancedInputComponent* InputComponent)
{
    if (!Character || !InputComponent)
    {
        return;
    }
    
    // Bind Look action
    if (Character->LookAction)
    {
        InputComponent->BindAction(Character->LookAction, ETriggerEvent::Triggered, Character, &ATranspersonalCharacter::Look);
        UE_LOG(LogCoreInput, Log, TEXT("Bound Look action"));
    }
}

void UCore_EnhancedInputSystem::BindInteractionActions(ATranspersonalCharacter* Character, UEnhancedInputComponent* InputComponent)
{
    if (!Character || !InputComponent)
    {
        return;
    }
    
    // Bind Interact action
    if (Character->InteractAction)
    {
        InputComponent->BindAction(Character->InteractAction, ETriggerEvent::Triggered, Character, &ATranspersonalCharacter::Interact);
        UE_LOG(LogCoreInput, Log, TEXT("Bound Interact action"));
    }
    
    // Bind Craft action
    if (Character->CraftAction)
    {
        InputComponent->BindAction(Character->CraftAction, ETriggerEvent::Triggered, Character, &ATranspersonalCharacter::OpenCraftingMenu);
        UE_LOG(LogCoreInput, Log, TEXT("Bound Craft action"));
    }
}