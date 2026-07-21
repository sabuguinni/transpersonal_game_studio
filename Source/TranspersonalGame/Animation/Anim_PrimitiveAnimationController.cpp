#include "Anim_PrimitiveAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_PrimitiveAnimationController::UAnim_PrimitiveAnimationController()
{
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::None;
    
    WalkThreshold = 10.0f;
    RunThreshold = 300.0f;
    IdleThreshold = 5.0f;
    
    bIsInCombat = false;
    bIsAttacking = false;
    bIsBlocking = false;
    
    FearLevel = 0.0f;
    StaminaLevel = 1.0f;
    bIsExhausted = false;
}

void UAnim_PrimitiveAnimationController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    
    if (OwningCharacter)
    {
        // Get movement component
        MovementComponent = OwningCharacter->GetCharacterMovement();
        
        if (MovementComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Successfully initialized for character %s"), 
                   *OwningCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: No movement component found on character %s"), 
                   *OwningCharacter->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: Failed to get owning character"));
    }
}

void UAnim_PrimitiveAnimationController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation data
    UpdateMovementData();
    UpdateMovementState();
    UpdateCombatState();
    UpdateSurvivalState();
    UpdateAnimationParameters();
    HandleStateTransitions();
}

void UAnim_PrimitiveAnimationController::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity and movement data
    MovementData.Velocity = MovementComponent->Velocity;
    MovementData.GroundSpeed = MovementData.Velocity.Size2D();
    MovementData.Speed = MovementData.GroundSpeed;
    
    // Calculate movement direction
    if (MovementData.GroundSpeed > IdleThreshold)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityDirection = MovementData.Velocity.GetSafeNormal2D();
        MovementData.Direction = FVector::DotProduct(ForwardVector, VelocityDirection);
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update movement flags
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    MovementData.bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
    MovementData.bShouldMove = MovementData.GroundSpeed > IdleThreshold && MovementData.bIsAccelerating;
}

void UAnim_PrimitiveAnimationController::UpdateMovementState()
{
    if (!MovementComponent)
    {
        return;
    }
    
    EAnim_MovementState NewState = CurrentMovementState;
    
    // Determine movement state based on current conditions
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0.0f)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (bIsInCombat)
    {
        NewState = EAnim_MovementState::Combat;
    }
    else if (MovementData.GroundSpeed > RunThreshold)
    {
        NewState = EAnim_MovementState::Running;
    }
    else if (MovementData.GroundSpeed > WalkThreshold)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else
    {
        NewState = EAnim_MovementState::Idle;
    }
    
    // Update state if changed
    if (NewState != CurrentMovementState)
    {
        SetMovementState(NewState);
    }
}

void UAnim_PrimitiveAnimationController::UpdateCombatState()
{
    // Combat state logic would be updated based on combat system
    // For now, just handle basic state management
    
    if (bIsAttacking)
    {
        SetCombatState(EAnim_CombatState::Attacking);
    }
    else if (bIsBlocking)
    {
        SetCombatState(EAnim_CombatState::Blocking);
    }
    else if (bIsInCombat)
    {
        // In combat but not performing specific action
        if (CurrentCombatState == EAnim_CombatState::None)
        {
            SetCombatState(EAnim_CombatState::None);
        }
    }
    else
    {
        SetCombatState(EAnim_CombatState::None);
    }
}

void UAnim_PrimitiveAnimationController::UpdateSurvivalState()
{
    // Update survival-related animation parameters
    // This would integrate with the survival system when available
    
    // Check stamina level (placeholder logic)
    if (MovementData.GroundSpeed > RunThreshold)
    {
        StaminaLevel = FMath::Max(0.0f, StaminaLevel - 0.01f);
    }
    else if (CurrentMovementState == EAnim_MovementState::Idle)
    {
        StaminaLevel = FMath::Min(1.0f, StaminaLevel + 0.005f);
    }
    
    bIsExhausted = StaminaLevel < 0.2f;
    
    // Fear level would be updated based on nearby threats
    // Placeholder: gradually reduce fear when not in danger
    if (FearLevel > 0.0f)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - 0.01f);
    }
}

void UAnim_PrimitiveAnimationController::UpdateAnimationParameters()
{
    // This function would update any additional animation parameters
    // that don't fit into the main state categories
}

void UAnim_PrimitiveAnimationController::HandleStateTransitions()
{
    // Handle any special logic needed during state transitions
    // This could include playing transition animations or sounds
}

void UAnim_PrimitiveAnimationController::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        EAnim_MovementState PreviousState = CurrentMovementState;
        CurrentMovementState = NewState;
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Movement state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UAnim_PrimitiveAnimationController::SetCombatState(EAnim_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        EAnim_CombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Combat state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UAnim_PrimitiveAnimationController::TriggerAttackAnimation()
{
    if (!bIsAttacking)
    {
        bIsAttacking = true;
        bIsInCombat = true;
        
        // Attack animation would be triggered here
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Attack animation triggered"));
        
        // Reset attack flag after a delay (would be handled by animation notify in practice)
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { bIsAttacking = false; },
            1.0f,
            false
        );
    }
}

void UAnim_PrimitiveAnimationController::TriggerBlockAnimation()
{
    bIsBlocking = true;
    bIsInCombat = true;
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Block animation triggered"));
}

void UAnim_PrimitiveAnimationController::TriggerDodgeAnimation()
{
    SetCombatState(EAnim_CombatState::Dodging);
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Dodge animation triggered"));
    
    // Reset dodge state after animation completes
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { 
            if (CurrentCombatState == EAnim_CombatState::Dodging) 
            {
                SetCombatState(EAnim_CombatState::None);
            }
        },
        0.8f,
        false
    );
}

void UAnim_PrimitiveAnimationController::TriggerFearReaction()
{
    FearLevel = FMath::Min(1.0f, FearLevel + 0.3f);
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Fear reaction triggered, level: %f"), FearLevel);
}