// VFXManager.cpp — VFX Agent #17
// PROD_CYCLE_AUTO_20260620_006
// Implements all VFX management for the prehistoric survival game.
// Campfire glow, footstep dust, blood splatter, weather particles, environmental ambience.

#include "VFXManager.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Campfire primary glow light
    CampfireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CampfireLight"));
    CampfireLight->SetupAttachment(RootComponent);
    CampfireLight->SetIntensity(3000.0f);
    CampfireLight->SetLightColor(FLinearColor(1.0f, 0.45f, 0.1f, 1.0f));
    CampfireLight->SetAttenuationRadius(600.0f);
    CampfireLight->SetCastShadows(true);
    CampfireLight->SetVisibility(false); // hidden until campfire is spawned

    // Campfire flicker secondary light
    CampfireFlickerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CampfireFlickerLight"));
    CampfireFlickerLight->SetupAttachment(RootComponent);
    CampfireFlickerLight->SetIntensity(1500.0f);
    CampfireFlickerLight->SetLightColor(FLinearColor(1.0f, 0.6f, 0.15f, 1.0f));
    CampfireFlickerLight->SetAttenuationRadius(300.0f);
    CampfireFlickerLight->SetCastShadows(false);
    CampfireFlickerLight->SetVisibility(false);
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    FlickerTimer = 0.0f;
    FlickerPhase = FMath::RandRange(0.0f, 6.28f); // random phase offset per campfire
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bFlickerActive)
    {
        UpdateCampfireFlicker(DeltaTime);
    }

    if (WeatherTransitionAlpha > 0.0f && WeatherTransitionAlpha < 1.0f)
    {
        UpdateWeatherTransition(DeltaTime);
    }
}

// ─── Campfire VFX ─────────────────────────────────────────────────────────────

void AVFXManager::SpawnCampfireVFX(const FVFX_CampfireData& CampfireData)
{
    if (!CampfireData.bIsActive)
    {
        ExtinguishCampfire(CampfireData.Location, false);
        return;
    }

    // Position the actor at campfire location
    SetActorLocation(CampfireData.Location);

    // Configure primary light
    if (CampfireLight)
    {
        CampfireLight->SetVisibility(true);
        CampfireLight->SetIntensity(3000.0f * CampfireData.FlameIntensity);
        CampfireLight->SetAttenuationRadius(CampfireData.LightRadius);
        CampfireLight->SetLightColor(FLinearColor(1.0f, 0.45f * CampfireData.FlameIntensity, 0.1f, 1.0f));
    }

    // Configure flicker light
    if (CampfireFlickerLight)
    {
        CampfireFlickerLight->SetVisibility(true);
        CampfireFlickerLight->SetIntensity(1500.0f * CampfireData.FlameIntensity);
        CampfireFlickerLight->SetAttenuationRadius(CampfireData.LightRadius * 0.5f);
    }

    bFlickerActive = true;

    // Register in active campfires list
    ActiveCampfires.Add(CampfireData);

    UE_LOG(LogTemp, Log, TEXT("VFX: Campfire spawned at (%.0f, %.0f, %.0f) intensity=%.2f"),
           CampfireData.Location.X, CampfireData.Location.Y, CampfireData.Location.Z,
           CampfireData.FlameIntensity);
}

void AVFXManager::ExtinguishCampfire(FVector Location, bool bPlaySteamEffect)
{
    if (CampfireLight)
    {
        CampfireLight->SetVisibility(false);
        CampfireLight->SetIntensity(0.0f);
    }

    if (CampfireFlickerLight)
    {
        CampfireFlickerLight->SetVisibility(false);
        CampfireFlickerLight->SetIntensity(0.0f);
    }

    bFlickerActive = false;

    // Remove from active list
    ActiveCampfires.RemoveAll([&Location](const FVFX_CampfireData& Data)
    {
        return FVector::DistSquared(Data.Location, Location) < (50.0f * 50.0f);
    });

    UE_LOG(LogTemp, Log, TEXT("VFX: Campfire extinguished at (%.0f, %.0f, %.0f) steam=%d"),
           Location.X, Location.Y, Location.Z, bPlaySteamEffect ? 1 : 0);
}

void AVFXManager::SetCampfireIntensity(FVector Location, float NewIntensity)
{
    NewIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);

    if (CampfireLight)
    {
        CampfireLight->SetIntensity(3000.0f * NewIntensity);
        CampfireLight->SetLightColor(FLinearColor(1.0f, 0.45f * FMath::Max(NewIntensity, 0.1f), 0.1f, 1.0f));
    }

    if (CampfireFlickerLight)
    {
        CampfireFlickerLight->SetIntensity(1500.0f * NewIntensity);
    }

    if (NewIntensity <= 0.01f)
    {
        ExtinguishCampfire(Location, false);
    }
}

// ─── Impact VFX ───────────────────────────────────────────────────────────────

