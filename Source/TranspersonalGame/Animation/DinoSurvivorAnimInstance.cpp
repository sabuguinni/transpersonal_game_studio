#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    LocomotionState = EAnim_LocomotionState::Idle;
    CombatStance = EAnim_CombatStance::Unarmed;
    SurvivalCondition = EAnim_SurvivalCondition::Healthy;
    bEnableFootIK = true;
    UpperBodyLayerWeight = 1.0f;
    AdditiveBreathingWeight = 1.0f;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

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

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
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
        return;
    }

    UpdateLocomotionState(DeltaSeconds);
    UpdateIKFootPlacement(DeltaSeconds);
    UpdateSurvivalBlends(DeltaSeconds);
    UpdateAimOffsets(DeltaSeconds);
}

void UDinoSurvivorAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Thread-safe property reads only — no world queries here
}

// ── Locomotion ───────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    const FVector Velocity = MovementComponent->Velocity;
    GroundSpeed = Velocity.Size2D();
    bIsMoving = GroundSpeed > 3.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Direction relative to character facing
    if (bIsMoving)
    {
        const FRotator ActorRot = OwnerCharacter->GetActorRotation();
        const FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
        MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
    }

    // Locomotion state machine
    if (bIsInAir)
    {
        const float VertVel = Velocity.Z;
        LocomotionData.VerticalVelocity = VertVel;
        JumpVelocity = VertVel;

        if (VertVel > 0.0f)
        {
            LocomotionState = EAnim_LocomotionState::Jump;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Fall;
        }

        // Landing recovery
        if (bWasInAir && !bIsInAir)
        {
            LocomotionState = EAnim_LocomotionState::Land;
            LandingRecoveryTimer = 0.35f;
        }
        bWasInAir = true;
    }
    else
    {
        bWasInAir = false;

        if (LandingRecoveryTimer > 0.0f)
        {
            LandingRecoveryTimer -= DeltaSeconds;
            LocomotionState = EAnim_LocomotionState::Land;
        }
        else if (bIsCrouching)
        {
            LocomotionState = bIsMoving ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
        }
        else if (!bIsMoving)
        {
            LocomotionState = EAnim_LocomotionState::Idle;
        }
        else
        {
            // Walk/Run/Sprint thresholds (cm/s)
            // Walk: 0-200, Run: 200-500, Sprint: 500+
            if (GroundSpeed < 200.0f)
            {
                LocomotionState = EAnim_LocomotionState::Walk;
            }
            else if (GroundSpeed < 500.0f)
            {
                LocomotionState = EAnim_LocomotionState::Run;
            }
            else
            {
                LocomotionState = EAnim_LocomotionState::Sprint;
            }
        }
    }

    // Walk/Run blend alpha for BlendSpace1D (0=walk, 1=run)
    WalkRunAlpha = FMath::Clamp((GroundSpeed - 200.0f) / 300.0f, 0.0f, 1.0f);

    // Lean angle based on acceleration
    const FVector Acceleration = MovementComponent->GetCurrentAcceleration();
    const float AccelMagnitude = Acceleration.Size2D();
    const float TargetLean = bIsMoving ? FMath::Clamp(AccelMagnitude / MovementComponent->GetMaxAcceleration() * 8.0f, -8.0f, 8.0f) : 0.0f;
    LocomotionData.LeanAngle = FMath::FInterpTo(LocomotionData.LeanAngle, TargetLean, DeltaSeconds, 6.0f);

    // Update locomotion data struct
    LocomotionData.Speed = GroundSpeed;
    LocomotionData.Direction = MovementDirection;
    LocomotionData.bIsInAir = bIsInAir;
    LocomotionData.bIsCrouching = bIsCrouching;
    LocomotionData.bIsAccelerating = AccelMagnitude > 10.0f;
    LocomotionData.VerticalVelocity = Velocity.Z;

    PreviousSpeed = GroundSpeed;
}

// ── IK Foot Placement ────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateIKFootPlacement(float DeltaSeconds)
{
    if (!bEnableFootIK || !OwnerCharacter || bIsInAir) return;

    SolveFootIK(true,  IKData.LeftFootLocation,  IKData.LeftFootRotation,  IKData.LeftFootAlpha);
    SolveFootIK(false, IKData.RightFootLocation, IKData.RightFootRotation, IKData.RightFootAlpha);

    // Pelvis offset = lower pelvis to accommodate the lower foot
    const float LeftZ  = IKData.LeftFootLocation.Z;
    const float RightZ = IKData.RightFootLocation.Z;
    const float LowestFoot = FMath::Min(LeftZ, RightZ);
    const float CharZ = OwnerCharacter->GetActorLocation().Z;
    const float TargetPelvisOffset = FMath::Clamp(LowestFoot - CharZ, -30.0f, 0.0f);
    IKData.PelvisOffset = FMath::FInterpTo(IKData.PelvisOffset, TargetPelvisOffset, DeltaSeconds, 12.0f);
}

