#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/PointLightComponent.h"
#include "VFX_EnvironmentalEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EnvironmentalEffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    CampfireFire    UMETA(DisplayName = "Campfire Fire"),
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    ForestFog       UMETA(DisplayName = "Forest Fog"),
    DesertHaze      UMETA(DisplayName = "Desert Heat Haze"),
    RainDroplets    UMETA(DisplayName = "Rain Droplets"),
    WindParticles   UMETA(DisplayName = "Wind Particles")
};

USTRUCT(BlueprintType)
struct FVFX_EnvironmentalEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EnvironmentalEffectType EffectType = EVFX_EnvironmentalEffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = -1.0f; // -1 = infinite

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bIsActive = true;

    FVFX_EnvironmentalEffect()
    {
        EffectType = EVFX_EnvironmentalEffectType::None;
        Location = FVector::ZeroVector;
        Intensity = 1.0f;
        Radius = 500.0f;
        Duration = -1.0f;
        bIsActive = true;
    }
};

UCLASS(Blueprintable, BlueprintType, Category = "VFX")
class TRANSPERSONALGAME_API AVFX_EnvironmentalEffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EnvironmentalEffectManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Environmental effects management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environmental")
    TArray<FVFX_EnvironmentalEffect> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environmental")
    int32 MaxActiveEffects = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environmental")
    float EffectUpdateInterval = 0.1f;

    // Campfire system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Campfire")
    TArray<FVector> CampfireLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Campfire")
    float CampfireLightIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Campfire")
    FLinearColor CampfireLightColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    // Atmospheric effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Atmosphere")
    float ForestFogDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Atmosphere")
    float DesertHazeDensity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Atmosphere")
    float WindParticleIntensity = 1.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float LODDistanceNear = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float LODDistanceFar = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bEnableLODSystem = true;

private:
    float LastUpdateTime = 0.0f;
    TArray<UParticleSystemComponent*> ParticleComponents;
    TArray<UPointLightComponent*> LightComponents;

public:
    // Environmental effect management
    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void CreateEnvironmentalEffect(EVFX_EnvironmentalEffectType EffectType, FVector Location, float Intensity = 1.0f, float Duration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void RemoveEnvironmentalEffect(int32 EffectIndex);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void UpdateEffectIntensity(int32 EffectIndex, float NewIntensity);

    // Campfire system
    UFUNCTION(BlueprintCallable, Category = "VFX Campfire")
    void CreateCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Campfire")
    void ExtinguishCampfire(FVector Location, float SearchRadius = 100.0f);

    // Footstep impact system
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateFootstepImpact(FVector Location, float ImpactForce = 1.0f);

    // Weather system integration
    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void SetWeatherIntensity(float RainIntensity, float FogIntensity, float WindIntensity);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void OptimizeEffectsForDistance(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void CleanupInactiveEffects();

protected:
    void InitializeCampfires();
    void UpdateEnvironmentalEffects(float DeltaTime);
    float CalculateLODLevel(FVector EffectLocation, FVector PlayerLocation);
    void ApplyLODToEffect(int32 EffectIndex, float LODLevel);
};