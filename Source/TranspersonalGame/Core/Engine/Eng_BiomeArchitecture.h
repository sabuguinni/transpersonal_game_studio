#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../../SharedTypes.h"
#include "Eng_BiomeArchitecture.generated.h"

// Forward declarations
class UPCGComponent;
class UStaticMeshComponent;
class ULandscapeComponent;

/**
 * Biome data structure defining environmental characteristics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    FEng_BiomeData()
        : BiomeType(EBiomeType::Grassland)
        , Temperature(20.0f)
        , Humidity(0.5f)
        , Elevation(0.0f)
        , VegetationDensity(0.7f)
        , WaterAvailability(0.5f)
        , DinosaurSpawnRate(1.0f)
        , ResourceAbundance(0.6f)
    {}

    /** Type of biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    /** Average temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "-50.0", ClampMax = "60.0"))
    float Temperature;

    /** Humidity level (0.0 = dry, 1.0 = very humid) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity;

    /** Elevation above sea level in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Elevation;

    /** Vegetation density (0.0 = barren, 1.0 = dense forest) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity;

    /** Water availability (0.0 = desert, 1.0 = wetland) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterAvailability;

    /** Dinosaur spawn rate multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wildlife", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float DinosaurSpawnRate;

    /** Resource abundance multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float ResourceAbundance;
};

/**
 * Biome transition data for smooth blending between biomes
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    FEng_BiomeTransition()
        : FromBiome(EBiomeType::Grassland)
        , ToBiome(EBiomeType::Forest)
        , TransitionDistance(1000.0f)
        , BlendCurve(nullptr)
    {}

    /** Source biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome;

    /** Target biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome;

    /** Distance over which transition occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float TransitionDistance;

    /** Curve defining transition blend */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    class UCurveFloat* BlendCurve;
};

/**
 * Core biome manager component that handles biome logic and transitions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_BiomeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_BiomeComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Get biome data at specified world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get blended biome data considering transitions */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBlendedBiomeData(const FVector& WorldLocation, float BlendRadius = 500.0f) const;

    /** Check if location is in biome transition zone */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsInTransitionZone(const FVector& WorldLocation) const;

    /** Get dominant biome type in area */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetDominantBiome(const FVector& CenterLocation, float Radius) const;

    /** Update biome data for specific region */
    UFUNCTION(BlueprintCallable, Category = "Biome", CallInEditor)
    void UpdateBiomeRegion(const FVector& CenterLocation, float Radius, const FEng_BiomeData& NewBiomeData);

    /** Initialize biome system with world data */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void InitializeBiomeSystem();

    /** Validate biome configuration */
    UFUNCTION(BlueprintCallable, Category = "Biome", CallInEditor)
    bool ValidateBiomeConfiguration() const;

protected:
    /** Array of biome regions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FEng_BiomeData> BiomeRegions;

    /** Biome transition definitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    /** Default biome data for unspecified areas */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    FEng_BiomeData DefaultBiome;

    /** Enable biome system debugging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableBiomeDebug;

    /** Debug visualization radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float DebugVisualizationRadius;

private:
    /** Calculate biome blend weight based on distance */
    float CalculateBlendWeight(float Distance, float MaxDistance) const;

    /** Find nearest biome transition */
    const FEng_BiomeTransition* FindNearestTransition(const FVector& Location) const;

    /** Internal biome lookup cache */
    mutable TMap<FVector, FEng_BiomeData> BiomeCache;

    /** Cache validity timestamp */
    mutable float CacheTimestamp;

    /** Cache update interval in seconds */
    static constexpr float CACHE_UPDATE_INTERVAL = 1.0f;
};

/**
 * Biome Manager Actor - central coordinator for biome system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    /** Get the global biome manager instance */
    UFUNCTION(BlueprintCallable, Category = "Biome", meta = (CallInEditor = "true"))
    static AEng_BiomeManager* GetBiomeManager(const UWorld* World);

    /** Register biome-aware actor */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeActor(AActor* Actor);

    /** Unregister biome-aware actor */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UnregisterBiomeActor(AActor* Actor);

    /** Update all registered actors with current biome data */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateRegisteredActors();

    /** Generate biome map for world */
    UFUNCTION(BlueprintCallable, Category = "Biome", CallInEditor)
    void GenerateBiomeMap();

    /** Get biome component */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    UEng_BiomeComponent* GetBiomeComponent() const { return BiomeComponent; }

protected:
    /** Core biome component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UEng_BiomeComponent* BiomeComponent;

    /** Root scene component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    /** Registered biome-aware actors */
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredActors;

    /** Update frequency for registered actors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float ActorUpdateFrequency;

    /** Maximum actors to update per tick */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxActorsPerTick;

private:
    /** Current actor update index */
    int32 CurrentUpdateIndex;

    /** Time since last actor update */
    float TimeSinceLastUpdate;

    /** Global biome manager instance */
    static TWeakObjectPtr<AEng_BiomeManager> GlobalBiomeManager;
};