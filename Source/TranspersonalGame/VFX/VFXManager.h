#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "../Core/SharedTypes.h"
#include "VFXManager.generated.h"

/**
 * VFX MANAGER - Transpersonal Game Studio
 * Agent #17 - VFX Agent
 * 
 * Manages all visual effects in the prehistoric survival game.
 * Handles footsteps, environmental effects, combat impacts, and atmospheric VFX.
 * Integrates with AudioManager for synchronized audio-visual effects.
 */

// VFX-specific enums (using VFX_ prefix to avoid conflicts)
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Footstep = 0        UMETA(DisplayName = "Footstep"),
    DustCloud = 1       UMETA(DisplayName = "Dust Cloud"),
    BloodImpact = 2     UMETA(DisplayName = "Blood Impact"),
    Fire = 3            UMETA(DisplayName = "Fire"),
    Water = 4           UMETA(DisplayName = "Water"),
    Wind = 5            UMETA(DisplayName = "Wind"),
    Explosion = 6       UMETA(DisplayName = "Explosion"),
    Environmental = 7   UMETA(DisplayName = "Environmental")
};

UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low = 0             UMETA(DisplayName = "Low"),
    Medium = 1          UMETA(DisplayName = "Medium"),
    High = 2            UMETA(DisplayName = "High"),
    Extreme = 3         UMETA(DisplayName = "Extreme")
};

UENUM(BlueprintType)
enum class EVFX_TerrainType : uint8
{
    Dirt = 0            UMETA(DisplayName = "Dirt"),
    Grass = 1           UMETA(DisplayName = "Grass"),
    Rock = 2            UMETA(DisplayName = "Rock"),
    Sand = 3            UMETA(DisplayName = "Sand"),
    Mud = 4             UMETA(DisplayName = "Mud"),
    Water = 5           UMETA(DisplayName = "Water"),
    Snow = 6            UMETA(DisplayName = "Snow")
};

// VFX configuration structs
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_EffectType EffectType = EVFX_EffectType::Footstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FLinearColor TintColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    FVFX_EffectConfig()
    {
        EffectType = EVFX_EffectType::Footstep;
        Intensity = EVFX_IntensityLevel::Medium;
        Duration = 2.0f;
        Scale = 1.0f;
        TintColor = FLinearColor::White;
        bAutoDestroy = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    EVFX_TerrainType TerrainType = EVFX_TerrainType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float CreatureWeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float MovementSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    bool bIsRunning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    FVector ImpactLocation = FVector::ZeroVector;

    FVFX_FootstepData()
    {
        TerrainType = EVFX_TerrainType::Dirt;
        CreatureWeight = 100.0f;
        MovementSpeed = 5.0f;
        bIsRunning = false;
        ImpactLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EnvironmentalSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EEng_BiomeType CurrentBiome = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bIsRaining = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TimeOfDay = 12.0f; // 0-24 hours

    FVFX_EnvironmentalSettings()
    {
        CurrentBiome = EEng_BiomeType::Forest;
        WindStrength = 0.5f;
        Humidity = 0.5f;
        Temperature = 20.0f;
        bIsRaining = false;
        TimeOfDay = 12.0f;
    }
};

/**
 * VFX Manager Component
 * Handles all visual effects in the game world
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_Manager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_Manager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void InitializeVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ShutdownVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateEnvironmentalSettings(const FVFX_EnvironmentalSettings& NewSettings);

    // Footstep VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void TriggerFootstepEffect(const FVFX_FootstepData& FootstepData);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void CreateDinosaurFootstep(FVector Location, float Weight, EVFX_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void CreatePlayerFootstep(FVector Location, bool bIsRunning);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateCampfireEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateWaterSplashEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateWindParticles(FVector Location, FVector WindDirection, float Strength);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateDustCloudEffect(FVector Location, float Radius, float Intensity);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void CreateBloodImpactEffect(FVector Location, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void CreateWeaponImpactEffect(FVector Location, EVFX_TerrainType SurfaceType);

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "VFX|Audio")
    void SynchronizeWithAudioTension(float TensionLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Audio")
    void TriggerHeartbeatVFX(float HeartbeatIntensity);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    EVFX_TerrainType DetectTerrainType(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    float CalculateEffectIntensity(float BaseIntensity, EVFX_IntensityLevel Level);

    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    void CleanupExpiredEffects();

protected:
    // VFX System Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_EnvironmentalSettings EnvironmentalSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxEffectDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVFXLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EEng_PerformanceTier PerformanceTier = EEng_PerformanceTier::High;

    // Effect Configuration Arrays
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TArray<FVFX_EffectConfig> FootstepConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TArray<FVFX_EffectConfig> EnvironmentalConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TArray<FVFX_EffectConfig> CombatConfigs;

    // Active Effects Tracking
    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    TArray<class UNiagaraComponent*> ActiveNiagaraEffects;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    int32 CurrentActiveEffects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    float LastCleanupTime = 0.0f;

private:
    // Internal helper functions
    UNiagaraComponent* SpawnNiagaraEffect(class UNiagaraSystem* System, FVector Location, FRotator Rotation);
    void ConfigureEffectParameters(UNiagaraComponent* Effect, const FVFX_EffectConfig& Config);
    bool ShouldCullEffect(FVector EffectLocation);
    void ApplyLODSettings(UNiagaraComponent* Effect, float Distance);
};

/**
 * VFX Manager Actor
 * World-placed actor that manages global VFX systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ManagerActor : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ManagerActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Global VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void InitializeGlobalVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void UpdateGlobalVFXSettings();

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    UVFX_Manager* GetVFXManager() const { return VFXManagerComponent; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UVFX_Manager* VFXManagerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoInitialize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalVFXScale = 1.0f;
};