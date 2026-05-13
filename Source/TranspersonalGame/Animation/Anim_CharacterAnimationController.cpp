#include "Anim_CharacterAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimationController::UAnim_CharacterAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    PreviousMovementState = EAnim_MovementState::Idle;
    CurrentEmotionalState = EAnim_EmotionalState::Calm;
    PreviousEmotionalState = EAnim_EmotionalState::Calm;

    bEnableFootIK = true;
    bEnableLookAtIK = false;
    FootIKTraceDistance = 50.0f;
    LeftFootBoneName = TEXT("foot_l");
    RightFootBoneName = TEXT("foot_r");

    DefaultBlendTime = 0.3f;
    MovementStateBlendTime = 0.2f;
    EmotionalStateBlendTime = 0.5f;

    LastMovementUpdateTime = 0.0f;
    StateChangeTimer = 0.0f;
    bIsBlending = false;
    LookAtTarget = nullptr;
}

void UAnim_CharacterAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeReferences();
}

void UAnim_CharacterAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || !MovementComponent || !MeshComponent)
    {
        return;
    }

    // Update movement data
    UpdateMovementData();

    // Update animation state
    UpdateAnimationState();

    // Update IK systems
    if (bEnableFootIK)
    {
        UpdateFootIK();
    }

    if (bEnableLookAtIK && LookAtTarget)
    {
        UpdateLookAtIK();
    }

    // Update timers
    StateChangeTimer += DeltaTime;
    LastMovementUpdateTime += DeltaTime;
}

void UAnim_CharacterAnimationController::InitializeReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        
        if (MeshComponent)
        {
            AnimInstance = MeshComponent->GetAnimInstance();
        }
    }
}

void UAnim_CharacterAnimationController::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }

    // Get velocity and calculate speed
    MovementData.Velocity = MovementComponent->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (MovementData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = MovementData.Velocity.GetSafeNormal();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }

    // Update movement flags
    MovementData.bIsMoving = MovementData.Speed > 5.0f;
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate input magnitude
    FVector InputVector = MovementComponent->GetCurrentAcceleration();
    MovementData.MovementInputMagnitude = FMath::Clamp(InputVector.Size() / MovementComponent->GetMaxAcceleration(), 0.0f, 1.0f);
}

void UAnim_CharacterAnimationController::UpdateAnimationState()
{
    EAnim_MovementState NewMovementState = CalculateMovementState();
    
    if (NewMovementState != CurrentMovementState)
    {
        SetMovementState(NewMovementState);
    }
}

EAnim_MovementState UAnim_CharacterAnimationController::CalculateMovementState()
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }

    // Check for air states first
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }

    // Check for crouching
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }

    // Check for swimming
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }

    // Ground movement states
    if (MovementData.bIsMoving)
    {
        // Determine if running or walking based on speed
        float WalkThreshold = MovementComponent->MaxWalkSpeed * 0.6f;
        if (MovementData.Speed > WalkThreshold)
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

void UAnim_CharacterAnimationController::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMovementState == NewState)
    {
        return;
    }

    PreviousMovementState = CurrentMovementState;
    CurrentMovementState = NewState;
    StateChangeTimer = 0.0f;

    // Trigger Blueprint event
    OnMovementStateChanged(PreviousMovementState, CurrentMovementState);

    // Handle specific state transitions
    switch (CurrentMovementState)
    {
        case EAnim_MovementState::Jumping:
            if (AnimationSet.JumpMontage)
            {
                PlayMontage(AnimationSet.JumpMontage);
            }
            break;
            
        case EAnim_MovementState::Falling:
            // Stop jump montage if playing
            if (AnimationSet.JumpMontage && IsPlayingMontage())
            {
                StopMontage(AnimationSet.JumpMontage);
            }
            break;
            
        case EAnim_MovementState::Crouching:
            if (AnimationSet.CrouchMontage)
            {
                PlayMontage(AnimationSet.CrouchMontage);
            }
            break;
    }
}

