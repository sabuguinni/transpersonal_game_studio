#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Engine/DataTable.h"
#include "JurassicBiomeManagerV2.generated.h"

/**
 * Jurassic Biome Manager V2
 * Manages the 5 specific biomes from the Geographic Guide:
 * Forest (starting point), Swamp, Savana, Desert, Snowy Rockside
 * 
 * Each biome has unique characteristics that affect gameplay:
 * - Visibility (fog, density)
 * - Dinosaur populations
 * - Resource availability
 * - Weather patterns
 * - Player safety levels
 */

UENUM(BlueprintType)
enum class EWorld_JurassicBiomeType : uint8
{
    Forest          UMETA(DisplayName = "Forest (Tropical Forest)"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Savana          UMETA(DisplayName = "Savana"),
    Desert          UMETA(DisplayName = "Desert"),
    SnowyRockside   UMETA(DisplayName = "Snowy Rockside"),
    Transition      UMETA(DisplayName = "Transition Zone")
};

UENUM(BlueprintType)
enum class ERiverFlowState : uint8
{
    FullAndFast     UMETA(DisplayName = "Full and Fast (Forest)"),
    FullAndSlow     UMETA(DisplayName = "Full and Slow (Swamp)"),
    Reduced         UMETA(DisplayName = "Reduced Flow (Savana)"),
    VeryDry         UMETA(DisplayName = "Very Dry (Desert)"),
    Frozen          UMETA(DisplayName = "Mostly Frozen (Snowy Rockside)")
};

UENUM(BlueprintType)
enum class EVisibilityLevel : uint8
{
    Full            UMETA(DisplayName = "Full Visibility (10km)"),
    Reduced         UMETA(DisplayName = "Reduced Visibility (5km)"),
    Limited         UMETA(DisplayName = "Limited Visibility (2km)"),
    VeryLimited     UMETA(DisplayName = "Very Limited (500m)"),
    Minimal         UMETA(DisplayName = "Minimal (100m)")
};

UENUM(BlueprintType)
enum class ESafetyLevel : uint8
{
    VeryHigh        UMETA(DisplayName = "Very High (Dense Forest)"),
    High            UMETA(DisplayName = "High (Forest Edge)"),
    Medium          UMETA(DisplayName = "Medium (Swamp)"),
    Low             UMETA(DisplayName = "Low (Savana)"),
    VeryLow         UMETA(DisplayName = "Very Low (Desert)"),
    Extreme         UMETA(DisplayName = "Extreme (Open Areas)")
};

USTRUCT(BlueprintType)
struct FJurassicClimateData
{
    GENERATED_BODY()

    /** Temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Temperature = 25.0f;

    /** Humidity (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Humidity = 0.7f;

    /** Wind strength (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float WindStrength = 0.3f;

    /** Has frequent storms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    bool bHasFrequentStorms = false;

    /** Has permanent fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    bool bHasPermanentFog = false;

    /** Has sandstorms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    bool bHasSandstorms = false;

    /** Has snow */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    bool bHasSnow = false;

    /** Rainfall intensity (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float RainfallIntensity = 0.5f;
};

USTRUCT(BlueprintType)
struct FJurassicBiomeDefinition
{
    GENERATED_BODY()

    /** Biome type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_JurassicBiomeType BiomeType = EWorld_JurassicBiomeType::Forest;

    /** Biome name for display */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Forest");

