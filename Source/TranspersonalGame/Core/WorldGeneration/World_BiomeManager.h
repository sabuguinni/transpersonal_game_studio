#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Swampland       UMETA(DisplayName = "Swampland"),
    Forest          UMETA(DisplayName = "Dense Forest"),
    Savanna         UMETA(DisplayName = "Open Savanna"),
    Desert          UMETA(DisplayName = "Arid Desert"),
    SnowyMountains  UMETA(DisplayName = "Snowy Mountains")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Forest;
        CenterLocation = FVector::ZeroVector;
        Radius = 5000.0f;
        Temperature = 20.0f;
        Humidity = 50.0f;
        Elevation = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TSoftObjectPtr<UStaticMesh> FeatureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_BiomeType AssociatedBiome;

    FWorld_TerrainFeature()
    {
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        Rotation = FRotator::ZeroRotator;
        AssociatedBiome = EWorld_BiomeType::Forest;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateBiome(EWorld_BiomeType BiomeType, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    // Terrain Features
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SpawnTerrainFeature(const FWorld_TerrainFeature& Feature);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PlaceVegetationInBiome(EWorld_BiomeType BiomeType, int32 Count);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ApplyBiomeWeather(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UpdateDayNightCycle(float TimeOfDay);

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "World Generation")
    void RegenerateAllBiomes();

    UFUNCTION(CallInEditor, Category = "World Generation")
    void ClearAllGeneratedContent();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeData> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    int32 RandomSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    float WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> GeneratedActors;

private:
    void SetupDefaultBiomes();
    void CreateSwamplandBiome(const FVector& Location, float Radius);
    void CreateForestBiome(const FVector& Location, float Radius);
    void CreateSavannaBiome(const FVector& Location, float Radius);
    void CreateDesertBiome(const FVector& Location, float Radius);
    void CreateSnowyMountainBiome(const FVector& Location, float Radius);

    FVector GetRandomLocationInRadius(const FVector& Center, float Radius) const;
    float CalculateDistanceToBiomeCenter(const FVector& Location, const FWorld_BiomeData& Biome) const;
};