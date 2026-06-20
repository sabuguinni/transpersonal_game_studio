#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    Anim_WalkSpeedThreshold = 10.0f;
    Anim_RunSpeedThreshold = 250.0f;
    Anim_SprintSpeedThreshold = 500.0f;
    Anim_FootIKTraceDistance = 60.0f;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        Anim_OwnerCharacter = Cast<ACharacter>(Pawn);
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
        APawn* Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            Anim_OwnerCharacter = Cast<ACharacter>(Pawn);
            if (Anim_OwnerCharacter)
            {
                Anim_MovementComponent = Anim_OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateAimOffset();
    UpdateSurvivalState();
}

void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
    if (!Anim_OwnerCharacter || !Anim_MovementComponent)
        return;

    // Speed and direction
    FVector Velocity = Anim_MovementComponent->Velocity;
    Anim_Speed = Velocity.Size2D();

    FRotator ActorRot = Anim_OwnerCharacter->GetActorRotation();
    FVector ForwardDir = ActorRot.Vector();
    FVector RightDir = FRotationMatrix(ActorRot).GetScaledAxis(EAxis::Y);

    if (Anim_Speed > 1.0f)
    {
        FVector VelNorm = Velocity.GetSafeNormal2D();
        float ForwardDot = FVector::DotProduct(ForwardDir, VelNorm);
        float RightDot = FVector::DotProduct(RightDir, VelNorm);
        Anim_Direction = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    }
    else
    {
        Anim_Direction = 0.0f;
    }

    // Air state
    Anim_bIsInAir = Anim_MovementComponent->IsFalling();

    // Crouch state
    Anim_bIsCrouching = Anim_MovementComponent->IsCrouching();

    // Lean (lateral acceleration)
    FVector Accel = Anim_MovementComponent->GetCurrentAcceleration();
    float LateralAccel = FVector::DotProduct(Accel, RightDir);
    Anim_LeanAngle = FMath::FInterpTo(Anim_LeanAngle, FMath::Clamp(LateralAccel * 0.01f, -15.0f, 15.0f), GetWorld()->GetDeltaSeconds(), 5.0f);

    // Determine locomotion state
    if (Anim_bIsInAir)
    {
        Anim_LocomotionState = (Velocity.Z > 0.0f) ? EAnim_LocomotionState::Jump : EAnim_LocomotionState::Fall;
    }
    else if (Anim_bIsCrouching)
    {
        Anim_LocomotionState = (Anim_Speed > Anim_WalkSpeedThreshold) ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else if (Anim_Speed >= Anim_SprintSpeedThreshold)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Sprint;
        Anim_bIsSprinting = true;
    }
    else if (Anim_Speed >= Anim_RunSpeedThreshold)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Run;
        Anim_bIsSprinting = false;
    }
    else if (Anim_Speed >= Anim_WalkSpeedThreshold)
    {
        Anim_LocomotionState = EAnim_LocomotionState::Walk;
        Anim_bIsSprinting = false;
    }
    else
    {
        Anim_LocomotionState = EAnim_LocomotionState::Idle;
        Anim_bIsSprinting = false;
    }
}

