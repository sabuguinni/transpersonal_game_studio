#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentMotionData.MovementState = EAnim_MovementState::Idle;
    CurrentMotionData.CombatState = EAnim_CombatState::None;
    
    // Set default thresholds for realistic movement
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    SprintSpeedThreshold = 600.0f;
    IdleTimeThreshold = 2.0f;
    DefaultTransitionTime = 0.25f;
    CombatTransitionTime = 0.15f;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Component initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Motion Matching Component attached to non-character actor"));
    }
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateMotionData(DeltaTime);
    }
}

void UAnim_MotionMatchingComponent::UpdateMotionData(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Store previous data
    PreviousMotionData = CurrentMotionData;
    
    // Update velocity and movement data
    UpdateVelocityData();
    
    // Analyze movement state
    AnalyzeMovementState(DeltaTime);
    
    // Update timing
    TimeSinceStateChange += DeltaTime;
    if (CurrentMotionData.MovementState != LastMovementState)
    {
        TimeSinceStateChange = 0.0f;
        LastMovementState = CurrentMotionData.MovementState;
    }
    
    if (CurrentMotionData.CombatState != LastCombatState)
    {
        LastCombatState = CurrentMotionData.CombatState;
    }
}

void UAnim_MotionMatchingComponent::UpdateVelocityData()
{
    CurrentMotionData.Velocity = MovementComponent->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    CurrentMotionData.Acceleration = MovementComponent->GetCurrentAcceleration();
    CurrentMotionData.bIsInAir = IsCharacterInAir();
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > 1.0f;
    CurrentMotionData.Direction = CalculateMovementDirection();
    
    // Update time since last movement
    if (CurrentMotionData.bIsMoving)
    {
        CurrentMotionData.TimeSinceLastMovement = 0.0f;
    }
    else
    {
        CurrentMotionData.TimeSinceLastMovement += GetWorld()->GetDeltaSeconds();
    }
}

void UAnim_MotionMatchingComponent::AnalyzeMovementState(float DeltaTime)
{
    EAnim_MovementState NewState = CurrentMotionData.MovementState;
    
    // Check if character is in air first
    if (CurrentMotionData.bIsInAir)
    {
        if (CurrentMotionData.Velocity.Z > 50.0f)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else if (CurrentMotionData.Velocity.Z < -50.0f)
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else
    {
        // Ground movement analysis
        if (CurrentMotionData.Speed < 1.0f)
        {
            // Check if we should be idle
            if (CurrentMotionData.TimeSinceLastMovement > IdleTimeThreshold)
            {
                NewState = EAnim_MovementState::Idle;
            }
        }
        else
        {
            // Moving - determine speed category
            if (CurrentMotionData.Speed < WalkSpeedThreshold)
            {
                NewState = EAnim_MovementState::Walking;
            }
            else if (CurrentMotionData.Speed < RunSpeedThreshold)
            {
                NewState = EAnim_MovementState::Running;
            }
            else if (CurrentMotionData.Speed < SprintSpeedThreshold)
            {
                NewState = EAnim_MovementState::Sprinting;
            }
        }
        
        // Check for crouching
        if (MovementComponent->IsCrouching())
        {
            if (CurrentMotionData.Speed < 1.0f)
            {
                NewState = EAnim_MovementState::Crouching;
            }
            else
            {
                NewState = EAnim_MovementState::Crawling;
            }
        }
        
        // Landing detection
        if (PreviousMotionData.bIsInAir && !CurrentMotionData.bIsInAir)
        {
            NewState = EAnim_MovementState::Landing;
        }
    }
    
    // Apply state change if valid
    if (CanTransitionTo(NewState))
    {
        CurrentMotionData.MovementState = NewState;
    }
}

bool UAnim_MotionMatchingComponent::IsCharacterInAir() const
{
    return MovementComponent && MovementComponent->IsFalling();
}

float UAnim_MotionMatchingComponent::CalculateMovementDirection() const
{
    if (!OwnerCharacter || CurrentMotionData.Speed < 1.0f)
    {
        return 0.0f;
    }
    
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityDirection = CurrentMotionData.Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
    
    return FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
}

UAnimSequence* UAnim_MotionMatchingComponent::SelectBestAnimation()
{
    // First check combat animations
    if (CurrentMotionData.CombatState != EAnim_CombatState::None)
    {
        if (UAnimSequence** CombatAnim = CombatAnimations.Find(CurrentMotionData.CombatState))
        {
            return *CombatAnim;
        }
    }
    
    // Then check movement animations
    if (UAnimSequence** MovementAnim = MovementAnimations.Find(CurrentMotionData.MovementState))
    {
        return *MovementAnim;
    }
    
    // Default fallback
    return nullptr;
}

void UAnim_MotionMatchingComponent::SetCombatState(EAnim_CombatState NewState)
{
    if (CurrentMotionData.CombatState != NewState)
    {
        CurrentMotionData.CombatState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Combat state changed to: %d"), (int32)NewState);
    }
}

bool UAnim_MotionMatchingComponent::CanTransitionTo(EAnim_MovementState NewState) const
{
    // Always allow transitions from landing state
    if (CurrentMotionData.MovementState == EAnim_MovementState::Landing)
    {
        return true;
    }
    
    // Prevent rapid state oscillation
    if (TimeSinceStateChange < 0.1f && NewState != CurrentMotionData.MovementState)
    {
        return false;
    }
    
    // Allow air state transitions
    if (NewState == EAnim_MovementState::Jumping || NewState == EAnim_MovementState::Falling)
    {
        return true;
    }
    
    // Allow ground state transitions when not in air
    if (!CurrentMotionData.bIsInAir)
    {
        return true;
    }
    
    return false;
}

float UAnim_MotionMatchingComponent::GetTransitionBlendTime(EAnim_MovementState FromState, EAnim_MovementState ToState) const
{
    // Combat transitions are faster
    if (CurrentMotionData.CombatState != EAnim_CombatState::None)
    {
        return CombatTransitionTime;
    }
    
    // Landing transitions are immediate
    if (ToState == EAnim_MovementState::Landing || FromState == EAnim_MovementState::Landing)
    {
        return 0.1f;
    }
    
    // Air transitions are faster
    if (ToState == EAnim_MovementState::Jumping || ToState == EAnim_MovementState::Falling)
    {
        return 0.15f;
    }
    
    return DefaultTransitionTime;
}