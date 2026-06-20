#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ACharacter>(Pawn);
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // ── Velocity / Speed ────────────────────────────────────────
    FVector Velocity = MovementComponent->Velocity;
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    Speed = HorizontalVelocity.Size();
    bIsMoving = Speed > 3.0f;

    // ── Direction (strafe angle) ─────────────────────────────────
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator VelocityRot = HorizontalVelocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ── Lean (lateral acceleration) ──────────────────────────────
    FVector Accel = MovementComponent->GetCurrentAcceleration();
    float LateralAccel = FVector::DotProduct(Accel, OwnerCharacter->GetActorRightVector());
    LeanAngleTarget = FMath::Clamp(LateralAccel / 600.0f * 15.0f, -15.0f, 15.0f);
    LeanAngleSmoothed = FMath::FInterpTo(LeanAngleSmoothed, LeanAngleTarget, DeltaSeconds, 6.0f);
    LeanAngle = LeanAngleSmoothed;

    // ── Air / Crouch / Sprint ────────────────────────────────────
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsSprinting = (Speed > 450.0f) && !bIsInAir && !bIsCrouching;

    // ── Locomotion State Machine ─────────────────────────────────
    if (bIsInAir)
    {
        LocomotionState = (Velocity.Z > 0.0f) ? EAnim_LocomotionState::Jump : EAnim_LocomotionState::Fall;
    }
    else if (bIsCrouching)
    {
        LocomotionState = bIsMoving ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > 200.0f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else if (bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }

    // ── Aim Offset ──────────────────────────────────────────────
    AController* Controller = OwnerCharacter->GetController();
    if (Controller)
    {
        FRotator ControlRot = Controller->GetControlRotation();
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
        AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
        AimYaw   = FMath::Clamp(DeltaRot.Yaw,   -90.0f, 90.0f);
    }

    // ── Foot IK ─────────────────────────────────────────────────
    if (!bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        FootIKData.LeftFootAlpha  = FMath::FInterpTo(FootIKData.LeftFootAlpha,  0.0f, DeltaSeconds, 10.0f);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, 10.0f);
        FootIKData.PelvisOffset   = FMath::FInterpTo(FootIKData.PelvisOffset,   0.0f, DeltaSeconds, 10.0f);
    }
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    FVector LeftTarget  = GetFootIKLocation(FName("foot_l"));
    FVector RightTarget = GetFootIKLocation(FName("foot_r"));

    // Smooth IK locations
    FootIKData.LeftFootLocation  = FMath::VInterpTo(FootIKData.LeftFootLocation,  LeftTarget,  DeltaSeconds, 15.0f);
    FootIKData.RightFootLocation = FMath::VInterpTo(FootIKData.RightFootLocation, RightTarget, DeltaSeconds, 15.0f);

    // Pelvis offset — lower pelvis to the lowest foot
    float LeftZ  = FootIKData.LeftFootLocation.Z;
    float RightZ = FootIKData.RightFootLocation.Z;
    float LowestFoot = FMath::Min(LeftZ, RightZ);
    float TargetPelvis = FMath::Clamp(LowestFoot - OwnerCharacter->GetActorLocation().Z, -30.0f, 0.0f);
    FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, TargetPelvis, DeltaSeconds, 8.0f);

    // Alpha — blend in IK when on uneven ground
    float HeightDiff = FMath::Abs(LeftZ - RightZ);
    float TargetAlpha = FMath::Clamp(HeightDiff / 20.0f, 0.0f, 1.0f);
    FootIKData.LeftFootAlpha  = FMath::FInterpTo(FootIKData.LeftFootAlpha,  TargetAlpha, DeltaSeconds, 10.0f);
    FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, TargetAlpha, DeltaSeconds, 10.0f);
}

FVector UDinoSurvivorAnimInstance::GetFootIKLocation(FName SocketName, float TraceDistance)
{
    if (!OwnerCharacter)
    {
        return FVector::ZeroVector;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        return FVector::ZeroVector;
    }

    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    FVector TraceStart = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z + TraceDistance);
    FVector TraceEnd   = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z - TraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        Params
    );

    if (bHit)
    {
        return HitResult.ImpactPoint;
    }

    return SocketLocation;
}
