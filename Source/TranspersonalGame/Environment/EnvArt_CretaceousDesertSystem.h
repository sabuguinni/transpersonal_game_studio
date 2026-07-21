#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousDesertSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_DesertFeatureType : uint8
{
    RockFormation,
    SandstoneCliff,
    DroughtVegetation,
    DriedRiverbed,
    Oasis,
    RockArch,
    DuneFormation,
    CanyonWall
};

USTRUCT(BlueprintType)
struct FEnvArt_DesertAtmosphereConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Atmosphere")
    float DustDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Atmosphere")
    float HeatShimmerIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Atmosphere")
    FLinearColor DustColor = FLinearColor(0.9f, 0.7f, 0.4f, 0.6f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Atmosphere")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Atmosphere")
    float VisibilityRange = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Atmosphere")
    float AmbientTemperature = 45.0f;
};

USTRUCT(BlueprintType)
struct FEnvArt_DesertFeatureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Feature")
    EEnvArt_DesertFeatureType FeatureType = EEnvArt_DesertFeatureType::RockFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Feature")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Feature")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Feature")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Feature")
    float WeatheringLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Feature")
    bool bHasShadowCasting = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousDesertSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousDesertSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* DustParticleSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* HeatShimmerSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* DesertAmbientAudio;

    // Desert Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Config")
    FEnvArt_DesertAtmosphereConfig AtmosphereConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Config")
    TArray<FEnvArt_DesertFeatureData> DesertFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Config")
    float BiomeRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Config")
    int32 MaxRockFormations = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Config")
    int32 MaxVegetationCount = 15;

    // Desert Management Functions
    UFUNCTION(BlueprintCallable, Category = "Desert System")
    void InitializeDesertBiome();

    UFUNCTION(BlueprintCallable, Category = "Desert System")
    void SpawnDesertFeature(EEnvArt_DesertFeatureType FeatureType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Desert System")
    void UpdateAtmosphericEffects(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Desert System")
    void SetDustIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Desert System")
    void SetHeatShimmerEffect(float Intensity, float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Desert System")
    void CreateOasisArea(FVector OasisLocation, float OasisRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Desert System")
    void GenerateRockFormations(int32 Count, float MinDistance = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Desert System")
    void CreateDriedRiverbed(FVector StartLocation, FVector EndLocation, float Width = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "Desert System")
    void SpawnDesertVegetation(int32 Count, float SparsityFactor = 0.3f);

    // Weather and Time Effects
    UFUNCTION(BlueprintCallable, Category = "Desert Weather")
    void TriggerSandstorm(float Duration = 120.0f, float Intensity = 0.8f);

    UFUNCTION(BlueprintCallable, Category = "Desert Weather")
    void UpdateDesertLighting(float SunAngle, FLinearColor SunColor);

    UFUNCTION(BlueprintCallable, Category = "Desert Weather")
    void SetDesertWindEffect(FVector WindDirection, float WindSpeed);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Desert Utilities")
    bool IsLocationInDesertBiome(FVector TestLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Desert Utilities")
    float GetDesertTemperatureAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Desert Utilities")
    FVector GetNearestOasisLocation(FVector FromLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Desert Utilities")
    TArray<AActor*> GetDesertFeaturesInRadius(FVector Center, float Radius) const;

private:
    // Internal state
    float CurrentDustIntensity;
    float CurrentHeatLevel;
    bool bSandstormActive;
    float SandstormTimer;
    
    TArray<AActor*> SpawnedDesertActors;
    TArray<FVector> OasisLocations;
    
    // Internal helper functions
    void UpdateDustParticles(float DeltaTime);
    void UpdateHeatShimmer(float DeltaTime);
    void CleanupOldActors();
    FVector GetRandomDesertLocation() const;
    bool IsValidSpawnLocation(FVector Location) const;
};