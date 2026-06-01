#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Landscape/Classes/Landscape.h"
#include "LandscapeProxy.h"
#include "SharedTypes.h"
#include "World_TerrainManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_TerrainType : uint8
{
    Plains      UMETA(DisplayName = "Plains"),
    Hills       UMETA(DisplayName = "Hills"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Valleys     UMETA(DisplayName = "Valleys"),
    Cliffs      UMETA(DisplayName = "Cliffs"),
    Canyons     UMETA(DisplayName = "Canyons")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_TerrainType TerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Elevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Roughness;

    FWorld_TerrainFeature()
    {
        TerrainType = EWorld_TerrainType::Plains;
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        Elevation = 0.0f;
        Roughness = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Terrain generation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 TerrainSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float HeightMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 TerrainSize;

    // Biome-specific terrain features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    TArray<FWorld_TerrainFeature> SavannaFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    TArray<FWorld_TerrainFeature> ForestFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    TArray<FWorld_TerrainFeature> DesertFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    TArray<FWorld_TerrainFeature> SwampFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Terrain")
    TArray<FWorld_TerrainFeature> MountainFeatures;

    // Terrain meshes for different features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Meshes")
    TObjectPtr<UStaticMesh> CliffMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Meshes")
    TObjectPtr<UStaticMesh> RockFormationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Meshes")
    TObjectPtr<UStaticMesh> CanyonMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Meshes")
    TObjectPtr<UStaticMesh> HillMesh;

    // Materials for different biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Materials")
    TObjectPtr<UMaterialInterface> SavannaMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Materials")
    TObjectPtr<UMaterialInterface> ForestMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Materials")
    TObjectPtr<UMaterialInterface> DesertMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Materials")
    TObjectPtr<UMaterialInterface> SwampMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Materials")
    TObjectPtr<UMaterialInterface> MountainMaterial;

public:
    // Terrain generation functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainForBiome(EBiomeType BiomeType, FVector BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void CreateTerrainFeature(const FWorld_TerrainFeature& Feature, FVector BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateCliffs(FVector Location, float Height, float Width);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateRockFormations(FVector Location, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateCanyon(FVector StartLocation, FVector EndLocation, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateHills(FVector CenterLocation, float Radius, int32 HillCount);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    float GetTerrainHeight(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    EWorld_TerrainType GetTerrainTypeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    TArray<FVector> GetSuitableSpawnLocations(EWorld_TerrainType TerrainType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Terrain Utilities")
    void ClearTerrainInRadius(FVector Location, float Radius);

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void RegenerateAllTerrain();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ClearAllTerrain();

protected:
    // Internal terrain generation helpers
    float GeneratePerlinNoise(float X, float Y, float Scale);
    FVector GetBiomeCenter(EBiomeType BiomeType);
    UMaterialInterface* GetBiomeMaterial(EBiomeType BiomeType);
    TArray<FWorld_TerrainFeature> GetBiomeFeatures(EBiomeType BiomeType);
};