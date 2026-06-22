// DinoSurvivorAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Complete AnimInstance implementation for the prehistoric survivor character.
// Drives locomotion blend space, IK foot placement, upper-body overlay,
// survival-state reactions, and dinosaur-proximity fear responses.

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
//  CONSTRUCTION
// ─────────────────────────────────────────────────────────────────────────────

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    // Locomotion defaults
    Speed              = 0.f;
    Direction          = 0.f;
    bIsInAir           = false;
    bIsCrouching       = false;
    bIsSprinting       = false;
    bShouldMove        = false;
    LeanAngle          = 0.f;

    // IK defaults
    bEnableFootIK      = true;
    LeftFootIKAlpha    = 1.f;
    RightFootIKAlpha   = 1.f;
    LeftFootOffset     = FVector::ZeroVector;
    RightFootOffset    = FVector::ZeroVector;
    LeftFootRotation   = FRotator::ZeroRotator;
    RightFootRotation  = FRotator::ZeroRotator;
    PelvisOffset       = 0.f;

    // Survival state defaults
    HealthNormalized   = 1.f;
    StaminaNormalized  = 1.f;
    HungerNormalized   = 1.f;
    ThirstNormalized   = 1.f;
    FearLevel          = 0.f;
    bIsExhausted       = false;
    bIsWounded         = false;
    bIsStarving        = false;

    // Dino proximity defaults
    NearestDinoDistance = 9999.f;
    bDinoInSight        = false;
    DinoThreatDirection = FVector::ForwardVector;

    // Upper-body overlay defaults
    bIsAiming          = false;
    bIsCarrying        = false;
    bIsClimbing        = false;
    AimPitch           = 0.f;
    AimYaw             = 0.f;

    // Internal
    CachedCharacter    = nullptr;
    CachedMovement     = nullptr;
    IKTraceDistance    = 55.f;
    PelvisSmoothSpeed  = 15.f;
    FootRotSmoothSpeed = 20.f;
    LeanSmoothSpeed    = 8.f;
    FearSmoothSpeed    = 3.f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  NATIVE INITIALIZE
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    CachedCharacter = Cast<ACharacter>(Pawn);
    if (CachedCharacter)
    {
        CachedMovement = CachedCharacter->GetCharacterMovement();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  NATIVE UPDATE — master dispatcher
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!CachedCharacter || !CachedMovement) return;

    UpdateLocomotionState(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalState(DeltaSeconds);
    UpdateDinoProximity(DeltaSeconds);
    UpdateUpperBodyOverlay(DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
//  1. LOCOMOTION STATE
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!CachedCharacter || !CachedMovement) return;

    const FVector Velocity   = CachedMovement->Velocity;
    const float   RawSpeed   = Velocity.Size2D();

    Speed       = RawSpeed;
    bIsInAir    = CachedMovement->IsFalling();
    bIsCrouching = CachedCharacter->bIsCrouched;
    bShouldMove = (RawSpeed > 3.f) && (CachedMovement->GetCurrentAcceleration().SizeSquared() > 0.f);

    // Sprint detection: speed > 400 and not crouching
    bIsSprinting = (RawSpeed > 400.f) && !bIsCrouching;

    // Direction relative to actor forward
    if (RawSpeed > 1.f)
    {
        const FRotator ActorRot = CachedCharacter->GetActorRotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(
            Velocity.Rotation(), ActorRot).Yaw;
    }
    else
    {
        Direction = 0.f;
    }

    // Lean — lateral acceleration feel
    const FVector LocalAccel = CachedCharacter->GetActorRotation().UnrotateVector(
        CachedMovement->GetCurrentAcceleration());
    const float TargetLean  = FMath::Clamp(LocalAccel.Y * 0.02f, -30.f, 30.f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, LeanSmoothSpeed);

    // Aim offsets (used when aiming)
    if (APlayerController* PC = Cast<APlayerController>(CachedCharacter->GetController()))
    {
        FRotator ControlRot  = PC->GetControlRotation();
        FRotator ActorRot    = CachedCharacter->GetActorRotation();
        FRotator DeltaRot    = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
        AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.f, 90.f);
        AimYaw   = FMath::Clamp(DeltaRot.Yaw,   -90.f, 90.f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  2. FOOT IK
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!bEnableFootIK || !CachedCharacter || bIsInAir) return;

    USkeletalMeshComponent* Mesh = CachedCharacter->GetMesh();
    if (!Mesh) return;

    UWorld* World = CachedCharacter->GetWorld();
    if (!World) return;

    // Trace for each foot socket
    FVector LeftOffset  = FVector::ZeroVector;
    FVector RightOffset = FVector::ZeroVector;
    FRotator LeftRot    = FRotator::ZeroRotator;
    FRotator RightRot   = FRotator::ZeroRotator;

    TraceFootIK(World, Mesh, FName("foot_l"), LeftOffset,  LeftRot);
    TraceFootIK(World, Mesh, FName("foot_r"), RightOffset, RightRot);

    // Smooth offsets
    LeftFootOffset  = FMath::VInterpTo(LeftFootOffset,  LeftOffset,  DeltaSeconds, 15.f);
    RightFootOffset = FMath::VInterpTo(RightFootOffset, RightOffset, DeltaSeconds, 15.f);
    LeftFootRotation  = FMath::RInterpTo(LeftFootRotation,  LeftRot,  DeltaSeconds, FootRotSmoothSpeed);
    RightFootRotation = FMath::RInterpTo(RightFootRotation, RightRot, DeltaSeconds, FootRotSmoothSpeed);

    // Pelvis offset = lowest foot drives pelvis down
    const float LowestFoot = FMath::Min(LeftFootOffset.Z, RightFootOffset.Z);
    const float TargetPelvis = FMath::Clamp(LowestFoot, -30.f, 0.f);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, PelvisSmoothSpeed);

    // IK alpha — fade out when moving fast
    const float AlphaTarget = (Speed < 200.f) ? 1.f : FMath::Clamp(1.f - (Speed - 200.f) / 200.f, 0.f, 1.f);
    LeftFootIKAlpha  = FMath::FInterpTo(LeftFootIKAlpha,  AlphaTarget, DeltaSeconds, 10.f);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, AlphaTarget, DeltaSeconds, 10.f);
}

