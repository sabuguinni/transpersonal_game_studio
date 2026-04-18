#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "VFXSystemManager.generated.h"

// VFX-specific enums for realistic prehistoric effects
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Impact_Footstep     UMETA(DisplayName = "Footstep Impact"),
    Impact_DinoStep     UMETA(DisplayName = "Dinosaur Footstep"),
    Impact_WeaponHit    UMETA(DisplayName = "Weapon Hit"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Fog         UMETA(DisplayName = "Fog"),
    Weather_Wind        UMETA(DisplayName = "Wind"),
    Blood_Splatter      UMETA(DisplayName = "Blood Splatter"),
    Dust_Cloud          UMETA(DisplayName = "Dust Cloud"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Smoke_Fire          UMETA(DisplayName = "Fire Smoke"),
    Sparks_Fire         UMETA(DisplayName = "Fire Sparks")
};

UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundCue> AudioCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_IntensityLevel DefaultIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        DefaultIntensity = EVFX_IntensityLevel::Medium;
        Duration = 5.0f;
        bLooping = false;
        DefaultScale = FVector(1.0f, 1.0f, 1.0f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ActiveEffect
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    int32 EffectID;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    UNiagaraComponent* NiagaraComponent;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    UAudioComponent* AudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    FVector Location;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float Duration;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    bool bIsLooping;

    FVFX_ActiveEffect()
    {
        EffectID = -1;
        EffectType = EVFX_EffectType::Fire_Campfire;
        NiagaraComponent = nullptr;
        AudioComponent = nullptr;
        Location = FVector::ZeroVector;
        StartTime = 0.0f;
        Duration = 0.0f;
        bIsLooping = false;
    }
};

/**
 * VFX System Manager - Handles all visual effects for the prehistoric survival game
 * Focuses on realistic effects: fire, smoke, dust, blood, weather, impacts
 * NO mystical/spiritual effects - only physically plausible prehistoric VFX
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

    // Core VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 PlayEffect(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(int32 EffectID);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffectsByType(EVFX_EffectType EffectType);

    // Specific effect functions for common game scenarios
    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 PlayCampfire(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 PlayDinosaurFootstep(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 PlayWeaponImpact(const FVector& Location, const FVector& ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 PlayBloodSplatter(const FVector& Location, const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 PlayDustCloud(const FVector& Location, float Intensity = 1.0f);

    // Weather effects
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartRain(EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopRain();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetFogIntensity(float Intensity);

    // Effect management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsEffectActive(int32 EffectID) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    TArray<FVFX_ActiveEffect> GetActiveEffects() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectData(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FVFX_EffectData GetEffectData(EVFX_EffectType EffectType) const;

protected:
    // Effect registry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectRegistry;

    // Active effects tracking
    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    TMap<int32, FVFX_ActiveEffect> ActiveEffects;

    // Global VFX settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float GlobalVFXScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bVFXEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxActiveEffects;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float EffectCullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bUseLOD;

private:
    // Internal management
    int32 NextEffectID;
    UWorld* CachedWorld;

    // Helper functions
    void InitializeEffectRegistry();
    void CleanupExpiredEffects();
    UNiagaraComponent* CreateNiagaraComponent(const FVFX_EffectData& EffectData, const FVector& Location, const FRotator& Rotation);
    UAudioComponent* CreateAudioComponent(const FVFX_EffectData& EffectData, const FVector& Location);
    void UpdateEffectIntensity(FVFX_ActiveEffect& Effect, EVFX_IntensityLevel Intensity);

    // Timer handle for cleanup
    FTimerHandle CleanupTimerHandle;
};