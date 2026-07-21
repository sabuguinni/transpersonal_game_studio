#include "Anim_CharacterAnimationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

UAnim_CharacterAnimationSystem::UAnim_CharacterAnimationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default values
    CurrentMovementData.Speed = 0.0f;
    CurrentMovementData.Direction = 0.0f;
    CurrentMovementData.bIsMoving = false;
    CurrentMovementData.bIsInAir = false;
    CurrentMovementData.bIsCrouching = false;
    CurrentMovementData.MovementState = EAnim_MovementState::Idle;

    CurrentActionData.CurrentAction = EAnim_ActionState::None;
    CurrentActionData.ActionProgress = 0.0f;
    CurrentActionData.bIsPerformingAction = false;
    CurrentActionData.ActionTargetLocation = FVector::ZeroVector;

    PreviousMovementState = EAnim_MovementState::Idle;
    PreviousActionState = EAnim_ActionState::None;
}

void UAnim_CharacterAnimationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    UpdateAnimationBlueprint();
}

void UAnim_CharacterAnimationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!SkeletalMeshComponent || !MovementComponent)
    {
        return;
    }

    // Update movement data
    FAnim_MovementData NewMovementData;
    NewMovementData.Speed = MovementComponent->Velocity.Size();
    NewMovementData.Direction = FMath::Atan2(MovementComponent->Velocity.Y, MovementComponent->Velocity.X);
    NewMovementData.bIsMoving = NewMovementData.Speed > 5.0f;
    NewMovementData.bIsInAir = MovementComponent->IsFalling();
    NewMovementData.bIsCrouching = MovementComponent->IsCrouching();
    NewMovementData.MovementState = DetermineMovementState();

    // Check for state changes
    if (NewMovementData.MovementState != CurrentMovementData.MovementState)
    {
        HandleStateTransition(NewMovementData.MovementState);
    }

    CurrentMovementData = NewMovementData;

    // Update action progress
    UpdateActionProgress(DeltaTime);

    // Update foot IK if enabled
    if (bUseFootIK)
    {
        UpdateFootIKTargets();
    }
}

void UAnim_CharacterAnimationSystem::UpdateMovementState(const FAnim_MovementData& MovementData)
{
    if (CurrentMovementData.MovementState != MovementData.MovementState)
    {
        HandleStateTransition(MovementData.MovementState);
    }
    
    CurrentMovementData = MovementData;
}

void UAnim_CharacterAnimationSystem::SetActionState(EAnim_ActionState NewActionState, float Duration)
{
    if (CurrentActionData.CurrentAction != NewActionState)
    {
        PreviousActionState = CurrentActionData.CurrentAction;
        CurrentActionData.CurrentAction = NewActionState;
        CurrentActionData.bIsPerformingAction = (NewActionState != EAnim_ActionState::None);
        CurrentActionData.ActionProgress = 0.0f;
        ActionTimer = 0.0f;
        ActionDuration = Duration;

        OnActionStarted(NewActionState);

        // Play appropriate montage based on action
        UAnimMontage* MontageToPlay = nullptr;
        switch (NewActionState)
        {
            case EAnim_ActionState::Attacking:
                MontageToPlay = MontageConfiguration.AttackMontage.LoadSynchronous();
                break;
            case EAnim_ActionState::Gathering:
                MontageToPlay = MontageConfiguration.GatherMontage.LoadSynchronous();
                break;
            default:
                break;
        }

        if (MontageToPlay)
        {
            PlayMontage(MontageToPlay);
        }
    }
}

void UAnim_CharacterAnimationSystem::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }

    AnimInstance->Montage_Play(Montage, PlayRate);
}

void UAnim_CharacterAnimationSystem::StopMontage(float BlendOutTime)
{
    if (!AnimInstance)
    {
        return;
    }

    AnimInstance->Montage_Stop(BlendOutTime);
}

void UAnim_CharacterAnimationSystem::EnableMotionMatching(bool bEnable)
{
    bUseMotionMatching = bEnable;
    
    if (MotionMatchingComponent)
    {
        MotionMatchingComponent->SetComponentTickEnabled(bEnable);
    }
}

void UAnim_CharacterAnimationSystem::SetMotionMatchingDatabase(UMotionMatchingComponent* Database)
{
    MotionMatchingComponent = Database;
}

void UAnim_CharacterAnimationSystem::EnableFootIK(bool bEnable)
{
    bUseFootIK = bEnable;
}

