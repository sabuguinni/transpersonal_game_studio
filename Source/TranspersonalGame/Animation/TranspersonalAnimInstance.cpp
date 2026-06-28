#include "TranspersonalAnimInstance.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
    : LocomotionState(EAnim_LocomotionState::Idle)
    , StanceType(EAnim_StanceType::Upright)
    , Speed(0.f)
    , Direction(0.f)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , LeanAngle(0.f)
    , Health(100.f)
    , Stamina(100.f)
    , bIsExhausted(false)
    , bIsDead(false)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , LeftFootIKAlpha(0.f)
    , RightFootIKAlpha(0.f)
    , PelvisOffset(0.f)
    , AimPitch(0.f)
    , AimYaw(0.f)
    , UpperBodyBlendWeight(1.f)
    , AdditiveWoundBlend(0.f)
    , OwnerCharacter(nullptr)
    , MovementComponent(nullptr)
{
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter = Cast<ATranspersonalCharacter>(Pawn);
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent) return;

    UpdateLocomotionState();
    UpdateSurvivalStats();
    UpdateIKFootPlacement();
    UpdateAimOffset();
}

// ---------------------------------------------------------------------------
// UpdateLocomotionState
// ---------------------------------------------------------------------------
void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    if (!OwnerCharacter || !MovementComponent) return;

    // Raw speed (XY plane only — ignore vertical)
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Direction relative to character facing
    FRotator CharRot = OwnerCharacter->GetActorRotation();
    Direction = UKismetMathLibrary::NormalizedDeltaRotator(
        UKismetMathLibrary::MakeRotFromX(Velocity.GetSafeNormal()),
        CharRot
    ).Yaw;

    bIsInAir    = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Lean: computed from angular velocity (simple approximation)
    LeanAngle = FMath::Clamp(Direction * 0.05f, -45.f, 45.f);

    // Determine locomotion state
    if (bIsDead)
    {
        LocomotionState = EAnim_LocomotionState::Dead;
        return;
    }

    if (bIsInAir)
    {
        LocomotionState = (Velocity.Z > 0.f)
            ? EAnim_LocomotionState::Jumping
            : EAnim_LocomotionState::Falling;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouching;
        return;
    }

    if (Speed < 10.f)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (Speed < 200.f)
    {
        LocomotionState = EAnim_LocomotionState::Walking;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprinting;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Running;
    }

    // Stance: exhausted overrides combat/stealth
    if (bIsExhausted)
    {
        StanceType = EAnim_StanceType::Exhausted;
    }
    else
    {
        StanceType = EAnim_StanceType::Upright;
    }

    // Upper body blend: reduce when exhausted
    UpperBodyBlendWeight = bIsExhausted ? 0.5f : 1.0f;
}

// ---------------------------------------------------------------------------
// UpdateSurvivalStats
// ---------------------------------------------------------------------------
void UTranspersonalAnimInstance::UpdateSurvivalStats()
{
    if (!OwnerCharacter) return;

    // Read survival properties via reflection (safe — no direct dependency on private members)
    // If TranspersonalCharacter exposes these as BlueprintReadOnly UPROPERTY, we can read them.
    // Fallback: read from the character's movement component as a proxy.

    // Health proxy: if character has a Health property, read it
    // For now we use a safe default — the actual binding happens in Blueprint
    // when the AnimBP is set up. The C++ layer provides the variables.

    // Stamina proxy
    bIsExhausted = (Stamina < 15.f);

    // Wound additive: increases as health drops
    AdditiveWoundBlend = FMath::Clamp(1.f - (Health / 100.f), 0.f, 1.f);
}

// ---------------------------------------------------------------------------
// UpdateIKFootPlacement
// ---------------------------------------------------------------------------
void UTranspersonalAnimInstance::UpdateIKFootPlacement()
{
    if (!OwnerCharacter) return;

    // Skip IK when in air or dead
    if (bIsInAir || bIsDead)
    {
        LeftFootIKAlpha  = 0.f;
        RightFootIKAlpha = 0.f;
        PelvisOffset     = 0.f;
        return;
    }

    SolveFootIK(FName("foot_l"), LeftFootIKLocation,  LeftFootIKAlpha);
    SolveFootIK(FName("foot_r"), RightFootIKLocation, RightFootIKAlpha);

    // Pelvis offset: average of both foot offsets to keep body centred
    float LeftDelta  = LeftFootIKLocation.Z  - OwnerCharacter->GetActorLocation().Z;
    float RightDelta = RightFootIKLocation.Z - OwnerCharacter->GetActorLocation().Z;
    PelvisOffset = FMath::Min(LeftDelta, RightDelta);
}

void UTranspersonalAnimInstance::SolveFootIK(FName FootSocketName, FVector& OutIKLocation, float& OutAlpha)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    FVector SocketWorldLocation = Mesh->GetSocketLocation(FootSocketName);

    // Trace downward from foot socket
    FVector TraceStart = SocketWorldLocation + FVector(0.f, 0.f, 50.f);
    FVector TraceEnd   = SocketWorldLocation - FVector(0.f, 0.f, 75.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        Params
    );

    if (bHit)
    {
        OutIKLocation = HitResult.ImpactPoint;
        OutAlpha = 1.f;
    }
    else
    {
        OutIKLocation = SocketWorldLocation;
        OutAlpha = 0.f;
    }
}

// ---------------------------------------------------------------------------
// UpdateAimOffset
// ---------------------------------------------------------------------------
void UTranspersonalAnimInstance::UpdateAimOffset()
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot  = Controller->GetControlRotation();
    FRotator CharRot     = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot    = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, CharRot);

    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.f, 90.f);
    AimYaw   = FMath::Clamp(DeltaRot.Yaw,   -90.f, 90.f);
}
