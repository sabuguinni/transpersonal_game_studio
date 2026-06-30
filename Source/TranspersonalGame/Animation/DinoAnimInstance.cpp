// DinoAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260630_003
// Dinosaur animation instance — quadruped/biped locomotion, attack montages,
// procedural spine/head tracking, tail physics simulation.

#include "DinoAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UDinoAnimInstance::UDinoAnimInstance()
{
    // Locomotion
    DinoSpeed = 0.0f;
    SmoothedDinoSpeed = 0.0f;
    DinoDirection = 0.0f;
    DinoVerticalVelocity = 0.0f;

    bDinoIsMoving = false;
    bDinoIsCharging = false;
    bDinoIsInAir = false;
    bDinoIsAttacking = false;
    bDinoIsEating = false;
    bDinoIsSleeping = false;
    bDinoIsAlerted = false;
    bDinoIsWounded = false;

    DinoLocomotionState = EAnim_DinoLocomotionState::DinoIdle;
    DinoBehaviorState = EAnim_DinoBehaviorState::DinoBehavior_Idle;
    DinoSpeciesType = EAnim_DinoSpecies::Raptor;

    // Procedural tracking
    HeadYaw = 0.0f;
    HeadPitch = 0.0f;
    SpineForwardLean = 0.0f;
    TailSwayAmount = 0.0f;
    TailSwayFrequency = 1.0f;
    TailSwayTime = 0.0f;

    // IK
    FrontLeftFootOffset = FVector::ZeroVector;
    FrontRightFootOffset = FVector::ZeroVector;
    BackLeftFootOffset = FVector::ZeroVector;
    BackRightFootOffset = FVector::ZeroVector;
    DinoIKAlpha = 0.0f;

    // Health
    DinoHealthNormalized = 1.0f;

    // Smoothing
    DinoSpeedSmoothingRate = 8.0f;
    HeadTrackSmoothingRate = 5.0f;
    TailSwaySmoothingRate = 3.0f;
}

void UDinoAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerPawn = TryGetPawnOwner();
}

void UDinoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        return;
    }

    UpdateDinoLocomotion(DeltaSeconds);
    UpdateDinoLocomotionState();
    UpdateProceduralSpine(DeltaSeconds);
    UpdateTailSway(DeltaSeconds);
    UpdateDinoFootIK(DeltaSeconds);
}

void UDinoAnimInstance::UpdateDinoLocomotion(float DeltaSeconds)
{
    FVector Velocity = OwnerPawn->GetVelocity();
    float RawSpeed = Velocity.Size2D();

    SmoothedDinoSpeed = FMath::FInterpTo(SmoothedDinoSpeed, RawSpeed, DeltaSeconds, DinoSpeedSmoothingRate);
    DinoSpeed = SmoothedDinoSpeed;
    DinoVerticalVelocity = Velocity.Z;

    FRotator PawnRot = OwnerPawn->GetActorRotation();
    FVector LocalVel = PawnRot.UnrotateVector(Velocity);
    DinoDirection = FMath::RadiansToDegrees(FMath::Atan2(LocalVel.Y, LocalVel.X));

    bDinoIsMoving = RawSpeed > 20.0f;
    bDinoIsInAir = DinoVerticalVelocity < -50.0f;

    // Charging = very high speed
    bDinoIsCharging = RawSpeed > 700.0f;

    // Spine forward lean based on speed
    float TargetLean = FMath::Clamp(RawSpeed * 0.015f, 0.0f, 25.0f);
    SpineForwardLean = FMath::FInterpTo(SpineForwardLean, TargetLean, DeltaSeconds, 4.0f);
}

void UDinoAnimInstance::UpdateDinoLocomotionState()
{
    EAnim_DinoLocomotionState NewState = DinoLocomotionState;

    if (bDinoIsSleeping)
    {
        NewState = EAnim_DinoLocomotionState::DinoSleep;
    }
    else if (bDinoIsAttacking)
    {
        NewState = EAnim_DinoLocomotionState::DinoAttack;
    }
    else if (bDinoIsEating)
    {
        NewState = EAnim_DinoLocomotionState::DinoEat;
    }
    else if (bDinoIsInAir)
    {
        NewState = EAnim_DinoLocomotionState::DinoFall;
    }
    else if (bDinoIsCharging)
    {
        NewState = EAnim_DinoLocomotionState::DinoCharge;
    }
    else if (bDinoIsMoving)
    {
        // Speed thresholds vary by species
        float WalkThreshold = 150.0f;
        float RunThreshold = 400.0f;

        if (DinoSpeciesType == EAnim_DinoSpecies::TRex || DinoSpeciesType == EAnim_DinoSpecies::Brachiosaurus)
        {
            WalkThreshold = 200.0f;
            RunThreshold = 500.0f;
        }

        if (DinoSpeed > RunThreshold)
            NewState = EAnim_DinoLocomotionState::DinoRun;
        else if (DinoSpeed > WalkThreshold)
            NewState = EAnim_DinoLocomotionState::DinoTrot;
        else
            NewState = EAnim_DinoLocomotionState::DinoWalk;
    }
    else
    {
        NewState = bDinoIsAlerted
            ? EAnim_DinoLocomotionState::DinoAlert
            : EAnim_DinoLocomotionState::DinoIdle;
    }

    DinoLocomotionState = NewState;
}