void UDinoSurvivorAnimInstance::SolveFootIK(bool bIsLeftFoot, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
{
    if (!OwnerCharacter) return;

    const FName SocketName = bIsLeftFoot ? FName("foot_l") : FName("foot_r");
    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    const FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart = SocketLoc + FVector(0, 0, 50.0f);
    const FVector TraceEnd   = SocketLoc - FVector(0, 0, 75.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {
        OutLocation = HitResult.ImpactPoint;
        // Align foot to surface normal
        const FVector SurfaceNormal = HitResult.ImpactNormal;
        const FRotator SurfaceRot = UKismetMathLibrary::MakeRotFromZX(SurfaceNormal, OwnerCharacter->GetActorForwardVector());
        OutRotation = FMath::RInterpTo(OutRotation, SurfaceRot, GetWorld()->GetDeltaSeconds(), 15.0f);
        OutAlpha = FMath::FInterpTo(OutAlpha, 1.0f, GetWorld()->GetDeltaSeconds(), 10.0f);
    }
    else
    {
        OutAlpha = FMath::FInterpTo(OutAlpha, 0.0f, GetWorld()->GetDeltaSeconds(), 10.0f);
    }
}

// ── Survival Blends ──────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalBlends(float DeltaSeconds)
{
    // Smooth interpolation of survival condition blends
    // These are driven externally via UpdateSurvivalCondition()
    const float InterpSpeed = 2.0f;

    // Breathing rate scales with exhaustion
    const float TargetBreathWeight = 1.0f + ExhaustionAlpha * 0.5f;
    AdditiveBreathingWeight = FMath::FInterpTo(AdditiveBreathingWeight, TargetBreathWeight, DeltaSeconds, InterpSpeed);

    // Upper body weight reduced when severely wounded
    const float TargetUpperWeight = FMath::Clamp(1.0f - WoundAlpha * 0.3f, 0.5f, 1.0f);
    UpperBodyLayerWeight = FMath::FInterpTo(UpperBodyLayerWeight, TargetUpperWeight, DeltaSeconds, InterpSpeed);
}

void UDinoSurvivorAnimInstance::UpdateAimOffsets(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Aim pitch/yaw for upper body aiming (used in AimOffset BlendSpace)
    const FRotator ControlRot = OwnerCharacter->GetControlRotation();
    const FRotator ActorRot   = OwnerCharacter->GetActorRotation();
    const FRotator DeltaRot   = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::ClampAngle(DeltaRot.Yaw,   -90.0f, 90.0f);
}

// ── Public API ───────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::SetCombatStance(EAnim_CombatStance NewStance)
{
    CombatStance = NewStance;
}

void UDinoSurvivorAnimInstance::TriggerAttackMontage()
{
    bIsAttacking = true;
    // Montage playback is handled by the AnimBlueprint via notify events
    // Reset after a frame to allow re-triggering
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        bIsAttacking = false;
    });
}

void UDinoSurvivorAnimInstance::TriggerDeathMontage()
{
    LocomotionState = EAnim_LocomotionState::Dead;
}

void UDinoSurvivorAnimInstance::UpdateSurvivalCondition(float Health, float Stamina, float Hunger, float Fear)
{
    // Normalise inputs to [0,1] assuming max values of 100
    const float HealthN  = FMath::Clamp(Health  / 100.0f, 0.0f, 1.0f);
    const float StaminaN = FMath::Clamp(Stamina / 100.0f, 0.0f, 1.0f);
    const float HungerN  = FMath::Clamp(Hunger  / 100.0f, 0.0f, 1.0f);
    const float FearN    = FMath::Clamp(Fear    / 100.0f, 0.0f, 1.0f);

    WoundAlpha      = 1.0f - HealthN;
    ExhaustionAlpha = 1.0f - StaminaN;
    StarvationAlpha = 1.0f - HungerN;
    FearAlpha       = FearN;

    // Determine dominant condition
    if (HealthN < 0.25f)
    {
        SurvivalCondition = EAnim_SurvivalCondition::Wounded;
    }
    else if (StaminaN < 0.2f)
    {
        SurvivalCondition = EAnim_SurvivalCondition::Exhausted;
    }
    else if (HungerN < 0.15f)
    {
        SurvivalCondition = EAnim_SurvivalCondition::Starving;
    }
    else if (FearN > 0.75f)
    {
        SurvivalCondition = EAnim_SurvivalCondition::Terrified;
    }
    else
    {
        SurvivalCondition = EAnim_SurvivalCondition::Healthy;
    }
}

bool UDinoSurvivorAnimInstance::ShouldPlayLimpAnimation() const
{
    return WoundAlpha > 0.5f && LocomotionState != EAnim_LocomotionState::Dead;
}

float UDinoSurvivorAnimInstance::GetLocomotionBlendAlpha() const
{
    return WalkRunAlpha;
}
