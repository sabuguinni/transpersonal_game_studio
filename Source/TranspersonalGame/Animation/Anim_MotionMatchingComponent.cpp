#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize motion data
    CurrentMotionData = FAnim_MotionData();
    PreviousMotionData = FAnim_MotionData();

    // Initialize thresholds
    WalkThreshold = 50.0f;
    RunThreshold = 300.0f;
    MotionSmoothingSpeed = 10.0f;

    // Initialize animation assets to null
    IdleAnimation = nullptr;
    MovementBlendSpace = nullptr;
    JumpStartAnimation = nullptr;
    JumpLoopAnimation = nullptr;
    JumpEndAnimation = nullptr;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Motion Matching Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Auto-update motion data from character movement
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector Velocity = MovementComp->Velocity;
            float Speed = Velocity.Size2D();
            float Direction = 0.0f;
            
            // Calculate direction relative to character forward
            if (Speed > 0.1f)
            {
                FVector ForwardVector = Character->GetActorForwardVector();
                FVector VelocityNormalized = Velocity.GetSafeNormal2D();
                Direction = FMath::Atan2(
                    FVector::CrossProduct(ForwardVector, VelocityNormalized).Z,
                    FVector::DotProduct(ForwardVector, VelocityNormalized)
                ) * 180.0f / PI;
            }

            bool bInAir = MovementComp->IsFalling();
            bool bCrouching = MovementComp->IsCrouching();

            UpdateMotionData(Speed, Direction, bInAir, bCrouching);
        }
    }

    // Smooth motion data
    SmoothMotionData(DeltaTime);
    
    // Calculate movement state
    CalculateMovementState();
}

void UAnim_MotionMatchingComponent::UpdateMotionData(float Speed, float Direction, bool bInAir, bool bCrouching)
{
    PreviousMotionData = CurrentMotionData;
    
    CurrentMotionData.Speed = Speed;
    CurrentMotionData.Direction = Direction;
    CurrentMotionData.bIsInAir = bInAir;
    CurrentMotionData.bIsCrouching = bCrouching;
}

EAnim_MovementState UAnim_MotionMatchingComponent::GetCurrentMovementState() const
{
    return CurrentMotionData.MovementState;
}

FAnim_MotionData UAnim_MotionMatchingComponent::GetMotionData() const
{
    return CurrentMotionData;
}

void UAnim_MotionMatchingComponent::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMotionData.MovementState != NewState)
    {
        UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), 
               (int32)CurrentMotionData.MovementState, (int32)NewState);
        CurrentMotionData.MovementState = NewState;
    }
}

void UAnim_MotionMatchingComponent::CalculateMovementState()
{
    EAnim_MovementState NewState = EAnim_MovementState::Idle;

    if (CurrentMotionData.bIsInAir)
    {
        NewState = EAnim_MovementState::Falling;
    }
    else if (CurrentMotionData.bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (CurrentMotionData.Speed > RunThreshold)
    {
        NewState = EAnim_MovementState::Running;
    }
    else if (CurrentMotionData.Speed > WalkThreshold)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else
    {
        NewState = EAnim_MovementState::Idle;
    }

    SetMovementState(NewState);
}

void UAnim_MotionMatchingComponent::SmoothMotionData(float DeltaTime)
{
    float SmoothingFactor = FMath::Clamp(MotionSmoothingSpeed * DeltaTime, 0.0f, 1.0f);
    
    // Smooth speed changes
    CurrentMotionData.Speed = FMath::FInterpTo(
        PreviousMotionData.Speed, 
        CurrentMotionData.Speed, 
        DeltaTime, 
        MotionSmoothingSpeed
    );
    
    // Smooth direction changes
    CurrentMotionData.Direction = FMath::FInterpAngle(
        PreviousMotionData.Direction,
        CurrentMotionData.Direction,
        DeltaTime,
        MotionSmoothingSpeed
    );
}