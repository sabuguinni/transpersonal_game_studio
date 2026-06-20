#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    Speed = 0.f;
    Direction = 0.f;
    LeanAngle = 0.f;
    LeanAngleTarget = 0.f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsMoving = false;

    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    LeftFootIKAlpha = 0.f;
    RightFootIKAlpha = 0.f;
    PelvisOffset = FVector::ZeroVector;

    AimPitch = 0.f;
    AimYaw = 0.f;

    StaminaNormalized = 1.f;
    FearNormalized = 0.f;
    bIsExhausted = false;
    bIsInjured = false;
    bIsCarrying = false;

    bJustLanded = false;
    FallSpeed = 0.f;
    PreviousSpeed = 0.f;

    OwnerCharacter = nullptr;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (!OwnerCharacter) return;
    }

    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    // --- Velocity & Speed ---
    FVector Velocity = MovComp->Velocity;
    Speed = Velocity.Size2D();
    bIsMoving = Speed > 10.f;

    // --- Direction (strafe angle) ---
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator VelRot = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.f;
    }

    // --- Lean (lateral acceleration feel) ---
    float SpeedDelta = Speed - PreviousSpeed;
    LeanAngleTarget = FMath::Clamp(Direction * 0.15f + SpeedDelta * 0.05f, -15.f, 15.f);
    LeanAngle = FMath::FInterpTo(LeanAngle, LeanAngleTarget, DeltaSeconds, 6.f);
    PreviousSpeed = Speed;

    // --- Air state ---
    bIsInAir = MovComp->IsFalling();
    FallSpeed = bIsInAir ? FMath::Clamp(Velocity.Z, -2000.f, 0.f) : 0.f;

    // --- Crouch & Sprint ---
    bIsCrouching = MovComp->IsCrouching();
    bIsSprinting = Speed > MovComp->MaxWalkSpeed * 0.85f && bIsMoving && !bIsInAir;

    // --- Aim Offset ---
    FRotator ControlRot = OwnerCharacter->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.f, 90.f);
    AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.f, 90.f);

    // --- Foot IK (only when grounded) ---
    if (!bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.f, DeltaSeconds, 10.f);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.f, DeltaSeconds, 10.f);
        PelvisOffset = FMath::VInterpTo(PelvisOffset, FVector::ZeroVector, DeltaSeconds, 10.f);
    }
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    FVector LeftNormal, RightNormal;
    FVector LeftTarget = TraceFootIK(FName("foot_l"), LeftNormal);
    FVector RightTarget = TraceFootIK(FName("foot_r"), RightNormal);

    // Smooth IK targets
    LeftFootIKLocation = FMath::VInterpTo(LeftFootIKLocation, LeftTarget, DeltaSeconds, 15.f);
    RightFootIKLocation = FMath::VInterpTo(RightFootIKLocation, RightTarget, DeltaSeconds, 15.f);

    // Foot rotation from surface normal
    FRotator LeftRot = UKismetMathLibrary::MakeRotFromZX(LeftNormal, OwnerCharacter->GetActorForwardVector());
    FRotator RightRot = UKismetMathLibrary::MakeRotFromZX(RightNormal, OwnerCharacter->GetActorForwardVector());
    LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, LeftRot, DeltaSeconds, 15.f);
    RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, RightRot, DeltaSeconds, 15.f);

    // Alpha: 1 when grounded, 0 in air
    LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 1.f, DeltaSeconds, 10.f);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 1.f, DeltaSeconds, 10.f);

    // Pelvis offset: lower pelvis when one foot is significantly lower
    float LeftDiff = LeftFootIKLocation.Z - OwnerCharacter->GetActorLocation().Z;
    float RightDiff = RightFootIKLocation.Z - OwnerCharacter->GetActorLocation().Z;
    float PelvisZ = FMath::Min(LeftDiff, RightDiff);
    PelvisOffset = FMath::VInterpTo(PelvisOffset, FVector(0.f, 0.f, PelvisZ * 0.5f), DeltaSeconds, 10.f);
}

FVector UDinoSurvivorAnimInstance::TraceFootIK(FName SocketName, FVector& OutNormal)
{
    OutNormal = FVector::UpVector;
    if (!OwnerCharacter) return FVector::ZeroVector;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return FVector::ZeroVector;

    FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLoc + FVector(0.f, 0.f, 50.f);
    FVector TraceEnd = SocketLoc - FVector(0.f, 0.f, 75.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {
        OutNormal = Hit.ImpactNormal;
        return Hit.ImpactPoint;
    }

    return SocketLoc;
}
