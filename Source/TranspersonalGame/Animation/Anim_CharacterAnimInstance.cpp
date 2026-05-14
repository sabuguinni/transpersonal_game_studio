#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    // Set default animation parameters
    WalkSpeed = 150.0f;
    RunSpeed = 400.0f;
    CrouchSpeed = 100.0f;
    
    // Initialize state
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::Unarmed;
    
    // Initialize blend values
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    DirectionalBlend = 0.0f;
    
    // Initialize transition flags
    bShouldEnterJump = false;
    bShouldEnterFalling = false;
    bShouldEnterLanding = false;
    
    // Initialize internal state
    LastGroundSpeed = 0.0f;
    StateChangeTimer = 0.0f;
    bWasInAir = false;
    bJustLanded = false;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        CharacterMovement = OwnerCharacter->GetCharacterMovement();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                FString::Printf(TEXT("Animation Instance initialized for: %s"), 
                *OwnerCharacter->GetName()));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("Failed to get character reference in AnimInstance"));
        }
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update state change timer
    StateChangeTimer += DeltaTimeX;
    
    // Update movement data
    UpdateMovementData();
    
    // Update movement state
    UpdateMovementState();
    
    // Update blend values
    UpdateBlendValues();
    
    // Update transition flags
    UpdateTransitionFlags();
}

void UAnim_CharacterAnimInstance::UpdateMovementData()
{
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get velocity and calculate speed
    MovementData.Velocity = CharacterMovement->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    MovementData.GroundSpeed = MovementData.Velocity.Size2D();
    
    // Calculate direction relative to character forward
    if (MovementData.GroundSpeed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = MovementData.Velocity.GetSafeNormal2D();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityDirection);
        if (RightDot < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update air state
    MovementData.bIsInAir = CharacterMovement->IsFalling();
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
    
    // Calculate jump height (distance from ground)
    if (MovementData.bIsInAir)
    {
        FHitResult HitResult;
        FVector StartLocation = OwnerCharacter->GetActorLocation();
        FVector EndLocation = StartLocation - FVector(0, 0, 2000.0f);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic))
        {
            MovementData.JumpHeight = HitResult.Distance;
        }
        else
        {
            MovementData.JumpHeight = 2000.0f;
        }
    }
    else
    {
        MovementData.JumpHeight = 0.0f;
    }
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    EAnim_MovementState NewState = CalculateMovementState();
    
    if (NewState != CurrentMovementState)
    {
        CurrentMovementState = NewState;
        StateChangeTimer = 0.0f;
        
        if (GEngine)
        {
            FString StateName = UEnum::GetValueAsString(CurrentMovementState);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                FString::Printf(TEXT("Movement State Changed: %s"), *StateName));
        }
    }
}

EAnim_MovementState UAnim_CharacterAnimInstance::CalculateMovementState() const
{
    if (!OwnerCharacter || !CharacterMovement)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if in air
    if (MovementData.bIsInAir)
    {
        if (CharacterMovement->Velocity.Z > 100.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check if just landed
    if (bJustLanded && StateChangeTimer < 0.5f)
    {
        return EAnim_MovementState::Landing;
    }
    
    // Check if crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check movement speed
    if (MovementData.GroundSpeed < 10.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MovementData.GroundSpeed < WalkSpeed + 50.0f)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_CharacterAnimInstance::UpdateBlendValues()
{
    // Calculate idle to walk blend
    float IdleToWalkThreshold = 50.0f;
    IdleToWalkBlend = FMath::Clamp(MovementData.GroundSpeed / IdleToWalkThreshold, 0.0f, 1.0f);
    
    // Calculate walk to run blend
    float WalkToRunRange = RunSpeed - WalkSpeed;
    if (WalkToRunRange > 0.0f)
    {
        float ExcessSpeed = FMath::Max(0.0f, MovementData.GroundSpeed - WalkSpeed);
        WalkToRunBlend = FMath::Clamp(ExcessSpeed / WalkToRunRange, 0.0f, 1.0f);
    }
    else
    {
        WalkToRunBlend = 0.0f;
    }
    
    // Calculate directional blend
    DirectionalBlend = CalculateDirectionalBlend();
}

float UAnim_CharacterAnimInstance::CalculateDirectionalBlend() const
{
    // Convert direction angle to blend value for animation
    // -180 to 180 degrees mapped to -1 to 1
    return FMath::Clamp(MovementData.Direction / 180.0f, -1.0f, 1.0f);
}

float UAnim_CharacterAnimInstance::CalculateSpeedBlend() const
{
    // Normalize speed for blend spaces
    float MaxSpeed = FMath::Max(RunSpeed, 1.0f);
    return FMath::Clamp(MovementData.GroundSpeed / MaxSpeed, 0.0f, 1.0f);
}

void UAnim_CharacterAnimInstance::UpdateTransitionFlags()
{
    // Update jump transition
    bool bCurrentlyInAir = MovementData.bIsInAir;
    if (!bWasInAir && bCurrentlyInAir)
    {
        bShouldEnterJump = true;
        bShouldEnterFalling = false;
        bJustLanded = false;
    }
    else if (bWasInAir && !bCurrentlyInAir)
    {
        bShouldEnterJump = false;
        bShouldEnterFalling = false;
        bShouldEnterLanding = true;
        bJustLanded = true;
        StateChangeTimer = 0.0f;
    }
    else if (bCurrentlyInAir && CharacterMovement->Velocity.Z < -100.0f)
    {
        bShouldEnterJump = false;
        bShouldEnterFalling = true;
    }
    
    bWasInAir = bCurrentlyInAir;
    
    // Reset landing flag after animation time
    if (bJustLanded && StateChangeTimer > 0.5f)
    {
        bJustLanded = false;
        bShouldEnterLanding = false;
    }
}

void UAnim_CharacterAnimInstance::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        CurrentMovementState = NewState;
        StateChangeTimer = 0.0f;
    }
}

void UAnim_CharacterAnimInstance::SetCombatState(EAnim_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        StateChangeTimer = 0.0f;
    }
}

void UAnim_CharacterAnimInstance::TriggerJumpAnimation()
{
    bShouldEnterJump = true;
    SetMovementState(EAnim_MovementState::Jumping);
}

void UAnim_CharacterAnimInstance::TriggerAttackAnimation()
{
    // This would trigger attack montage in Animation Blueprint
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Attack Animation Triggered"));
    }
}

void UAnim_CharacterAnimInstance::TriggerBlockAnimation()
{
    SetCombatState(EAnim_CombatState::Blocking);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Block Animation Triggered"));
    }
}

void UAnim_CharacterAnimInstance::OnJumpAnimationFinished()
{
    bShouldEnterJump = false;
    if (MovementData.bIsInAir)
    {
        SetMovementState(EAnim_MovementState::Falling);
    }
}

void UAnim_CharacterAnimInstance::OnAttackAnimationFinished()
{
    // Reset to unarmed state after attack
    SetCombatState(EAnim_CombatState::Unarmed);
}

void UAnim_CharacterAnimInstance::OnLandingAnimationFinished()
{
    bShouldEnterLanding = false;
    bJustLanded = false;
    
    // Transition to appropriate movement state
    SetMovementState(CalculateMovementState());
}