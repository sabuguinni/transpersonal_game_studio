#include "Anim_PrimitiveMovementController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimMontage.h"

UAnim_PrimitiveMovementController::UAnim_PrimitiveMovementController()
{
    // Initialize default values
    OwningCharacter = nullptr;
    CharacterMovement = nullptr;
    
    SpeedSmoothingRate = 10.0f;
    DirectionSmoothingRate = 15.0f;
    MovementThreshold = 3.0f;
    WalkSpeedThreshold = 200.0f;
    RunSpeedThreshold = 400.0f;
    SprintSpeedThreshold = 600.0f;
    
    SmoothedSpeed = 0.0f;
    SmoothedDirection = 0.0f;
    
    // Initialize movement data
    MovementData = FAnim_PrimitiveMovementData();
}

void UAnim_PrimitiveMovementController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
    }
}

void UAnim_PrimitiveMovementController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    UpdateMovementData(DeltaTimeX);
    UpdateMovementState();
}

void UAnim_PrimitiveMovementController::UpdateMovementData(float DeltaTime)
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get current velocity and speed
    FVector CurrentVelocity = CharacterMovement->Velocity;
    MovementData.Velocity = CurrentVelocity;
    
    float CurrentSpeed = CurrentVelocity.Size2D();
    
    // Smooth speed
    SmoothedSpeed = UKismetMathLibrary::FInterpTo(SmoothedSpeed, CurrentSpeed, DeltaTime, SpeedSmoothingRate);
    MovementData.Speed = SmoothedSpeed;
    
    // Calculate direction
    if (CurrentSpeed > MovementThreshold)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal2D();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        float TargetDirection = UKismetMathLibrary::Atan2(CrossProduct, DotProduct) * 180.0f / PI;
        
        // Smooth direction
        SmoothedDirection = UKismetMathLibrary::FInterpTo(SmoothedDirection, TargetDirection, DeltaTime, DirectionSmoothingRate);
        MovementData.Direction = SmoothedDirection;
    }
    else
    {
        MovementData.Direction = 0.0f;
        SmoothedDirection = 0.0f;
    }
    
    // Update movement flags
    MovementData.bIsMoving = CurrentSpeed > MovementThreshold;
    MovementData.bIsInAir = CharacterMovement->IsFalling();
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
}

void UAnim_PrimitiveMovementController::UpdateMovementState()
{
    MovementData.MovementState = CalculateMovementState();
}

EAnim_PrimitiveMovementState UAnim_PrimitiveMovementController::CalculateMovementState() const
{
    if (!OwningCharacter || !CharacterMovement)
    {
        return EAnim_PrimitiveMovementState::Idle;
    }
    
    // Check if performing an action
    if (MovementData.CurrentAction != EAnim_PrimitiveAction::None)
    {
        switch (MovementData.CurrentAction)
        {
            case EAnim_PrimitiveAction::SpearThrow:
            case EAnim_PrimitiveAction::StoneKnapping:
            case EAnim_PrimitiveAction::ToolCrafting:
                return EAnim_PrimitiveMovementState::Combat;
            case EAnim_PrimitiveAction::BerryPicking:
            case EAnim_PrimitiveAction::WaterDrinking:
                return EAnim_PrimitiveMovementState::Gathering;
            case EAnim_PrimitiveAction::FireMaking:
            case EAnim_PrimitiveAction::ShelterBuilding:
            case EAnim_PrimitiveAction::HideWorking:
                return EAnim_PrimitiveMovementState::Crafting;
            default:
                break;
        }
    }
    
    // Check air state
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0)
        {
            return EAnim_PrimitiveMovementState::Jumping;
        }
        else
        {
            return EAnim_PrimitiveMovementState::Falling;
        }
    }
    
    // Check crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_PrimitiveMovementState::Crouching;
    }
    
    // Check movement speed
    if (MovementData.bIsMoving)
    {
        if (MovementData.Speed >= SprintSpeedThreshold)
        {
            return EAnim_PrimitiveMovementState::Sprinting;
        }
        else if (MovementData.Speed >= RunSpeedThreshold)
        {
            return EAnim_PrimitiveMovementState::Running;
        }
        else if (MovementData.Speed >= WalkSpeedThreshold)
        {
            return EAnim_PrimitiveMovementState::Walking;
        }
    }
    
    return EAnim_PrimitiveMovementState::Idle;
}

void UAnim_PrimitiveMovementController::PlayActionMontage(EAnim_PrimitiveAction Action)
{
    UAnimMontage* MontageToPlay = GetMontageForAction(Action);
    
    if (MontageToPlay)
    {
        // Stop any currently playing montage
        StopActionMontage();
        
        // Play the new montage
        Montage_Play(MontageToPlay);
        
        // Set current action
        MovementData.CurrentAction = Action;
    }
}

void UAnim_PrimitiveMovementController::StopActionMontage()
{
    if (IsPlayingActionMontage())
    {
        Montage_Stop(0.2f);
    }
    
    MovementData.CurrentAction = EAnim_PrimitiveAction::None;
}

bool UAnim_PrimitiveMovementController::IsPlayingActionMontage() const
{
    return IsAnyMontagePlaying();
}

void UAnim_PrimitiveMovementController::SetCurrentAction(EAnim_PrimitiveAction NewAction)
{
    MovementData.CurrentAction = NewAction;
}

void UAnim_PrimitiveMovementController::UpdateSurvivalStats(float Stamina, float Health, float Fear)
{
    MovementData.StaminaPercentage = FMath::Clamp(Stamina, 0.0f, 1.0f);
    MovementData.HealthPercentage = FMath::Clamp(Health, 0.0f, 1.0f);
    MovementData.FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
}

UAnimMontage* UAnim_PrimitiveMovementController::GetMontageForAction(EAnim_PrimitiveAction Action) const
{
    switch (Action)
    {
        case EAnim_PrimitiveAction::SpearThrow:
            return SpearThrowMontage;
        case EAnim_PrimitiveAction::StoneKnapping:
        case EAnim_PrimitiveAction::ToolCrafting:
        case EAnim_PrimitiveAction::FireMaking:
        case EAnim_PrimitiveAction::ShelterBuilding:
        case EAnim_PrimitiveAction::HideWorking:
            return CraftingMontage;
        case EAnim_PrimitiveAction::BerryPicking:
        case EAnim_PrimitiveAction::WaterDrinking:
            return GatheringMontage;
        default:
            return nullptr;
    }
}