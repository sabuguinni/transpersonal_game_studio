#include "Anim_MotionMatchingController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingController::UAnim_MotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize states
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentActionState = EAnim_ActionState::None;
    PreviousMovementState = EAnim_MovementState::Idle;

    // Initialize timers
    StateTransitionTimer = 0.0f;
    bIsTransitioning = false;
    ActionCooldownTimer = 0.0f;

    // Initialize motion data
    MotionData = FAnim_MotionData();
    LeftFootIK = FAnim_IKFootData();
    RightFootIK = FAnim_IKFootData();
}

void UAnim_MotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    CacheComponents();
}

void UAnim_MotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update core systems
    UpdateMotionData(DeltaTime);
    UpdateMovementState(DeltaTime);
    UpdateActionState(DeltaTime);
    UpdateFootIK(DeltaTime);

    // Update timers
    if (bIsTransitioning)
    {
        StateTransitionTimer -= DeltaTime;
        if (StateTransitionTimer <= 0.0f)
        {
            bIsTransitioning = false;
        }
    }

    if (ActionCooldownTimer > 0.0f)
    {
        ActionCooldownTimer -= DeltaTime;
    }
}

void UAnim_MotionMatchingController::CacheComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
    }
}

void UAnim_MotionMatchingController::UpdateMotionData(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }

    // Update basic motion properties
    MotionData.Velocity = MovementComponent->Velocity;
    MotionData.Speed = MotionData.Velocity.Size();
    MotionData.Acceleration = MovementComponent->GetCurrentAcceleration();
    MotionData.bIsInAir = MovementComponent->IsFalling();
    MotionData.bIsCrouching = MovementComponent->IsCrouching();

    // Calculate movement direction relative to character facing
    MotionData.Direction = CalculateMovementDirection();

    // Update ground distance for foot IK
    if (OwnerCharacter)
    {
        FVector Start = OwnerCharacter->GetActorLocation();
        FVector End = Start - FVector(0, 0, FootIKTraceDistance);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwnerCharacter);

        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
        {
            MotionData.GroundDistance = HitResult.Distance;
        }
        else
        {
            MotionData.GroundDistance = FootIKTraceDistance;
        }

        MotionData.BodyRotation = OwnerCharacter->GetActorRotation();
    }
}

void UAnim_MotionMatchingController::UpdateMovementState(float DeltaTime)
{
    EAnim_MovementState NewState = CurrentMovementState;

    // Determine new state based on motion data
    if (MotionData.bIsInAir)
    {
        if (MotionData.Velocity.Z > 0)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else if (MotionData.bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else if (MotionData.Speed < 10.0f)
    {
        NewState = EAnim_MovementState::Idle;
    }
    else if (MotionData.Speed < WalkSpeedThreshold)
    {
        NewState = EAnim_MovementState::Walking;
    }
    else if (MotionData.Speed < RunSpeedThreshold)
    {
        NewState = EAnim_MovementState::Running;
    }
    else if (MotionData.Speed >= SprintSpeedThreshold)
    {
        NewState = EAnim_MovementState::Sprinting;
    }
    else
    {
        NewState = EAnim_MovementState::Running;
    }

    // Handle landing state
    if (PreviousMovementState == EAnim_MovementState::Falling && !MotionData.bIsInAir)
    {
        NewState = EAnim_MovementState::Landing;
        StateTransitionTimer = 0.3f; // Landing animation duration
        bIsTransitioning = true;
    }

    // Apply state change if valid
    if (NewState != CurrentMovementState && CanTransitionToState(NewState))
    {
        SetMovementState(NewState);
    }
}

void UAnim_MotionMatchingController::UpdateActionState(float DeltaTime)
{
    // Handle action state cooldowns and transitions
    if (CurrentActionState != EAnim_ActionState::None && ActionCooldownTimer <= 0.0f)
    {
        SetActionState(EAnim_ActionState::None);
    }
}

void UAnim_MotionMatchingController::SetMovementState(EAnim_MovementState NewState)
{
    if (NewState == CurrentMovementState)
    {
        return;
    }

    PreviousMovementState = CurrentMovementState;
    CurrentMovementState = NewState;

    // Log state change for debugging
    UE_LOG(LogTemp, Log, TEXT("Movement State Changed: %d -> %d"), 
           (int32)PreviousMovementState, (int32)CurrentMovementState);
}

void UAnim_MotionMatchingController::SetActionState(EAnim_ActionState NewState)
{
    if (NewState == CurrentActionState)
    {
        return;
    }

    CurrentActionState = NewState;

    // Set cooldown for actions
    if (NewState != EAnim_ActionState::None)
    {
        ActionCooldownTimer = 2.0f; // Default action duration
    }

    UE_LOG(LogTemp, Log, TEXT("Action State Changed: %d"), (int32)CurrentActionState);
}

bool UAnim_MotionMatchingController::CanTransitionToState(EAnim_MovementState TargetState) const
{
    // Don't interrupt landing animation
    if (bIsTransitioning && CurrentMovementState == EAnim_MovementState::Landing)
    {
        return false;
    }

    // Don't interrupt certain action states
    if (CurrentActionState != EAnim_ActionState::None && ActionCooldownTimer > 0.0f)
    {
        // Only allow basic movement during actions
        return TargetState == EAnim_MovementState::Idle || 
               TargetState == EAnim_MovementState::Walking;
    }

    return IsValidTransition(CurrentMovementState, TargetState);
}

bool UAnim_MotionMatchingController::IsValidTransition(EAnim_MovementState From, EAnim_MovementState To) const
{
    // All transitions are valid by default for this primitive system
    // More complex state machines can add restrictions here
    return true;
}

void UAnim_MotionMatchingController::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !MeshComponent)
    {
        return;
    }

    // Only perform foot IK when grounded and not in certain states
    bool bShouldPerformIK = !MotionData.bIsInAir && 
                           CurrentMovementState != EAnim_MovementState::Jumping &&
                           CurrentMovementState != EAnim_MovementState::Falling;

    if (bShouldPerformIK)
    {
        PerformFootTrace(true, LeftFootIK);   // Left foot
        PerformFootTrace(false, RightFootIK); // Right foot
    }
    else
    {
        // Reset IK when not needed
        LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, 0.0f, DeltaTime, FootIKInterpSpeed);
        RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, 0.0f, DeltaTime, FootIKInterpSpeed);
    }
}

