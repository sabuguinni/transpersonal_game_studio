#include "DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    LocomotionState     = ENPC_DinoLocomotionState::Idle;
    SpeciesType         = ENPC_DinoSpeciesType::Generic;
    GroundSpeed         = 0.f;
    MovementDirection   = 0.f;
    TurnRate            = 0.f;
    bIsMoving           = false;
    bIsSprinting        = false;

    bIsAttacking        = false;
    bIsAlerted          = false;
    bIsFeeding          = false;
    bIsFleeing          = false;
    ThreatLevel         = 0.f;
    DistanceToTarget    = 9999.f;

    HealthNormalized    = 1.f;
    bIsWounded          = false;
    bIsDead             = false;

    LeftFrontFootEffector  = FVector::ZeroVector;
    RightFrontFootEffector = FVector::ZeroVector;
    LeftRearFootEffector   = FVector::ZeroVector;
    RightRearFootEffector  = FVector::ZeroVector;
    IKAlpha                = 1.f;
    IKTraceDistance        = 80.f;
    bEnableFootIK          = true;

    HeadLookAtRotation  = FRotator::ZeroRotator;
    HeadLookAtAlpha     = 0.f;

    PreviousYaw         = 0.f;
    OwnerPawn           = nullptr;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerPawn = TryGetPawnOwner();
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn) return;

    UpdateLocomotion(DeltaSeconds);
    UpdateAIState(DeltaSeconds);
    UpdateSurvivalState(DeltaSeconds);

    if (bEnableFootIK && !bIsDead)
    {
        UpdateFootIK(DeltaSeconds);
    }

    UpdateHeadLookAt(DeltaSeconds);
}

// ─── Locomotion ───────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    const FVector Velocity = OwnerPawn->GetVelocity();
    GroundSpeed = Velocity.Size2D();
    bIsMoving   = GroundSpeed > 10.f;

    // Turn rate from yaw delta
    const float CurrentYaw = OwnerPawn->GetActorRotation().Yaw;
    TurnRate    = (CurrentYaw - PreviousYaw) / FMath::Max(DeltaSeconds, 0.001f);
    PreviousYaw = CurrentYaw;

    // Movement direction (relative to actor facing)
    if (bIsMoving)
    {
        const FVector ForwardDir = OwnerPawn->GetActorForwardVector();
        const FVector VelocityDir = Velocity.GetSafeNormal();
        const float Dot = FVector::DotProduct(ForwardDir, VelocityDir);
        const FVector Cross = FVector::CrossProduct(ForwardDir, VelocityDir);
        MovementDirection = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));
        if (Cross.Z < 0.f) MovementDirection = -MovementDirection;
    }
    else
    {
        MovementDirection = 0.f;
    }

    // Sprint threshold — species-specific max walk speeds
    float SprintThreshold = 400.f;
    switch (SpeciesType)
    {
        case ENPC_DinoSpeciesType::TRex:          SprintThreshold = 600.f;  break;
        case ENPC_DinoSpeciesType::Velociraptor:  SprintThreshold = 900.f;  break;
        case ENPC_DinoSpeciesType::Triceratops:   SprintThreshold = 350.f;  break;
        case ENPC_DinoSpeciesType::Brachiosaurus: SprintThreshold = 200.f;  break;
        default:                                  SprintThreshold = 400.f;  break;
    }
    bIsSprinting = GroundSpeed > SprintThreshold * 0.75f;

    // Resolve locomotion state from AI flags
    if (bIsDead)
    {
        LocomotionState = ENPC_DinoLocomotionState::Dead;
    }
    else if (bIsAttacking)
    {
        LocomotionState = ENPC_DinoLocomotionState::Attack;
    }
    else if (bIsFleeing)
    {
        LocomotionState = ENPC_DinoLocomotionState::Fleeing;
    }
    else if (bIsSprinting && bIsAlerted)
    {
        LocomotionState = ENPC_DinoLocomotionState::Chase;
    }
    else if (bIsFeeding)
    {
        LocomotionState = ENPC_DinoLocomotionState::Feeding;
    }
    else if (bIsAlerted)
    {
        LocomotionState = ENPC_DinoLocomotionState::Alert;
    }
    else if (bIsMoving)
    {
        LocomotionState = ENPC_DinoLocomotionState::Patrol;
    }
    else
    {
        LocomotionState = ENPC_DinoLocomotionState::Idle;
    }
}

