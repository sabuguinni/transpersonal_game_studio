#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    Speed = 0.f;
    SmoothedSpeed = 0.f;
    Direction = 0.f;
    LeanAmount = 0.f;
    VerticalVelocity = 0.f;
    LocomotionState = EAnim_LocomotionState::Idle;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsMoving = false;
    bJustLanded = false;
    WeaponState = EAnim_WeaponState::Unarmed;
    StaminaNormalized = 1.f;
    HealthNormalized = 1.f;
    FearLevel = 0.f;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    IKAlpha = 1.f;
    OwnerCharacter = nullptr;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (!OwnerCharacter) return;
    }

    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    // --- Velocity & Speed ---
    FVector Velocity = MovComp->Velocity;
    Speed = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;
    SmoothedSpeed = SmoothFloat(SmoothedSpeed, Speed, 8.f, DeltaSeconds);

    // --- Direction (relative to actor facing) ---
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // --- Lean (based on direction change rate) ---
    static float PrevDirection = 0.f;
    float DirectionDelta = Direction - PrevDirection;
    LeanAmount = SmoothFloat(LeanAmount, FMath::Clamp(DirectionDelta * 0.1f, -1.f, 1.f), 5.f, DeltaSeconds);
    PrevDirection = Direction;

    // --- State flags ---
    bIsInAir = MovComp->IsFalling();
    bIsCrouching = MovComp->IsCrouching();
    bIsMoving = Speed > 10.f;

    // Sprint detection: speed > 400 and not crouching
    bIsSprinting = Speed > 400.f && !bIsCrouching && !bIsInAir;

    // Land detection
    static bool bWasInAir = false;
    bJustLanded = bWasInAir && !bIsInAir;
    bWasInAir = bIsInAir;

    // --- Update sub-systems ---
    UpdateLocomotionState();
    UpdateIK(DeltaSeconds);
    UpdateSurvivalDrivenAnim();
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (bIsInAir)
    {
        LocomotionState = EAnim_LocomotionState::InAir;
        return;
    }
    if (bJustLanded)
    {
        LocomotionState = EAnim_LocomotionState::Land;
        return;
    }
    if (bIsCrouching)
    {
        LocomotionState = Speed > 10.f ? EAnim_LocomotionState::Sneak : EAnim_LocomotionState::Crouch;
        return;
    }
    if (!bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
        return;
    }
    if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
        return;
    }
    // Walk vs Run threshold: 200 cm/s
    LocomotionState = Speed > 200.f ? EAnim_LocomotionState::Run : EAnim_LocomotionState::Walk;
}

void UPlayerAnimInstance::UpdateIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    // Foot IK — raycast down from foot bone approximate positions
    auto DoFootTrace = [&](FName SocketName, FVector& OutOffset) -> void
    {
        FVector SocketLoc = OwnerCharacter->GetMesh()
            ? OwnerCharacter->GetMesh()->GetSocketLocation(SocketName)
            : OwnerCharacter->GetActorLocation();

        FVector TraceStart = SocketLoc + FVector(0, 0, 50.f);
        FVector TraceEnd   = SocketLoc - FVector(0, 0, 100.f);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            float TargetZ = Hit.ImpactPoint.Z - SocketLoc.Z;
            OutOffset.Z = SmoothFloat(OutOffset.Z, TargetZ, 12.f, DeltaSeconds);
        }
        else
        {
            OutOffset.Z = SmoothFloat(OutOffset.Z, 0.f, 12.f, DeltaSeconds);
        }
    };

    DoFootTrace(FName("foot_l"), LeftFootIKOffset);
    DoFootTrace(FName("foot_r"), RightFootIKOffset);

    // IK alpha: full when on ground, fade out in air
    float TargetAlpha = bIsInAir ? 0.f : 1.f;
    IKAlpha = SmoothFloat(IKAlpha, TargetAlpha, 8.f, DeltaSeconds);
}

void UPlayerAnimInstance::UpdateSurvivalDrivenAnim()
{
    // These values are set externally by the character's survival component.
    // When stamina is low, animations will play at reduced intensity via blend weights.
    // When fear is high, movement animations get a subtle tremor additive layer.
    // Default safe values — actual values pushed from TranspersonalCharacter each tick.
    if (StaminaNormalized < 0.f) StaminaNormalized = 0.f;
    if (StaminaNormalized > 1.f) StaminaNormalized = 1.f;
    if (HealthNormalized < 0.f)  HealthNormalized  = 0.f;
    if (HealthNormalized > 1.f)  HealthNormalized  = 1.f;
    FearLevel = FMath::Clamp(FearLevel, 0.f, 1.f);
}

float UPlayerAnimInstance::SmoothFloat(float Current, float Target, float SmoothSpeed, float DeltaSeconds)
{
    return FMath::FInterpTo(Current, Target, DeltaSeconds, SmoothSpeed);
}
