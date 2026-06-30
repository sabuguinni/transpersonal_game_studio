#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    FootIKTraceLength = 50.0f;
    PelvisAdjustSpeed = 10.0f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
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

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
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

    // Raw velocity data
    FVector Velocity = MovementComponent->Velocity;
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    Speed = HorizontalVelocity.Size();
    FallSpeed = Velocity.Z;

    // Direction relative to character facing
    FRotator CharacterRotation = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = CharacterRotation.UnrotateVector(HorizontalVelocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // State flags
    bIsMoving = Speed > 3.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Lean angle from angular velocity
    FVector AngularVelocity = OwnerCharacter->GetVelocity();
    LeanAngle = FMath::Clamp(Direction * 0.1f, -45.0f, 45.0f);

    // Jump apex fraction
    if (bIsInAir && FallSpeed > 0.0f)
    {
        JumpApexFraction = FMath::Clamp(FallSpeed / 600.0f, 0.0f, 1.0f);
    }
    else
    {
        JumpApexFraction = 0.0f;
    }

    // Smooth blend space values
    SmoothBlendSpaceValues(DeltaSeconds);

    // Update subsystems
    UpdateLocomotionState();
    UpdateSurvivalAnimState();
    UpdateFootIK(DeltaSeconds);
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Thread-safe property reads can go here for multi-threaded anim
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (bIsInAir)
    {
        LocomotionState = EAnim_LocomotionState::InAir;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = bIsMoving ? EAnim_LocomotionState::Sneak : EAnim_LocomotionState::Crouch;
        return;
    }

    if (!bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
        return;
    }

    if (bIsSprinting && Stamina > 0.1f && !bIsExhausted)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > 300.0f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir)
    {
        // Reset IK when airborne
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, 10.0f);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, 10.0f);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // Foot bone socket names (must match skeleton)
    const FName LeftFootSocket = TEXT("foot_l");
    const FName RightFootSocket = TEXT("foot_r");

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    auto TraceFootIK = [&](FName SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
    {
        FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
        FVector TraceStart = SocketLoc + FVector(0.0f, 0.0f, FootIKTraceLength);
        FVector TraceEnd   = SocketLoc - FVector(0.0f, 0.0f, FootIKTraceLength * 2.0f);

        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd,
            ECollisionChannel::ECC_Visibility, Params);

        if (bHit)
        {
            OutLocation = HitResult.ImpactPoint;
            FVector Normal = HitResult.ImpactNormal;
            OutRotation = FRotator(
                FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
                0.0f,
                -FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
            );
            OutAlpha = FMath::FInterpTo(OutAlpha, 1.0f, DeltaSeconds, 15.0f);
        }
        else
        {
            OutAlpha = FMath::FInterpTo(OutAlpha, 0.0f, DeltaSeconds, 15.0f);
        }
    };

    TraceFootIK(LeftFootSocket,  FootIKData.LeftFootLocation,  FootIKData.LeftFootRotation,  FootIKData.LeftFootAlpha);
    TraceFootIK(RightFootSocket, FootIKData.RightFootLocation, FootIKData.RightFootRotation, FootIKData.RightFootAlpha);

    // Pelvis adjustment — lower pelvis to accommodate the lower foot
    float LeftDelta  = FootIKData.LeftFootLocation.Z  - Mesh->GetComponentLocation().Z;
    float RightDelta = FootIKData.RightFootLocation.Z - Mesh->GetComponentLocation().Z;
    float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);
    FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, TargetPelvisOffset, DeltaSeconds, PelvisAdjustSpeed);
}

void UPlayerAnimInstance::UpdateSurvivalAnimState()
{
    // Exhaustion threshold
    bIsExhausted = Stamina < 0.15f;

    // Blend space stamina weight — tired characters move differently
    BS_StaminaWeight = FMath::Lerp(0.6f, 1.0f, FMath::Clamp(Stamina, 0.0f, 1.0f));

    // Fear affects posture — high fear = crouched defensive stance
    if (Fear > 0.75f && !bIsInAir)
    {
        LeanAngle = FMath::Lerp(LeanAngle, LeanAngle * 1.3f, Fear);
    }
}

void UPlayerAnimInstance::SmoothBlendSpaceValues(float DeltaSeconds)
{
    const float SmoothSpeed = 8.0f;
    SmoothedSpeed     = FMath::FInterpTo(SmoothedSpeed,     Speed,     DeltaSeconds, SmoothSpeed);
    SmoothedDirection = FMath::FInterpTo(SmoothedDirection, Direction, DeltaSeconds, SmoothSpeed);

    BS_Speed     = SmoothedSpeed;
    BS_Direction = SmoothedDirection;
}

void UPlayerAnimInstance::PlayAttackMontage(EAnim_WeaponState InWeaponState)
{
    WeaponState = InWeaponState;
    // Montage assets are assigned in Blueprint — C++ sets state, BP plays montage
}

void UPlayerAnimInstance::PlayLandMontage(float ImpactVelocity)
{
    // Hard land threshold: > 600 cm/s
    if (FMath::Abs(ImpactVelocity) > 600.0f)
    {
        LocomotionState = EAnim_LocomotionState::Land;
    }
}

void UPlayerAnimInstance::PlayCraftingMontage()
{
    // Triggered from interaction system — crafting upper-body additive layer
}

bool UPlayerAnimInstance::IsInLocomotionState(EAnim_LocomotionState InState) const
{
    return LocomotionState == InState;
}

float UPlayerAnimInstance::GetLocomotionBlendWeight() const
{
    // Returns 0-1 blend weight for locomotion vs action layers
    switch (LocomotionState)
    {
        case EAnim_LocomotionState::Idle:   return 0.0f;
        case EAnim_LocomotionState::Walk:   return 0.3f;
        case EAnim_LocomotionState::Run:    return 0.7f;
        case EAnim_LocomotionState::Sprint: return 1.0f;
        default:                            return 0.5f;
    }
}
