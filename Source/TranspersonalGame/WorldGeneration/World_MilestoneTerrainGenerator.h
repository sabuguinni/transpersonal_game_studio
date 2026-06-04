#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_MilestoneTerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainPiece
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString TerrainType = TEXT("Platform");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Height = 0.0f;

    FWorld_TerrainPiece()
    {
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        TerrainType = TEXT("Platform");
        Height = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeArea
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeType = TEXT("Forest");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationDensity = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;

    FWorld_BiomeArea()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        BiomeType = TEXT("Forest");
        VegetationDensity = 50;
        bHasWater = false;
    }
};

/**
 * MILESTONE 1 Terrain Generator - Creates landscape with height variation for WALK AROUND gameplay
 * Generates hills, valleys, water bodies, and distinct biome areas for character movement testing
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_MilestoneTerrainGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_MilestoneTerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // MILESTONE 1 Core Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "MILESTONE 1")
    void GenerateWalkAroundTerrain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "MILESTONE 1")
    void CreateHeightVariation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "MILESTONE 1")
    void CreateWaterBodies();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "MILESTONE 1")
    void CreateVegetationVariety();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "MILESTONE 1")
    void CreateDistinctBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "MILESTONE 1")
    bool ValidateMilestone1Requirements();

    // Terrain Management
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void ClearExistingTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    AStaticMeshActor* CreateTerrainPiece(const FVector& Location, const FVector& Scale, const FString& Label);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void CreateHill(const FVector& Location, float Height, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void CreateValley(const FVector& Location, float Depth, float Radius);

    // Water System
    UFUNCTION(BlueprintCallable, Category = "Water")
    void CreateRiver(const FVector& StartLocation, const FVector& EndLocation, float Width);

    UFUNCTION(BlueprintCallable, Category = "Water")
    void CreateLake(const FVector& Location, float Radius);

    // Vegetation System
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void PlantTree(const FVector& Location, float Size, const FString& TreeType);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void CreateForestArea(const FVector& CenterLocation, float Radius, int32 TreeCount);

    // Biome System
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateForestBiome(const FVector& CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreatePlainsBiome(const FVector& CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateRockyBiome(const FVector& CenterLocation, float Radius);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1")
    bool bAutoGenerateOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1")
    float TerrainSize = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1")
    int32 HillCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1")
    int32 ValleyCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1")
    float MaxHillHeight = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1")
    float MaxValleyDepth = 150.0f;

    // Terrain Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain State")
    TArray<FWorld_TerrainPiece> GeneratedTerrain;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain State")
    TArray<FWorld_BiomeArea> BiomeAreas;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain State")
    int32 TotalActorsCreated = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain State")
    bool bMilestone1Complete = false;

    // Performance Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastGenerationTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 ActorCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bPerformanceOptimized = true;

private:
    // Internal helpers
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
    float GetRandomFloat(float Min, float Max);
    FString GenerateUniqueLabel(const FString& BaseName);
    
    // Validation helpers
    bool ValidateTerrainVariation();
    bool ValidateWaterBodies();
    bool ValidateVegetationDiversity();
    bool ValidateBiomeDiversity();
    
    // Performance helpers
    void OptimizeActorCount();
    void UpdatePerformanceMetrics();
};