void UAnim_MotionMatchingController::PerformFootTrace(bool bIsLeftFoot, FAnim_IKFootData& FootData)
{
    if (!OwnerCharacter)
    {
        return;
    }

    // Get foot bone location (approximate)
    FVector FootBoneLocation = OwnerCharacter->GetActorLocation();
    FootBoneLocation.X += bIsLeftFoot ? -20.0f : 20.0f; // Offset left/right
    FootBoneLocation.Z -= 90.0f; // Approximate foot height

    // Trace downward from foot
    FVector Start = FootBoneLocation + FVector(0, 0, 50.0f);
    FVector End = FootBoneLocation - FVector(0, 0, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

    if (bHit)
    {
        // Calculate IK offset
        float TargetZ = HitResult.Location.Z + FootIKOffset;
        float CurrentZ = FootBoneLocation.Z;
        float IKOffset = TargetZ - CurrentZ;

        // Update foot data
        FootData.FootLocation = HitResult.Location;
        FootData.FootRotation = UKismetMathLibrary::MakeRotFromZ(HitResult.Normal);
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 1.0f, GetWorld()->GetDeltaSeconds(), FootIKInterpSpeed);
        FootData.bIsPlanted = true;

        // Debug visualization
        if (CVarShowFootIK.GetValueOnGameThread())
        {
            DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 8, FColor::Green, false, 0.1f);
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f);
        }
    }
    else
    {
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, GetWorld()->GetDeltaSeconds(), FootIKInterpSpeed);
        FootData.bIsPlanted = false;
    }
}

float UAnim_MotionMatchingController::CalculateMovementDirection() const
{
    if (!OwnerCharacter || MotionData.Speed < 10.0f)
    {
        return 0.0f;
    }

    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityDirection = MotionData.Velocity.GetSafeNormal();

    float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;

    return FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
}

// === SURVIVAL ACTION IMPLEMENTATIONS ===

void UAnim_MotionMatchingController::TriggerSpearThrow()
{
    if (ActionCooldownTimer > 0.0f)
    {
        return;
    }

    SetActionState(EAnim_ActionState::SpearThrow);
    ActionCooldownTimer = 1.5f;

    // Play montage if available
    if (SpearThrowMontage && MeshComponent && MeshComponent->GetAnimInstance())
    {
        MeshComponent->GetAnimInstance()->Montage_Play(SpearThrowMontage);
    }

    UE_LOG(LogTemp, Log, TEXT("Spear Throw Triggered"));
}

void UAnim_MotionMatchingController::TriggerStoneKnapping()
{
    if (ActionCooldownTimer > 0.0f)
    {
        return;
    }

    SetActionState(EAnim_ActionState::StoneKnapping);
    ActionCooldownTimer = 3.0f;

    if (StoneKnappingMontage && MeshComponent && MeshComponent->GetAnimInstance())
    {
        MeshComponent->GetAnimInstance()->Montage_Play(StoneKnappingMontage);
    }

    UE_LOG(LogTemp, Log, TEXT("Stone Knapping Triggered"));
}

void UAnim_MotionMatchingController::TriggerGathering()
{
    if (ActionCooldownTimer > 0.0f)
    {
        return;
    }

    SetActionState(EAnim_ActionState::Gathering);
    ActionCooldownTimer = 2.0f;

    if (GatheringMontage && MeshComponent && MeshComponent->GetAnimInstance())
    {
        MeshComponent->GetAnimInstance()->Montage_Play(GatheringMontage);
    }

    UE_LOG(LogTemp, Log, TEXT("Gathering Triggered"));
}

void UAnim_MotionMatchingController::TriggerCrafting()
{
    if (ActionCooldownTimer > 0.0f)
    {
        return;
    }

    SetActionState(EAnim_ActionState::Crafting);
    ActionCooldownTimer = 4.0f;

    if (CraftingMontage && MeshComponent && MeshComponent->GetAnimInstance())
    {
        MeshComponent->GetAnimInstance()->Montage_Play(CraftingMontage);
    }

    UE_LOG(LogTemp, Log, TEXT("Crafting Triggered"));
}

// Debug console variable for foot IK visualization
static TAutoConsoleVariable<bool> CVarShowFootIK(
    TEXT("anim.ShowFootIK"),
    false,
    TEXT("Show foot IK debug visualization"),
    ECVF_Default
);