void AVFXManager::SpawnImpactVFX(const FVFX_ImpactData& ImpactData)
{
    // In full implementation, this spawns the appropriate Niagara system.
    // For now, log the impact for debugging and draw a debug sphere.
    UWorld* World = GetWorld();
    if (!World) return;

    float DebugRadius = ImpactData.DustRadius * ImpactData.ImpactForce;

#if WITH_EDITOR
    DrawDebugSphere(World, ImpactData.HitLocation, DebugRadius, 8,
                    FColor::Orange, false, 0.5f, 0, 1.0f);
#endif

    UE_LOG(LogTemp, Verbose, TEXT("VFX: Impact type=%d at (%.0f,%.0f,%.0f) force=%.2f"),
           (int32)ImpactData.ImpactType,
           ImpactData.HitLocation.X, ImpactData.HitLocation.Y, ImpactData.HitLocation.Z,
           ImpactData.ImpactForce);
}

void AVFXManager::SpawnDinoFootstepDust(FVector FootLocation, FVector FootNormal, float DinoMass)
{
    FVFX_ImpactData Impact;
    Impact.HitLocation = FootLocation;
    Impact.HitNormal = FootNormal;
    Impact.ImpactForce = FMath::Clamp(DinoMass / 5000.0f, 0.1f, 3.0f); // T-Rex ~8000kg → force 1.6
    Impact.DustRadius = FMath::Clamp(DinoMass / 100.0f, 30.0f, 200.0f); // cm

    // Terrain type determines dust vs mud vs rock particles
    // For now default to dirt — terrain query will be added when landscape is queryable
    Impact.ImpactType = EVFX_ImpactType::DinoFootstepDirt;

    SpawnImpactVFX(Impact);

    UE_LOG(LogTemp, Log, TEXT("VFX: Dino footstep dust at (%.0f,%.0f,%.0f) mass=%.0fkg radius=%.0fcm"),
           FootLocation.X, FootLocation.Y, FootLocation.Z, DinoMass, Impact.DustRadius);
}

void AVFXManager::SpawnBloodSplatter(FVector HitLocation, FVector HitDirection, float DamageAmount)
{
    FVFX_ImpactData Impact;
    Impact.HitLocation = HitLocation;
    Impact.HitNormal = HitDirection.GetSafeNormal();
    Impact.ImpactType = EVFX_ImpactType::BloodSplatter;
    Impact.ImpactForce = FMath::Clamp(DamageAmount / 50.0f, 0.1f, 2.0f);
    Impact.DustRadius = 40.0f;

    SpawnImpactVFX(Impact);
}

// ─── Weather VFX ──────────────────────────────────────────────────────────────

void AVFXManager::SetWeatherVFX(EVFX_WeatherType WeatherType, float TransitionTime)
{
    if (CurrentWeather == WeatherType) return;

    CurrentWeather = WeatherType;
    WeatherTransitionAlpha = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("VFX: Weather transition to type=%d over %.1fs"),
           (int32)WeatherType, TransitionTime);
}

// ─── Environmental VFX ────────────────────────────────────────────────────────

void AVFXManager::SetZoneAmbientVFX(EVFX_ZoneType ZoneType, FVector ZoneCenter, float ZoneRadius)
{
    ActiveZoneType = ZoneType;

    UE_LOG(LogTemp, Log, TEXT("VFX: Zone ambient VFX set to type=%d center=(%.0f,%.0f,%.0f) radius=%.0f"),
           (int32)ZoneType, ZoneCenter.X, ZoneCenter.Y, ZoneCenter.Z, ZoneRadius);
}

void AVFXManager::TriggerVolcanicAshBurst(FVector EruptionDirection, float Intensity)
{
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("VFX: Volcanic ash burst dir=(%.2f,%.2f,%.2f) intensity=%.2f"),
           EruptionDirection.X, EruptionDirection.Y, EruptionDirection.Z, Intensity);
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

void AVFXManager::UpdateCampfireFlicker(float DeltaTime)
{
    FlickerTimer += DeltaTime;
    FlickerPhase += DeltaTime * 3.5f; // flicker frequency ~3.5 Hz

    // Perlin-like flicker using multiple sine waves
    float Flicker = 0.85f
        + 0.08f * FMath::Sin(FlickerPhase * 1.0f + FlickerTimer * 2.3f)
        + 0.04f * FMath::Sin(FlickerPhase * 2.7f + FlickerTimer * 5.1f)
        + 0.03f * FMath::Sin(FlickerPhase * 4.1f + FlickerTimer * 1.7f);

    Flicker = FMath::Clamp(Flicker, 0.6f, 1.15f);

    if (CampfireFlickerLight && CampfireFlickerLight->IsVisible())
    {
        float BaseIntensity = 1500.0f;
        CampfireFlickerLight->SetIntensity(BaseIntensity * Flicker);

        // Slight color temperature shift during flicker
        float Warmth = 0.55f + 0.1f * FMath::Sin(FlickerPhase * 0.8f);
        CampfireFlickerLight->SetLightColor(FLinearColor(1.0f, Warmth, 0.1f, 1.0f));
    }
}

void AVFXManager::UpdateWeatherTransition(float DeltaTime)
{
    WeatherTransitionAlpha = FMath::Min(WeatherTransitionAlpha + DeltaTime * 0.33f, 1.0f);
}
