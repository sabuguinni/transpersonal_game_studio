#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Landscape/Landscape.h"
#include "LandscapeProxy.h"
#include "SharedTypes.h"
#include "World_AdvancedTerrain.generated.h"

// Forward declarations
class ALandscape;
class UMaterialInterface;
class UStaticMesh;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    FWorld_BiomeDefinition()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector2D::ZeroVector;
        Radius = 10000.0f;
        BaseHeight = 100.0f;
        HeightVariation = 50.0f;
        BiomeColor = FLinearColor::White;
        Temperature = 20.0f;
        Humidity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    UMaterialInterface* LayerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightThreshold;

    FWorld_TerrainLayer()
    {
        LayerName = TEXT("Base");
        LayerMaterial = nullptr;
        BlendWeight = 1.0f;
        NoiseScale = 1.0f;
        HeightThreshold = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_AdvancedTerrain : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_AdvancedTerrain();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<FWorld_TerrainLayer> TerrainLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 TerrainResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseAmplitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 NoiseOctaves;

    // Runtime terrain data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    ALandscape* GeneratedLandscape;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AStaticMeshActor*> BiomeMarkers;

    // Public methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateAdvancedTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void CreateBiomeMarkers();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    FWorld_BiomeDefinition GetBiomeAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    float GetHeightAtLocation(FVector2D WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ApplyTerrainLayers();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateRivers();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void PlaceEnvironmentalFeatures();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void RegenerateAllTerrain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void ClearGeneratedTerrain();

private:
    // Internal generation methods
    void InitializeDefaultBiomes();
    void InitializeTerrainLayers();
    float CalculatePerlinNoise(float X, float Y, int32 Octaves, float Frequency, float Amplitude);
    float CalculateBiomeInfluence(FVector2D Location, const FWorld_BiomeDefinition& Biome);
    FLinearColor BlendBiomeColors(FVector2D Location);
    void CreateTerrainMesh();
    void ApplyBiomeMaterials();
    void GenerateVegetationPlacements();
    void CreateWaterBodies();
    void SetupTerrainCollision();

    // Utility methods
    bool IsValidTerrainLocation(FVector Location);
    void LogTerrainGenerationStats();
};