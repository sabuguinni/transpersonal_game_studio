#include "DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    LocomotionState = EAnim_DinoLocomotionState::Idle;
    DinoSpecies = EAnim_DinoSpecies::Generic;
    GroundSpeed = 0.0f;
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    bEnableFootIK = true;
    FootIKTraceDistance = 100.0f;
    FootIKAlpha = 1.0f;
    FootIKAlphaTarget = 1.0f;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        UE_LOG(LogTemp, Log, TEXT("DinosaurAnimInstance: Initialized for %s"), *OwnerPawn->GetName());
    }
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // Re-cache pawn if needed
    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
    }

    if (!OwnerPawn)
    {
        return;
    }

    // Update locomotion data
    UpdateLocomotionState();

    // Update foot IK
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Smoothly fade out IK
        FootIKAlphaTarget = 0.0f;
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, FootIKAlphaTarget, DeltaSeconds, 5.0f);
    }
}

void UDinosaurAnimInstance::UpdateLocomotionState()
{
    if (!OwnerPawn)
    {
        return;
    }

    // Get velocity
    FVector Velocity = OwnerPawn->GetVelocity();
    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

    // Update locomotion data struct
    LocomotionData.Speed = GroundSpeed;
    LocomotionData.bIsAttacking = bTriggerAttack;
    LocomotionData.bIsDead = false;

    // Check if in air (for characters)
    ACharacter* OwnerChar = Cast<ACharacter>(OwnerPawn);
    if (OwnerChar && OwnerChar->GetCharacterMovement())
    {
        LocomotionData.bIsInAir = OwnerChar->GetCharacterMovement()->IsFalling();
    }

    // Compute direction relative to actor facing
    FRotator ActorRot = OwnerPawn->GetActorRotation();
    FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
    LocomotionData.Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // Resolve state
    LocomotionState = ResolveLocomotionState(
        GroundSpeed,
        LocomotionData.bIsAttacking,
        LocomotionData.bIsEating,
        LocomotionData.bIsDead
    );

    // Consume one-shot triggers
    if (bTriggerRoar && LocomotionState == EAnim_DinoLocomotionState::Idle)
    {
        LocomotionState = EAnim_DinoLocomotionState::Roar;
        bTriggerRoar = false;
    }
}

EAnim_DinoLocomotionState UDinosaurAnimInstance::ResolveLocomotionState(
    float Speed,
    bool bAttacking,
    bool bEating,
    bool bDead) const
{
    if (bDead)
    {
        return EAnim_DinoLocomotionState::Death;
    }
    if (bAttacking)
    {
        return EAnim_DinoLocomotionState::Attack;
    }
    if (bEating)
    {
        return EAnim_DinoLocomotionState::Eat;
    }
    if (Speed >= RunSpeedThreshold)
    {
        return EAnim_DinoLocomotionState::Run;
    }
    if (Speed >= WalkSpeedThreshold)
    {
        return EAnim_DinoLocomotionState::Walk;
    }
    return EAnim_DinoLocomotionState::Idle;
}

void UDinosaurAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerPawn)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
    if (!MeshComp)
    {
        return;
    }

    UWorld* World = OwnerPawn->GetWorld();
    if (!World)
    {
        return;
    }

    // Only apply IK when grounded and moving slowly
    bool bShouldApplyIK = !LocomotionData.bIsInAir && GroundSpeed < RunSpeedThreshold;
    FootIKAlphaTarget = bShouldApplyIK ? 1.0f : 0.0f;
    FootIKAlpha = FMath::FInterpTo(FootIKAlpha, FootIKAlphaTarget, DeltaSeconds, 8.0f);

    if (FootIKAlpha < 0.01f)
    {
        return;
    }

    // Trace for left foot
    FName LeftFootBone = TEXT("foot_l");
    FName RightFootBone = TEXT("foot_r");

    // Try to get bone locations
    FVector LeftFootLoc = MeshComp->GetBoneLocation(LeftFootBone, EBoneSpaces::WorldSpace);
    FVector RightFootLoc = MeshComp->GetBoneLocation(RightFootBone, EBoneSpaces::WorldSpace);

    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerPawn);

    // Left foot trace
    FHitResult LeftHit;
    FVector LeftStart = LeftFootLoc + FVector(0, 0, FootIKTraceDistance);
    FVector LeftEnd = LeftFootLoc - FVector(0, 0, FootIKTraceDistance);

    if (World->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd, ECC_WorldStatic, TraceParams))
    {
        LeftFootIKTarget = LeftHit.ImpactPoint;
    }
    else
    {
        LeftFootIKTarget = LeftFootLoc;
    }

    // Right foot trace
    FHitResult RightHit;
    FVector RightStart = RightFootLoc + FVector(0, 0, FootIKTraceDistance);
    FVector RightEnd = RightFootLoc - FVector(0, 0, FootIKTraceDistance);

    if (World->LineTraceSingleByChannel(RightHit, RightStart, RightEnd, ECC_WorldStatic, TraceParams))
    {
        RightFootIKTarget = RightHit.ImpactPoint;
    }
    else
    {
        RightFootIKTarget = RightFootLoc;
    }
}
