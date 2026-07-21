#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SharedTypes.h"
#include "VFX_NiagaraManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ParticleSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float ParticleScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EmissionRate = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float LifeSpan = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector VelocityRange = FVector(100.0f, 100.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FLinearColor ParticleColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EnvironmentalEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    EWeatherType WeatherType = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    FVector EffectLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_CombatEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    EDamageType DamageType = EDamageType::Blunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    float ImpactForce = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    FVector ImpactDirection = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    bool bShowBloodEffect = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    float EffectDuration = 2.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateWeatherEffect(EWeatherType WeatherType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateBiomeAmbientEffect(EBiomeType BiomeType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void UpdateDayNightEffects(float TimeOfDay, float SunIntensity);

    // Dinosaur VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateBreathVaporEffect(FVector Location, FVector Direction, float Temperature);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateRoarDistortionEffect(FVector Location, float RoarIntensity);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateBloodImpactEffect(FVector Location, FVector ImpactDirection, EDamageType DamageType);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateWeaponImpactEffect(FVector Location, EWeaponType WeaponType, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateDustCloudEffect(FVector Location, float Radius, float Duration = 3.0f);

    // Fire and crafting VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateCampfireEffect(FVector Location, float FireIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateSparkEffect(FVector Location, ECraftingType CraftingType);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CreateCookingSmokeEffect(FVector Location, float CookingProgress);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void StopAllEffectsInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void SetGlobalVFXQuality(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void UpdateLODSettings(FVector PlayerLocation);

private:
    // VFX settings
    UPROPERTY()
    TMap<EWeatherType, FVFX_ParticleSettings> WeatherEffectSettings;

    UPROPERTY()
    TMap<EBiomeType, FVFX_ParticleSettings> BiomeEffectSettings;

    UPROPERTY()
    TMap<EDamageType, FVFX_CombatEffect> CombatEffectSettings;

    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    TArray<FVFX_EnvironmentalEffect> ActiveEnvironmentalEffects;

    // Performance settings
    UPROPERTY()
    int32 MaxActiveEffects = 50;

    UPROPERTY()
    float LODDistanceThreshold = 2000.0f;

    UPROPERTY()
    int32 CurrentVFXQuality = 2; // 0=Low, 1=Medium, 2=High

    // Helper functions
    void InitializeEffectSettings();
    void CleanupExpiredEffects();
    UNiagaraComponent* SpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation = FRotator::ZeroRotator);
    void ApplyLODToEffect(UNiagaraComponent* Effect, float DistanceToPlayer);
    bool ShouldCullEffect(FVector EffectLocation, FVector PlayerLocation);
};