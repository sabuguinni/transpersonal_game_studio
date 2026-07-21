#include "Char_PrimitiveHumanController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"

AChar_PrimitiveHumanController::AChar_PrimitiveHumanController()
{
    // Set default values
    BaseLookUpRate = 45.0f;
    BaseTurnRate = 45.0f;
    bSurvivalCameraMode = true;
    bIsInCraftingMode = false;
    bInventoryOpen = false;
    CurrentSurvivalState = ESurvivalState::Normal;
    
    // Initialize pointers
    SurvivalHUD = nullptr;
    InteractionWidget = nullptr;
    DefaultMappingContext = nullptr;
    JumpAction = nullptr;
    MoveAction = nullptr;
    LookAction = nullptr;
    InteractAction = nullptr;
    CraftAction = nullptr;
    InventoryAction = nullptr;
}

void AChar_PrimitiveHumanController::BeginPlay()
{
    Super::BeginPlay();
    
    // Add Input Mapping Context
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
    
    // Initialize survival camera mode
    EnableSurvivalCameraMode(true);
    
    // Set initial camera distance for third-person survival view
    SetCameraDistance(400.0f);
}

void AChar_PrimitiveHumanController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // Jumping
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AChar_PrimitiveHumanController::Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AChar_PrimitiveHumanController::StopJumping);
        }
        
        // Moving
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AChar_PrimitiveHumanController::Move);
        }
        
        // Looking
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AChar_PrimitiveHumanController::Look);
        }
        
        // Interaction
        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AChar_PrimitiveHumanController::Interact);
        }
        
        // Crafting
        if (CraftAction)
        {
            EnhancedInputComponent->BindAction(CraftAction, ETriggerEvent::Triggered, this, &AChar_PrimitiveHumanController::OpenCraftingMenu);
        }
        
        // Inventory
        if (InventoryAction)
        {
            EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Triggered, this, &AChar_PrimitiveHumanController::ToggleInventory);
        }
    }
}

void AChar_PrimitiveHumanController::Jump()
{
    if (ACharacter* ControlledCharacter = GetCharacter())
    {
        ControlledCharacter->Jump();
    }
}

void AChar_PrimitiveHumanController::StopJumping()
{
    if (ACharacter* ControlledCharacter = GetCharacter())
    {
        ControlledCharacter->StopJumping();
    }
}

