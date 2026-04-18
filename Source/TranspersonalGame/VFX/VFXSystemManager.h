#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "VFXSystemManager.generated.h"

// VFX-specific enums for prehistoric survival effects
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fire            UMETA(DisplayName = "Fire"),
    Dust            UMETA(DisplayName = "Dust"),
    Water           UMETA(DisplayName = "Water"),
    Blood           UMETA(DisplayName = "Blood"),
    Impact          UMETA(DisplayName = "Impact"),
    Weather         UMETA(DisplayName = "Weather"),
    Footstep        UMETA(DisplayName = "Footstep"),
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

// VFX effect data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_IntensityLevel Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundCue> AudioCue;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        Intensity = EVFX_IntensityLevel::Medium;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

/**
 * VFX System Manager - Handles all visual effects for the prehistoric survival game
 * Manages Niagara particle systems, audio-visual synchronization, and performance optimization
 * Focus: Realistic prehistoric effects (fire, dust, water, blood, weather, impacts)
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_SystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnEffect(const FVFX_EffectData& EffectData, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFireEffect(const FVector& Location, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDustImpact(const FVector& Location, const FVector& ImpactNormal, float ImpactForce = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepEffect(const FVector& Location, float CreatureSize = 1.0f, bool bIsWet = false);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(const FVector& Location, const FVector& Direction, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplash(const FVector& Location, float SplashRadius = 100.0f);

    // Weather and environmental effects
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartRainEffect(EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopRainEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBreathVapor(const FVector& Location, const FRotator& Direction);

    // Performance and cleanup
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQualityLevel(int32 QualityLevel);

    // Effect management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectCullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableAudioSync;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalVFXScale;

protected:
    // Internal effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    UNiagaraComponent* RainEffectComponent;

    // Effect templates
    UPROPERTY(EditAnywhere, Category = "VFX Templates")
    TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> EffectTemplates;

    UPROPERTY(EditAnywhere, Category = "VFX Audio")
    TMap<EVFX_EffectType, TSoftObjectPtr<USoundCue>> EffectAudioTemplates;

    // Internal helper functions
    UNiagaraComponent* CreateEffectComponent(UNiagaraSystem* NiagaraSystem, const FVector& Location, const FRotator& Rotation);
    void PlayEffectAudio(EVFX_EffectType EffectType, const FVector& Location, float VolumeMultiplier = 1.0f);
    void RegisterActiveEffect(UNiagaraComponent* Effect);
    void UnregisterActiveEffect(UNiagaraComponent* Effect);
    bool ShouldCullEffect(const FVector& EffectLocation) const;

private:
    // Performance tracking
    int32 CurrentActiveEffectCount;
    float LastCleanupTime;
    static constexpr float CleanupInterval = 5.0f;
};