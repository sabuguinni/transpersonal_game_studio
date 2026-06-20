#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXNiagaraController.generated.h"

// ============================================================
// VFX Niagara Controller — Agent #17 VFX
// Manages Niagara particle system spawning and lifecycle for
// the prehistoric survival game. All effects are physically
// plausible (no magic/spiritual content).
// ============================================================

UENUM(BlueprintType)
enum class EVFX_NiagaraEffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    CampfireFlame   UMETA(DisplayName = "Campfire Flame"),
    CampfireSmoke   UMETA(DisplayName = "Campfire Smoke"),
    CampfireEmbers  UMETA(DisplayName = "Campfire Embers"),
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    FootstepMud     UMETA(DisplayName = "Footstep Mud"),
    FootstepWater   UMETA(DisplayName = "Footstep Water Splash"),
    RainDrops       UMETA(DisplayName = "Rain Drops"),
    RainSplash      UMETA(DisplayName = "Rain Ground Splash"),
    SnowFall        UMETA(DisplayName = "Snow Fall"),
    FogMist         UMETA(DisplayName = "Fog Mist"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    VolcanicEmbers  UMETA(DisplayName = "Volcanic Embers"),
    BloodImpact     UMETA(DisplayName = "Blood Impact"),
    BloodTrail      UMETA(DisplayName = "Blood Trail"),
    DustCloud       UMETA(DisplayName = "Dust Cloud"),
    WaterRipple     UMETA(DisplayName = "Water Ripple"),
    WaterfallMist   UMETA(DisplayName = "Waterfall Mist"),
    BreathVapor     UMETA(DisplayName = "Breath Vapor"),
    InsectSwarm     UMETA(DisplayName = "Insect Swarm"),
    PollenDrift     UMETA(DisplayName = "Pollen Drift"),
    SparkCrafting   UMETA(DisplayName = "Crafting Sparks"),
    CookingSmoke    UMETA(DisplayName = "Cooking Smoke"),
    DinoRoarDistort UMETA(DisplayName = "Dino Roar Air Distortion"),
    WeaponImpact    UMETA(DisplayName = "Weapon Impact Debris"),
};

UENUM(BlueprintType)
enum class EVFX_LODLevel : uint8
{
    High   UMETA(DisplayName = "High (Near)"),
    Medium UMETA(DisplayName = "Medium (Mid)"),
    Low    UMETA(DisplayName = "Low (Far)"),
    Culled UMETA(DisplayName = "Culled (Beyond Range)"),
};

UENUM(BlueprintType)
enum class EVFX_TerrainType : uint8
{
    Dirt  UMETA(DisplayName = "Dirt"),
    Rock  UMETA(DisplayName = "Rock"),
    Mud   UMETA(DisplayName = "Mud"),
    Sand  UMETA(DisplayName = "Sand"),
    Water UMETA(DisplayName = "Water"),
    Snow  UMETA(DisplayName = "Snow"),
    Grass UMETA(DisplayName = "Grass"),
};

// ============================================================
// Structs (global scope — RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FVFX_NiagaraEffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    EVFX_NiagaraEffectType EffectType = EVFX_NiagaraEffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float SpawnRate = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float ParticleLifetime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float EmitterRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    FLinearColor TintColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float IntensityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float LOD_HighDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float LOD_MediumDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float LOD_CullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bAutoActivate = true;
};

USTRUCT(BlueprintType)
struct FVFX_FootstepParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    EVFX_TerrainType TerrainType = EVFX_TerrainType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float FootSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float ImpactVelocity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    bool bIsHeavyCreature = false;
};

USTRUCT(BlueprintType)
struct FVFX_CampfireState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FuelLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float WindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    FVector WindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    bool bIsRaining = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FlameHeight = 80.0f;
};

USTRUCT(BlueprintType)
struct FVFX_WeatherParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float SnowIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float FogDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WindSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    FVector WindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float VolcanicAshDensity = 0.0f;
};

// ============================================================
// UVFX_NiagaraController — Actor Component
// ============================================================

UCLASS(ClassGroup = "VFX", meta = (BlueprintSpawnableComponent), DisplayName = "VFX Niagara Controller")
class TRANSPERSONALGAME_API UVFX_NiagaraController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraController();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Campfire ----
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void SpawnCampfireEffect(FVector Location, FVFX_CampfireState State);

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void UpdateCampfireState(FVFX_CampfireState NewState);

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void ExtinguishCampfire();

    // ---- Footstep ----
    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void SpawnFootstepEffect(FVector ImpactLocation, FVFX_FootstepParams Params);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void SpawnDinosaurFootstep(FVector ImpactLocation, float DinoMassKg, EVFX_TerrainType Terrain);

    // ---- Weather ----
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherParams(FVFX_WeatherParams Params);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void TransitionWeather(FVFX_WeatherParams TargetParams, float TransitionDuration);

    // ---- Combat ----
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodImpact(FVector Location, FVector ImpactNormal, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnWeaponImpact(FVector Location, FVector ImpactNormal, EVFX_TerrainType Surface);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnDinoRoarDistortion(FVector DinoLocation, float RoarIntensity);

    // ---- Environment ----
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnDustCloud(FVector Location, float Radius, float Duration);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnWaterfallMist(FVector BaseLocation, float WaterfallHeight);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnVolcanicAsh(FVector Location, FVFX_WeatherParams WeatherParams);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnBreathVapor(FVector MouthLocation, FVector ForwardVector, float Temperature);

    // ---- LOD ----
    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    EVFX_LODLevel CalculateLODLevel(FVector EffectLocation) const;

    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    float GetSpawnRateForLOD(float BaseRate, EVFX_LODLevel LOD) const;

    // ---- Config ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    TArray<FVFX_NiagaraEffectConfig> EffectConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float GlobalVFXIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bEnableBloodVFX = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bEnableWeatherVFX = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bEnableFootstepVFX = true;

private:
    UPROPERTY()
    FVFX_CampfireState CurrentCampfireState;

    UPROPERTY()
    FVFX_WeatherParams CurrentWeatherParams;

    UPROPERTY()
    FVFX_WeatherParams TargetWeatherParams;

    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 0.0f;
    bool bTransitioningWeather = false;

    void TickWeatherTransition(float DeltaTime);
    FVFX_NiagaraEffectConfig GetConfigForEffect(EVFX_NiagaraEffectType EffectType) const;
    FLinearColor GetTerrainDustColor(EVFX_TerrainType Terrain) const;
};
