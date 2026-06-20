#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    Anim_FootIKTraceDistance = 50.0f;
    Anim_bFootIKEnabled = true;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        Anim_OwnerCharacter = Cast<ACharacter>(OwnerPawn);
        if (Anim_OwnerCharacter)
        {
            Anim_MovementComponent = Anim_OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!Anim_OwnerCharacter || !Anim_MovementComponent)
    {
        // Re-cache if lost
        APawn* OwnerPawn = TryGetPawnOwner();
        if (OwnerPawn)
        {
            Anim_OwnerCharacter = Cast<ACharacter>(OwnerPawn);
            if (Anim_OwnerCharacter)
            {
                Anim_MovementComponent = Anim_OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    UpdateLocomotionState();
    UpdateAimOffset();
    UpdateSurvivalState();

    if (Anim_bFootIKEnabled && !Anim_bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Reset foot IK when in air
        Anim_FootIK.LeftFootAlpha = 0.0f;
        Anim_FootIK.RightFootAlpha = 0.0f;
        Anim_FootIK.PelvisOffset = 0.0f;
    }
}

void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
    if (!Anim_OwnerCharacter || !Anim_MovementComponent) return;

    // Velocity and speed
    FVector Velocity = Anim_MovementComponent->Velocity;
    Anim_Speed = Velocity.Size2D();
    Anim_bIsMoving = Anim_Speed > 3.0f;
    Anim_bIsInAir = Anim_MovementComponent->IsFalling();
    Anim_bIsCrouching = Anim_OwnerCharacter->bIsCrouched;
    Anim_FallSpeed = Velocity.Z;

    // Direction (relative to character facing)
    if (Anim_bIsMoving)
    {
        FRotator ActorRot = Anim_OwnerCharacter->GetActorRotation();
        FRotator VelocityRot = Velocity.Rotation();
        Anim_Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        Anim_Direction = 0.0f;
    }

    // Determine locomotion state
    if (Anim_bIsInAir)
    {
        Anim_LocomotionState = (Anim_FallSpeed > 0.0f)
            ? EAnim_LocomotionState::Jump
            : EAnim_LocomotionState::Fall;
    }
    else if (Anim_bIsCrouching)
    {
        Anim_LocomotionState = Anim_bIsMoving
            ? EAnim_LocomotionState::CrouchWalk
            : EAnim_LocomotionState::Crouch;
    }
    else if (!Anim_bIsMoving)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (Anim_bIsSprinting || Anim_Speed > 500.0f)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Anim_Speed > 250.0f)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        Anim_LocomotionState = EAnim_LocomotionState::Walk;
    }
}

void UDinoSurvivorAnimInstance::UpdateAimOffset()
{
    if (!Anim_OwnerCharacter) return;

    AController* Controller = Anim_OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot = Anim_OwnerCharacter->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    // Clamp to valid aim offset range
    Anim_AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    Anim_AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.0f, 90.0f);
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!Anim_OwnerCharacter) return;

    FVector LeftLoc, RightLoc;
    FRotator LeftRot, RightRot;
    float LeftAlpha, RightAlpha;

    bool bLeftHit = TraceFootIK(FName("foot_l"), LeftLoc, LeftRot, LeftAlpha);
    bool bRightHit = TraceFootIK(FName("foot_r"), RightLoc, RightRot, RightAlpha);

    // Smooth interpolation for foot positions
    float InterpSpeed = 15.0f;
    Anim_FootIK.LeftFootLocation = FMath::VInterpTo(Anim_FootIK.LeftFootLocation, LeftLoc, DeltaSeconds, InterpSpeed);
    Anim_FootIK.RightFootLocation = FMath::VInterpTo(Anim_FootIK.RightFootLocation, RightLoc, DeltaSeconds, InterpSpeed);
    Anim_FootIK.LeftFootRotation = FMath::RInterpTo(Anim_FootIK.LeftFootRotation, LeftRot, DeltaSeconds, InterpSpeed);
    Anim_FootIK.RightFootRotation = FMath::RInterpTo(Anim_FootIK.RightFootRotation, RightRot, DeltaSeconds, InterpSpeed);
    Anim_FootIK.LeftFootAlpha = FMath::FInterpTo(Anim_FootIK.LeftFootAlpha, LeftAlpha, DeltaSeconds, InterpSpeed);
    Anim_FootIK.RightFootAlpha = FMath::FInterpTo(Anim_FootIK.RightFootAlpha, RightAlpha, DeltaSeconds, InterpSpeed);

    // Pelvis offset — lower pelvis to accommodate higher foot
    float MaxOffset = FMath::Min(Anim_FootIK.LeftFootLocation.Z, Anim_FootIK.RightFootLocation.Z);
    Anim_FootIK.PelvisOffset = FMath::FInterpTo(Anim_FootIK.PelvisOffset, MaxOffset, DeltaSeconds, InterpSpeed * 0.5f);
}

bool UDinoSurvivorAnimInstance::TraceFootIK(const FName& BoneName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
{
    if (!Anim_OwnerCharacter) return false;

    USkeletalMeshComponent* Mesh = Anim_OwnerCharacter->GetMesh();
    if (!Mesh) return false;

    FVector BoneLocation = Mesh->GetBoneLocation(BoneName);
    FVector TraceStart = BoneLocation + FVector(0.0f, 0.0f, Anim_FootIKTraceDistance);
    FVector TraceEnd = BoneLocation - FVector(0.0f, 0.0f, Anim_FootIKTraceDistance * 2.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Anim_OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        Params
    );

    if (bHit)
    {
        // Offset relative to character root
        FVector CharLocation = Anim_OwnerCharacter->GetActorLocation();
        OutLocation = FVector(0.0f, 0.0f, HitResult.ImpactPoint.Z - CharLocation.Z);

        // Foot rotation from surface normal
        FVector Normal = HitResult.ImpactNormal;
        OutRotation = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z)) * -1.0f
        );
        OutAlpha = 1.0f;
        return true;
    }

    OutLocation = FVector::ZeroVector;
    OutRotation = FRotator::ZeroRotator;
    OutAlpha = 0.0f;
    return false;
}

void UDinoSurvivorAnimInstance::UpdateSurvivalState()
{
    // Exhaustion threshold: stamina below 20%
    Anim_bIsExhausted = Anim_StaminaNormalized < 0.2f;

    // Injured threshold: health below 30%
    Anim_bIsInjured = Anim_HealthNormalized < 0.3f;

    // Reduce sprint if exhausted
    if (Anim_bIsExhausted && Anim_LocomotionState == EAnim_LocomotionState::Sprint)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Run;
    }
}