void AChar_PrimitiveHumanController::Move(const FInputActionValue& Value)
{
    // Input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();
    
    if (APawn* ControlledPawn = GetPawn())
    {
        // Find out which way is forward
        const FRotator Rotation = GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // Get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        
        // Get right vector
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
        // Add movement
        ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
        ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AChar_PrimitiveHumanController::Look(const FInputActionValue& Value)
{
    // Input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    
    if (APawn* ControlledPawn = GetPawn())
    {
        // Add yaw and pitch input to controller
        AddYawInput(LookAxisVector.X * BaseTurnRate * GetWorld()->GetDeltaSeconds());
        AddPitchInput(LookAxisVector.Y * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
    }
}

void AChar_PrimitiveHumanController::Interact()
{
    // Perform interaction logic
    if (ACharacter* ControlledCharacter = GetCharacter())
    {
        // Trace forward to find interactable objects
        FVector Start = ControlledCharacter->GetActorLocation();
        FVector ForwardVector = ControlledCharacter->GetActorForwardVector();
        FVector End = Start + (ForwardVector * 200.0f); // 2 meter interaction range
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(ControlledCharacter);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
        {
            if (AActor* HitActor = HitResult.GetActor())
            {
                // Log interaction for now
                UE_LOG(LogTemp, Warning, TEXT("Interacting with: %s"), *HitActor->GetName());
                
                // TODO: Implement actual interaction system
                // Check if actor has interaction interface
                // Execute interaction behavior
            }
        }
    }
}

void AChar_PrimitiveHumanController::OpenCraftingMenu()
{
    bIsInCraftingMode = !bIsInCraftingMode;
    
    if (bIsInCraftingMode)
    {
        // Show crafting UI
        UE_LOG(LogTemp, Warning, TEXT("Opening crafting menu"));
        // TODO: Create and display crafting widget
    }
    else
    {
        // Hide crafting UI
        UE_LOG(LogTemp, Warning, TEXT("Closing crafting menu"));
        // TODO: Hide crafting widget
    }
}

void AChar_PrimitiveHumanController::ToggleInventory()
{
    bInventoryOpen = !bInventoryOpen;
    
    if (bInventoryOpen)
    {
        // Show inventory UI
        UE_LOG(LogTemp, Warning, TEXT("Opening inventory"));
        // TODO: Create and display inventory widget
    }
    else
    {
        // Hide inventory UI
        UE_LOG(LogTemp, Warning, TEXT("Closing inventory"));
        // TODO: Hide inventory widget
    }
}

void AChar_PrimitiveHumanController::UpdateSurvivalStats(float Health, float Hunger, float Thirst, float Stamina, float Fear)
{
    // Update survival HUD with current stats
    if (SurvivalHUD)
    {
        // TODO: Update HUD widget with new values
        UE_LOG(LogTemp, Log, TEXT("Survival Stats - Health: %.1f, Hunger: %.1f, Thirst: %.1f, Stamina: %.1f, Fear: %.1f"), 
               Health, Hunger, Thirst, Stamina, Fear);
    }
    
    // Determine survival state based on stats
    if (Health < 20.0f || Hunger < 10.0f || Thirst < 10.0f)
    {
        CurrentSurvivalState = ESurvivalState::Critical;
    }
    else if (Health < 50.0f || Hunger < 30.0f || Thirst < 30.0f || Fear > 70.0f)
    {
        CurrentSurvivalState = ESurvivalState::Stressed;
    }
    else
    {
        CurrentSurvivalState = ESurvivalState::Normal;
    }
}

void AChar_PrimitiveHumanController::ShowInteractionPrompt(const FString& InteractionText)
{
    if (InteractionWidget)
    {
        // TODO: Update interaction widget text and show it
        UE_LOG(LogTemp, Log, TEXT("Interaction Prompt: %s"), *InteractionText);
    }
}

void AChar_PrimitiveHumanController::HideInteractionPrompt()
{
    if (InteractionWidget)
    {
        // TODO: Hide interaction widget
        UE_LOG(LogTemp, Log, TEXT("Hiding interaction prompt"));
    }
}

void AChar_PrimitiveHumanController::SetCameraDistance(float Distance)
{
    if (ACharacter* ControlledCharacter = GetCharacter())
    {
        if (USpringArmComponent* SpringArm = ControlledCharacter->FindComponentByClass<USpringArmComponent>())
        {
            SpringArm->TargetArmLength = Distance;
            UE_LOG(LogTemp, Log, TEXT("Camera distance set to: %.1f"), Distance);
        }
    }
}

void AChar_PrimitiveHumanController::EnableSurvivalCameraMode(bool bEnable)
{
    bSurvivalCameraMode = bEnable;
    
    if (ACharacter* ControlledCharacter = GetCharacter())
    {
        if (USpringArmComponent* SpringArm = ControlledCharacter->FindComponentByClass<USpringArmComponent>())
        {
            if (bSurvivalCameraMode)
            {
                // Configure for survival gameplay
                SpringArm->bUsePawnControlRotation = true;
                SpringArm->bDoCollisionTest = true;
                SpringArm->TargetArmLength = 400.0f;
                SpringArm->SocketOffset = FVector(0.0f, 0.0f, 80.0f);
            }
            else
            {
                // Standard third-person setup
                SpringArm->bUsePawnControlRotation = true;
                SpringArm->bDoCollisionTest = false;
                SpringArm->TargetArmLength = 300.0f;
                SpringArm->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
            }
        }
    }
}