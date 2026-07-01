// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Complete AnimInstance implementation: locomotion state machine, foot IK, lean, jump

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsAccelerating = false;
    bIsDead = false;
    LocomotionState = EAnim_LocomotionState::Idle;

    // Lean defaults
    LeanData.LeanForwardBack = 0.0f;
    LeanData.LeanLeftRight = 0.0f;
    LeanData.LeanAlpha = 0.0f;

    // Foot IK defaults
    FootIKLeft.FootLocation = FVector::ZeroVector;
    FootIKLeft.FootRotation = FRotator::ZeroRotator;
    FootIKLeft.IKAlpha = 0.0f;
    FootIKLeft.bIsGrounded = false;
    FootIKRight.FootLocation = FVector::ZeroVector;
    FootIKRight.FootRotation = FRotator::ZeroRotator;
    FootIKRight.IKAlpha = 0.0f;
    FootIKRight.bIsGrounded = false;

    // Foot IK settings
    FootIKTraceLength = 55.0f;
    FootIKInterpSpeed = 15.0f;
    HipOffsetInterpSpeed = 10.0f;
    HipOffset = 0.0f;
    TargetHipOffset = 0.0f;

    // Jump/fall
    JumpVelocity = 0.0f;
    FallTime = 0.0f;
    bJustLanded = false;
    LandImpactAlpha = 0.0f;

    // Survival stats
    HealthNormalized = 1.0f;
    StaminaNormalized = 1.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;
    bIsInjured = false;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache the owning character
    OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwningCharacter)
    {
        MovementComponent = OwningCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("PlayerAnimInstance: Initialized for %s"), *OwningCharacter->GetName());
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwningCharacter || !MovementComponent)
    {
        // Attempt re-cache
        OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwningCharacter)
        {
            MovementComponent = OwningCharacter->GetCharacterMovement();
        }
        return;
    }

    // Update all animation data
    UpdateLocomotionData(DeltaSeconds);
    UpdateAirborneData(DeltaSeconds);
    UpdateLeanData(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateLocomotionState();
}

void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwningCharacter || !MovementComponent) return;

    // Velocity and speed
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D(); // Horizontal speed only

    // Direction relative to character facing
    FRotator ActorRotation = OwningCharacter->GetActorRotation();
    FVector LocalVelocity = ActorRotation.UnrotateVector(Velocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // Movement state flags
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;

    // Sprint detection — speed above walk threshold
    float MaxWalkSpeed = MovementComponent->MaxWalkSpeed;
    bIsSprinting = Speed > (MaxWalkSpeed * 0.85f) && !bIsInAir;
}

void UPlayerAnimInstance::UpdateAirborneData(float DeltaSeconds)
{
    if (!MovementComponent) return;

    FVector Velocity = MovementComponent->Velocity;
    JumpVelocity = Velocity.Z;

    if (bIsInAir)
    {
        FallTime += DeltaSeconds;
    }
    else
    {
        if (FallTime > 0.1f)
        {
            // Just landed
            bJustLanded = true;
            // Impact alpha based on fall time — longer fall = harder impact
            LandImpactAlpha = FMath::Clamp(FallTime / 2.0f, 0.0f, 1.0f);
        }
        FallTime = 0.0f;
    }

    // Decay land impact
    if (bJustLanded)
    {
        LandImpactAlpha = FMath::FInterpTo(LandImpactAlpha, 0.0f, DeltaSeconds, 3.0f);
        if (LandImpactAlpha < 0.01f)
        {
            bJustLanded = false;
            LandImpactAlpha = 0.0f;
        }
    }
}

