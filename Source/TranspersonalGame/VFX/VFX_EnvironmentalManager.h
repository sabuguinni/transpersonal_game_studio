#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "../SharedTypes.h"
#include "VFX_EnvironmentalManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        DefaultScale = FVector(1.0f);
        DefaultLifetime = 5.0f;
        bAutoDestroy = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.0f;

    FVFX_WeatherSettings()
    {
        RainIntensity = 0.0f;
        WindStrength = 0.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        FogDensity = 0.0f;
    }
};

/**
 * Environmental VFX Manager - Handles realistic prehistoric environmental effects
 * Manages fire, weather, dust, water, and atmospheric effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_EnvironmentalManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UVFX_EnvironmentalManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Fire Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Fire")
    UNiagaraComponent* SpawnCampfire(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Fire")
    UNiagaraComponent* SpawnTorchFlame(const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "VFX|Fire")
    void ExtinguishFire(UNiagaraComponent* FireEffect);

    // Weather Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherSettings(const FVFX_WeatherSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StartRain(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StopRain();

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StartSnow(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StopSnow();

    // Impact Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    UNiagaraComponent* SpawnDustImpact(const FVector& Location, const FVector& Normal);

    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    UNiagaraComponent* SpawnWaterSplash(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    UNiagaraComponent* SpawnBloodImpact(const FVector& Location, const FVector& Direction);

    // Dinosaur Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* SpawnFootstepDust(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* SpawnBreathSteam(const FVector& Location, const FRotator& Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* SpawnRoarDistortion(const FVector& Location, float Intensity = 1.0f);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetVFXLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    int32 GetActiveEffectCount() const;

protected:
    // VFX Data
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Fire")
    FVFX_EffectData CampfireData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Fire")
    FVFX_EffectData TorchData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Weather")
    FVFX_EffectData RainData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Weather")
    FVFX_EffectData SnowData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Impact")
    FVFX_EffectData DustImpactData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Impact")
    FVFX_EffectData WaterSplashData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Impact")
    FVFX_EffectData BloodImpactData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Dinosaur")
    FVFX_EffectData FootstepDustData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Dinosaur")
    FVFX_EffectData BreathSteamData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Dinosaur")
    FVFX_EffectData RoarDistortionData;

    // Current Settings
    UPROPERTY(BlueprintReadOnly, Category = "VFX|Weather")
    FVFX_WeatherSettings CurrentWeatherSettings;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|Performance")
    int32 CurrentLODLevel = 0;

    // Active Effects Tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<UNiagaraComponent>> ActiveEffects;

    // Weather Components
    UPROPERTY()
    TWeakObjectPtr<UNiagaraComponent> RainComponent;

    UPROPERTY()
    TWeakObjectPtr<UNiagaraComponent> SnowComponent;

private:
    UNiagaraComponent* SpawnEffect(const FVFX_EffectData& EffectData, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, float Scale = 1.0f);
    void InitializeEffectData();
    void UpdateLODSettings();
    bool ShouldSpawnEffect(const FVector& Location) const;
    float GetDistanceToPlayer(const FVector& Location) const;
};