    /** Climate data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    FJurassicClimateData ClimateData;

    /** Elevation range in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D ElevationRange = FVector2D(0.0f, 500.0f);

    /** Slope range in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D SlopeRange = FVector2D(0.0f, 30.0f);

    /** River flow state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    ERiverFlowState RiverFlowState = ERiverFlowState::FullAndFast;

    /** Requires water proximity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bRequiresWaterProximity = false;

    /** Optimal distance from water in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float OptimalWaterDistance = 1000.0f;

    /** Visibility level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    EVisibilityLevel VisibilityLevel = EVisibilityLevel::Full;

    /** Safety level for player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    ESafetyLevel SafetyLevel = ESafetyLevel::High;

    /** Transition zone width in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionZoneWidth = 2000.0f;

    /** Adjacent biomes for natural transitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    TArray<EWorld_JurassicBiomeType> AdjacentBiomes;

    /** PCG Graph for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    class UPCGGraph* BiomePCGGraph = nullptr;

    /** Biome coverage percentage of total world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CoveragePercentage = 0.2f;

    /** Priority for biome placement (higher = placed first) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    int32 PlacementPriority = 1;
};

USTRUCT(BlueprintType)
struct FBiomeTransitionData
{
    GENERATED_BODY()

    /** Source biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EWorld_JurassicBiomeType SourceBiome = EWorld_JurassicBiomeType::Forest;

    /** Target biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EWorld_JurassicBiomeType TargetBiome = EWorld_JurassicBiomeType::Swamp;

    /** Transition smoothness (0.0 = hard edge, 1.0 = very smooth) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TransitionSmoothness = 0.7f;

    /** Transition width in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionWidth = 1500.0f;

    /** PCG Graph for transition zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    class UPCGGraph* TransitionPCGGraph = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AJurassicBiomeManagerV2 : public AActor
{
    GENERATED_BODY()

public:
    AJurassicBiomeManagerV2();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Root scene component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    /** Master PCG component for biome management */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPCGComponent* BiomePCGComponent;

    /** All biome definitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FJurassicBiomeDefinition> BiomeDefinitions;

    /** Biome transition configurations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    TArray<FBiomeTransitionData> BiomeTransitions;

    /** Master biome distribution graph */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    class UPCGGraph* MasterBiomeGraph;

    /** World size in kilometers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float WorldSizeKm = 200.0f;

    /** Biome distribution seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 BiomeSeed = 54321;

    /** Enable biome transitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bEnableBiomeTransitions = true;

    /** Minimum biome patch size in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinBiomePatchSize = 5000.0f;

public:
    /** Initialize biome system */
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomeSystem();

    /** Generate biome distribution */
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void GenerateBiomeDistribution();

    /** Get biome at world location */
    UFUNCTION(BlueprintPure, Category = "Biome Management")
    EWorld_JurassicBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get biome definition by type */
    UFUNCTION(BlueprintPure, Category = "Biome Management")
    FJurassicBiomeDefinition GetBiomeDefinition(EWorld_JurassicBiomeType BiomeType) const;

    /** Get climate data at location */
    UFUNCTION(BlueprintPure, Category = "Biome Management")
    FJurassicClimateData GetClimateAtLocation(const FVector& WorldLocation) const;

    /** Get visibility level at location */
    UFUNCTION(BlueprintPure, Category = "Biome Management")
    EVisibilityLevel GetVisibilityAtLocation(const FVector& WorldLocation) const;

    /** Get safety level at location */
    UFUNCTION(BlueprintPure, Category = "Biome Management")
    ESafetyLevel GetSafetyAtLocation(const FVector& WorldLocation) const;

    /** Check if location is in transition zone */
    UFUNCTION(BlueprintPure, Category = "Biome Management")
    bool IsLocationInTransitionZone(const FVector& WorldLocation) const;

    /** Get all biomes within radius */
    UFUNCTION(BlueprintPure, Category = "Biome Management")
    TArray<EWorld_JurassicBiomeType> GetBiomesInRadius(const FVector& CenterLocation, float Radius) const;

protected:
    /** Setup default biome definitions */
    void SetupDefaultBiomes();

    /** Setup biome transitions */
    void SetupBiomeTransitions();

    /** Calculate biome distribution using noise */
    EWorld_JurassicBiomeType CalculateBiomeAtLocation(const FVector& WorldLocation) const;

    /** Calculate transition influence */
    float CalculateTransitionInfluence(const FVector& WorldLocation, EWorld_JurassicBiomeType BiomeType) const;

    /** Generate Perlin noise for biome distribution */
    float GenerateBiomeNoise(const FVector& WorldLocation, float Frequency, int32 Octaves = 4) const;

private:
    /** Cached biome map for performance */
    mutable TMap<FIntPoint, EWorld_JurassicBiomeType> BiomeCache;

    /** Cache grid size in meters */
    float BiomeCacheGridSize = 1000.0f;

    /** Is biome system initialized */
    bool bBiomeSystemInitialized = false;
};