void UPlayerAnimInstance::UpdateLeanData(float DeltaSeconds)
{
    if (!OwningCharacter || !MovementComponent) return;

    FVector Velocity = MovementComponent->Velocity;
    FVector Acceleration = MovementComponent->GetCurrentAcceleration();
    FRotator ActorRotation = OwningCharacter->GetActorRotation();

    // Transform acceleration to local space
    FVector LocalAccel = ActorRotation.UnrotateVector(Acceleration);
    float AccelMagnitude = Acceleration.Size2D();

    // Lean forward/back based on forward acceleration
    float TargetLeanFB = 0.0f;
    float TargetLeanLR = 0.0f;

    if (AccelMagnitude > 10.0f)
    {
        TargetLeanFB = FMath::Clamp(LocalAccel.X / MovementComponent->MaxAcceleration, -1.0f, 1.0f) * 15.0f;
        TargetLeanLR = FMath::Clamp(LocalAccel.Y / MovementComponent->MaxAcceleration, -1.0f, 1.0f) * 10.0f;
    }

    // Smooth lean interpolation
    LeanData.LeanForwardBack = FMath::FInterpTo(LeanData.LeanForwardBack, TargetLeanFB, DeltaSeconds, 4.0f);
    LeanData.LeanLeftRight = FMath::FInterpTo(LeanData.LeanLeftRight, TargetLeanLR, DeltaSeconds, 4.0f);

    // Lean alpha — active when moving
    float TargetAlpha = (Speed > 10.0f && !bIsInAir) ? 1.0f : 0.0f;
    LeanData.LeanAlpha = FMath::FInterpTo(LeanData.LeanAlpha, TargetAlpha, DeltaSeconds, 5.0f);
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwningCharacter || bIsInAir || bIsDead) 
    {
        // Blend out IK when airborne
        FootIKLeft.IKAlpha = FMath::FInterpTo(FootIKLeft.IKAlpha, 0.0f, DeltaSeconds, 10.0f);
        FootIKRight.IKAlpha = FMath::FInterpTo(FootIKRight.IKAlpha, 0.0f, DeltaSeconds, 10.0f);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // Foot bone socket names
    static const FName LeftFootSocket(TEXT("foot_l"));
    static const FName RightFootSocket(TEXT("foot_r"));

    USkeletalMeshComponent* MeshComp = OwningCharacter->GetMesh();
    if (!MeshComp) return;

    // Process left foot
    FVector LeftFootLocation = MeshComp->GetSocketLocation(LeftFootSocket);
    ProcessFootIKTrace(World, LeftFootLocation, FootIKLeft, DeltaSeconds);

    // Process right foot
    FVector RightFootLocation = MeshComp->GetSocketLocation(RightFootSocket);
    ProcessFootIKTrace(World, RightFootLocation, FootIKRight, DeltaSeconds);

    // Calculate hip offset — lower hips to accommodate foot on lower ground
    float LowestFoot = FMath::Min(FootIKLeft.FootLocation.Z, FootIKRight.FootLocation.Z);
    float CharacterBase = OwningCharacter->GetActorLocation().Z - OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    TargetHipOffset = FMath::Min(0.0f, LowestFoot - CharacterBase);
    HipOffset = FMath::FInterpTo(HipOffset, TargetHipOffset, DeltaSeconds, HipOffsetInterpSpeed);
}

void UPlayerAnimInstance::ProcessFootIKTrace(UWorld* World, const FVector& FootLocation, FAnim_FootIKData& FootData, float DeltaSeconds)
{
    if (!World) return;

    FVector TraceStart = FootLocation + FVector(0.0f, 0.0f, FootIKTraceLength * 0.5f);
    FVector TraceEnd = FootLocation - FVector(0.0f, 0.0f, FootIKTraceLength);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        FootData.bIsGrounded = true;

        // Target foot location at hit point
        FVector TargetLocation = HitResult.ImpactPoint;

        // Smooth interpolation
        FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, TargetLocation, DeltaSeconds, FootIKInterpSpeed);

        // Foot rotation to match surface normal
        FVector SurfaceNormal = HitResult.ImpactNormal;
        FRotator TargetRotation = FRotationMatrix::MakeFromZX(SurfaceNormal, OwningCharacter->GetActorForwardVector()).Rotator();
        FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, TargetRotation, DeltaSeconds, FootIKInterpSpeed);

        // Blend in IK alpha
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootData.bIsGrounded = false;
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (bIsDead)
    {
        LocomotionState = EAnim_LocomotionState::Dead;
        return;
    }

    if (bIsInAir)
    {
        if (JumpVelocity > 0.0f)
        {
            LocomotionState = EAnim_LocomotionState::Jump;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Fall;
        }
        return;
    }

    if (bJustLanded)
    {
        LocomotionState = EAnim_LocomotionState::Land;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = (Speed > 10.0f) ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
        return;
    }

    if (Speed < 10.0f)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > 200.0f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
}

void UPlayerAnimInstance::SetSurvivalStats(float Health, float Stamina, float Fear)
{
    HealthNormalized = FMath::Clamp(Health, 0.0f, 1.0f);
    StaminaNormalized = FMath::Clamp(Stamina, 0.0f, 1.0f);
    FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);

    // Derived states
    bIsExhausted = StaminaNormalized < 0.15f;
    bIsInjured = HealthNormalized < 0.3f;
    bIsDead = HealthNormalized <= 0.0f;
}

void UPlayerAnimInstance::TriggerDeathAnimation()
{
    bIsDead = true;
    LocomotionState = EAnim_LocomotionState::Dead;
    UE_LOG(LogTemp, Log, TEXT("PlayerAnimInstance: Death animation triggered"));
}

EAnim_LocomotionState UPlayerAnimInstance::GetLocomotionState() const
{
    return LocomotionState;
}

float UPlayerAnimInstance::GetSpeed() const
{
    return Speed;
}

float UPlayerAnimInstance::GetDirection() const
{
    return Direction;
}

bool UPlayerAnimInstance::GetIsInAir() const
{
    return bIsInAir;
}

FAnim_FootIKData UPlayerAnimInstance::GetLeftFootIKData() const
{
    return FootIKLeft;
}

FAnim_FootIKData UPlayerAnimInstance::GetRightFootIKData() const
{
    return FootIKRight;
}

FAnim_LeanData UPlayerAnimInstance::GetLeanData() const
{
    return LeanData;
}
