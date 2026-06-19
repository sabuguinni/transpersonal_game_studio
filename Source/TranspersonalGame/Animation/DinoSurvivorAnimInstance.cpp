#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
    : Speed(0.f)
    , Direction(0.f)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , LeanAngle(0.f)
    , StaminaNormalized(1.f)
    , FearLevel(0.f)
    , bIsExhausted(false)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , IKAlpha(0.f)
    , AimPitch(0.f)
    , AimYaw(0.f)
    , bIsAiming(false)
    , OwnerCharacter(nullptr)
    , SmoothSpeed(0.f)
    , SmoothLean(0.f)
{
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

    UpdateMovementState(DeltaSeconds);
    UpdateSurvivalState();
    UpdateFootIK();
    UpdateAimState();
}

void UDinoSurvivorAnimInstance::UpdateMovementState(float DeltaSeconds)
{
    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    FVector Velocity = OwnerCharacter->GetVelocity();
    float RawSpeed = Velocity.Size2D();

    // Smooth speed transitions
    SmoothSpeed = FMath::FInterpTo(SmoothSpeed, RawSpeed, DeltaSeconds, 8.f);
    Speed = SmoothSpeed;

    bIsInAir = MovComp->IsFalling();
    bIsCrouching = MovComp->IsCrouching();

    // Sprint detection: max walk speed > 400 = sprinting
    bIsSprinting = (RawSpeed > 400.f) && !bIsInAir;

    // Direction: angle between character forward and velocity
    if (RawSpeed > 10.f)
    {
        FRotator CharRot = OwnerCharacter->GetActorRotation();
        FVector ForwardDir = CharRot.Vector();
        FVector VelDir = Velocity.GetSafeNormal2D();
        float DotF = FVector::DotProduct(ForwardDir, VelDir);
        FVector RightDir = FRotationMatrix(CharRot).GetScaledAxis(EAxis::Y);
        float DotR = FVector::DotProduct(RightDir, VelDir);
        Direction = FMath::RadiansToDegrees(FMath::Atan2(DotR, DotF));
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.f, DeltaSeconds, 5.f);
    }

    // Lean based on acceleration
    FVector Accel = MovComp->GetCurrentAcceleration();
    float TargetLean = 0.f;
    if (bIsSprinting && Accel.Size2D() > 10.f)
    {
        FRotator CharRot = OwnerCharacter->GetActorRotation();
        FVector RightDir = FRotationMatrix(CharRot).GetScaledAxis(EAxis::Y);
        TargetLean = FVector::DotProduct(RightDir, Accel.GetSafeNormal2D()) * 8.f;
    }
    SmoothLean = FMath::FInterpTo(SmoothLean, TargetLean, DeltaSeconds, 6.f);
    LeanAngle = SmoothLean;
}

void UDinoSurvivorAnimInstance::UpdateSurvivalState()
{
    // Default values — real survival stats hooked via TranspersonalCharacter
    // Fear affects posture: hunched when high fear
    bIsExhausted = (StaminaNormalized < 0.15f);
}

void UDinoSurvivorAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter) return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    // Only apply IK when on ground
    if (bIsInAir)
    {
        IKAlpha = FMath::FInterpTo(IKAlpha, 0.f, World->GetDeltaSeconds(), 5.f);
        return;
    }

    IKAlpha = FMath::FInterpTo(IKAlpha, 1.f, World->GetDeltaSeconds(), 5.f);

    auto TraceFootIK = [&](FName SocketName) -> FVector
    {
        FVector SocketLoc = OwnerCharacter->GetMesh()
            ? OwnerCharacter->GetMesh()->GetSocketLocation(SocketName)
            : OwnerCharacter->GetActorLocation();

        FVector TraceStart = SocketLoc + FVector(0, 0, 50.f);
        FVector TraceEnd   = SocketLoc - FVector(0, 0, 75.f);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            return Hit.ImpactPoint;
        }
        return SocketLoc;
    };

    LeftFootIKLocation  = TraceFootIK(FName("foot_l"));
    RightFootIKLocation = TraceFootIK(FName("foot_r"));
}

void UDinoSurvivorAnimInstance::UpdateAimState()
{
    if (!OwnerCharacter) return;

    AController* Ctrl = OwnerCharacter->GetController();
    if (!Ctrl) return;

    FRotator CtrlRot  = Ctrl->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator Delta    = UKismetMathLibrary::NormalizedDeltaRotator(CtrlRot, ActorRot);

    AimPitch = FMath::ClampAngle(Delta.Pitch, -90.f, 90.f);
    AimYaw   = FMath::ClampAngle(Delta.Yaw,   -90.f, 90.f);
}