// ─── AI State (Blackboard) ────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateAIState(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    AAIController* AIC = Cast<AAIController>(OwnerPawn->GetController());
    if (!AIC) return;

    UBlackboardComponent* BB = AIC->GetBlackboardComponent();
    if (!BB) return;

    // Read standard blackboard keys set by the Behavior Tree
    bIsAttacking     = BB->GetValueAsBool(FName("bIsAttacking"));
    bIsAlerted       = BB->GetValueAsBool(FName("bIsAlerted"));
    bIsFeeding       = BB->GetValueAsBool(FName("bIsFeeding"));
    bIsFleeing       = BB->GetValueAsBool(FName("bIsFleeing"));
    ThreatLevel      = BB->GetValueAsFloat(FName("ThreatLevel"));

    // Distance to target (player or prey)
    UObject* TargetObj = BB->GetValueAsObject(FName("TargetActor"));
    AActor* TargetActor = Cast<AActor>(TargetObj);
    if (TargetActor)
    {
        DistanceToTarget = FVector::Dist(OwnerPawn->GetActorLocation(), TargetActor->GetActorLocation());
    }
    else
    {
        DistanceToTarget = 9999.f;
    }

    // Head look-at alpha scales with threat level and alert state
    HeadLookAtAlpha = bIsAlerted ? FMath::Lerp(0.3f, 1.0f, ThreatLevel) : 0.f;
}

// ─── Survival State ───────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
    // HealthNormalized is expected to be set externally by the health component.
    // Here we derive booleans from it.
    HealthNormalized = FMath::Clamp(HealthNormalized, 0.f, 1.f);
    bIsWounded       = HealthNormalized < 0.35f && HealthNormalized > 0.f;
    bIsDead          = HealthNormalized <= 0.f;

    // Wounded dinosaurs slow down — feed back into locomotion
    if (bIsWounded && !bIsDead)
    {
        // Reduce IK alpha so wounded limping is more pronounced
        IKAlpha = FMath::Lerp(0.5f, 1.0f, HealthNormalized / 0.35f);
    }
    else if (!bIsDead)
    {
        IKAlpha = 1.f;
    }
    else
    {
        IKAlpha = 0.f;
    }
}

// ─── Foot IK ─────────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    LeftFrontFootEffector  = TraceFootPosition(FName("foot_fl"));
    RightFrontFootEffector = TraceFootPosition(FName("foot_fr"));
    LeftRearFootEffector   = TraceFootPosition(FName("foot_rl"));
    RightRearFootEffector  = TraceFootPosition(FName("foot_rr"));
}

FVector UDinosaurAnimInstance::TraceFootPosition(FName SocketName) const
{
    if (!OwnerPawn) return FVector::ZeroVector;

    USkeletalMeshComponent* Mesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) return FVector::ZeroVector;

    const FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart = SocketLoc + FVector(0.f, 0.f, IKTraceDistance);
    const FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, IKTraceDistance);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    UWorld* World = OwnerPawn->GetWorld();
    if (!World) return FVector::ZeroVector;

    if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        return Hit.ImpactPoint;
    }

    return SocketLoc;
}

// ─── Head Look-At ─────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateHeadLookAt(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    AAIController* AIC = Cast<AAIController>(OwnerPawn->GetController());
    if (!AIC) return;

    UBlackboardComponent* BB = AIC->GetBlackboardComponent();
    if (!BB) return;

    UObject* TargetObj = BB->GetValueAsObject(FName("TargetActor"));
    AActor* TargetActor = Cast<AActor>(TargetObj);

    if (TargetActor && HeadLookAtAlpha > 0.f)
    {
        const FVector ToTarget = (TargetActor->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
        const FRotator LookRot = ToTarget.Rotation();
        const FRotator ActorRot = OwnerPawn->GetActorRotation();

        // Delta rotation clamped for natural head range
        float DeltaYaw   = FMath::ClampAngle(LookRot.Yaw   - ActorRot.Yaw,   -60.f, 60.f);
        float DeltaPitch = FMath::ClampAngle(LookRot.Pitch  - ActorRot.Pitch, -30.f, 30.f);

        HeadLookAtRotation = FMath::RInterpTo(
            HeadLookAtRotation,
            FRotator(DeltaPitch, DeltaYaw, 0.f),
            DeltaSeconds,
            5.f
        );
    }
    else
    {
        HeadLookAtRotation = FMath::RInterpTo(
            HeadLookAtRotation,
            FRotator::ZeroRotator,
            DeltaSeconds,
            3.f
        );
    }
}
