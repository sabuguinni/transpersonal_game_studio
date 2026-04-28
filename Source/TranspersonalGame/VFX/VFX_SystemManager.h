#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Core/SharedTypes.h"
#include "VFX_SystemManager.generated.h"

/**
 * VFX System Manager - Handles all visual effects in the prehistoric world
 * Manages Niagara particle systems, environmental effects, and impact VFX
 * Agent #17 - VFX Agent
 */

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Impact          UMETA(DisplayName = "Impact"),
    Environmental   UMETA(DisplayName = "Environmental"),
    Weather         UMETA(DisplayName = "Weather"),
    Fire            UMETA(DisplayName = "Fire"),
    Water           UMETA(DisplayName = "Water"),
    Dust            UMETA(DisplayName = "Dust"),
    Blood           UMETA(DisplayName = "Blood"),
    Destruction     UMETA(DisplayName = "Destruction")
};

UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Minimal = 0     UMETA(DisplayName = "Minimal"),
    Low = 1         UMETA(DisplayName = "Low"),
    Medium = 2      UMETA(DisplayName = "Medium"),
    High = 3        UMETA(DisplayName = "High"),
    Extreme = 4     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Impact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bPlaySound = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Impact;
        Intensity = EVFX_IntensityLevel::Medium;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Duration = 2.0f;
        bAutoDestroy = true;
        bPlaySound = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_SystemSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_PerformanceTier PerformanceTier = EEng_PerformanceTier::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    float ParticleQualityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bEnableDistortion = true;

    FVFX_SystemSettings()
    {
        PerformanceTier = EEng_PerformanceTier::High;
        MaxActiveEffects = 50;
        EffectCullDistance = 5000.0f;
        bEnableLOD = true;
        ParticleQualityMultiplier = 1.0f;
        bEnableVolumetricFog = true;
        bEnableDistortion = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_SystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnEffect(const FVFX_EffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstep(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnImpactEffect(const FVector& Location, const FVector& Normal, EVFX_EffectType ImpactType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EVFX_EffectType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodEffect(const FVector& Location, const FVector& Direction, float Amount = 1.0f);

    // System management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetPerformanceTier(EEng_PerformanceTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateLODSettings();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FVFX_SystemSettings GetSystemSettings() const { return SystemSettings; }

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetSystemSettings(const FVFX_SystemSettings& NewSettings);

protected:
    // System settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FVFX_SystemSettings SystemSettings;

    // Effect templates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates")
    TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> EffectTemplates;

    // Sound effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EVFX_EffectType, TSoftObjectPtr<USoundBase>> EffectSounds;

    // Active effects tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance monitoring
    UPROPERTY()
    float LastCleanupTime = 0.0f;

    UPROPERTY()
    float CleanupInterval = 5.0f;

private:
    // Internal helper functions
    UNiagaraSystem* GetEffectTemplate(EVFX_EffectType EffectType);
    USoundBase* GetEffectSound(EVFX_EffectType EffectType);
    void PlayEffectSound(const FVector& Location, EVFX_EffectType EffectType, float VolumeMultiplier = 1.0f);
    void RegisterActiveEffect(UNiagaraComponent* Effect);
    void UnregisterActiveEffect(UNiagaraComponent* Effect);
    FVector CalculateLODScale(const FVector& EffectLocation, float BaseScale);
    bool ShouldCullEffect(const FVector& EffectLocation);
};