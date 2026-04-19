#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "VFXSystemManager.generated.h"

// VFX-specific enums and structs
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Impact          UMETA(DisplayName = "Impact"),
    Fire            UMETA(DisplayName = "Fire"),
    Weather         UMETA(DisplayName = "Weather"),
    Dust            UMETA(DisplayName = "Dust"),
    Blood           UMETA(DisplayName = "Blood"),
    Breath          UMETA(DisplayName = "Breath"),
    Destruction     UMETA(DisplayName = "Destruction")
};

UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low             UMETA(DisplayName = "Low"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Extreme         UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_IntensityLevel Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundCue> SoundEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Impact;
        Intensity = EVFX_IntensityLevel::Medium;
        Duration = 3.0f;
        Scale = 1.0f;
        bAutoDestroy = true;
    }
};

/**
 * VFX System Manager - Handles all visual effects in the prehistoric world
 * Focuses on realistic, physics-based effects: fire, dust, impacts, weather
 */
UCLASS()
class TRANSPERSONALGAME_API UVFX_SystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(FVector Location, bool bWithSound = true);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodImpact(FVector Location, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Radius = 1000.0f);

    // Effect management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetGlobalVFXQuality(int32 QualityLevel);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateLODDistances();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

protected:
    // Effect templates
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Templates")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectTemplates;

    // Active effects tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveSounds;

    // Performance settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    float LODDistance_High;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    float LODDistance_Medium;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
    float LODDistance_Low;

private:
    void InitializeEffectTemplates();
    void CleanupExpiredEffects();
    UNiagaraComponent* CreateNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation, float Scale);
    UAudioComponent* CreateSoundEffect(USoundCue* Sound, FVector Location, float Volume = 1.0f);
    
    // Timer for cleanup
    FTimerHandle CleanupTimerHandle;
};