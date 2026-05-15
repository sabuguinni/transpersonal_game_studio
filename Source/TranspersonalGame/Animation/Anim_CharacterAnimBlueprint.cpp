#include "Anim_CharacterAnimBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimBlueprint::UAnim_CharacterAnimBlueprint()
{
    // Set default animation thresholds
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 400.0f;
    JumpVelocityThreshold = 50.0f;
    
    // Set default blend times
    IdleToWalkBlendTime = 0.2f;
    WalkToRunBlendTime = 0.3f;
    
    // Initialize movement data
    MovementData = FAnim_MovementData();
    
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_CharacterAnimBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get the owning character
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        // Get the movement component
        MovementComponent = OwningCharacter->GetCharacterMovement();
        
        UE_LOG(LogTemp, Warning, TEXT("Animation Blueprint initialized for character: %s"), 
               *OwningCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get owning character in animation blueprint"));
    }
}

void UAnim_CharacterAnimBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update movement data every frame
    UpdateMovementData();
    UpdateMovementState();
}

void UAnim_CharacterAnimBlueprint::UpdateMovementData()
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size2D();
    
    // Calculate movement direction relative to character rotation
    if (MovementData.Speed > 0.1f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        
        MovementData.Direction = UKismetMathLibrary::DegAtan2(CrossProduct, DotProduct);
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Check if character is in air
    MovementData.bIsInAir = MovementComponent->IsFalling();
    
    // Check if character is crouching
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_CharacterAnimBlueprint::UpdateMovementState()
{
    MovementData.MovementState = CalculateMovementState();
}

EAnim_MovementState UAnim_CharacterAnimBlueprint::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if jumping or falling
    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > JumpVelocityThreshold)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check if crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check movement speed for ground states
    if (MovementData.Speed < WalkSpeedThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}