#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "World_VolcanicLandscapeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    EWorld_VolcanicType VolcanicType = EWorld_VolcanicType::Dormant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float DangerRadius = 500.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_GeothermalZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    float AverageTemperature = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    int32 NumHotSprings = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    TArray<FVector> HotSpringLocations;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_VolcanicLandscapeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_VolcanicLandscapeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Volcanic landscape generation
    UFUNCTION(BlueprintCallable, Category = "Volcanic Landscape")
    void GenerateVolcanicLandscape();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Landscape")
    void CreateVolcanicCone(const FVector& Location, float Scale, EWorld_VolcanicType VolcanicType);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Landscape")
    void CreateGeothermalZone(const FVector& CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Landscape")
    void CreateHotSprings(const FWorld_GeothermalZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Landscape")
    void CreateVolcanicRocks(const FVector& CenterLocation, float Radius, int32 NumRocks);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void UpdateVolcanicActivity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void TriggerVolcanicEruption(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void CreateVolcanicSmoke(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void CreateLavaFlow(const FVector& StartLocation, const FVector& EndLocation);

    // Biome integration
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    bool IsLocationInVolcanicZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    float GetVolcanicTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    EWorld_BiomeType GetVolcanicBiomeType(const FVector& Location) const;

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeVolcanicFeatures(const FVector& PlayerLocation, float ViewDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableVolcanicFeature(int32 FeatureIndex, bool bEnable);

protected:
    // Core volcanic features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Features")
    TArray<FWorld_VolcanicFeature> VolcanicFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Features")
    TArray<FWorld_GeothermalZone> GeothermalZones;

    // Spawned actors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawned Actors")
    TArray<AStaticMeshActor*> VolcanicCones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawned Actors")
    TArray<AStaticMeshActor*> HotSprings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawned Actors")
    TArray<AStaticMeshActor*> VolcanicRocks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawned Actors")
    TArray<APointLight*> GeothermalLights;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<UAudioComponent*> VolcanicAudioSources;

    // Particle systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<UParticleSystemComponent*> SmokeEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<UParticleSystemComponent*> LavaEffects;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxVolcanicFeatures = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float VolcanicActivityUpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float GeothermalEffectRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableVolcanicSounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableVolcanicEffects = true;

private:
    float LastVolcanicUpdateTime = 0.0f;
    int32 ActiveVolcanicFeatures = 0;
};