#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    GroundSpeed = 0.0f;
    GroundSpeedNormalized = 0.0f;
    bIsMoving = false;
    bIsFalling = false;
    bIsCrouching = false;
    bIsSprinting = false;

    MovementDirection = 0.0f;
    LeanAngle = 0.0f;

    StaminaNormalized = 1.0f;
    bIsExhausted = false;
    bIsInjured = false;

    bIsInCombat = false;
    bIsAiming = false;
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    IKAlpha = 1.0f;

    WalkSpeedThreshold = 10.0f;
    RunSpeedThreshold = 150.0f;
    SprintSpeedThreshold = 400.0f;
    IKTraceDistance = 80.0f;

    OwnerCharacter = nullptr;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(GetOwningActor());
        if (!OwnerCharacter) return;
    }

    UpdateLocomotion();
    UpdateFootIK();
}

void UDinoSurvivorAnimInstance::UpdateLocomotion()
{
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    // Ground speed (horizontal only)
    FVector Velocity = OwnerCharacter->GetVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    GroundSpeed = HorizontalVelocity.Size();

    // Normalize 0-1 over sprint range
    GroundSpeedNormalized = FMath::Clamp(GroundSpeed / SprintSpeedThreshold, 0.0f, 1.0f);

    bIsMoving = GroundSpeed > WalkSpeedThreshold;
    bIsFalling = MovComp->IsFalling();
    bIsCrouching = MovComp->IsCrouching();
    bIsSprinting = GroundSpeed >= SprintSpeedThreshold;

    // IK alpha — disable during fall, reduce when exhausted
    IKAlpha = bIsFalling ? 0.0f : (bIsExhausted ? 0.5f : 1.0f);

    // Movement direction relative to actor forward
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator VelocityRot = HorizontalVelocity.Rotation();
        FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot);
        MovementDirection = DeltaRot.Yaw;

        // Lean — proportional to speed and direction change
        LeanAngle = FMath::Clamp(DeltaRot.Yaw * 0.1f, -15.0f, 15.0f);
    }
    else
    {
        MovementDirection = 0.0f;
        LeanAngle = FMath::FInterpTo(LeanAngle, 0.0f, GetWorld()->GetDeltaSeconds(), 5.0f);
    }
}

void UDinoSurvivorAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter || bIsFalling) return;

    SolveFootIK(FName("foot_l"), LeftFootIKLocation, LeftFootIKRotation);
    SolveFootIK(FName("foot_r"), RightFootIKLocation, RightFootIKRotation);
}

void UDinoSurvivorAnimInstance::SolveFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    // Get foot socket world location
    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);

    // Trace downward from foot
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, IKTraceDistance);
    FVector TraceEnd   = SocketLocation - FVector(0.0f, 0.0f, IKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        Params
    );

    if (bHit)
    {
        OutLocation = HitResult.ImpactPoint;

        // Compute foot rotation from surface normal
        FVector Normal = HitResult.ImpactNormal;
        FRotator SurfaceRot = UKismetMathLibrary::MakeRotFromZX(Normal, OwnerCharacter->GetActorForwardVector());
        OutRotation = SurfaceRot;
    }
    else
    {
        // No hit — use default foot position
        OutLocation = SocketLocation;
        OutRotation = FRotator::ZeroRotator;
    }
}
