#include "DinosaurAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    GroundSpeed = 0.0f;
    bIsMoving = false;
    bIsInAir = false;
    LocomotionState = EAnim_DinoLocomotionState::Idle;
    WalkRunBlend = 0.0f;
    AttackBlendWeight = 0.0f;
    IKFootBlendWeight = 1.0f;
    AttackCooldown = 0.0f;
    RoarCooldown = 0.0f;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    // Initialize IK foot locations to zero — will be updated each tick
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (DeltaSeconds <= 0.0f)
    {
        return;
    }

    UpdateLocomotionData(DeltaSeconds);
    UpdateIKFootPlacement();
    UpdateBlendWeights(DeltaSeconds);

    // Tick cooldowns
    if (AttackCooldown > 0.0f)
    {
        AttackCooldown -= DeltaSeconds;
    }
    if (RoarCooldown > 0.0f)
    {
        RoarCooldown -= DeltaSeconds;
    }
}

void UDinosaurAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    // Calculate ground speed from velocity (ignore Z)
    FVector Velocity = OwnerPawn->GetVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    GroundSpeed = HorizontalVelocity.Size();

    bIsMoving = GroundSpeed > 10.0f;

    // Update locomotion data struct
    LocomotionData.Speed = GroundSpeed;
    LocomotionData.bIsMoving = bIsMoving;
    LocomotionData.bIsAttacking = (LocomotionState == EAnim_DinoLocomotionState::Attack);
    LocomotionData.bIsRoaring = (LocomotionState == EAnim_DinoLocomotionState::Roar);
    LocomotionData.bIsDead = (LocomotionState == EAnim_DinoLocomotionState::Death);
    LocomotionData.CurrentState = LocomotionState;

    // Auto-transition locomotion state based on speed (unless in special state)
    if (LocomotionState != EAnim_DinoLocomotionState::Attack &&
        LocomotionState != EAnim_DinoLocomotionState::Roar &&
        LocomotionState != EAnim_DinoLocomotionState::Death &&
        LocomotionState != EAnim_DinoLocomotionState::Eat &&
        LocomotionState != EAnim_DinoLocomotionState::Sleep)
    {
        if (GroundSpeed < 10.0f)
        {
            LocomotionState = EAnim_DinoLocomotionState::Idle;
        }
        else if (GroundSpeed < 400.0f)
        {
            LocomotionState = EAnim_DinoLocomotionState::Walk;
        }
        else
        {
            LocomotionState = EAnim_DinoLocomotionState::Run;
        }
    }

    // Calculate walk/run blend (0 = walk, 1 = run)
    if (bIsMoving)
    {
        float NormalizedSpeed = FMath::Clamp(GroundSpeed / 800.0f, 0.0f, 1.0f);
        WalkRunBlend = NormalizedSpeed;
    }
    else
    {
        WalkRunBlend = 0.0f;
    }
}

void UDinosaurAnimInstance::UpdateIKFootPlacement()
{
    USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
    if (!MeshComp)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Trace down from each foot bone to find ground contact point
    auto TraceForFootIK = [&](const FName& FootBoneName, FVector& OutIKLocation) -> bool
    {
        int32 BoneIndex = MeshComp->GetBoneIndex(FootBoneName);
        if (BoneIndex == INDEX_NONE)
        {
            return false;
        }

        FVector FootWorldLocation = MeshComp->GetBoneLocation(FootBoneName);
        FVector TraceStart = FootWorldLocation + FVector(0.0f, 0.0f, 50.0f);
        FVector TraceEnd = FootWorldLocation - FVector(0.0f, 0.0f, 100.0f);

        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(MeshComp->GetOwner());

        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
        {
            OutIKLocation = HitResult.ImpactPoint;
            return true;
        }

        OutIKLocation = FootWorldLocation;
        return false;
    };

    TraceForFootIK(FName("foot_l"), LeftFootIKLocation);
    TraceForFootIK(FName("foot_r"), RightFootIKLocation);

    // Blend IK weight — reduce during fast movement for performance
    float TargetIKWeight = bIsMoving ? FMath::Clamp(1.0f - (WalkRunBlend * 0.5f), 0.5f, 1.0f) : 1.0f;
    IKFootBlendWeight = FMath::FInterpTo(IKFootBlendWeight, TargetIKWeight, GetWorld()->GetDeltaSeconds(), 5.0f);
}

void UDinosaurAnimInstance::UpdateBlendWeights(float DeltaSeconds)
{
    // Smoothly blend attack weight in/out
    float TargetAttackWeight = (LocomotionState == EAnim_DinoLocomotionState::Attack) ? 1.0f : 0.0f;
    AttackBlendWeight = FMath::FInterpTo(AttackBlendWeight, TargetAttackWeight, DeltaSeconds, 8.0f);
}

void UDinosaurAnimInstance::SetLocomotionState(EAnim_DinoLocomotionState NewState)
{
    LocomotionState = NewState;
    LocomotionData.CurrentState = NewState;
}

void UDinosaurAnimInstance::TriggerAttack()
{
    if (AttackCooldown <= 0.0f)
    {
        SetLocomotionState(EAnim_DinoLocomotionState::Attack);
        AttackCooldown = 2.0f; // 2 second cooldown between attacks
    }
}

void UDinosaurAnimInstance::TriggerRoar()
{
    if (RoarCooldown <= 0.0f)
    {
        SetLocomotionState(EAnim_DinoLocomotionState::Roar);
        RoarCooldown = 5.0f; // 5 second cooldown between roars
    }
}

bool UDinosaurAnimInstance::IsInAttackState() const
{
    return LocomotionState == EAnim_DinoLocomotionState::Attack;
}
