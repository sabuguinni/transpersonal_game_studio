#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../Core/SharedTypes.h"
#include "VFXEnvironmentManager.generated.h"

// VFX Environment Types
UENUM(BlueprintType)
enum class EVFX_EnvironmentType : uint8
{
    None        UMETA(DisplayName = "None"),
    Rain        UMETA(DisplayName = "Rain"),
    Snow        UMETA(DisplayName = "Snow"),
    Fog         UMETA(DisplayName = "Fog"),
    Dust        UMETA(DisplayName = "Dust Storm"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

// VFX Intensity Levels
UENUM(BlueprintType)
enum class EVFX_Intensity : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Extreme     UMETA(DisplayName = "Extreme")
};

// Environment VFX Configuration
USTRUCT(BlueprintType)
struct FVFX_EnvironmentConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    EVFX_EnvironmentType EnvironmentType = EVFX_EnvironmentType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    EVFX_Intensity Intensity = EVFX_Intensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    float Duration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    float FadeInTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    float FadeOutTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    FVector SpawnScale = FVector::OneVector;

    FVFX_EnvironmentConfig()
    {
        EnvironmentType = EVFX_EnvironmentType::None;
        Intensity = EVFX_Intensity::Medium;
        Duration = 60.0f;
        FadeInTime = 5.0f;
        FadeOutTime = 5.0f;
        SpawnLocation = FVector::ZeroVector;
        SpawnScale = FVector::OneVector;
    }
};

/**
 * VFX Environment Manager - Handles weather effects, atmospheric particles, and environmental VFX
 * Manages rain, snow, fog, dust storms, and volcanic ash effects across all biomes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_EnvironmentManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UVFX_EnvironmentManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Environment VFX Control
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StartEnvironmentEffect(EVFX_EnvironmentType EffectType, EVFX_Intensity Intensity, float Duration = 60.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StopEnvironmentEffect(EVFX_EnvironmentType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StopAllEnvironmentEffects();

    // Biome-Specific Effects
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StartBiomeWeather(EBiomeType BiomeType, EVFX_EnvironmentType WeatherType, EVFX_Intensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void TransitionWeather(EVFX_EnvironmentType FromType, EVFX_EnvironmentType ToType, float TransitionTime = 10.0f);

    // Instant Effects
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnDustCloud(FVector Location, float Radius = 500.0f, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnVolcanicAsh(FVector Location, float Radius = 1000.0f, float Duration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnBreathSteam(FVector Location, FVector Direction, float Duration = 3.0f);

    // Query Functions
    UFUNCTION(BlueprintPure, Category = "VFX Environment")
    bool IsEnvironmentEffectActive(EVFX_EnvironmentType EffectType) const;

    UFUNCTION(BlueprintPure, Category = "VFX Environment")
    EVFX_Intensity GetCurrentWeatherIntensity() const;

    UFUNCTION(BlueprintPure, Category = "VFX Environment")
    TArray<EVFX_EnvironmentType> GetActiveEffects() const;

protected:
    // Environment Effect Storage
    UPROPERTY()
    TMap<EVFX_EnvironmentType, class UNiagaraComponent*> ActiveEnvironmentEffects;

    UPROPERTY()
    TArray<class UNiagaraComponent*> TemporaryEffects;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config", meta = (AllowPrivateAccess = "true"))
    TMap<EVFX_EnvironmentType, FVFX_EnvironmentConfig> EnvironmentConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config", meta = (AllowPrivateAccess = "true"))
    float MaxEffectDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveEffects = 10;

private:
    // Internal Management
    void CreateEnvironmentEffect(EVFX_EnvironmentType EffectType, const FVFX_EnvironmentConfig& Config);
    void DestroyEnvironmentEffect(EVFX_EnvironmentType EffectType);
    void CleanupTemporaryEffects();
    UNiagaraSystem* GetNiagaraSystemForEnvironmentType(EVFX_EnvironmentType EffectType) const;
    
    // Timer Handles
    FTimerHandle EffectCleanupTimer;
    FTimerHandle WeatherTransitionTimer;

    // Current State
    EVFX_EnvironmentType CurrentWeatherType = EVFX_EnvironmentType::None;
    EVFX_Intensity CurrentIntensity = EVFX_Intensity::Medium;
};