void UAnim_CharacterAnimationSystem::UpdateFootIKTargets()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }

    // Update left foot IK
    FVector LeftFootIKTarget;
    FRotator LeftFootIKRotation;
    PerformFootIKTrace(LeftFootBoneName, LeftFootIKTarget, LeftFootIKRotation);

    // Update right foot IK
    FVector RightFootIKTarget;
    FRotator RightFootIKRotation;
    PerformFootIKTrace(RightFootBoneName, RightFootIKTarget, RightFootIKRotation);

    // Apply IK targets to animation instance
    if (AnimInstance)
    {
        // Set IK targets via animation blueprint variables
        // This would typically be done through animation blueprint communication
    }
}

bool UAnim_CharacterAnimationSystem::IsPlayingMontage() const
{
    if (!AnimInstance)
    {
        return false;
    }

    return AnimInstance->IsAnyMontagePlaying();
}

void UAnim_CharacterAnimationSystem::InitializeComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Get skeletal mesh component
    SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMeshComponent)
    {
        AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    }

    // Get movement component
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        MovementComponent = Character->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimationSystem::UpdateAnimationBlueprint()
{
    if (!SkeletalMeshComponent || !AnimInstance)
    {
        return;
    }

    // Update animation blueprint variables
    // This would typically involve setting variables on the animation blueprint
    // that control blend spaces and state machines
}

EAnim_MovementState UAnim_CharacterAnimationSystem::DetermineMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }

    if (MovementComponent->IsFalling())
    {
        return EAnim_MovementState::Falling;
    }

    if (MovementComponent->IsCrouching())
    {
        return EAnim_MovementState::Crouching;
    }

    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }

    float CurrentSpeed = MovementComponent->Velocity.Size();
    
    if (CurrentSpeed < WalkSpeedThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (CurrentSpeed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_CharacterAnimationSystem::HandleStateTransition(EAnim_MovementState NewState)
{
    EAnim_MovementState OldState = CurrentMovementData.MovementState;
    
    // Handle specific transitions
    switch (NewState)
    {
        case EAnim_MovementState::Jumping:
            if (MontageConfiguration.JumpMontage.LoadSynchronous())
            {
                PlayMontage(MontageConfiguration.JumpMontage.LoadSynchronous());
            }
            break;
        
        case EAnim_MovementState::Walking:
            if (MontageConfiguration.WalkMontage.LoadSynchronous())
            {
                PlayMontage(MontageConfiguration.WalkMontage.LoadSynchronous());
            }
            break;
        
        case EAnim_MovementState::Running:
            if (MontageConfiguration.RunMontage.LoadSynchronous())
            {
                PlayMontage(MontageConfiguration.RunMontage.LoadSynchronous());
            }
            break;
        
        case EAnim_MovementState::Idle:
            if (MontageConfiguration.IdleMontage.LoadSynchronous())
            {
                PlayMontage(MontageConfiguration.IdleMontage.LoadSynchronous());
            }
            break;
        
        default:
            break;
    }

    PreviousMovementState = OldState;
    OnAnimationStateChanged(OldState, NewState);
}

void UAnim_CharacterAnimationSystem::UpdateActionProgress(float DeltaTime)
{
    if (!CurrentActionData.bIsPerformingAction || ActionDuration <= 0.0f)
    {
        return;
    }

    ActionTimer += DeltaTime;
    CurrentActionData.ActionProgress = FMath::Clamp(ActionTimer / ActionDuration, 0.0f, 1.0f);

    // Check if action is complete
    if (CurrentActionData.ActionProgress >= 1.0f)
    {
        EAnim_ActionState CompletedAction = CurrentActionData.CurrentAction;
        CurrentActionData.CurrentAction = EAnim_ActionState::None;
        CurrentActionData.bIsPerformingAction = false;
        CurrentActionData.ActionProgress = 0.0f;
        ActionTimer = 0.0f;

        OnActionCompleted(CompletedAction);
    }
}

void UAnim_CharacterAnimationSystem::PerformFootIKTrace(const FName& BoneName, FVector& OutIKTarget, FRotator& OutIKRotation)
{
    if (!SkeletalMeshComponent)
    {
        OutIKTarget = FVector::ZeroVector;
        OutIKRotation = FRotator::ZeroRotator;
        return;
    }

    // Get bone location in world space
    FVector BoneLocation = SkeletalMeshComponent->GetBoneLocation(BoneName);
    FVector TraceStart = BoneLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = BoneLocation - FVector(0, 0, FootIKTraceDistance);

    // Perform line trace
    FHitResult HitResult;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());

    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        GetWorld(),
        TraceStart,
        TraceEnd,
        UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        HitResult,
        true
    );

    if (bHit)
    {
        OutIKTarget = HitResult.Location;
        OutIKRotation = FRotationMatrix::MakeFromZ(HitResult.Normal).Rotator();
    }
    else
    {
        OutIKTarget = TraceEnd;
        OutIKRotation = FRotator::ZeroRotator;
    }
}