void UDinoSurvivorAnimInstance::TraceFootIK(
    UWorld* World,
    USkeletalMeshComponent* Mesh,
    FName SocketName,
    FVector& OutOffset,
    FRotator& OutRotation)
{
    if (!World || !Mesh) return;

    const FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart = SocketLoc + FVector(0.f, 0.f, IKTraceDistance);
    const FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, IKTraceDistance);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(CachedCharacter);

    if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        // Offset from socket to hit point
        OutOffset = Hit.ImpactPoint - SocketLoc;
        OutOffset.X = 0.f;
        OutOffset.Y = 0.f;

        // Surface normal → foot rotation
        const FVector Normal = Hit.ImpactNormal;
        OutRotation.Pitch = FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z));
        OutRotation.Roll  = FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z)) * -1.f;
        OutRotation.Yaw   = 0.f;
    }
    else
    {
        OutOffset   = FVector::ZeroVector;
        OutRotation = FRotator::ZeroRotator;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  3. SURVIVAL STATE
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
    // Read survival stats from the character if it exposes them
    // We use a generic property read approach to avoid hard coupling
    if (!CachedCharacter) return;

    // Try to read float properties by name (works if TranspersonalCharacter exposes them)
    auto ReadFloat = [&](const FName& PropName, float DefaultVal) -> float
    {
        if (UFloatProperty* Prop = FindField<UFloatProperty>(CachedCharacter->GetClass(), PropName))
        {
            return Prop->GetPropertyValue_InContainer(CachedCharacter);
        }
        return DefaultVal;
    };

    HealthNormalized  = FMath::Clamp(ReadFloat(FName("Health"),   100.f) / 100.f, 0.f, 1.f);
    StaminaNormalized = FMath::Clamp(ReadFloat(FName("Stamina"),  100.f) / 100.f, 0.f, 1.f);
    HungerNormalized  = FMath::Clamp(ReadFloat(FName("Hunger"),   100.f) / 100.f, 0.f, 1.f);
    ThirstNormalized  = FMath::Clamp(ReadFloat(FName("Thirst"),   100.f) / 100.f, 0.f, 1.f);

    bIsExhausted = (StaminaNormalized < 0.1f);
    bIsWounded   = (HealthNormalized  < 0.3f);
    bIsStarving  = (HungerNormalized  < 0.1f) || (ThirstNormalized < 0.1f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  4. DINO PROXIMITY
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateDinoProximity(float DeltaSeconds)
{
    if (!CachedCharacter) return;

    UWorld* World = CachedCharacter->GetWorld();
    if (!World) return;

    const FVector CharLoc = CachedCharacter->GetActorLocation();
    float ClosestDist = 9999.f;
    AActor* ClosestDino = nullptr;

    // Find all actors with "dino", "rex", "raptor", "brach" in their label
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        const FString Label = Actor->GetActorLabel().ToLower();
        if (Label.Contains(TEXT("rex"))    || Label.Contains(TEXT("raptor")) ||
            Label.Contains(TEXT("brach"))  || Label.Contains(TEXT("dino"))   ||
            Label.Contains(TEXT("trex"))   || Label.Contains(TEXT("veloci")))
        {
            const float Dist = FVector::Dist(CharLoc, Actor->GetActorLocation());
            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                ClosestDino = Actor;
            }
        }
    }

    NearestDinoDistance = ClosestDist;
    bDinoInSight        = (ClosestDist < 1500.f);

    if (ClosestDino)
    {
        DinoThreatDirection = (ClosestDino->GetActorLocation() - CharLoc).GetSafeNormal();
    }

    // Fear level — inverse distance, smoothed
    const float TargetFear = bDinoInSight
        ? FMath::Clamp(1.f - (ClosestDist / 1500.f), 0.f, 1.f)
        : 0.f;
    FearLevel = FMath::FInterpTo(FearLevel, TargetFear, DeltaSeconds, FearSmoothSpeed);
}

