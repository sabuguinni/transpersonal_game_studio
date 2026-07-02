// VFXSystemManager.h — VFX Agent #17
// Manages all Niagara particle systems for the prehistoric survival game.
// Categories: Campfire, Dinosaur Footstep, Weather Rain, Blood Impact, Breath Vapor, Volcanic Ash

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/PointLightComponent.h"
#include "VFXSystemManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EVFX_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Fog         UMETA(DisplayName = "Fog"),
    AshFall     UMETA(DisplayName = "Volcanic Ash Fall"),
};

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    BloodFlesh  UMETA(DisplayName = "Blood on Flesh"),
    BloodGround UMETA(DisplayName = "Blood on Ground"),
    DustDirt    UMETA(DisplayName = "Dust on Dirt"),
    RockSpark   UMETA(DisplayName = "Rock Spark"),
    WaterSplash UMETA(DisplayName = "Water Splash"),
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FVFX_CampfireConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FlameIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float SmokeOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float EmberRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    bool bIsWindy = false;
};

USTRUCT(BlueprintType)
struct FVFX_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DinosaurMassKg = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DustRadiusCm = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float ShockwaveRadiusCm = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    bool bIsWetGround = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// MAIN CLASS
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "TranspersonalGame|VFX", meta = (DisplayName = "VFX System Manager"))
class TRANSPERSONALGAME_API AVFX_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── CAMPFIRE ──────────────────────────────────────────────────────────────

    /** Spawn campfire VFX (fire + smoke + embers) at the given world location */
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void SpawnCampfireVFX(FVector Location, FVFX_CampfireConfig Config);

    /** Update campfire intensity (e.g., wind gust, dying fire) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void UpdateCampfireIntensity(float NewIntensity);

    /** Extinguish the active campfire VFX */
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void ExtinguishCampfire();

    // ── DINOSAUR FOOTSTEP ─────────────────────────────────────────────────────

    /** Spawn footstep dust + ground crack decal at impact location */
    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void SpawnFootstepVFX(FVector ImpactLocation, FVFX_FootstepConfig Config);

    /** Spawn breath vapor from predator (cold environment only) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Breath")
    void SpawnBreathVaporVFX(FVector NostrilLocation, FRotator ForwardDirection, float BreathStrength = 1.0f);

    // ── WEATHER ───────────────────────────────────────────────────────────────

    /** Transition to a new weather state with blend time */
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherState(EVFX_WeatherState NewState, float BlendTimeSec = 5.0f);

    /** Get current weather state */
    UFUNCTION(BlueprintPure, Category = "VFX|Weather")
    EVFX_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    // ── COMBAT IMPACT ─────────────────────────────────────────────────────────

    /** Spawn impact VFX at hit location (blood, dust, sparks, water) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void SpawnImpactVFX(FVector HitLocation, FVector HitNormal, EVFX_ImpactType ImpactType, float Scale = 1.0f);

    /** Spawn blood trail decal on ground (for wounded creature moving) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void SpawnBloodTrailDecal(FVector Location, float BloodAmount = 1.0f);

    // ── VOLCANIC ASH ──────────────────────────────────────────────────────────

    /** Enable/disable volcanic ash particle drift (global ambient effect) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void SetVolcanicAshEnabled(bool bEnabled, float AshDensity = 0.5f);

    /** Spawn distant volcano eruption glow (post-process + particle column) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void TriggerVolcanicEruptionGlow(FVector VolcanoLocation, float Intensity = 1.0f);

    // ── LOD MANAGEMENT ───────────────────────────────────────────────────────

    /** Set VFX LOD level based on distance from camera (0=full, 1=medium, 2=minimal) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetVFXLODLevel(int32 LODLevel);

    /** Get recommended LOD level for a given distance from camera */
    UFUNCTION(BlueprintPure, Category = "VFX|Performance")
    int32 GetRecommendedLOD(float DistanceFromCamera) const;

protected:
    // ── NIAGARA ASSET REFERENCES ──────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_Smoke;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_BreathVapor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Rain;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_AshFall;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_BloodImpact;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_DustImpact;

    // ── ACTIVE COMPONENTS ─────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State",
              meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* ActiveCampfireComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State",
              meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* ActiveWeatherComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State",
              meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* ActiveAshComponent;

    // ── CAMPFIRE LIGHT SIMULATION ─────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Campfire",
              meta = (AllowPrivateAccess = "true"))
    UPointLightComponent* CampfireFlameLight;

    // ── STATE ─────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State",
              meta = (AllowPrivateAccess = "true"))
    EVFX_WeatherState CurrentWeatherState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State",
              meta = (AllowPrivateAccess = "true"))
    int32 CurrentLODLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State",
              meta = (AllowPrivateAccess = "true"))
    bool bCampfireActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State",
              meta = (AllowPrivateAccess = "true"))
    bool bVolcanicAshActive;

private:
    // Campfire flicker simulation
    float CampfireFlickerTimer;
    float CampfireBaseIntensity;

    void TickCampfireFlicker(float DeltaTime);
    void ApplyLODToComponent(UNiagaraComponent* Component, int32 LODLevel);
};
