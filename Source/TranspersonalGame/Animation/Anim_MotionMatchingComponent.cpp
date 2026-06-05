#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize motion matching settings
    MotionMatchingSettings = FAnim_MotionMatchingSettings();
    
    // Initialize state
    CurrentMovementState = EAnim_MovementState::Idle;
    PreviousMovementState = EAnim_MovementState::Idle;
    LastUpdateTime = 0.0f;
    PreviousVelocity = FVector::ZeroVector;
    PreviousLocation = FVector::ZeroVector;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache character references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        PreviousLocation = OwnerCharacter->GetActorLocation();
    }
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateMotionData();
}

void UAnim_MotionMatchingComponent::UpdateMotionData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DeltaTime = CurrentTime - LastUpdateTime;
    
    if (DeltaTime <= 0.0f)
    {
        return;
    }
    
    // Update velocity and acceleration
    UpdateVelocityAndAcceleration(DeltaTime);
    
    // Update movement state
    UpdateMovementState();
    
    // Update ground distance
    UpdateGroundDistance();
    
    // Update other motion data
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > 1.0f;
    CurrentMotionData.bIsInAir = MovementComponent->IsFalling();
    CurrentMotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate direction relative to actor forward
    if (CurrentMotionData.bIsMoving)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentMotionData.Velocity.GetSafeNormal();
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
        
        // Check if moving backwards
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityDirection);
        if (RightDot < 0.0f)
        {
            CurrentMotionData.Direction = -CurrentMotionData.Direction;
        }
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }
    
    // Check for state changes
    EAnim_MovementState NewState = DetermineMovementState();
    if (NewState != CurrentMovementState)
    {
        PreviousMovementState = CurrentMovementState;
        CurrentMovementState = NewState;
        OnMotionStateChanged(CurrentMovementState, PreviousMovementState);
        
        // Handle specific state transitions
        if (CurrentMovementState == EAnim_MovementState::Jumping)
        {
            OnJumpStarted();
            if (JumpMontage)
            {
                PlayAnimationMontage(JumpMontage);
            }
        }
        else if (PreviousMovementState == EAnim_MovementState::Falling && CurrentMovementState != EAnim_MovementState::Falling)
        {
            OnLanded();
            if (LandMontage)
            {
                PlayAnimationMontage(LandMontage);
            }
        }
    }
    
    // Update blend space if available
    if (LocomotionBlendSpace && MeshComponent && MeshComponent->GetAnimInstance())
    {
        SetLocomotionBlendSpaceValues(CurrentMotionData.Speed, CurrentMotionData.Direction);
    }
    
    LastUpdateTime = CurrentTime;
}

void UAnim_MotionMatchingComponent::UpdateVelocityAndAcceleration(float DeltaTime)
{
    FVector CurrentLocation = OwnerCharacter->GetActorLocation();
    FVector CurrentVelocity = MovementComponent->Velocity;
    
    // Calculate acceleration
    CurrentMotionData.Acceleration = (CurrentVelocity - PreviousVelocity) / DeltaTime;
    CurrentMotionData.Velocity = CurrentVelocity;
    
    // Store previous values
    PreviousVelocity = CurrentVelocity;
    PreviousLocation = CurrentLocation;
}

void UAnim_MotionMatchingComponent::UpdateGroundDistance()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 1000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        CurrentMotionData.GroundDistance = HitResult.Distance;
    }
    else
    {
        CurrentMotionData.GroundDistance = 1000.0f; // Max distance if no ground found
    }
}

EAnim_MovementState UAnim_MotionMatchingComponent::DetermineMovementState()
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    if (MovementComponent->IsFalling())
    {
        if (CurrentMotionData.Velocity.Z > 50.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    if (MovementComponent->IsCrouching())
    {
        if (CurrentMotionData.bIsMoving)
        {
            return EAnim_MovementState::CrouchWalking;
        }
        else
        {
            return EAnim_MovementState::Crouching;
        }
    }
    
    if (CurrentMotionData.bIsMoving)
    {
        if (CurrentMotionData.Speed > 400.0f)
        {
            return EAnim_MovementState::Running;
        }
        else
        {
            return EAnim_MovementState::Walking;
        }
    }
    
    return EAnim_MovementState::Idle;
}

void UAnim_MotionMatchingComponent::UpdateMovementState()
{
    // This function is called from UpdateMotionData
    // State logic is handled in DetermineMovementState
}

float UAnim_MotionMatchingComponent::CalculateMotionScore(const FAnim_MotionMatchingData& TargetData, const FAnim_MotionMatchingData& CandidateData)
{
    float Score = 0.0f;
    
    // Velocity matching
    float VelocityDiff = FVector::Dist(TargetData.Velocity, CandidateData.Velocity);
    Score += VelocityDiff * MotionMatchingSettings.VelocityWeight;
    
    // Acceleration matching
    float AccelerationDiff = FVector::Dist(TargetData.Acceleration, CandidateData.Acceleration);
    Score += AccelerationDiff * MotionMatchingSettings.AccelerationWeight;
    
    // Direction matching
    float DirectionDiff = FMath::Abs(TargetData.Direction - CandidateData.Direction);
    Score += DirectionDiff * MotionMatchingSettings.DirectionWeight;
    
    return Score;
}

void UAnim_MotionMatchingComponent::PlayAnimationMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage || !MeshComponent)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

void UAnim_MotionMatchingComponent::SetLocomotionBlendSpaceValues(float Speed, float Direction)
{
    if (!MeshComponent)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
    if (AnimInstance)
    {
        // Set blend space parameters
        // Note: This would typically be done through Animation Blueprint variables
        // For now, we'll log the values that would be set
        UE_LOG(LogTemp, Log, TEXT("Setting Locomotion BlendSpace - Speed: %f, Direction: %f"), Speed, Direction);
    }
}