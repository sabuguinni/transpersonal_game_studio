#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Components/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_VolcanicBiomeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicFeatureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    EWorld_VolcanicFeatureType FeatureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    bool bIsActive;

    FWorld_VolcanicFeatureData()
    {
        Location = FVector::ZeroVector;
        FeatureType = EWorld_VolcanicFeatureType::HotSpring;
        Intensity = 1.0f;
        Radius = 500.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicBiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float VolcanicActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float GeothermalDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float LavaFlowRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float AmbientTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    bool bEnableVolcanicWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    TArray<FWorld_VolcanicFeatureData> VolcanicFeatures;

    FWorld_VolcanicBiomeConfig()
    {
        VolcanicActivity = 0.7f;
        GeothermalDensity = 0.5f;
        LavaFlowRate = 0.3f;
        AmbientTemperature = 45.0f;
        bEnableVolcanicWeather = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_VolcanicBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_VolcanicBiomeSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VolcanicTerrainMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* VolcanicGlowLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VolcanicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* SteamParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* LavaParticles;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Biome")
    FWorld_VolcanicBiomeConfig BiomeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Biome")
    float VolcanicIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Biome")
    bool bIsErupting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Biome")
    float EruptionTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Biome")
    float EruptionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Biome")
    TArray<AActor*> NearbyActors;

    // Core volcanic system methods
    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void InitializeVolcanicBiome();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateVolcanicActivity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void TriggerVolcanicEruption();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void StopVolcanicEruption();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void CreateVolcanicFeature(const FWorld_VolcanicFeatureData& FeatureData);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void RemoveVolcanicFeature(int32 FeatureIndex);

    // Geothermal system integration
    UFUNCTION(BlueprintCallable, Category = "Geothermal")
    void UpdateGeothermalActivity();

    UFUNCTION(BlueprintCallable, Category = "Geothermal")
    void CreateHotSpring(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Geothermal")
    void CreateGeothermalVent(const FVector& Location, float SteamIntensity);

    // Lava flow system
    UFUNCTION(BlueprintCallable, Category = "Lava Flow")
    void InitializeLavaFlow(const FVector& StartLocation, const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category = "Lava Flow")
    void UpdateLavaFlow(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Lava Flow")
    void CoolLavaFlow();

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void UpdateVolcanicLighting();

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void UpdateVolcanicAudio();

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void UpdateVolcanicParticles();

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeVolcanicEffects(float PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVolcanicLOD(int32 LODLevel);

    // Integration with other systems
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithWeatherSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithBiomeAudio();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void NotifyNearbyActors();

protected:
    // Internal volcanic state
    float CurrentVolcanicActivity;
    float GeothermalHeatLevel;
    float LavaTemperature;
    bool bVolcanicSystemActive;
    
    // Timers and intervals
    float VolcanicUpdateInterval;
    float LastVolcanicUpdate;
    float GeothermalUpdateInterval;
    float LastGeothermalUpdate;
    
    // Performance tracking
    int32 CurrentLODLevel;
    float LastPerformanceCheck;
    float PerformanceCheckInterval;
    
    // Integration references
    class AWorld_BiomeAudioController* BiomeAudioController;
    class AWorld_DynamicWeatherController* WeatherController;
    class AWorld_PerformanceTerrainSystem* TerrainSystem;

private:
    // Internal helper methods
    void UpdateInternalVolcanicState(float DeltaTime);
    void CheckVolcanicTriggers();
    void ProcessVolcanicEffects();
    void ValidateVolcanicConfiguration();
    FLinearColor CalculateVolcanicGlowColor() const;
    float CalculateVolcanicSoundVolume() const;
    void CleanupInactiveFeatures();
};