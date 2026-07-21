#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Rocky       UMETA(DisplayName = "Rocky"),
    Wetland     UMETA(DisplayName = "Wetland"),
    River       UMETA(DisplayName = "River"),
    Lake        UMETA(DisplayName = "Lake")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::Green;
};

USTRUCT(BlueprintType)
struct FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Depth = 50.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Biome Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    TArray<FWorld_RiverSegment> RiverSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 VegetationSeed = 12345;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float TerrainNoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MaxTerrainHeight = 500.0f;

    // Biome Functions
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FLinearColor GetBiomeColorAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Generation", CallInEditor)
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Generation", CallInEditor)
    void GenerateRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "Generation", CallInEditor)
    void ClearGeneratedContent();

    // Terrain Functions
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTerrainHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FVector GetTerrainNormalAtLocation(const FVector& WorldLocation) const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToNearestWater(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsLocationInWater(const FVector& WorldLocation) const;

protected:
    // Internal generation functions
    void SpawnVegetationForBiome(const FWorld_BiomeData& BiomeData);
    void CreateRiverSegment(const FWorld_RiverSegment& Segment);
    void CreateBiomeMarkers();
    
    // Noise generation
    float GeneratePerlinNoise(float X, float Y, float Scale) const;
    float GenerateTerrainNoise(float X, float Y) const;

private:
    UPROPERTY()
    TArray<AActor*> GeneratedActors;

    UPROPERTY()
    TArray<AActor*> GeneratedVegetation;

    UPROPERTY()
    TArray<AActor*> GeneratedWaterBodies;
};