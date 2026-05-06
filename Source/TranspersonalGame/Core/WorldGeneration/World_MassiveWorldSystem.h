#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape.h"
#include "SharedTypes.h"
#include "World_MassiveWorldSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    FWorld_BiomeDefinition()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        BiomeSize = FVector(10000.0f, 10000.0f, 1000.0f);
        BiomeColor = FLinearColor::White;
        ElevationMin = 0.0f;
        ElevationMax = 1000.0f;
        Temperature = 20.0f;
        Humidity = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandmarkDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FString LandmarkName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FString BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    bool bIsNavigationPoint;

    FWorld_LandmarkDefinition()
    {
        LandmarkName = TEXT("Unknown");
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        BiomeType = TEXT("Savanna");
        bIsNavigationPoint = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed;

    FWorld_RiverSegment()
    {
        StartPoint = FVector::ZeroVector;
        EndPoint = FVector(1000.0f, 0.0f, 0.0f);
        Width = 500.0f;
        Depth = 100.0f;
        FlowSpeed = 100.0f;
    }
};

/**
 * Massive World System for 200km2 procedural world generation
 * Manages biomes, landmarks, rivers, and streaming for large-scale environments
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UWorld_MassiveWorldSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_MassiveWorldSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // World Scale Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scale")
    float WorldSizeKm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scale")
    FVector WorldBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scale")
    int32 StreamingGridSize;

    // Biome System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<AActor*> BiomeMarkers;

    // Landmark System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmarks")
    TArray<FWorld_LandmarkDefinition> LandmarkDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmarks")
    TArray<AActor*> LandmarkActors;

    // River System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    TArray<FWorld_RiverSegment> RiverNetwork;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    TArray<AActor*> RiverActors;

    // Landscape Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    ALandscape* MassiveLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector LandscapeScale;

    // World Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeMassiveWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateBiomeMarkers();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateLandmarks();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateRiverNetwork();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ExpandLandscapeTo200km();

    // Biome Functions
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeDefinition GetBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<FVector> GetBiomeSpawnPoints(const FString& BiomeType, int32 Count);

    // Landmark Functions
    UFUNCTION(BlueprintCallable, Category = "Landmarks")
    void SpawnLandmarkAtLocation(const FWorld_LandmarkDefinition& LandmarkDef);

    UFUNCTION(BlueprintCallable, Category = "Landmarks")
    TArray<FVector> GetNavigationLandmarks();

    // River Functions
    UFUNCTION(BlueprintCallable, Category = "Rivers")
    void CreateRiverSegment(const FWorld_RiverSegment& Segment);

    UFUNCTION(BlueprintCallable, Category = "Rivers")
    TArray<FVector> GetRiverCrossingPoints();

    // Streaming Functions
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void SetupWorldPartitionStreaming();

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void CreateStreamingVolumes();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsLocationInBiome(FVector Location, const FString& BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToNearestLandmark(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomLocationInBiome(const FString& BiomeType);

private:
    // Internal helper functions
    void InitializeBiomeDefinitions();
    void InitializeLandmarkDefinitions();
    void InitializeRiverNetwork();
    void CleanupExistingActors();
    FVector CalculateMidpoint(FVector Start, FVector End);
    float CalculateDistance(FVector A, FVector B);
};