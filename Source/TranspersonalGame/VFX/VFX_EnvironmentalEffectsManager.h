#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "VFX_EnvironmentalEffectsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EnvironmentalEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> EffectSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector EffectOffset = FVector::ZeroVector;

    FVFX_EnvironmentalEffectData()
    {
        EffectScale = 1.0f;
        EffectDuration = 5.0f;
        bAutoDestroy = true;
        EffectOffset = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_BiomeEffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    FVFX_EnvironmentalEffectData AmbientEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    FVFX_EnvironmentalEffectData WeatherEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    FVFX_EnvironmentalEffectData FootstepEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float EffectDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float EffectRange = 5000.0f;

    FVFX_BiomeEffectSettings()
    {
        EffectDensity = 1.0f;
        EffectRange = 5000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_EnvironmentalEffectsManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UVFX_EnvironmentalEffectsManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Environmental VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnEnvironmentalEffect(const FVector& Location, EBiomeType BiomeType, const FString& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnFootstepEffect(const FVector& Location, EBiomeType BiomeType, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnWeatherEffect(const FVector& Location, EWeatherType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnCampfireEffect(const FVector& Location, float FireIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnImpactEffect(const FVector& Location, const FVector& ImpactNormal, EImpactType ImpactType);

    // Biome-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Biome")
    void UpdateBiomeEffects(EBiomeType NewBiome, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX Biome")
    void SetBiomeEffectSettings(EBiomeType BiomeType, const FVFX_BiomeEffectSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX Biome")
    FVFX_BiomeEffectSettings GetBiomeEffectSettings(EBiomeType BiomeType) const;

    // VFX System Management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetGlobalVFXQuality(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetVFXEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveEffectCount() const;

protected:
    // VFX Data Storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EBiomeType, FVFX_BiomeEffectSettings> BiomeEffectSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<FString, FVFX_EnvironmentalEffectData> EnvironmentalEffects;

    // Active VFX Components
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    // VFX System Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX System")
    bool bVFXEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX System")
    int32 GlobalVFXQuality = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX System")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX System")
    float EffectCleanupInterval = 5.0f;

    // Timer Handles
    FTimerHandle EffectCleanupTimer;

private:
    // Internal VFX Methods
    UNiagaraComponent* CreateVFXComponent(UNiagaraSystem* System, const FVector& Location, const FVector& Scale = FVector::OneVector);
    void InitializeBiomeEffectSettings();
    void InitializeEnvironmentalEffects();
    void RegisterVFXComponent(UNiagaraComponent* Component);
    void UnregisterVFXComponent(UNiagaraComponent* Component);
    UNiagaraSystem* LoadVFXSystem(const FString& SystemPath);
    
    // Cleanup Methods
    void CleanupTimerCallback();
    bool IsEffectExpired(UNiagaraComponent* Component) const;
};