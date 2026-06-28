// BiomeManager.h — Agent #05 Procedural World Generator
// Manages biome zones, transitions, and environmental properties
// Cycle: PROD_CYCLE_AUTO_20260628_006

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Biome type enum — unique prefix World_ to avoid conflicts
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Badlands    UMETA(DisplayName = "Badlands"),
    RiverValley UMETA(DisplayName = "River Valley"),
    Coastal     UMETA(DisplayName = "Coastal"),
    Cave        UMETA(DisplayName = "Cave")
};

// Biome environmental properties
USTRUCT(BlueprintType)
struct FWorld_BiomeProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Plains");

    // Temperature in Celsius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float AmbientTemperature = 25.0f;

    // Humidity 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float Humidity = 0.5f;

    // Fog density 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float FogDensity = 0.02f;

    // Wind speed m/s
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment")
    float WindSpeed = 5.0f;

    // Vegetation density 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation")
    float VegetationDensity = 0.5f;

    // Predator spawn rate multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Danger")
    float PredatorSpawnMultiplier = 1.0f;

    // Ambient sound category
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    FString AmbientSoundCategory = TEXT("jungle_ambient");

    // Biome center location in world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    FVector BiomeCenter = FVector::ZeroVector;

    // Biome radius in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    float BiomeRadius = 5000.0f;

    // Sky color tint for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor SkyColorTint = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    // Fog color for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
};

// Biome transition data
USTRUCT(BlueprintType)
struct FWorld_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    EWorld_BiomeType FromBiome = EWorld_BiomeType::Plains;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    EWorld_BiomeType ToBiome = EWorld_BiomeType::Jungle;

    // 0 = fully in FromBiome, 1 = fully in ToBiome
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    float BlendAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    bool bIsTransitioning = false;
};

/**
 * ABiomeManager — manages biome zones and environmental transitions
 * Placed in the level to define biome boundaries and properties.
 * Provides query API for other systems (audio, AI, weather, vegetation).
 */
UCLASS(ClassGroup = "World", meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Biome Query API ---

    // Get biome at a world location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Get full biome properties at a world location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeProperties GetBiomePropertiesAtLocation(const FVector& WorldLocation) const;

    // Get current transition data for a location
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeTransition GetBiomeTransitionAtLocation(const FVector& WorldLocation) const;

    // Get temperature at location (accounts for biome + time of day)
    UFUNCTION(BlueprintCallable, Category = "Biome|Environment")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Get danger level at location (0-1, based on predator spawn rate)
    UFUNCTION(BlueprintCallable, Category = "Biome|Danger")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    // Register a biome zone
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Setup")
    void RegisterBiomeZone(const FWorld_BiomeProperties& BiomeProperties);

    // Clear all registered biomes
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Setup")
    void ClearAllBiomes();

    // Initialize default biomes for the MinPlayableMap
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Setup")
    void InitializeDefaultBiomes();

    // Debug: draw biome boundaries in viewport
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeBoundaries();

    // --- Properties ---

    // All registered biome zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Zones")
    TArray<FWorld_BiomeProperties> BiomeZones;

    // Transition blend distance (overlap between biomes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Transition")
    float TransitionBlendDistance = 500.0f;

    // Default biome when no zone matches
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType DefaultBiome = EWorld_BiomeType::Plains;

    // Enable debug visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDebugVisualization = false;

    // Tick interval for biome updates (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Performance")
    float BiomeUpdateInterval = 1.0f;

private:
    // Find nearest biome to a location
    int32 FindNearestBiomeIndex(const FVector& WorldLocation) const;

    // Find second nearest biome (for blending)
    int32 FindSecondNearestBiomeIndex(const FVector& WorldLocation, int32 ExcludeIndex) const;

    // Timer for biome updates
    float BiomeUpdateTimer = 0.0f;

    // Cached player location for performance
    FVector CachedPlayerLocation = FVector::ZeroVector;

    // Current player biome
    EWorld_BiomeType CurrentPlayerBiome = EWorld_BiomeType::Plains;
};
