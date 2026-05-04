#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_MassiveTerrainSystem.generated.h"

/**
 * Massive Terrain System for 200km2 world generation
 * Manages landscape expansion, biome zones, and terrain features
 * Handles river systems, cliffs, valleys, and natural boundaries
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MassiveTerrainSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MassiveTerrainSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core terrain management
    UFUNCTION(BlueprintCallable, Category = "Massive Terrain")
    void InitializeMassiveTerrain();

    UFUNCTION(BlueprintCallable, Category = "Massive Terrain")
    void CreateBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Massive Terrain")
    void GenerateRiverSystems();

    UFUNCTION(BlueprintCallable, Category = "Massive Terrain")
    void CreateTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "Massive Terrain")
    void EstablishBiomeBoundaries();

    // Landscape management
    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void ExpandLandscapeTo200km2();

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void SetupWorldComposition();

    // Biome zone properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FWorld_BiomeZoneData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    float BiomeZoneRadius = 5000000.0f; // 50km radius per biome

    // River system properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    TArray<FWorld_RiverSystemData> RiverSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    int32 MaxRiverSegments = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float DefaultRiverWidth = 30000.0f; // 300m wide

    // Terrain feature properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Features")
    int32 MaxTerrainFeatures = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Features")
    float TerrainFeatureSpacing = 1000000.0f; // 10km spacing

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxRenderDistance = 10000000.0f; // 100km render distance

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerBatch = 30;

    // Audio zone integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bCreateAudioZones = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AudioZoneOverlap = 500000.0f; // 5km overlap between zones

protected:
    // Internal terrain generation
    void CreateLandmarkActors(const FWorld_BiomeZoneData& BiomeData, int32 Count);
    void CreateWaterBodies(const FWorld_BiomeZoneData& BiomeData);
    void CreateWeatherZones(const FWorld_BiomeZoneData& BiomeData);
    
    // River generation helpers
    void CreateRiverSegments(const FWorld_RiverSystemData& RiverData);
    FVector CalculateRiverCurve(const FVector& Start, const FVector& End, float T, float CurveIntensity);
    
    // Terrain feature helpers
    void CreateCliffs(const FVector& Center, int32 Count);
    void CreateValleys(const FVector& Center, int32 Count);
    void CreateHills(const FVector& Center, int32 Count);
    void CreateDunes(const FVector& Center, int32 Count);
    void CreateCraters(const FVector& Center, int32 Count);

    // Boundary creation
    void CreateBiomeBoundaryMarkers(const FVector& Start, const FVector& End, const FString& BoundaryName);

    // Cleanup utilities
    UFUNCTION(BlueprintCallable, Category = "Cleanup")
    void CleanupDuplicateActors();

    // Current state tracking
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bTerrainInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 TotalActorsCreated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float WorldSize = 20000000.0f; // 200km in UE units

    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Landscape reference
    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    class ALandscape* PrimaryLandscape;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastGenerationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorsPerSecond = 0;
};