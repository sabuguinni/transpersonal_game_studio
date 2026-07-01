// FootIKComponent.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Standalone UActorComponent that computes foot IK data via line traces.
// Attach to TranspersonalCharacter. Reads bone socket locations from the
// owning SkeletalMeshComponent and outputs FAnim_FootIKData each tick.

#include "FootIKComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

UFootIKComponent::UFootIKComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup    = TG_PrePhysics;

    // Defaults
    bFootIKActive       = true;
    TraceDistance       = 80.0f;
    InterpSpeed         = 15.0f;
    MaxFootOffset       = 25.0f;
    LeftFootBoneName    = FName("foot_l");
    RightFootBoneName   = FName("foot_r");
    TraceChannel        = ECC_Visibility;
    bDrawDebugTraces    = false;

    // Zero-init IK data
    CachedIKData = FAnim_FootIKData();
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UFootIKComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerMesh = OwnerCharacter->GetMesh();
        UE_LOG(LogTemp, Log, TEXT("[FootIKComponent] Initialized on: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[FootIKComponent] Owner is not an ACharacter — IK disabled."));
        bFootIKActive = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void UFootIKComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bFootIKActive && OwnerMesh && OwnerCharacter)
    {
        UpdateFootIK(DeltaTime);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateFootIK — main IK computation
// ─────────────────────────────────────────────────────────────────────────────

void UFootIKComponent::UpdateFootIK(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // ── Left foot trace ──
    TraceAndUpdateFoot(
        DeltaTime, World,
        LeftFootBoneName,
        CachedIKData.LeftFootLocation,
        CachedIKData.LeftFootRotation,
        CachedIKData.LeftFootOffset
    );

    // ── Right foot trace ──
    TraceAndUpdateFoot(
        DeltaTime, World,
        RightFootBoneName,
        CachedIKData.RightFootLocation,
        CachedIKData.RightFootRotation,
        CachedIKData.RightFootOffset
    );

    // ── Pelvis correction — drive pelvis down to lowest foot ──
    const float LowestZ = FMath::Min(CachedIKData.LeftFootOffset.Z, CachedIKData.RightFootOffset.Z);
    const float ClampedOffset = FMath::Clamp(LowestZ, -MaxFootOffset, 0.0f);
    CachedIKData.PelvisOffset = FMath::FInterpTo(
        CachedIKData.PelvisOffset, ClampedOffset, DeltaTime, InterpSpeed * 0.5f);

    // ── Terrain flag — true when either foot has significant offset ──
    CachedIKData.bIsOnUnevenTerrain =
        (FMath::Abs(CachedIKData.LeftFootOffset.Z)  > 5.0f) ||
        (FMath::Abs(CachedIKData.RightFootOffset.Z) > 5.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// TraceAndUpdateFoot — helper for single foot
// ─────────────────────────────────────────────────────────────────────────────

void UFootIKComponent::TraceAndUpdateFoot(float DeltaTime, UWorld* World,
                                           const FName& BoneName,
                                           FVector& OutLocation,
                                           FRotator& OutRotation,
                                           FVector& OutOffset)
{
    if (!OwnerMesh) return;

    const FVector BoneLoc    = OwnerMesh->GetSocketLocation(BoneName);
    const FVector TraceStart = FVector(BoneLoc.X, BoneLoc.Y, BoneLoc.Z + 30.0f);
    const FVector TraceEnd   = FVector(BoneLoc.X, BoneLoc.Y, BoneLoc.Z - TraceDistance);

    FHitResult Hit;
    FCollisionQueryParams Params(NAME_None, false, OwnerCharacter);

    const bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TraceChannel, Params);

    if (bDrawDebugTraces)
    {
        DrawDebugLine(World, TraceStart, TraceEnd,
            bHit ? FColor::Green : FColor::Red, false, -1.0f, 0, 1.5f);
        if (bHit) DrawDebugSphere(World, Hit.Location, 5.0f, 8, FColor::Yellow, false, -1.0f);
    }

    if (bHit)
    {
        // Offset = how much the foot needs to move vertically
        const FVector TargetOffset = FVector(0.0f, 0.0f,
            FMath::Clamp(Hit.Location.Z - BoneLoc.Z, -MaxFootOffset, MaxFootOffset));

        OutOffset   = FMath::VInterpTo(OutOffset,   TargetOffset,          DeltaTime, InterpSpeed);
        OutLocation = FMath::VInterpTo(OutLocation, Hit.Location,           DeltaTime, InterpSpeed);

        // Derive foot rotation from surface normal
        const FRotator TargetRot = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Hit.Normal.X, Hit.Normal.Z)),
            0.0f,
            -FMath::RadiansToDegrees(FMath::Atan2(Hit.Normal.Y, Hit.Normal.Z))
        );
        OutRotation = FMath::RInterpTo(OutRotation, TargetRot, DeltaTime, InterpSpeed);
    }
    else
    {
        // Smoothly return to neutral
        OutOffset   = FMath::VInterpTo(OutOffset,   FVector::ZeroVector,   DeltaTime, InterpSpeed);
        OutLocation = FMath::VInterpTo(OutLocation, BoneLoc,               DeltaTime, InterpSpeed);
        OutRotation = FMath::RInterpTo(OutRotation, FRotator::ZeroRotator, DeltaTime, InterpSpeed);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// GetFootIKData — Blueprint callable accessor
// ─────────────────────────────────────────────────────────────────────────────

FAnim_FootIKData UFootIKComponent::GetFootIKData() const
{
    return CachedIKData;
}

// ─────────────────────────────────────────────────────────────────────────────
// SetFootIKEnabled — runtime toggle (e.g., disable during swimming/climbing)
// ─────────────────────────────────────────────────────────────────────────────

void UFootIKComponent::SetFootIKEnabled(bool bEnabled)
{
    bFootIKActive = bEnabled;

    if (!bEnabled)
    {
        // Reset to neutral immediately
        CachedIKData = FAnim_FootIKData();
    }

    UE_LOG(LogTemp, Log, TEXT("[FootIKComponent] Foot IK %s"), bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}
