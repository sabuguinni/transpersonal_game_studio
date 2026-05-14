#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Eng_BiomeArchitecture.generated.h"

// ============================================================================
// BIOME ARCHITECTURE SYSTEM - AGENT #2
// Defines the technical architecture for the 5 biomes in MinPlayableMap
// ============================================================================

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Swamp,          // SW - Wetlands with dense vegetation and water features
    Forest,         // NW - Dense prehistoric forest with tall trees
    Savanna,        // Center - Open grasslands with scattered trees
    Desert,         // E - Arid landscape with minimal vegetation
    Mountains       // NE - Rocky terrain with elevation changes
};

UENUM(BlueprintType)
enum class EEng_BiomeStatus : uint8
{
    Uninitialized,  // Biome not yet created
    Terrain,        // Basic terrain generated
    Vegetation,     // Vegetation populated
    Wildlife,       // Dinosaurs and fauna added
    Complete        // Fully functional biome
};

UENUM(BlueprintType)
enum class EEng_TerrainComplexity : uint8
{
    Simple,         // Basic height variations
    Moderate,       // Hills and valleys
    Complex,        // Cliffs, caves, water features
    Extreme         // Vertical terrain, overhangs
};

USTRUCT(BlueprintType)
struct FEng_BiomeSpecification
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector WorldLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector2D BiomeSize;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_TerrainComplexity TerrainComplexity;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    int32 MaxDinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<FString> RequiredAssetTypes;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float PerformanceBudgetMS;

    FEng_BiomeSpecification()
    {
        BiomeType = EEng_BiomeType::Savanna;
        WorldLocation = FVector::ZeroVector;
        BiomeSize = FVector2D(2000.0f, 2000.0f);
        TerrainComplexity = EEng_TerrainComplexity::Moderate;
        VegetationDensity = 0.5f;
        MaxDinosaurCount = 10;
        RequiredAssetTypes.Empty();
        PerformanceBudgetMS = 3.33f; // 20% of 16.67ms frame budget
    }
};

USTRUCT(BlueprintType)
struct FEng_BiomeMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EEng_BiomeStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 VegetationCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bPerformanceWithinBudget;

    FEng_BiomeMetrics()
    {
        CurrentStatus = EEng_BiomeStatus::Uninitialized;
        ActorCount = 0;
        VegetationCount = 0;
        DinosaurCount = 0;
        MemoryUsageMB = 0.0f;
        FrameTimeMS = 0.0f;
        bPerformanceWithinBudget = true;
    }
};

USTRUCT(BlueprintType)
struct FEng_AssetRequirement
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Asset")
    FString AssetType;

    UPROPERTY(BlueprintReadOnly, Category = "Asset")
    FString AssetPath;

    UPROPERTY(BlueprintReadOnly, Category = "Asset")
    int32 MinInstances;

    UPROPERTY(BlueprintReadOnly, Category = "Asset")
    int32 MaxInstances;

    UPROPERTY(BlueprintReadOnly, Category = "Asset")
    float SpawnProbability;

    UPROPERTY(BlueprintReadOnly, Category = "Asset")
    bool bIsCommercialAsset;

    FEng_AssetRequirement()
    {
        AssetType = TEXT("");
        AssetPath = TEXT("");
        MinInstances = 1;
        MaxInstances = 10;
        SpawnProbability = 1.0f;
        bIsCommercialAsset = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // BIOME SPECIFICATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void InitializeBiomeSpecifications();

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    FEng_BiomeSpecification GetBiomeSpecification(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SetBiomeSpecification(EEng_BiomeType BiomeType, const FEng_BiomeSpecification& Specification);

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    TArray<FEng_BiomeSpecification> GetAllBiomeSpecifications() const;

    // ========================================================================
    // BIOME VALIDATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool ValidateBiomeLayout() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeMetrics GetBiomeMetrics(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void UpdateBiomeMetrics(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool CheckBiomePerformance(EEng_BiomeType BiomeType) const;

    // ========================================================================
    // ASSET MANAGEMENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void RegisterAssetRequirement(EEng_BiomeType BiomeType, const FEng_AssetRequirement& Requirement);

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    TArray<FEng_AssetRequirement> GetAssetRequirements(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool ValidateAssetAvailability(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void PrepareForCommercialAssets();

    // ========================================================================
    // PERFORMANCE MONITORING
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void MonitorBiomePerformance();

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    float GetTotalPerformanceBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void EnforceBiomePerformanceLimits();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Architecture")
    void RunBiomeArchitecturalAudit();

    // ========================================================================
    // INTEGRATION SUPPORT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void ValidateWorldPartitionSetup() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SetupBiomeStreamingLevels();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool CheckBiomeReadinessForAssets() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<EEng_BiomeType, FEng_BiomeSpecification> BiomeSpecifications;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<EEng_BiomeType, FEng_BiomeMetrics> BiomeMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<EEng_BiomeType, TArray<FEng_AssetRequirement>> AssetRequirements;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    float TotalPerformanceBudgetMS;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    bool bBiomeArchitectureInitialized;

private:
    void InitializeSwampSpecification();
    void InitializeForestSpecification();
    void InitializeSavannaSpecification();
    void InitializeDesertSpecification();
    void InitializeMountainSpecification();
    
    void SetupDefaultAssetRequirements();
    void CalculatePerformanceBudgets();
    void ValidateBiomeSpacing() const;
    void CheckBiomeOverlaps() const;
};

#include "Eng_BiomeArchitecture.generated.h"