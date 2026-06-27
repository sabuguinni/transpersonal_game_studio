#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Movement defaults
    MovementState = EAnim_MovementState::Idle;
    CombatState = EAnim_CombatState::Unarmed;
    GroundSpeed = 0.0f;
    VelocityMagnitude = 0.0f;
    MovementDirection = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsMoving = false;

    // Survival defaults
    HealthRatio = 1.0f;
    StaminaRatio = 1.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;

    // IK defaults
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    PelvisOffset = 0.0f;
    IKAlpha = 0.0f;
    SmoothedPelvisOffset = 0.0f;

    // Lean/Aim defaults
    LeanAmount = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // Config defaults
    MovementThreshold = 3.0f;
    RunThreshold = 300.0f;
    ExhaustionThreshold = 0.15f;
    IKTraceDistance = 75.0f;
    LeanSmoothSpeed = 5.0f;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    PreviousVelocity = FVector::ZeroVector;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner and movement component
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

void UTranspersonalAnimInstance::NativeBeginPlay()
{
    Super::NativeBeginPlay();

    // Re-cache in case NativeInitializeAnimation was too early
    if (!OwnerCharacter)
    {
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
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    UpdateMovementState();
    UpdateSurvivalState();
    UpdateLeanAndAim(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
}

void UTranspersonalAnimInstance::UpdateMovementState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    FVector Velocity = OwnerCharacter->GetVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);

    GroundSpeed = HorizontalVelocity.Size();
    VelocityMagnitude = Velocity.Size();

    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsMoving = GroundSpeed > MovementThreshold;
    bIsSprinting = GroundSpeed > RunThreshold;

    // Calculate movement direction relative to character facing
    if (bIsMoving)
    {
        FRotator CharacterRotation = OwnerCharacter->GetActorRotation();
        FRotator VelocityRotation = HorizontalVelocity.Rotation();
        FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, CharacterRotation);
        MovementDirection = DeltaRotation.Yaw;
    }
    else
    {
        MovementDirection = 0.0f;
    }

    // Determine movement state
    if (bIsInAir)
    {
        if (Velocity.Z > 0.0f)
        {
            MovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            MovementState = EAnim_MovementState::Falling;
        }
    }
    else if (bIsCrouching)
    {
        MovementState = EAnim_MovementState::Crouching;
    }
    else if (bIsSprinting)
    {
        MovementState = EAnim_MovementState::Running;
    }
    else if (bIsMoving)
    {
        MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        MovementState = EAnim_MovementState::Idle;
    }

    // IK alpha — disable in air, full on ground
    float TargetIKAlpha = bIsInAir ? 0.0f : 1.0f;
    IKAlpha = FMath::FInterpTo(IKAlpha, TargetIKAlpha, GetWorld()->GetDeltaSeconds(), 8.0f);
}

void UTranspersonalAnimInstance::UpdateSurvivalState()
{
    // Try to read survival stats from the character
    // These are exposed as properties on TranspersonalCharacter
    // We use a safe property access pattern to avoid hard dependency
    if (!OwnerCharacter)
    {
        return;
    }

    // Default values — will be overridden by Blueprint or property binding
    // In production, this would use a USurvivalComponent interface
    // For now, we maintain the cached values and let Blueprint update them

    bIsExhausted = StaminaRatio < ExhaustionThreshold;
}

void UTranspersonalAnimInstance::UpdateLeanAndAim(float DeltaSeconds)
{
    if (!OwnerCharacter)
    {
        return;
    }

    // Lean calculation — based on velocity change (lateral acceleration)
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    FVector VelocityDelta = CurrentVelocity - PreviousVelocity;
    PreviousVelocity = CurrentVelocity;

    // Project delta onto character's right axis for lean
    FVector RightAxis = OwnerCharacter->GetActorRightVector();
    float LateralAccel = FVector::DotProduct(VelocityDelta, RightAxis);
    float TargetLean = FMath::Clamp(LateralAccel * 0.005f, -1.0f, 1.0f);
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, LeanSmoothSpeed);

    // Aim offset — based on control rotation vs actor rotation
    FRotator ControlRot = OwnerCharacter->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir || IKAlpha < 0.01f)
    {
        return;
    }

    FVector LeftLoc = FVector::ZeroVector;
    FVector RightLoc = FVector::ZeroVector;
    FRotator LeftRot = FRotator::ZeroRotator;
    FRotator RightRot = FRotator::ZeroRotator;

    bool bLeftHit = TraceFootIK(FName("foot_l"), LeftLoc, LeftRot);
    bool bRightHit = TraceFootIK(FName("foot_r"), RightLoc, RightRot);

    if (bLeftHit)
    {
        LeftFootIKLocation = LeftLoc;
        LeftFootIKRotation = LeftRot;
    }

    if (bRightHit)
    {
        RightFootIKLocation = RightLoc;
        RightFootIKRotation = RightRot;
    }

    // Pelvis offset — lower pelvis to the lowest foot
    float LeftOffset = bLeftHit ? (LeftLoc.Z - OwnerCharacter->GetActorLocation().Z) : 0.0f;
    float RightOffset = bRightHit ? (RightLoc.Z - OwnerCharacter->GetActorLocation().Z) : 0.0f;
    float TargetPelvisOffset = FMath::Min(LeftOffset, RightOffset);

    SmoothedPelvisOffset = FMath::FInterpTo(SmoothedPelvisOffset, TargetPelvisOffset, DeltaSeconds, 10.0f);
    PelvisOffset = SmoothedPelvisOffset;
}

bool UTranspersonalAnimInstance::TraceFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation)
{
    if (!OwnerCharacter)
    {
        return false;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        return false;
    }

    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    FVector TraceStart = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z + IKTraceDistance);
    FVector TraceEnd = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z - IKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutLocation = HitResult.ImpactPoint;

        // Calculate foot rotation from surface normal
        FVector Normal = HitResult.ImpactNormal;
        FRotator SurfaceRotation = UKismetMathLibrary::MakeRotFromZX(Normal, OwnerCharacter->GetActorForwardVector());
        OutRotation = SurfaceRotation;

        return true;
    }

    return false;
}

float UTranspersonalAnimInstance::GetLocomotionBlendWeight() const
{
    // Returns normalized blend weight for walk/run blend space
    // 0.0 = idle/walk, 1.0 = full sprint
    if (RunThreshold <= MovementThreshold)
    {
        return 0.0f;
    }
    return FMath::Clamp((GroundSpeed - MovementThreshold) / (RunThreshold - MovementThreshold), 0.0f, 1.0f);
}

bool UTranspersonalAnimInstance::IsSlotActive(FName SlotName) const
{
    return IsAnyMontagePlaying();
}
