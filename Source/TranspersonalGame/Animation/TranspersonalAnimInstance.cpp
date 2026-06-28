#include "TranspersonalAnimInstance.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    LocomotionState = EAnim_LocomotionState::Idle;
    StanceType = EAnim_StanceType::Standing;
    Speed = 0.0f;
    Direction = 0.0f;
    VerticalVelocity = 0.0f;

    // State defaults
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsAttacking = false;
    bIsDead = false;
    bIsMoving = false;

    // IK defaults
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    PelvisOffset = 0.0f;

    // Survival defaults
    FatigueLevel = 0.0f;
    FearLevel = 0.0f;
    InjuryLevel = 0.0f;

    // Aim offset defaults
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // Internal
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        OwnerCharacter = Cast<ATranspersonalCharacter>(OwnerPawn);
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
        // Try to re-acquire references if lost
        APawn* OwnerPawn = TryGetPawnOwner();
        if (OwnerPawn)
        {
            OwnerCharacter = Cast<ATranspersonalCharacter>(OwnerPawn);
            if (OwnerCharacter)
            {
                MovementComponent = OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    // ─── Velocity & Speed ───────────────────────────────────────────────────
    const FVector Velocity = MovementComponent->Velocity;
    const FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    Speed = HorizontalVelocity.Size();
    VerticalVelocity = Velocity.Z;
    bIsMoving = Speed > MovingSpeedThreshold;

    // ─── Direction (strafe angle) ────────────────────────────────────────────
    if (bIsMoving)
    {
        const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        const FRotator VelocityRotation = UKismetMathLibrary::MakeRotFromX(HorizontalVelocity);
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ─── State Booleans ─────────────────────────────────────────────────────
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // ─── Locomotion State ───────────────────────────────────────────────────
    LocomotionState = DetermineLocomotionState();

    // ─── Stance ─────────────────────────────────────────────────────────────
    if (bIsCrouching)
    {
        StanceType = EAnim_StanceType::Crouched;
    }
    else
    {
        StanceType = EAnim_StanceType::Standing;
    }

    // ─── Aim Offset ─────────────────────────────────────────────────────────
    if (AController* Controller = OwnerCharacter->GetController())
    {
        const FRotator ControlRotation = Controller->GetControlRotation();
        const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);
        AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
        AimYaw = FMath::Clamp(DeltaRot.Yaw, -180.0f, 180.0f);
    }

    // ─── Foot IK ────────────────────────────────────────────────────────────
    // Only active when on ground and not in transition
    const bool bShouldDoFootIK = !bIsInAir && LocomotionState != EAnim_LocomotionState::Dead;
    if (bShouldDoFootIK)
    {
        UpdateFootIK(FName("foot_l"), LeftFootIKLocation, LeftFootIKAlpha);
        UpdateFootIK(FName("foot_r"), RightFootIKLocation, RightFootIKAlpha);
    }
    else
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, IKInterpSpeed);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, IKInterpSpeed);
    }
}

void UTranspersonalAnimInstance::UpdateFootIK(const FName& FootSocketName, FVector& OutIKLocation, float& OutIKAlpha)
{
    if (!OwnerCharacter)
    {
        OutIKAlpha = 0.0f;
        return;
    }

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        OutIKAlpha = 0.0f;
        return;
    }

    // Get foot socket world location
    const FVector FootLocation = MeshComp->GetSocketLocation(FootSocketName);

    // Trace downward from foot position
    const FVector TraceStart = FootLocation + FVector(0.0f, 0.0f, FootIKTraceDistance * 0.5f);
    const FVector TraceEnd = FootLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        OutIKAlpha = 0.0f;
        return;
    }

    const bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Smooth interpolation to hit location
        const FVector TargetLocation = HitResult.ImpactPoint;
        OutIKLocation = FMath::VInterpTo(OutIKLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
        OutIKAlpha = FMath::FInterpTo(OutIKAlpha, 1.0f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);

        // Compute pelvis offset from lowest foot
        const float FootDelta = TargetLocation.Z - FootLocation.Z;
        PelvisOffset = FMath::FInterpTo(PelvisOffset, FMath::Min(FootDelta, 0.0f), GetWorld()->GetDeltaSeconds(), IKInterpSpeed * 0.5f);
    }
    else
    {
        OutIKAlpha = FMath::FInterpTo(OutIKAlpha, 0.0f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
    }
}

EAnim_LocomotionState UTranspersonalAnimInstance::DetermineLocomotionState() const
{
    if (bIsDead)
    {
        return EAnim_LocomotionState::Dead;
    }

    if (bIsAttacking)
    {
        return EAnim_LocomotionState::Attacking;
    }

    if (bIsInAir)
    {
        return VerticalVelocity > 0.0f ? EAnim_LocomotionState::Jumping : EAnim_LocomotionState::Falling;
    }

    if (bIsCrouching)
    {
        return EAnim_LocomotionState::Crouching;
    }

    if (!bIsMoving)
    {
        return EAnim_LocomotionState::Idle;
    }

    if (bIsSprinting)
    {
        return EAnim_LocomotionState::Sprinting;
    }

    return EAnim_LocomotionState::Walking;
}
