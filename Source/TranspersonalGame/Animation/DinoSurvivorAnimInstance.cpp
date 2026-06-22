#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    Anim_LocomotionState = EAnim_LocomotionState::Idle;
    Anim_CombatStance = EAnim_CombatStance::Unarmed;
    Anim_Speed = 0.0f;
    Anim_Direction = 0.0f;
    Anim_bIsInAir = false;
    Anim_bIsCrouching = false;
    Anim_bIsSprinting = false;
    Anim_AimPitch = 0.0f;
    Anim_AimYaw = 0.0f;
    Anim_StaminaNormalized = 1.0f;
    Anim_HealthNormalized = 1.0f;
    Anim_FearNormalized = 0.0f;
    Anim_bIsAttacking = false;
    Anim_bIsBlocking = false;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    Anim_OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!Anim_OwnerCharacter)
    {
        Anim_OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (!Anim_OwnerCharacter) return;
    }

    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateAimOffset();
    UpdateSurvivalStats();
}

void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
    if (!Anim_OwnerCharacter) return;

    UCharacterMovementComponent* MovComp = Anim_OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    FVector Velocity = Anim_OwnerCharacter->GetVelocity();
    Anim_Speed = Velocity.Size2D();

    // Direction: angle between character forward and velocity
    FRotator ActorRot = Anim_OwnerCharacter->GetActorRotation();
    FRotator VelRot = UKismetMathLibrary::MakeRotFromX(Velocity);
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot);
    Anim_Direction = DeltaRot.Yaw;

    Anim_bIsInAir = MovComp->IsFalling();
    Anim_bIsCrouching = Anim_OwnerCharacter->bIsCrouched;

    // Sprint detection: speed above sprint threshold
    Anim_bIsSprinting = (Anim_Speed >= Anim_SprintSpeedThreshold);

    // State machine
    if (Anim_bIsInAir)
    {
        Anim_LocomotionState = (Velocity.Z > 0.0f)
            ? EAnim_LocomotionState::Jump
            : EAnim_LocomotionState::Fall;
    }
    else if (Anim_bIsCrouching)
    {
        Anim_LocomotionState = (Anim_Speed > Anim_WalkSpeedThreshold)
            ? EAnim_LocomotionState::CrouchWalk
            : EAnim_LocomotionState::Crouch;
    }
    else if (Anim_Speed >= Anim_SprintSpeedThreshold)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Anim_Speed >= Anim_RunSpeedThreshold)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Run;
    }
    else if (Anim_Speed >= Anim_WalkSpeedThreshold)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Walk;
    }
    else
    {
        Anim_LocomotionState = EAnim_LocomotionState::Idle;
    }
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!Anim_OwnerCharacter) return;
    if (Anim_bIsInAir)
    {
        // Disable IK in air — blend out
        Anim_FootIK.LeftFootAlpha = FMath::FInterpTo(Anim_FootIK.LeftFootAlpha, 0.0f, DeltaSeconds, 10.0f);
        Anim_FootIK.RightFootAlpha = FMath::FInterpTo(Anim_FootIK.RightFootAlpha, 0.0f, DeltaSeconds, 10.0f);
        Anim_FootIK.PelvisOffset = FMath::FInterpTo(Anim_FootIK.PelvisOffset, 0.0f, DeltaSeconds, 10.0f);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    auto TraceFootIK = [&](FName SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
    {
        FVector SocketLoc = Anim_OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
        FVector TraceStart = SocketLoc + FVector(0.0f, 0.0f, Anim_FootIKTraceDistance);
        FVector TraceEnd = SocketLoc - FVector(0.0f, 0.0f, Anim_FootIKTraceDistance);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(Anim_OwnerCharacter);

        bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
        if (bHit)
        {
            OutLocation = Hit.ImpactPoint;
            OutRotation = FRotator(
                FMath::RadiansToDegrees(FMath::Atan2(Hit.ImpactNormal.X, Hit.ImpactNormal.Z)) * -1.0f,
                0.0f,
                FMath::RadiansToDegrees(FMath::Atan2(Hit.ImpactNormal.Y, Hit.ImpactNormal.Z))
            );
            OutAlpha = FMath::FInterpTo(OutAlpha, 1.0f, DeltaSeconds, 15.0f);
        }
        else
        {
            OutAlpha = FMath::FInterpTo(OutAlpha, 0.0f, DeltaSeconds, 15.0f);
        }
    };

    TraceFootIK(FName("foot_l"), Anim_FootIK.LeftFootLocation, Anim_FootIK.LeftFootRotation, Anim_FootIK.LeftFootAlpha);
    TraceFootIK(FName("foot_r"), Anim_FootIK.RightFootLocation, Anim_FootIK.RightFootRotation, Anim_FootIK.RightFootAlpha);

    // Pelvis offset: push pelvis down to the lowest foot
    float LeftDelta = Anim_FootIK.LeftFootLocation.Z - Anim_OwnerCharacter->GetActorLocation().Z;
    float RightDelta = Anim_FootIK.RightFootLocation.Z - Anim_OwnerCharacter->GetActorLocation().Z;
    float TargetPelvis = FMath::Min(LeftDelta, RightDelta);
    Anim_FootIK.PelvisOffset = FMath::FInterpTo(Anim_FootIK.PelvisOffset, TargetPelvis, DeltaSeconds, 10.0f);
}

void UDinoSurvivorAnimInstance::UpdateAimOffset()
{
    if (!Anim_OwnerCharacter) return;

    AController* Controller = Anim_OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot = Anim_OwnerCharacter->GetActorRotation();
    FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    Anim_AimPitch = FMath::ClampAngle(Delta.Pitch, -90.0f, 90.0f);
    Anim_AimYaw = FMath::ClampAngle(Delta.Yaw, -90.0f, 90.0f);
}

void UDinoSurvivorAnimInstance::UpdateSurvivalStats()
{
    // Survival stats are set externally by the character/game state
    // Clamp to valid range
    Anim_StaminaNormalized = FMath::Clamp(Anim_StaminaNormalized, 0.0f, 1.0f);
    Anim_HealthNormalized = FMath::Clamp(Anim_HealthNormalized, 0.0f, 1.0f);
    Anim_FearNormalized = FMath::Clamp(Anim_FearNormalized, 0.0f, 1.0f);
}
