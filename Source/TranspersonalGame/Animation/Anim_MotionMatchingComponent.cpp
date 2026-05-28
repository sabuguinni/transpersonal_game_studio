#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame for smooth animation
    
    // Initialize default values
    WalkThreshold = 50.0f;
    RunThreshold = 300.0f;
    BlendTime = 0.2f;
    DirectionSmoothingSpeed = 10.0f;
    
    CurrentMotionData.MovementState = EAnim_MovementState::Idle;
    CurrentMotionData.EmotionalState = EAnim_EmotionalState::Calm;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponents();
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (MovementComponent && AnimInstance)
    {
        UpdateMotionData();
        SmoothDirectionChange(DeltaTime);
        UpdateBlendSpaceParameters();
    }
}

void UAnim_MotionMatchingComponent::InitializeComponents()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        MovementComponent = Character->GetCharacterMovement();
        
        if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
        {
            AnimInstance = MeshComp->GetAnimInstance();
        }
    }
    
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: No CharacterMovementComponent found on %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    }
    
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingComponent: No AnimInstance found on %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UAnim_MotionMatchingComponent::UpdateMotionData()
{
    if (!MovementComponent) return;
    
    // Update basic motion parameters
    CurrentMotionData.Velocity = MovementComponent->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    CurrentMotionData.Acceleration = MovementComponent->GetCurrentAcceleration().Size();
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();
    CurrentMotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate direction relative to actor forward
    if (CurrentMotionData.Speed > 1.0f)
    {
        FVector ForwardVector = GetOwner()->GetActorForwardVector();
        FVector VelocityDirection = CurrentMotionData.Velocity.GetSafeNormal();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }
    
    // Update movement state
    CalculateMovementState();
}

void UAnim_MotionMatchingComponent::CalculateMovementState()
{
    if (CurrentMotionData.bIsInAir)
    {
        if (CurrentMotionData.Velocity.Z > 0.0f)
        {
            CurrentMotionData.MovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            CurrentMotionData.MovementState = EAnim_MovementState::Falling;
        }
    }
    else if (CurrentMotionData.bIsCrouching)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Crouching;
    }
    else if (CurrentMotionData.Speed < WalkThreshold)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Idle;
    }
    else if (CurrentMotionData.Speed < RunThreshold)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Running;
    }
}

void UAnim_MotionMatchingComponent::SmoothDirectionChange(float DeltaTime)
{
    if (CurrentMotionData.Speed > 1.0f)
    {
        SmoothedDirection = FMath::FInterpTo(SmoothedDirection, CurrentMotionData.Direction, DeltaTime, DirectionSmoothingSpeed);
    }
    else
    {
        SmoothedDirection = 0.0f;
    }
}

void UAnim_MotionMatchingComponent::UpdateBlendSpaceParameters()
{
    if (!AnimInstance || !MovementBlendSpace) return;
    
    // Set blend space parameters for locomotion
    // X-axis: Speed (0-600)
    // Y-axis: Direction (-180 to 180)
    float NormalizedSpeed = FMath::Clamp(CurrentMotionData.Speed / 600.0f, 0.0f, 1.0f);
    float NormalizedDirection = SmoothedDirection / 180.0f;
    
    // Apply emotional state modifiers
    switch (CurrentMotionData.EmotionalState)
    {
        case EAnim_EmotionalState::Fearful:
            NormalizedSpeed *= 1.2f; // Faster, more erratic movement
            break;
        case EAnim_EmotionalState::Exhausted:
            NormalizedSpeed *= 0.7f; // Slower movement
            break;
        case EAnim_EmotionalState::Injured:
            NormalizedSpeed *= 0.5f; // Much slower movement
            break;
        default:
            break;
    }
    
    // Update animation variables (these would be read by the Animation Blueprint)
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("Speed")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("Speed"))->SetFloatPropertyValue(AnimInstance, CurrentMotionData.Speed);
    }
    
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("Direction")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("Direction"))->SetFloatPropertyValue(AnimInstance, SmoothedDirection);
    }
    
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("bIsInAir")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("bIsInAir"))->SetBoolPropertyValue(AnimInstance, CurrentMotionData.bIsInAir);
    }
}

void UAnim_MotionMatchingComponent::SetEmotionalState(EAnim_EmotionalState NewState)
{
    if (CurrentMotionData.EmotionalState != NewState)
    {
        CurrentMotionData.EmotionalState = NewState;
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("MotionMatching: Emotional state changed to %d"), (int32)NewState);
        
        // Trigger any immediate animation changes based on emotional state
        switch (NewState)
        {
            case EAnim_EmotionalState::Alert:
                // Could trigger alert posture animation
                break;
            case EAnim_EmotionalState::Fearful:
                // Could trigger defensive posture
                break;
            case EAnim_EmotionalState::Aggressive:
                // Could trigger aggressive stance
                break;
            default:
                break;
        }
    }
}

void UAnim_MotionMatchingComponent::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

void UAnim_MotionMatchingComponent::TriggerJump()
{
    if (JumpMontage && !CurrentMotionData.bIsInAir)
    {
        PlayMontage(JumpMontage, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("MotionMatching: Jump animation triggered"));
    }
}

void UAnim_MotionMatchingComponent::TriggerCombatAction()
{
    if (CombatMontage)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Combat;
        PlayMontage(CombatMontage, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("MotionMatching: Combat animation triggered"));
    }
}

void UAnim_MotionMatchingComponent::TriggerGatheringAction()
{
    if (GatheringMontage)
    {
        CurrentMotionData.MovementState = EAnim_MovementState::Gathering;
        PlayMontage(GatheringMontage, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("MotionMatching: Gathering animation triggered"));
    }
}