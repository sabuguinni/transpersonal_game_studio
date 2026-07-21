#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_DynamicBiomeGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = "DefaultBiome";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D BiomeCenter = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterPresence = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainRoughness = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    FWorld_BiomeConfiguration()
    {
        VegetationTypes = {"Tree", "Bush", "Grass"};
        DinosaurSpecies = {"TRex", "Raptor"};
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString FeatureName = "DefaultFeature";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float InfluenceRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType AssociatedBiome = EBiomeType::Savanna;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_DynamicBiomeGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_DynamicBiomeGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core biome generation methods
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiome(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PopulateBiomeWithVegetation(const FWorld_BiomeConfiguration& BiomeConfig, int32 VegetationCount = 100);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PopulateBiomeWithDinosaurs(const FWorld_BiomeConfiguration& BiomeConfig, int32 DinosaurCount = 20);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateTerrainFeatures(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateWaterBodies(const FWorld_BiomeConfiguration& BiomeConfig);

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void RefreshBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeConfiguration GetBiomeConfiguration(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetBiomeConfiguration(EBiomeType BiomeType, const FWorld_BiomeConfiguration& NewConfig);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetBiomeInfluenceAtLocation(const FVector& WorldLocation, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<AActor*> GetActorsInBiome(EBiomeType BiomeType) const;

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void OptimizeBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetLODDistances(float NearDistance, float FarDistance);

protected:
    // Biome configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<EBiomeType, FWorld_BiomeConfiguration> BiomeConfigurations;

    // Terrain features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Features")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxRenderDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerBiome = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 1.0f;

    // Internal state
    UPROPERTY()
    TMap<EBiomeType, TArray<AActor*>> BiomeActors;

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    // Helper methods
    FVector GetRandomLocationInBiome(const FWorld_BiomeConfiguration& BiomeConfig) const;
    bool IsLocationInBiome(const FVector& Location, const FWorld_BiomeConfiguration& BiomeConfig) const;
    void SpawnVegetationActor(const FVector& Location, const FString& VegetationType);
    void SpawnDinosaurActor(const FVector& Location, const FString& DinosaurSpecies);
    void CreateRockFormation(const FVector& Location, float Scale);
    void CreateWaterPlane(const FVector& Location, const FVector& Scale);
    void InitializeDefaultBiomeConfigurations();
};