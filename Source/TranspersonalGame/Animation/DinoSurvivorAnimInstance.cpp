#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
    : Speed(0.f)
    , Direction(0.f)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , LocomotionState(EAnim_LocomotionState::Idle)
    , CombatStance(EAnim_CombatStance::Unarmed)
    , bIsAttacking(false)
    , bIsBlocking(false)
    , StaminaRatio(1.f)
    , HealthRatio(1.f)
    , bIsExhausted(false)
    , bIsInjured(false)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , IKAlpha(0.f)
    , AimPitch(0.f)
    , AimYaw(0.f)
    , LeanAngle(0.f)
    , WalkSpeedThreshold(150.f)
    , RunSpeedThreshold(350.f)
    , SprintSpeedThreshold(550.f)
    , IKTraceDistance(80.f)
    , OwnerCharacter(nullptr)
    , MovementComponent(nullptr)
    , AttackCooldown(0.f)
    , PreviousSpeed(0.f)
{
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter = Cast<ACharacter>(Pawn);
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent) return;

    UpdateLocomotionState(DeltaSeconds);
    UpdateCombatState(DeltaSeconds);
    UpdateSurvivalState(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
// Locomotion
// ─────────────────────────────────────────────────────────────────────────────
void UDinoSurvivorAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    // Ground speed (ignore Z)
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Direction relative to actor forward (-180..180)
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    bIsInAir    = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Sprint: detect via max walk speed proxy
    bIsSprinting = (Speed > SprintSpeedThreshold);

    // Lean: rate of direction change (smoothed)
    float SpeedDelta = Speed - PreviousSpeed;
    LeanAngle = FMath::FInterpTo(LeanAngle, SpeedDelta * 0.5f, DeltaSeconds, 6.f);
    PreviousSpeed = Speed;

    // Determine locomotion state
    if (bIsInAir)
    {
        LocomotionState = EAnim_LocomotionState::InAir;
    }
    else if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouch;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > RunSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else if (Speed > WalkSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Combat
// ─────────────────────────────────────────────────────────────────────────────
void UDinoSurvivorAnimInstance::UpdateCombatState(float DeltaSeconds)
{
    // Tick attack cooldown
    if (AttackCooldown > 0.f)
    {
        AttackCooldown -= DeltaSeconds;
        if (AttackCooldown <= 0.f)
        {
            AttackCooldown = 0.f;
            bIsAttacking = false;
        }
    }
}

void UDinoSurvivorAnimInstance::SetCombatStance(EAnim_CombatStance NewStance)
{
    CombatStance = NewStance;
}

void UDinoSurvivorAnimInstance::TriggerAttack()
{
    if (AttackCooldown > 0.f) return;
    bIsAttacking = true;
    AttackCooldown = 0.6f; // 600ms attack window
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival stats
// ─────────────────────────────────────────────────────────────────────────────
void UDinoSurvivorAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
    // Exhaustion affects locomotion blend weight
    bIsExhausted = (StaminaRatio < 0.15f);
    bIsInjured   = (HealthRatio  < 0.30f);

    // IK alpha: disable when in air or exhausted
    float TargetIKAlpha = (bIsInAir || bIsExhausted) ? 0.f : 1.f;
    IKAlpha = FMath::FInterpTo(IKAlpha, TargetIKAlpha, DeltaSeconds, 8.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Foot IK — two-bone line traces to adapt feet to terrain
// ─────────────────────────────────────────────────────────────────────────────
void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || IKAlpha < 0.01f) return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    auto TraceFootSocket = [&](const FName& SocketName, FVector& OutLocation)
    {
        USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
        if (!Mesh) return;

        FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
        FVector TraceStart = SocketLoc + FVector(0.f, 0.f, IKTraceDistance);
        FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, IKTraceDistance);

        FHitResult Hit;
        FCollisionQueryParams Params(NAME_None, false, OwnerCharacter);
        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            OutLocation = Hit.ImpactPoint;
        }
        else
        {
            OutLocation = SocketLoc;
        }
    };

    TraceFootSocket(FName("foot_l"), LeftFootIKLocation);
    TraceFootSocket(FName("foot_r"), RightFootIKLocation);
}

// ─────────────────────────────────────────────────────────────────────────────
// Aim Offset — pitch/yaw for upper-body aiming
// ─────────────────────────────────────────────────────────────────────────────
void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    FRotator ControlRot  = OwnerCharacter->GetControlRotation();
    FRotator ActorRot    = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot    = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.f, 90.f);
    AimYaw   = FMath::ClampAngle(DeltaRot.Yaw,   -90.f, 90.f);
}
