#include "Anim_CharacterMovementBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterMovementBlueprint::UAnim_CharacterMovementBlueprint()
{
    // Initialize movement variables
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsCrouching = false;
    bIsRunning = false;
    bIsIdle = true;
    bIsWalking = false;

    // Initialize combat variables
    bIsInCombat = false;
    bIsAttacking = false;
    bIsBlocking = false;
    bIsDodging = false;

    // Initialize survival variables
    bIsInjured = false;
    bIsExhausted = false;
    bIsHungry = false;
    bIsThirsty = false;

    // Set animation thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    IdleThreshold = 10.0f;

    // Initialize references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;

    // Initialize internal state
    LastFrameVelocity = FVector::ZeroVector;
    AccelerationTimer = 0.0f;
    IdleTimer = 0.0f;
}

void UAnim_CharacterMovementBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get owner character and movement component
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterMovementBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update all animation variables
    UpdateMovementVariables();
    UpdateCombatVariables();
    UpdateSurvivalVariables();
    CalculateDirection();
    DetermineMovementState();

    // Update timers
    if (Speed > IdleThreshold)
    {
        IdleTimer = 0.0f;
        AccelerationTimer += DeltaTimeX;
    }
    else
    {
        IdleTimer += DeltaTimeX;
        AccelerationTimer = 0.0f;
    }

    // Store velocity for next frame
    LastFrameVelocity = MovementComponent->Velocity;
}

void UAnim_CharacterMovementBlueprint::UpdateMovementVariables()
{
    if (!MovementComponent)
    {
        return;
    }

    // Calculate speed from velocity
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Check if character is in air
    bIsInAir = MovementComponent->IsFalling();

    // Check if character is crouching
    bIsCrouching = MovementComponent->IsCrouching();

    // Calculate acceleration
    FVector CurrentAcceleration = (Velocity - LastFrameVelocity);
    bIsAccelerating = CurrentAcceleration.Size2D() > 50.0f;
}

void UAnim_CharacterMovementBlueprint::UpdateCombatVariables()
{
    // TODO: Integrate with combat system when available
    // For now, use placeholder logic based on input or game state
    
    // These would typically be set by a combat component or game state
    // bIsInCombat = CombatComponent ? CombatComponent->IsInCombat() : false;
    // bIsAttacking = CombatComponent ? CombatComponent->IsAttacking() : false;
    // bIsBlocking = CombatComponent ? CombatComponent->IsBlocking() : false;
    // bIsDodging = CombatComponent ? CombatComponent->IsDodging() : false;
}

void UAnim_CharacterMovementBlueprint::UpdateSurvivalVariables()
{
    // TODO: Integrate with survival system when available
    // For now, use placeholder logic
    
    // These would typically be set by a survival component or character stats
    // bIsInjured = SurvivalComponent ? SurvivalComponent->GetHealth() < 0.5f : false;
    // bIsExhausted = SurvivalComponent ? SurvivalComponent->GetStamina() < 0.2f : false;
    // bIsHungry = SurvivalComponent ? SurvivalComponent->GetHunger() < 0.3f : false;
    // bIsThirsty = SurvivalComponent ? SurvivalComponent->GetThirst() < 0.3f : false;
}

void UAnim_CharacterMovementBlueprint::CalculateDirection()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        Direction = 0.0f;
        return;
    }

    // Get character's forward vector and velocity
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector Velocity = MovementComponent->Velocity;

    if (Velocity.Size2D() > IdleThreshold)
    {
        // Normalize velocity to get direction
        FVector VelocityDirection = Velocity.GetSafeNormal2D();
        
        // Calculate angle between forward vector and velocity direction
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        // Convert to angle in degrees (-180 to 180)
        Direction = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
    }
    else
    {
        Direction = 0.0f;
    }
}

void UAnim_CharacterMovementBlueprint::DetermineMovementState()
{
    // Reset all movement states
    bIsIdle = false;
    bIsWalking = false;
    bIsRunning = false;

    // Determine current movement state based on speed
    if (Speed <= IdleThreshold)
    {
        bIsIdle = true;
    }
    else if (Speed <= WalkSpeedThreshold)
    {
        bIsWalking = true;
    }
    else if (Speed <= RunSpeedThreshold)
    {
        bIsRunning = true;
    }
    else
    {
        // Sprint state - use running animation for now
        bIsRunning = true;
    }

    // Override states based on special conditions
    if (bIsInAir)
    {
        // In air - use jump/fall animations
        bIsIdle = false;
        bIsWalking = false;
        bIsRunning = false;
    }

    if (bIsCrouching)
    {
        // Crouching - modify states accordingly
        if (Speed > IdleThreshold)
        {
            bIsWalking = true;
            bIsRunning = false;
        }
    }
}