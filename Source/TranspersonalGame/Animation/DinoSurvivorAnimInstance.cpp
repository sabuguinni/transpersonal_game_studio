#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
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

    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateAimOffset();
    UpdateSurvivalState();
}

void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    FVector Velocity = OwnerCharacter->GetVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    Speed = HorizontalVelocity.Size();
    bIsMoving = Speed > 10.0f;
    bIsInAir = MovComp->IsFalling();
    bIsCrouching = MovComp->IsCrouching();

    // Calculate direction relative to character facing
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = ActorRot.UnrotateVector(HorizontalVelocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // Lean angle based on direction change rate
    LeanAngle = FMath::Clamp(Direction * 0.1f, -15.0f, 15.0f);

    // Determine locomotion state
    if (bIsInAir)
    {
        LocomotionState = Velocity.Z > 0.0f ? EAnim_LocomotionState::Jump : EAnim_LocomotionState::Fall;
    }
    else if (bIsCrouching)
    {
        LocomotionState = bIsMoving ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else if (bIsSprinting && Speed > 400.0f)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > 200.0f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else if (Speed > 10.0f)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    UWorld* World = GetWorld();
    if (!World) return;

    auto TraceFootIK = [&](FName SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha) -> bool
    {
        FVector SocketLoc = OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
        FVector TraceStart = FVector(SocketLoc.X, SocketLoc.Y, SocketLoc.Z + FootIKTraceDistance);
        FVector TraceEnd   = FVector(SocketLoc.X, SocketLoc.Y, SocketLoc.Z - FootIKTraceDistance);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
        if (bHit)
        {
            FVector TargetLoc = Hit.ImpactPoint;
            FRotator SurfaceRot = FRotator(
                FMath::RadiansToDegrees(FMath::Atan2(Hit.ImpactNormal.X, Hit.ImpactNormal.Z)) * -1.0f,
                0.0f,
                FMath::RadiansToDegrees(FMath::Atan2(Hit.ImpactNormal.Y, Hit.ImpactNormal.Z))
            );

            OutLocation = FMath::VInterpTo(OutLocation, TargetLoc, DeltaSeconds, FootIKInterpSpeed);
            OutRotation = FMath::RInterpTo(OutRotation, SurfaceRot, DeltaSeconds, FootIKInterpSpeed);
            OutAlpha = FMath::FInterpTo(OutAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
            return true;
        }
        else
        {
            OutAlpha = FMath::FInterpTo(OutAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
            return false;
        }
    };

    // Only apply foot IK when grounded and not moving fast
    if (!bIsInAir && Speed < 400.0f)
    {
        bool bLeftHit  = TraceFootIK(FName("foot_l"), FootIKData.LeftFootLocation,  FootIKData.LeftFootRotation,  FootIKData.LeftFootAlpha);
        bool bRightHit = TraceFootIK(FName("foot_r"), FootIKData.RightFootLocation, FootIKData.RightFootRotation, FootIKData.RightFootAlpha);

        // Pelvis offset: push pelvis down to the lower foot
        if (bLeftHit && bRightHit)
        {
            float LeftDelta  = FootIKData.LeftFootLocation.Z  - OwnerCharacter->GetActorLocation().Z;
            float RightDelta = FootIKData.RightFootLocation.Z - OwnerCharacter->GetActorLocation().Z;
            float MinDelta = FMath::Min(LeftDelta, RightDelta);
            FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, MinDelta, DeltaSeconds, FootIKInterpSpeed);
        }
    }
    else
    {
        FootIKData.LeftFootAlpha  = FMath::FInterpTo(FootIKData.LeftFootAlpha,  0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.PelvisOffset   = FMath::FInterpTo(FootIKData.PelvisOffset,   0.0f, DeltaSeconds, FootIKInterpSpeed);
    }
}

void UDinoSurvivorAnimInstance::UpdateAimOffset()
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot  = Controller->GetControlRotation();
    FRotator ActorRot    = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot    = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::Clamp(DeltaRot.Yaw,   -90.0f, 90.0f);
}

void UDinoSurvivorAnimInstance::UpdateSurvivalState()
{
    // Exhaustion affects animation blend
    bIsExhausted = StaminaNormalized < 0.2f;

    // Fear influences movement urgency (used in blend space weights)
    // High fear = more erratic movement, less controlled
}
