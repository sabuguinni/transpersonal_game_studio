#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoAnimInstance::UDinoAnimInstance()
{
    // Safe defaults — CDO construction happens before world exists
    CurrentStance       = EAnim_DinoStance::Idle;
    LocomotionSpeed     = 0.f;
    LocomotionDirection = 0.f;
    FootIKAlpha         = 1.f;
    bPlayAttackMontage  = false;
    bPlayRoarMontage    = false;
    AttackCooldown      = 0.f;
    RoarCooldown        = 0.f;
}

void UDinoAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache owner pawn — safe to do here, world exists at init time
    OwnerPawn = TryGetPawnOwner();
}

void UDinoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;
    }

    UpdateLocomotionState(DeltaSeconds);
    UpdateFootIK();
    DetermineStance();

    // Tick cooldowns
    if (AttackCooldown > 0.f)
    {
        AttackCooldown -= DeltaSeconds;
        if (AttackCooldown <= 0.f)
        {
            bPlayAttackMontage = false;
            AttackCooldown = 0.f;
        }
    }

    if (RoarCooldown > 0.f)
    {
        RoarCooldown -= DeltaSeconds;
        if (RoarCooldown <= 0.f)
        {
            bPlayRoarMontage = false;
            RoarCooldown = 0.f;
        }
    }
}

void UDinoAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    const FVector Velocity = OwnerPawn->GetVelocity();
    const float   Speed    = Velocity.Size2D();

    // Smooth speed update — prevents pop between states
    LocomotionSpeed = FMath::FInterpTo(LocomotionSpeed, Speed, DeltaSeconds, 8.f);

    // Direction: angle between forward and velocity (for strafing blend spaces)
    if (Speed > 10.f)
    {
        const FVector Forward = OwnerPawn->GetActorForwardVector();
        const FVector VelNorm = Velocity.GetSafeNormal2D();
        const float   DotF   = FVector::DotProduct(Forward, VelNorm);
        const FVector Right   = OwnerPawn->GetActorRightVector();
        const float   DotR   = FVector::DotProduct(Right, VelNorm);
        LocomotionDirection = FMath::RadiansToDegrees(FMath::Atan2(DotR, DotF));
    }
    else
    {
        LocomotionDirection = FMath::FInterpTo(LocomotionDirection, 0.f, DeltaSeconds, 5.f);
    }

    // Update locomotion data struct (exposed to Blueprint)
    LocomotionData.Speed       = LocomotionSpeed;
    LocomotionData.Direction   = LocomotionDirection;
    LocomotionData.bIsMoving   = LocomotionSpeed > 50.f;
}

void UDinoAnimInstance::UpdateFootIK()
{
    // Foot IK via line traces — adapts feet to uneven terrain
    USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
    if (!MeshComp || !OwnerPawn) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Trace parameters
    FCollisionQueryParams TraceParams(FName("DinoFootIK"), true, OwnerPawn);
    TraceParams.bReturnPhysicalMaterial = false;

    const float TraceLength = 150.f;

    // Left foot bone socket (generic — works for bipeds and quadrupeds)
    const FVector LeftFootLoc  = MeshComp->GetSocketLocation(FName("foot_l"));
    const FVector RightFootLoc = MeshComp->GetSocketLocation(FName("foot_r"));

    // Left foot trace
    FHitResult LeftHit;
    const FVector LeftStart = LeftFootLoc + FVector(0, 0, TraceLength * 0.5f);
    const FVector LeftEnd   = LeftFootLoc - FVector(0, 0, TraceLength);
    if (World->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd, ECC_WorldStatic, TraceParams))
    {
        LeftFootIKLocation = LeftHit.ImpactPoint;
    }
    else
    {
        LeftFootIKLocation = LeftFootLoc;
    }

    // Right foot trace
    FHitResult RightHit;
    const FVector RightStart = RightFootLoc + FVector(0, 0, TraceLength * 0.5f);
    const FVector RightEnd   = RightFootLoc - FVector(0, 0, TraceLength);
    if (World->LineTraceSingleByChannel(RightHit, RightStart, RightEnd, ECC_WorldStatic, TraceParams))
    {
        RightFootIKLocation = RightHit.ImpactPoint;
    }
    else
    {
        RightFootIKLocation = RightFootLoc;
    }

    // Alpha: full IK when moving slowly, blend out at high speed
    const float TargetAlpha = LocomotionSpeed < 200.f ? 1.f : FMath::Clamp(1.f - (LocomotionSpeed - 200.f) / 400.f, 0.f, 1.f);
    FootIKAlpha = FMath::FInterpTo(FootIKAlpha, TargetAlpha, GetWorld()->GetDeltaSeconds(), 5.f);
}

void UDinoAnimInstance::DetermineStance()
{
    // Priority: Dead > Attacking > Roaring > Running > Walking > Idle
    if (bPlayAttackMontage)
    {
        CurrentStance = EAnim_DinoStance::Attacking;
        LocomotionData.bIsAttacking = true;
        return;
    }

    LocomotionData.bIsAttacking = false;

    if (bPlayRoarMontage)
    {
        CurrentStance = EAnim_DinoStance::Roaring;
        return;
    }

    if (LocomotionSpeed > 400.f)
    {
        CurrentStance = EAnim_DinoStance::Running;
    }
    else if (LocomotionSpeed > 50.f)
    {
        CurrentStance = EAnim_DinoStance::Walking;
    }
    else
    {
        CurrentStance = EAnim_DinoStance::Idle;
    }
}

void UDinoAnimInstance::SetLocomotionSpeed(float NewSpeed)
{
    LocomotionSpeed = FMath::Max(0.f, NewSpeed);
    LocomotionData.Speed    = LocomotionSpeed;
    LocomotionData.bIsMoving = LocomotionSpeed > 50.f;
}

void UDinoAnimInstance::TriggerAttack()
{
    if (AttackCooldown > 0.f) return;  // Still in previous attack
    bPlayAttackMontage = true;
    AttackCooldown     = 1.5f;  // 1.5s attack window
    LocomotionData.bIsAttacking = true;
}

void UDinoAnimInstance::TriggerRoar()
{
    if (RoarCooldown > 0.f) return;
    bPlayRoarMontage = true;
    RoarCooldown     = 2.5f;  // 2.5s roar duration
}
