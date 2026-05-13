#include "Anim_PrehistoricMovementBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_PrehistoricMovementBlueprint::UAnim_PrehistoricMovementBlueprint()
{
    // Initialize default values
    CurrentState = EAnim_PrehistoricState::Idle;
    StateTransitionTime = 0.0f;
    IdleTime = 0.0f;
    MovementTime = 0.0f;
    
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;

    // Initialize movement and survival data
    MovementData = FAnim_MovementData();
    SurvivalState = FAnim_SurvivalState();

    // Set default animation assets to null - will be set in Blueprint
    LocomotionBlendSpace = nullptr;
    JumpMontage = nullptr;
    CraftingMontage = nullptr;
    GatheringMontage = nullptr;
    CombatMontage = nullptr;

    OwningCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_PrehistoricMovementBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Get character and movement component references
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }

    // Initialize state
    CurrentState = EAnim_PrehistoricState::Idle;
    StateTransitionTime = 0.0f;
    IdleTime = 0.0f;
    MovementTime = 0.0f;
}

void UAnim_PrehistoricMovementBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }

    // Update all animation data
    UpdateMovementData(DeltaTimeX);
    UpdateSurvivalState(DeltaTimeX);
    UpdateAnimationState(DeltaTimeX);
    UpdateStateTimers(DeltaTimeX);
    UpdateFootIK(DeltaTimeX);
}

void UAnim_PrehistoricMovementBlueprint::UpdateMovementData(float DeltaTime)
{
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }

    // Get velocity and movement data
    FVector CurrentVelocity = MovementComponent->Velocity;
    MovementData.Velocity = CurrentVelocity;
    MovementData.VelocityZ = CurrentVelocity.Z;
    MovementData.GroundSpeed = CurrentVelocity.Size2D();
    MovementData.Speed = MovementData.GroundSpeed;

    // Calculate movement direction relative to character
    if (MovementData.Speed > 1.0f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentVelocity.GetSafeNormal2D();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        MovementData.Direction = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
        MovementData.bIsMoving = true;
    }
    else
    {
        MovementData.Direction = 0.0f;
        MovementData.bIsMoving = false;
    }

    // Update movement states
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_PrehistoricMovementBlueprint::UpdateSurvivalState(float DeltaTime)
{
    // This would typically get data from a survival component
    // For now, we'll use placeholder logic
    
    // Update exhaustion based on movement
    if (MovementData.Speed > 400.0f) // Running
    {
        SurvivalState.StaminaPercentage = FMath::Max(0.0f, SurvivalState.StaminaPercentage - (DeltaTime * 5.0f));
    }
    else if (MovementData.Speed < 50.0f) // Resting
    {
        SurvivalState.StaminaPercentage = FMath::Min(100.0f, SurvivalState.StaminaPercentage + (DeltaTime * 2.0f));
    }

    // Update state flags
    SurvivalState.bIsExhausted = SurvivalState.StaminaPercentage < 20.0f;
    SurvivalState.bIsInjured = SurvivalState.HealthPercentage < 50.0f;
    SurvivalState.bIsAfraid = SurvivalState.FearLevel > 70.0f;

    // Slowly decrease fear over time
    SurvivalState.FearLevel = FMath::Max(0.0f, SurvivalState.FearLevel - (DeltaTime * 10.0f));
}

void UAnim_PrehistoricMovementBlueprint::UpdateAnimationState(float DeltaTime)
{
    EAnim_PrehistoricState DesiredState = CalculateDesiredState();
    
    if (DesiredState != CurrentState && CanTransitionToState(DesiredState))
    {
        EAnim_PrehistoricState OldState = CurrentState;
        CurrentState = DesiredState;
        OnStateChanged(OldState, CurrentState);
        StateTransitionTime = 0.0f;
    }
}

void UAnim_PrehistoricMovementBlueprint::UpdateStateTimers(float DeltaTime)
{
    StateTransitionTime += DeltaTime;

    if (MovementData.bIsMoving)
    {
        MovementTime += DeltaTime;
        IdleTime = 0.0f;
    }
    else
    {
        IdleTime += DeltaTime;
        MovementTime = 0.0f;
    }
}

EAnim_PrehistoricState UAnim_PrehistoricMovementBlueprint::CalculateDesiredState()
{
    // Priority order for state calculation

    // Check for survival states first
    if (SurvivalState.bIsAfraid && SurvivalState.FearLevel > 80.0f)
    {
        return EAnim_PrehistoricState::Afraid;
    }

    if (SurvivalState.bIsExhausted)
    {
        return EAnim_PrehistoricState::Exhausted;
    }

    if (SurvivalState.bIsInjured && SurvivalState.HealthPercentage < 25.0f)
    {
        return EAnim_PrehistoricState::Injured;
    }

    // Check movement states
    if (MovementData.bIsInAir)
    {
        if (MovementData.VelocityZ > 100.0f)
        {
            return EAnim_PrehistoricState::Jumping;
        }
        else if (MovementData.VelocityZ < -100.0f)
        {
            return EAnim_PrehistoricState::Falling;
        }
    }

    if (MovementData.bIsCrouching)
    {
        if (MovementData.bIsMoving)
        {
            return EAnim_PrehistoricState::Sneaking;
        }
        else
        {
            return EAnim_PrehistoricState::Crouching;
        }
    }

    // Check for locomotion states
    if (MovementData.bIsMoving)
    {
        if (MovementData.Speed > 400.0f)
        {
            return EAnim_PrehistoricState::Running;
        }
        else if (MovementData.Speed > 50.0f)
        {
            return EAnim_PrehistoricState::Walking;
        }
    }

    // Default to idle
    return EAnim_PrehistoricState::Idle;
}

