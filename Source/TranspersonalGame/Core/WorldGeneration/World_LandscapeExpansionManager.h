#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_LandscapeExpansionManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_TerrainType : uint8
{
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"), 
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BoundsMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BoundsMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_TerrainType TerrainType;

    FWorld_BiomeRegion()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        BoundsMin = FVector::ZeroVector;
        BoundsMax = FVector::ZeroVector;
        TerrainType = EWorld_TerrainType::Savanna;
    }
};

/**
 * Manages landscape expansion from current 2.4km2 to target 200km2
 * Handles biome-specific terrain generation and landscape streaming
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_LandscapeExpansionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_LandscapeExpansionManager();

protected:
    virtual void BeginPlay() override;

    // Current and target landscape dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector2D CurrentLandscapeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector2D TargetLandscapeSize;

    // Biome regions with coordinates from memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeRegion> BiomeRegions;

public:
    // Core expansion functionality
    UFUNCTION(BlueprintCallable, Category = "Landscape Expansion")
    bool ExpandLandscapeTo200km2();

    UFUNCTION(BlueprintCallable, Category = "Landscape Expansion")
    void CreateBiomeSpecificTerrain();

    // Biome query functions
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeRegion GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsLocationInBiome(const FVector& Location, const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<FWorld_BiomeRegion> GetAllBiomeRegions() const;

private:
    // Internal expansion methods
    void InitializeBiomeData();
    bool CreateLandscapeExpansionSections();
    void CreateTerrainFeaturesForBiome(const FWorld_BiomeRegion& BiomeRegion);
    void CreateTerrainFeatureByType(EWorld_TerrainType TerrainType, const FVector& Location, 
                                   int32 Index, const FString& BiomeName);
};

#include "World_LandscapeExpansionManager.generated.h"