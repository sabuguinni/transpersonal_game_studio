#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_TerrainGenerator.generated.h"

/**
 * Procedural terrain generator that creates terrain features across biomes
 * Handles rock formations, cliffs, and geological features
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    /** Generate terrain features for a specific biome */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrainForBiome(EBiomeType BiomeType, FVector BiomeCenter, int32 FeatureCount = 50);

    /** Create rock formations at specified locations */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<AActor*> CreateRockFormations(const TArray<FVector>& Locations, float MinScale = 2.0f, float MaxScale = 8.0f);

    /** Generate cliff faces along biome boundaries */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateCliffFaces(FVector StartPoint, FVector EndPoint, int32 CliffCount = 10);

    /** Create cave entrances in mountainous areas */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<AActor*> CreateCaveEntrances(FVector MountainCenter, int32 CaveCount = 5);

    /** Clear existing terrain features in a biome */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearBiomeTerrainFeatures(EBiomeType BiomeType);

protected:
    /** Static mesh for rock formations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Assets")
    class UStaticMesh* RockMesh;

    /** Static mesh for cliff faces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Assets")
    class UStaticMesh* CliffMesh;

    /** Static mesh for cave entrances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Assets")
    class UStaticMesh* CaveMesh;

    /** Maximum number of terrain features per biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxFeaturesPerBiome = 100;

    /** Minimum distance between terrain features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float MinFeatureDistance = 1000.0f;

    /** Height variation for terrain features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    FVector2D HeightRange = FVector2D(50.0f, 300.0f);

    /** Scale variation for terrain features */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    FVector2D ScaleRange = FVector2D(1.0f, 10.0f);

private:
    /** Get random position within biome radius */
    FVector GetRandomBiomePosition(FVector BiomeCenter, float Radius = 15000.0f);

    /** Check if position is valid for terrain feature placement */
    bool IsValidTerrainPosition(FVector Position);

    /** Get terrain feature scale based on biome type */
    float GetBiomeSpecificScale(EBiomeType BiomeType);

    /** Array to track created terrain actors for cleanup */
    UPROPERTY()
    TArray<AActor*> CreatedTerrainActors;
};