bool UDinoSurvivorAnimInstance::DoFootIKTrace(FName SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
{
    if (!Anim_OwnerCharacter)
        return false;

    USkeletalMeshComponent* Mesh = Anim_OwnerCharacter->GetMesh();
    if (!Mesh)
        return false;

    FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLoc + FVector(0.0f, 0.0f, Anim_FootIKTraceDistance);
    FVector TraceEnd   = SocketLoc - FVector(0.0f, 0.0f, Anim_FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Anim_OwnerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {
        OutLocation = HitResult.ImpactPoint;
        FVector Normal = HitResult.ImpactNormal;
        OutRotation = FRotationMatrix::MakeFromZX(Normal, Anim_OwnerCharacter->GetActorForwardVector()).Rotator();
        OutAlpha = 1.0f;
        return true;
    }

    OutLocation = SocketLoc;
    OutRotation = FRotator::ZeroRotator;
    OutAlpha = 0.0f;
    return false;
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!Anim_OwnerCharacter || Anim_bIsInAir)
    {
        // Blend out IK when in air
        Anim_FootIK.LeftFootAlpha  = FMath::FInterpTo(Anim_FootIK.LeftFootAlpha,  0.0f, DeltaSeconds, 10.0f);
        Anim_FootIK.RightFootAlpha = FMath::FInterpTo(Anim_FootIK.RightFootAlpha, 0.0f, DeltaSeconds, 10.0f);
        Anim_FootIK.PelvisOffset   = FMath::FInterpTo(Anim_FootIK.PelvisOffset,   0.0f, DeltaSeconds, 10.0f);
        return;
    }

    FVector LeftLoc, RightLoc;
    FRotator LeftRot, RightRot;
    float LeftAlpha, RightAlpha;

    DoFootIKTrace(FName("foot_l"), LeftLoc, LeftRot, LeftAlpha);
    DoFootIKTrace(FName("foot_r"), RightLoc, RightRot, RightAlpha);

    // Smooth interpolation
    Anim_FootIK.LeftFootLocation  = FMath::VInterpTo(Anim_FootIK.LeftFootLocation,  LeftLoc,  DeltaSeconds, 15.0f);
    Anim_FootIK.RightFootLocation = FMath::VInterpTo(Anim_FootIK.RightFootLocation, RightLoc, DeltaSeconds, 15.0f);
    Anim_FootIK.LeftFootRotation  = FMath::RInterpTo(Anim_FootIK.LeftFootRotation,  LeftRot,  DeltaSeconds, 15.0f);
    Anim_FootIK.RightFootRotation = FMath::RInterpTo(Anim_FootIK.RightFootRotation, RightRot, DeltaSeconds, 15.0f);
    Anim_FootIK.LeftFootAlpha     = FMath::FInterpTo(Anim_FootIK.LeftFootAlpha,     LeftAlpha,  DeltaSeconds, 10.0f);
    Anim_FootIK.RightFootAlpha    = FMath::FInterpTo(Anim_FootIK.RightFootAlpha,    RightAlpha, DeltaSeconds, 10.0f);

    // Pelvis offset: lower pelvis to the lowest foot
    float LeftDelta  = Anim_FootIK.LeftFootLocation.Z  - Anim_OwnerCharacter->GetActorLocation().Z;
    float RightDelta = Anim_FootIK.RightFootLocation.Z - Anim_OwnerCharacter->GetActorLocation().Z;
    float TargetPelvis = FMath::Min(LeftDelta, RightDelta);
    Anim_FootIK.PelvisOffset = FMath::FInterpTo(Anim_FootIK.PelvisOffset, FMath::Clamp(TargetPelvis, -20.0f, 0.0f), DeltaSeconds, 8.0f);
}

void UDinoSurvivorAnimInstance::UpdateAimOffset()
{
    if (!Anim_OwnerCharacter)
        return;

    AController* Controller = Anim_OwnerCharacter->GetController();
    if (!Controller)
        return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot   = Anim_OwnerCharacter->GetActorRotation();
    FRotator DeltaRot   = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    Anim_AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    Anim_AimYaw   = FMath::Clamp(DeltaRot.Yaw,   -90.0f, 90.0f);
}

void UDinoSurvivorAnimInstance::UpdateSurvivalState()
{
    // Survival stats integration — reads from TranspersonalCharacter if available
    // Defaults to healthy state if character doesn't expose stats
    if (!Anim_OwnerCharacter)
        return;

    // Exhaustion affects animation blend weight
    Anim_bIsExhausted = (Anim_StaminaNormalized < 0.15f);
    Anim_bIsInjured   = (Anim_HealthNormalized  < 0.30f);
}

void UDinoSurvivorAnimInstance::Anim_TriggerAttack()
{
    Anim_bIsAttacking = true;
    // Reset handled by AnimNotify in Blueprint
}

void UDinoSurvivorAnimInstance::Anim_SetCombatStance(EAnim_CombatStance NewStance)
{
    Anim_CombatStance = NewStance;
}
