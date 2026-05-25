#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_BiomeEcosystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_DinosaurSpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    int32 PopulationSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bIsPredator;

    FWorld_DinosaurSpawnConfig()
    {
        AssetPath = "";
        PopulationSize = 1;
        SpawnRadius = 2000.0f;
        bIsPredator = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SpawnDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString MeshAssetPath;

    FWorld_TerrainFeature()
    {
        FeatureName = "Generic_Feature";
        Scale = FVector(1.0f, 1.0f, 1.0f);
        SpawnDensity = 0.1f;
        MeshAssetPath = "";
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_DinosaurSpawnConfig> DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWaterBody;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WaterBodyLocation;

    FWorld_BiomeConfiguration()
    {
        BiomeType = EBiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        BiomeRadius = 5000.0f;
        bHasWaterBody = false;
        WaterBodyLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeEcosystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeEcosystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    bool bAutoPopulateBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    float EcosystemUpdateInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    int32 TotalSpawnedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Ecosystem", meta = (AllowPrivateAccess = "true"))
    int32 TotalDinosaurPopulation;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void PopulateBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void SpawnDinosaurInBiome(const FWorld_DinosaurSpawnConfig& SpawnConfig, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void CreateTerrainFeatures(const FWorld_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void CreateWaterBody(const FVector& Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    int32 GetBiomePopulation(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    TArray<AActor*> GetActorsInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Ecosystem")
    void EditorPopulateAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Ecosystem")
    void EditorClearAllBiomes();

private:
    void SetupDefaultBiomeConfigurations();
    FVector GetBiomeCenterLocation(EBiomeType BiomeType) const;
    float LastEcosystemUpdate;
};