void UAnim_CharacterAnimationController::SetEmotionalState(EAnim_EmotionalState NewState)
{
    if (CurrentEmotionalState == NewState)
    {
        return;
    }

    PreviousEmotionalState = CurrentEmotionalState;
    CurrentEmotionalState = NewState;

    // Trigger Blueprint event
    OnEmotionalStateChanged(PreviousEmotionalState, CurrentEmotionalState);

    // Play emotional animation if available
    if (AnimationSet.EmotionalMontages.IsValidIndex((int32)NewState))
    {
        UAnimMontage* EmotionalMontage = AnimationSet.EmotionalMontages[(int32)NewState];
        if (EmotionalMontage)
        {
            PlayMontage(EmotionalMontage, 1.0f);
        }
    }
}

void UAnim_CharacterAnimationController::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }

    AnimInstance->Montage_Play(Montage, PlayRate);
    OnMontageStarted(Montage);
}

void UAnim_CharacterAnimationController::StopMontage(UAnimMontage* Montage)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }

    AnimInstance->Montage_Stop(DefaultBlendTime, Montage);
    OnMontageEnded(Montage);
}

void UAnim_CharacterAnimationController::BlendToMovementState(EAnim_MovementState TargetState, float BlendTime)
{
    if (TargetState == CurrentMovementState)
    {
        return;
    }

    bIsBlending = true;
    
    // Set the target state
    SetMovementState(TargetState);
    
    // Custom blending logic can be implemented here
    // For now, we rely on the animation blueprint to handle blending
}

void UAnim_CharacterAnimationController::BlendToEmotionalState(EAnim_EmotionalState TargetState, float BlendTime)
{
    if (TargetState == CurrentEmotionalState)
    {
        return;
    }

    // Set the target state with custom blend time
    SetEmotionalState(TargetState);
}

void UAnim_CharacterAnimationController::EnableFootIK(bool bEnable)
{
    bEnableFootIK = bEnable;
}

void UAnim_CharacterAnimationController::EnableLookAtIK(bool bEnable, AActor* TargetActor)
{
    bEnableLookAtIK = bEnable;
    LookAtTarget = TargetActor;
}

void UAnim_CharacterAnimationController::UpdateFootIK()
{
    if (!MeshComponent || !OwnerCharacter)
    {
        return;
    }

    // Perform foot IK traces for both feet
    float LeftFootOffset = 0.0f;
    float RightFootOffset = 0.0f;

    PerformFootIKTrace(LeftFootBoneName, LeftFootOffset);
    PerformFootIKTrace(RightFootBoneName, RightFootOffset);

    // Apply IK offsets through animation blueprint
    // This would typically be done through animation blueprint variables
    // For now, we store the values for Blueprint access
}

void UAnim_CharacterAnimationController::UpdateLookAtIK()
{
    if (!LookAtTarget || !MeshComponent)
    {
        return;
    }

    FVector LookDirection = CalculateLookAtDirection();
    
    // Apply look-at rotation through animation blueprint
    // This would typically modify bone transforms in the animation blueprint
}

void UAnim_CharacterAnimationController::PerformFootIKTrace(const FName& BoneName, float& OutOffset)
{
    if (!MeshComponent || !OwnerCharacter)
    {
        OutOffset = 0.0f;
        return;
    }

    // Get bone location in world space
    FVector BoneLocation = MeshComponent->GetBoneLocation(BoneName);
    
    // Trace downward from bone location
    FVector TraceStart = BoneLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = BoneLocation - FVector(0, 0, FootIKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // Calculate offset needed to place foot on ground
        OutOffset = HitResult.Location.Z - BoneLocation.Z;
    }
    else
    {
        OutOffset = 0.0f;
    }
}

FVector UAnim_CharacterAnimationController::CalculateLookAtDirection()
{
    if (!LookAtTarget || !OwnerCharacter)
    {
        return FVector::ForwardVector;
    }

    FVector OwnerLocation = OwnerCharacter->GetActorLocation();
    FVector TargetLocation = LookAtTarget->GetActorLocation();
    
    return (TargetLocation - OwnerLocation).GetSafeNormal();
}

bool UAnim_CharacterAnimationController::IsPlayingMontage() const
{
    if (!AnimInstance)
    {
        return false;
    }

    return AnimInstance->IsAnyMontagePlaying();
}