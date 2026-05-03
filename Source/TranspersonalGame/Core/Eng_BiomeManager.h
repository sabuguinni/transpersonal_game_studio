#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

/**
 * Engine Architect's Biome Management System
 * Defines the 5 biome regions and manages biome-specific spawning rules
 * This is the ARCHITECTURAL FOUNDATION for all world generation systems
 */

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Pantano     UMETA(DisplayName = "Pantano (Swamp)"),
    Floresta    UMETA(DisplayName = "Floresta (Forest)"),
    Savana      UMETA(DisplayName = "Savana (Savanna)"),
    Deserto     UMETA(DisplayName = "Deserto (Desert)"),
    Montanha    UMETA(DisplayName = "Montanha (Mountain)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeRegion
{
    GENERATED_BODY()

    // Biome type identifier
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Savana;

    // Center point of the biome
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    // Valid spawn bounds (min/max X and Y coordinates)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    FVector MinBounds = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    FVector MaxBounds = FVector::ZeroVector;

    // Environmental properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float Elevation = 0.0f;

    FEng_BiomeRegion()
    {
        BiomeType = EEng_BiomeType::Savana;
        Center = FVector::ZeroVector;
        MinBounds = FVector::ZeroVector;
        MaxBounds = FVector::ZeroVector;
        Temperature = 20.0f;
        Humidity = 50.0f;
        Elevation = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SpawnParameters
{
    GENERATED_BODY()

    // Target biome for spawning
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    EEng_BiomeType TargetBiome = EEng_BiomeType::Savana;

    // Random variation from biome center
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    float RandomRadius = 5000.0f;

    // Elevation offset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    float ElevationOffset = 0.0f;

    // Density multiplier for this spawn type
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    float DensityMultiplier = 1.0f;

    FEng_SpawnParameters()
    {
        TargetBiome = EEng_BiomeType::Savana;
        RandomRadius = 5000.0f;
        ElevationOffset = 0.0f;
        DensityMultiplier = 1.0f;
    }
};

/**
 * Biome Manager Actor - Central authority for biome-based world generation
 * This class enforces the architectural rules for all spawning systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;

    // The 5 biome regions with exact coordinates
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_BiomeRegion> BiomeRegions;

    // Visual representation component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* VisualizationMesh;

public:
    // Core architectural methods - MUST be implemented by all world generation systems

    /**
     * Get biome type at a specific world location
     * @param WorldLocation The location to check
     * @return The biome type at that location
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /**
     * Get valid spawn location within a biome
     * @param BiomeType Target biome
     * @param RandomRadius Random variation from center
     * @return Valid spawn location within biome bounds
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FVector GetValidSpawnLocation(EEng_BiomeType BiomeType, float RandomRadius = 5000.0f) const;

    /**
     * Check if location is within biome bounds
     * @param Location Location to check
     * @param BiomeType Biome to check against
     * @return True if location is within biome bounds
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType) const;

    /**
     * Get biome region data
     * @param BiomeType The biome to get data for
     * @return Biome region data structure
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeRegion GetBiomeRegion(EEng_BiomeType BiomeType) const;

    /**
     * Get all biome regions
     * @return Array of all biome regions
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    TArray<FEng_BiomeRegion> GetAllBiomeRegions() const { return BiomeRegions; }

    /**
     * Calculate spawn parameters for a specific biome
     * @param BiomeType Target biome
     * @param ActorType Type of actor being spawned (for density calculation)
     * @return Optimized spawn parameters
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_SpawnParameters CalculateSpawnParameters(EEng_BiomeType BiomeType, const FString& ActorType) const;

    /**
     * Validate spawn location against architectural rules
     * @param Location Proposed spawn location
     * @param ActorType Type of actor being spawned
     * @return True if location is valid according to architectural rules
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool ValidateSpawnLocation(const FVector& Location, const FString& ActorType) const;

    // Architectural enforcement methods

    /**
     * Initialize biome regions with exact coordinates from memory
     * Called during BeginPlay to set up the 5 biome system
     */
    UFUNCTION(CallInEditor, Category = "Architecture")
    void InitializeBiomeRegions();

    /**
     * Validate current world state against biome architecture
     * Checks for actors spawned outside biome bounds
     */
    UFUNCTION(CallInEditor, Category = "Architecture")
    void ValidateWorldArchitecture();

    /**
     * Generate architectural report
     * Outputs current biome usage and architectural compliance
     */
    UFUNCTION(CallInEditor, Category = "Architecture")
    void GenerateArchitecturalReport();

private:
    // Internal helper methods
    void SetupDefaultBiomeRegions();
    float CalculateDistanceToNearestBiome(const FVector& Location) const;
    bool IsWithinMapBounds(const FVector& Location) const;
};