// ─────────────────────────────────────────────────────────────────────────────
//  5. UPPER-BODY OVERLAY
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateUpperBodyOverlay(float DeltaSeconds)
{
    if (!CachedCharacter) return;

    // Aiming: check if controller is looking significantly away from actor forward
    if (APlayerController* PC = Cast<APlayerController>(CachedCharacter->GetController()))
    {
        // bIsAiming driven externally (e.g. from input component) — we just maintain aim angles
        // Already computed in UpdateLocomotionState
    }

    // Carrying / climbing flags — read from character tags
    bIsCarrying = CachedCharacter->ActorHasTag(FName("Carrying"));
    bIsClimbing = CachedCharacter->ActorHasTag(FName("Climbing"));
}

// ─────────────────────────────────────────────────────────────────────────────
//  BLUEPRINT CALLABLE HELPERS
// ─────────────────────────────────────────────────────────────────────────────

float UDinoSurvivorAnimInstance::GetLocomotionSpeed() const
{
    return Speed;
}

float UDinoSurvivorAnimInstance::GetLocomotionDirection() const
{
    return Direction;
}

bool UDinoSurvivorAnimInstance::GetIsInAir() const
{
    return bIsInAir;
}

float UDinoSurvivorAnimInstance::GetFearLevel() const
{
    return FearLevel;
}

float UDinoSurvivorAnimInstance::GetHealthNormalized() const
{
    return HealthNormalized;
}

float UDinoSurvivorAnimInstance::GetStaminaNormalized() const
{
    return StaminaNormalized;
}

bool UDinoSurvivorAnimInstance::GetIsDinoNearby() const
{
    return bDinoInSight;
}

float UDinoSurvivorAnimInstance::GetNearestDinoDistance() const
{
    return NearestDinoDistance;
}