bool UAnim_PrehistoricMovementBlueprint::CanTransitionToState(EAnim_PrehistoricState NewState)
{
    // Prevent rapid state changes
    if (StateTransitionTime < 0.1f)
    {
        return false;
    }

    // Allow all transitions for now
    // In a more complex system, you might have state transition rules
    return true;
}

void UAnim_PrehistoricMovementBlueprint::OnStateChanged(EAnim_PrehistoricState OldState, EAnim_PrehistoricState NewState)
{
    // Handle state-specific logic when transitioning
    switch (NewState)
    {
        case EAnim_PrehistoricState::Jumping:
            PlayJumpAnimation();
            break;
        case EAnim_PrehistoricState::Landing:
            // Could play a landing sound or effect
            break;
        default:
            break;
    }
}

void UAnim_PrehistoricMovementBlueprint::SetAnimationState(EAnim_PrehistoricState NewState)
{
    if (CanTransitionToState(NewState))
    {
        EAnim_PrehistoricState OldState = CurrentState;
        CurrentState = NewState;
        OnStateChanged(OldState, CurrentState);
        StateTransitionTime = 0.0f;
    }
}

void UAnim_PrehistoricMovementBlueprint::PlayJumpAnimation()
{
    if (JumpMontage)
    {
        Montage_Play(JumpMontage, 1.0f);
    }
}

void UAnim_PrehistoricMovementBlueprint::PlayCraftingAnimation()
{
    if (CraftingMontage)
    {
        Montage_Play(CraftingMontage, 1.0f);
        SetAnimationState(EAnim_PrehistoricState::Crafting);
    }
}

void UAnim_PrehistoricMovementBlueprint::PlayGatheringAnimation()
{
    if (GatheringMontage)
    {
        Montage_Play(GatheringMontage, 1.0f);
        SetAnimationState(EAnim_PrehistoricState::Gathering);
    }
}

void UAnim_PrehistoricMovementBlueprint::PlayCombatAnimation()
{
    if (CombatMontage)
    {
        Montage_Play(CombatMontage, 1.0f);
        SetAnimationState(EAnim_PrehistoricState::Combat);
    }
}

void UAnim_PrehistoricMovementBlueprint::StopAllMontages()
{
    Montage_Stop(0.2f);
}

void UAnim_PrehistoricMovementBlueprint::UpdateFootIK(float DeltaTime)
{
    if (!OwningCharacter || MovementData.bIsInAir)
    {
        // Gradually reduce IK when in air
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaTime, 10.0f);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaTime, 10.0f);
        return;
    }

    // Perform foot traces
    FVector LeftFootTrace = PerformFootTrace(FName("foot_l"), 50.0f);
    FVector RightFootTrace = PerformFootTrace(FName("foot_r"), 50.0f);

    // Update IK locations and rotations
    if (!LeftFootTrace.IsZero())
    {
        LeftFootIKLocation = LeftFootTrace;
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 1.0f, DeltaTime, 15.0f);
    }
    else
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaTime, 10.0f);
    }

    if (!RightFootTrace.IsZero())
    {
        RightFootIKLocation = RightFootTrace;
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 1.0f, DeltaTime, 15.0f);
    }
    else
    {
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaTime, 10.0f);
    }
}

FVector UAnim_PrehistoricMovementBlueprint::PerformFootTrace(const FName& SocketName, float TraceDistance)
{
    if (!OwningCharacter)
    {
        return FVector::ZeroVector;
    }

    USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh();
    if (!MeshComp)
    {
        return FVector::ZeroVector;
    }

    // Get socket location
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector StartTrace = SocketLocation + FVector(0, 0, 20.0f);
    FVector EndTrace = SocketLocation - FVector(0, 0, TraceDistance);

    // Perform line trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartTrace,
        EndTrace,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Calculate foot rotation based on surface normal
        if (SocketName == FName("foot_l"))
        {
            LeftFootIKRotation = CalculateFootRotation(HitResult.Normal);
        }
        else if (SocketName == FName("foot_r"))
        {
            RightFootIKRotation = CalculateFootRotation(HitResult.Normal);
        }

        return HitResult.Location;
    }

    return FVector::ZeroVector;
}

FRotator UAnim_PrehistoricMovementBlueprint::CalculateFootRotation(const FVector& ImpactNormal)
{
    FVector UpVector = FVector::UpVector;
    FVector RightVector = FVector::CrossProduct(UpVector, ImpactNormal).GetSafeNormal();
    FVector ForwardVector = FVector::CrossProduct(ImpactNormal, RightVector).GetSafeNormal();

    return FRotationMatrix::MakeFromXZ(ForwardVector, ImpactNormal).Rotator();
}