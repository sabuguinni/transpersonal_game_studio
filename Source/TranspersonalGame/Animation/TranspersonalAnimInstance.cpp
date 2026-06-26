#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    LateralSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInAir = false;
    bIsMoving = false;
    bIsCrouching = false;
    bIsSprinting = false;
    VerticalVelocity = 0.0f;
    LocomotionState = EAnim_LocomotionState::Idle;
    StanceState = EAnim_StanceState::Upright;

    // Survival defaults
    Health = 100.0f;
    Stamina = 100.0f;
    FearLevel = 0.0f;
    bIsInjured = false;
    bIsExhausted = false;

    // IK defaults
    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    PelvisOffset = 0.0f;
    bIsFootIKActive = false;

    // Aim offset defaults
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // Blend weights
    BreathingBlendWeight = 1.0f;
    FearTremorBlendWeight = 0.0f;

    // Configuration
    WalkRunThreshold = 200.0f;
    RunSprintThreshold = 500.0f;
    FootIKTraceDistance = 80.0f;
    IKSmoothingSpeed = 15.0f;

    // Internal state
    PreviousSpeed = 0.0f;
    TimeSinceLanded = 0.0f;
    bIsLanding = false;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Cache references to owning character and movement component
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // Re-cache if null (can happen after respawn)
    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        return;
    }

    if (!MovementComponent)
    {
        return;
    }

    UpdateLocomotionData(DeltaSeconds);
    UpdateSurvivalData(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateBlendWeights(DeltaSeconds);
    DetermineLocomotionState();
    DetermineStanceState();

    // Update landing timer
    if (bIsLanding)
    {
        TimeSinceLanded += DeltaSeconds;
        if (TimeSinceLanded > 0.5f)
        {
            bIsLanding = false;
            TimeSinceLanded = 0.0f;
        }
    }
}

void UTranspersonalAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Foot IK runs thread-safe for performance
    UpdateFootIK(DeltaSeconds);
}

void UTranspersonalAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Get velocity
    FVector Velocity = MovementComponent->Velocity;
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);

    // Calculate speed
    PreviousSpeed = Speed;
    Speed = HorizontalVelocity.Size();
    VerticalVelocity = Velocity.Z;

    // Calculate movement direction relative to character facing
    if (Speed > 1.0f)
    {
        FRotator CharacterRotation = OwnerCharacter->GetActorRotation();
        FVector ForwardVector = CharacterRotation.Vector();
        FVector RightVector = FRotationMatrix(CharacterRotation).GetScaledAxis(EAxis::Y);

        float ForwardDot = FVector::DotProduct(HorizontalVelocity.GetSafeNormal(), ForwardVector);
        float RightDot = FVector::DotProduct(HorizontalVelocity.GetSafeNormal(), RightVector);

        MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
        LateralSpeed = RightDot * Speed;
    }
    else
    {
        MovementDirection = 0.0f;
        LateralSpeed = 0.0f;
    }

    // Update boolean states
    bIsMoving = Speed > 5.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Foot IK only active on ground
    bIsFootIKActive = !bIsInAir;
}

void UTranspersonalAnimInstance::UpdateSurvivalData(float DeltaSeconds)
{
    // Try to read survival stats from the character
    // These are exposed as properties on TranspersonalCharacter
    if (!OwnerCharacter)
    {
        return;
    }

    // Read health and stamina via reflection if available
    // Falls back to defaults if properties don't exist
    static FName HealthPropName = TEXT("Health");
    static FName StaminaPropName = TEXT("Stamina");
    static FName FearPropName = TEXT("Fear");
    static FName SprintingPropName = TEXT("bIsSprinting");

    // Try to get bIsSprinting from character
    FBoolProperty* SprintProp = FindFProperty<FBoolProperty>(OwnerCharacter->GetClass(), SprintingPropName);
    if (SprintProp)
    {
        bIsSprinting = SprintProp->GetPropertyValue_InContainer(OwnerCharacter);
    }
    else
    {
        // Estimate sprinting from speed
        bIsSprinting = Speed > RunSprintThreshold;
    }

    // Try to get health
    FFloatProperty* HealthProp = FindFProperty<FFloatProperty>(OwnerCharacter->GetClass(), HealthPropName);
    if (HealthProp)
    {
        Health = HealthProp->GetPropertyValue_InContainer(OwnerCharacter);
    }

    // Try to get stamina
    FFloatProperty* StaminaProp = FindFProperty<FFloatProperty>(OwnerCharacter->GetClass(), StaminaPropName);
    if (StaminaProp)
    {
        Stamina = StaminaProp->GetPropertyValue_InContainer(OwnerCharacter);
    }

    // Try to get fear
    FFloatProperty* FearProp = FindFProperty<FFloatProperty>(OwnerCharacter->GetClass(), FearPropName);
    if (FearProp)
    {
        FearLevel = FearProp->GetPropertyValue_InContainer(OwnerCharacter);
    }

    // Derive boolean states
    bIsInjured = Health < 30.0f;
    bIsExhausted = Stamina < 15.0f;
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || !bIsFootIKActive)
    {
        // Reset IK when in air
        LeftFootIKLocation = FVector::ZeroVector;
        RightFootIKLocation = FVector::ZeroVector;
        LeftFootIKRotation = FRotator::ZeroRotator;
        RightFootIKRotation = FRotator::ZeroRotator;
        PelvisOffset = 0.0f;
        return;
    }

    FVector NewLeftLoc = LeftFootIKLocation;
    FVector NewLeftNormal = FVector::UpVector;
    FVector NewRightLoc = RightFootIKLocation;
    FVector NewRightNormal = FVector::UpVector;

    bool bLeftHit = TraceFootIK(TEXT("foot_l"), NewLeftLoc, LeftFootIKRotation);
    bool bRightHit = TraceFootIK(TEXT("foot_r"), NewRightLoc, RightFootIKRotation);

    // Smooth IK transitions
    float SmoothAlpha = FMath::Clamp(IKSmoothingSpeed * DeltaSeconds, 0.0f, 1.0f);

    if (bLeftHit)
    {
        LeftFootIKLocation = FMath::VInterpTo(LeftFootIKLocation, NewLeftLoc, DeltaSeconds, IKSmoothingSpeed);
    }
    if (bRightHit)
    {
        RightFootIKLocation = FMath::VInterpTo(RightFootIKLocation, NewRightLoc, DeltaSeconds, IKSmoothingSpeed);
    }

    // Calculate pelvis offset — lower pelvis to accommodate the lower foot
    float LeftDelta = LeftFootIKLocation.Z - OwnerCharacter->GetActorLocation().Z;
    float RightDelta = RightFootIKLocation.Z - OwnerCharacter->GetActorLocation().Z;
    float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);
    TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, -30.0f, 0.0f);

    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaSeconds, IKSmoothingSpeed);
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
    FVector TraceStart = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z + 50.0f);
    FVector TraceEnd = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z - FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutLocation = HitResult.ImpactPoint;

        // Calculate foot rotation from surface normal
        FVector SurfaceNormal = HitResult.ImpactNormal;
        FRotator SurfaceRotation = FRotationMatrix::MakeFromZX(SurfaceNormal, OwnerCharacter->GetActorForwardVector()).Rotator();
        OutRotation = FMath::RInterpTo(OutRotation, SurfaceRotation, GetWorld()->GetDeltaSeconds(), IKSmoothingSpeed);
    }

    return bHit;
}

void UTranspersonalAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter)
    {
        return;
    }

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller)
    {
        return;
    }

    FRotator ControlRotation = Controller->GetControlRotation();
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    FRotator DeltaRotation = (ControlRotation - ActorRotation).GetNormalized();

    // Smooth aim offset
    AimPitch = FMath::FInterpTo(AimPitch, FMath::Clamp(DeltaRotation.Pitch, -90.0f, 90.0f), DeltaSeconds, 10.0f);
    AimYaw = FMath::FInterpTo(AimYaw, FMath::Clamp(DeltaRotation.Yaw, -90.0f, 90.0f), DeltaSeconds, 10.0f);
}

void UTranspersonalAnimInstance::UpdateBlendWeights(float DeltaSeconds)
{
    // Breathing blend weight — always present, increases when exhausted
    float TargetBreathing = bIsExhausted ? 2.0f : 1.0f;
    BreathingBlendWeight = FMath::FInterpTo(BreathingBlendWeight, TargetBreathing, DeltaSeconds, 2.0f);

    // Fear tremor — increases with fear level
    float TargetFearTremor = FMath::GetMappedRangeValueClamped(
        FVector2D(50.0f, 100.0f),
        FVector2D(0.0f, 1.0f),
        FearLevel
    );
    FearTremorBlendWeight = FMath::FInterpTo(FearTremorBlendWeight, TargetFearTremor, DeltaSeconds, 3.0f);
}

void UTranspersonalAnimInstance::DetermineLocomotionState()
{
    if (bIsInAir)
    {
        if (VerticalVelocity > 0.0f)
        {
            LocomotionState = EAnim_LocomotionState::Jump;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Fall;
        }
        return;
    }

    if (bIsLanding)
    {
        LocomotionState = EAnim_LocomotionState::Land;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouch;
        return;
    }

    if (!bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
        return;
    }

    if (bIsSprinting || Speed > RunSprintThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > WalkRunThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
}

void UTranspersonalAnimInstance::DetermineStanceState()
{
    if (bIsInjured)
    {
        StanceState = EAnim_StanceState::Injured;
        return;
    }

    if (bIsExhausted)
    {
        StanceState = EAnim_StanceState::Exhausted;
        return;
    }

    if (bIsCrouching)
    {
        StanceState = EAnim_StanceState::Stealth;
        return;
    }

    // Default upright stance
    StanceState = EAnim_StanceState::Upright;
}

void UTranspersonalAnimInstance::OnLanded()
{
    bIsLanding = true;
    TimeSinceLanded = 0.0f;
    LocomotionState = EAnim_LocomotionState::Land;
}

void UTranspersonalAnimInstance::SetLocomotionState(EAnim_LocomotionState NewState)
{
    LocomotionState = NewState;
}

void UTranspersonalAnimInstance::SetStanceState(EAnim_StanceState NewState)
{
    StanceState = NewState;
}

FVector2D UTranspersonalAnimInstance::GetLocomotionBlendSpacePosition() const
{
    // X = forward speed (0 to max sprint), Y = lateral speed (-max to max)
    return FVector2D(Speed, LateralSpeed);
}
