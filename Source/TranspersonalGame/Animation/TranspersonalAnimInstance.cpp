#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    FootIKTraceDistance = 100.0f;
    FootIKInterpSpeed = 15.0f;
    WalkSpeedThreshold = 180.0f;
    RunSpeedThreshold = 450.0f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner character and movement component
    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        OwnerCharacter = Cast<ACharacter>(OwnerPawn);
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        // Re-attempt cache if not set
        APawn* OwnerPawn = TryGetPawnOwner();
        if (OwnerPawn)
        {
            OwnerCharacter = Cast<ACharacter>(OwnerPawn);
            if (OwnerCharacter)
            {
                MovementComponent = OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    // ─── Velocity & Speed ─────────────────────────────────────────────
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D(); // Horizontal speed only
    VerticalVelocity = Velocity.Z;

    // Direction (angle between character forward and velocity)
    if (Speed > 1.0f)
    {
        FRotator AimRotation = OwnerCharacter->GetActorRotation();
        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Velocity);
        FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
        Direction = DeltaRot.Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ─── Movement State ───────────────────────────────────────────────
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Acceleration magnitude for lean effects
    AccelerationMagnitude = MovementComponent->GetCurrentAcceleration().Size();

    // Landing detection
    if (bWasInAir && !bIsInAir)
    {
        LandingTimer = 0.35f; // Hold land state for 350ms
    }
    bWasInAir = bIsInAir;

    if (LandingTimer > 0.0f)
    {
        LandingTimer -= DeltaSeconds;
    }

    // ─── Locomotion State Machine ──────────────────────────────────────
    UpdateLocomotionState();

    // ─── Foot IK ──────────────────────────────────────────────────────
    if (!bIsInAir && !bIsDead)
    {
        UpdateFootIK(FName("foot_l"), LeftFootIK);
        UpdateFootIK(FName("foot_r"), RightFootIK);

        // Pelvis offset = lowest foot offset to prevent floating
        PelvisOffset = FMath::Min(LeftFootIK.EffectorOffset, RightFootIK.EffectorOffset);
        PelvisOffset = FMath::Clamp(PelvisOffset, -30.0f, 0.0f);
    }
    else
    {
        // Reset foot IK when in air
        LeftFootIK.EffectorOffset = FMath::FInterpTo(LeftFootIK.EffectorOffset, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        RightFootIK.EffectorOffset = FMath::FInterpTo(RightFootIK.EffectorOffset, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }
}

void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    PreviousLocomotionState = LocomotionState;

    if (bIsDead)
    {
        LocomotionState = EAnim_LocomotionState::Death;
        return;
    }

    if (LandingTimer > 0.0f)
    {
        LocomotionState = EAnim_LocomotionState::Land;
        return;
    }

    if (bIsInAir)
    {
        if (VerticalVelocity > 0.0f)
        {
            LocomotionState = EAnim_LocomotionState::Jump;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Fall;
        }
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouch;
        return;
    }

    // Speed-based locomotion
    // Low health causes forced walk (limping)
    float EffectiveRunThreshold = RunSpeedThreshold;
    if (HealthRatio < 0.3f)
    {
        EffectiveRunThreshold = 9999.0f; // Can't run when critically injured
    }

    if (Speed < 10.0f)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (Speed < WalkSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else if (Speed < EffectiveRunThreshold || !bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
}

void UTranspersonalAnimInstance::UpdateFootIK(const FName& FootBoneName, FAnim_FootIKData& OutFootData)
{
    if (!OwnerCharacter)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    // Get foot bone world location
    FVector FootWorldLocation = MeshComp->GetSocketLocation(FootBoneName);

    // Trace downward from foot
    FVector TraceStart = FootWorldLocation + FVector(0.0f, 0.0f, FootIKTraceDistance * 0.5f);
    FVector TraceEnd = FootWorldLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutFootData.bFootOnGround = true;

        // Calculate how much the foot needs to move down/up
        float HitZ = HitResult.ImpactPoint.Z;
        float FootZ = FootWorldLocation.Z;
        float TargetOffset = HitZ - FootZ;

        // Smooth interpolation
        OutFootData.EffectorOffset = FMath::FInterpTo(
            OutFootData.EffectorOffset,
            TargetOffset,
            GetWorld()->GetDeltaSeconds(),
            FootIKInterpSpeed
        );

        // Foot rotation from surface normal
        FVector Normal = HitResult.ImpactNormal;
        FRotator TargetRot = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(-Normal.Y, Normal.Z))
        );

        OutFootData.FootRotation = FMath::RInterpTo(
            OutFootData.FootRotation,
            TargetRot,
            GetWorld()->GetDeltaSeconds(),
            FootIKInterpSpeed
        );

        OutFootData.FootLocation = HitResult.ImpactPoint;
    }
    else
    {
        OutFootData.bFootOnGround = false;
        OutFootData.EffectorOffset = FMath::FInterpTo(
            OutFootData.EffectorOffset,
            0.0f,
            GetWorld()->GetDeltaSeconds(),
            FootIKInterpSpeed
        );
    }
}

void UTranspersonalAnimInstance::SetDead(bool bDead)
{
    bIsDead = bDead;
    if (bDead)
    {
        LocomotionState = EAnim_LocomotionState::Death;
    }
}

void UTranspersonalAnimInstance::SetCombatStance(bool bCombat)
{
    bIsInCombat = bCombat;
    // Smoothly blend upper body layer weight
    UpperBodyLayerWeight = bCombat ? 1.0f : 0.0f;
}

void UTranspersonalAnimInstance::UpdateSurvivalStats(float Health, float Stamina, float Fear)
{
    HealthRatio = FMath::Clamp(Health, 0.0f, 1.0f);
    StaminaRatio = FMath::Clamp(Stamina, 0.0f, 1.0f);
    FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);

    // Low stamina forces walk
    if (StaminaRatio < 0.1f)
    {
        bIsSprinting = false;
    }
}