void UDinoAnimInstance::UpdateProceduralSpine(float DeltaSeconds)
{
    // Head tracking toward a target (set externally by AI)
    // Smooth head rotation toward TargetLookAtLocation
    if (!OwnerPawn) return;

    FVector PawnLoc = OwnerPawn->GetActorLocation();
    FVector ToTarget = (TargetLookAtLocation - PawnLoc).GetSafeNormal();
    FRotator LookRot = ToTarget.Rotation();
    FRotator PawnRot = OwnerPawn->GetActorRotation();
    FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, PawnRot);

    float TargetHeadYaw = FMath::Clamp(Delta.Yaw, -60.0f, 60.0f);
    float TargetHeadPitch = FMath::Clamp(Delta.Pitch, -30.0f, 30.0f);

    HeadYaw = FMath::FInterpTo(HeadYaw, TargetHeadYaw, DeltaSeconds, HeadTrackSmoothingRate);
    HeadPitch = FMath::FInterpTo(HeadPitch, TargetHeadPitch, DeltaSeconds, HeadTrackSmoothingRate);
}

void UDinoAnimInstance::UpdateTailSway(float DeltaSeconds)
{
    // Procedural tail sway — sinusoidal, speed-modulated
    float SpeedFactor = FMath::Clamp(DinoSpeed / 600.0f, 0.2f, 1.0f);
    TailSwayFrequency = FMath::Lerp(0.8f, 2.5f, SpeedFactor);

    TailSwayTime += DeltaSeconds * TailSwayFrequency;
    float TargetSway = FMath::Sin(TailSwayTime) * SpeedFactor * 15.0f;
    TailSwayAmount = FMath::FInterpTo(TailSwayAmount, TargetSway, DeltaSeconds, TailSwaySmoothingRate);
}

void UDinoAnimInstance::UpdateDinoFootIK(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    UWorld* World = GetWorld();
    if (!World) return;

    USkeletalMeshComponent* Mesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) return;

    // Quadruped foot socket names (biped dinosaurs like T-Rex use only back feet)
    static const FName SocketNames[4] = {
        FName("foot_fl"), FName("foot_fr"),
        FName("foot_bl"), FName("foot_br")
    };

    FVector* Offsets[4] = {
        &FrontLeftFootOffset, &FrontRightFootOffset,
        &BackLeftFootOffset, &BackRightFootOffset
    };

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    float TraceHalfHeight = 60.0f;

    for (int32 i = 0; i < 4; ++i)
    {
        if (!Mesh->DoesSocketExist(SocketNames[i]))
        {
            *Offsets[i] = FVector::ZeroVector;
            continue;
        }

        FVector FootLoc = Mesh->GetSocketLocation(SocketNames[i]);
        FVector TraceStart = FootLoc + FVector(0, 0, TraceHalfHeight);
        FVector TraceEnd = FootLoc - FVector(0, 0, TraceHalfHeight);

        FHitResult Hit;
        bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

        FVector TargetOffset = FVector::ZeroVector;
        if (bHit)
        {
            float Delta = Hit.ImpactPoint.Z - FootLoc.Z;
            TargetOffset = FVector(0, 0, FMath::Clamp(Delta, -30.0f, 30.0f));
        }

        *Offsets[i] = FMath::VInterpTo(*Offsets[i], TargetOffset, DeltaSeconds, 12.0f);
    }

    // IK alpha — grounded, not charging
    float TargetAlpha = (!bDinoIsInAir && !bDinoIsCharging) ? 1.0f : 0.0f;
    DinoIKAlpha = FMath::FInterpTo(DinoIKAlpha, TargetAlpha, DeltaSeconds, 10.0f);
}

void UDinoAnimInstance::SetDinoHealthNormalized(float Value)
{
    DinoHealthNormalized = FMath::Clamp(Value, 0.0f, 1.0f);
    bDinoIsWounded = DinoHealthNormalized < 0.35f;
}

void UDinoAnimInstance::SetDinoBehaviorState(EAnim_DinoBehaviorState NewState)
{
    DinoBehaviorState = NewState;

    // Map behavior to animation flags
    bDinoIsAttacking = (NewState == EAnim_DinoBehaviorState::DinoBehavior_Attack);
    bDinoIsEating = (NewState == EAnim_DinoBehaviorState::DinoBehavior_Eat);
    bDinoIsSleeping = (NewState == EAnim_DinoBehaviorState::DinoBehavior_Sleep);
    bDinoIsAlerted = (NewState == EAnim_DinoBehaviorState::DinoBehavior_Alert ||
                      NewState == EAnim_DinoBehaviorState::DinoBehavior_Stalk);
}

void UDinoAnimInstance::SetTargetLookAt(FVector TargetLocation)
{
    TargetLookAtLocation = TargetLocation;
}

EAnim_DinoLocomotionState UDinoAnimInstance::GetDinoLocomotionState() const
{
    return DinoLocomotionState;
}
