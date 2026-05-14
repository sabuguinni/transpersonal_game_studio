#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Eng_BiomeArchitecture.generated.h"

// ============================================================================
// BIOME ARCHITECTURE SYSTEM - AGENT #2
// Defines the technical architecture for the 5 biomes required for asset purchase
// Supports the 10km² landscape expansion and biome separation criteria
// ============================================================================

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Swamp,          // Southwest - Wetlands with dinosaur nesting areas
    Forest,         // Northwest - Dense prehistoric forest
    Savanna,        // Center - Open grasslands with herds
    Desert,         // East - Arid badlands with fossils
    Mountains       // Northeast - Rocky highlands with caves
};

UENUM(BlueprintType)
enum class EEng_BiomeStatus : uint8
{
    NotGenerated,   // Biome not yet created
    Generated,      // Basic terrain generated
    Populated,      // Flora/fauna added
    Optimized,      // LOD and performance optimized
    Complete        // Ready for asset integration
};

USTRUCT(BlueprintType)
struct FEng_BiomeRequirements
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector2D WorldPosition;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector2D BiomeSize;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float MinElevation;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float MaxElevation;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<FString> RequiredFoliageTypes;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<FString> RequiredDinosaurTypes;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    int32 MaxActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float PerformanceWeight;

    FEng_BiomeRequirements()
    {
        BiomeType = EEng_BiomeType::Savanna;
        WorldPosition = FVector2D::ZeroVector;
        BiomeSize = FVector2D(2000.0f, 2000.0f);
        MinElevation = 0.0f;
        MaxElevation = 500.0f;
        RequiredFoliageTypes.Empty();
        RequiredDinosaurTypes.Empty();
        MaxActorCount = 1000;
        PerformanceWeight = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FEng_BiomeMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EEng_BiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EEng_BiomeStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CurrentActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bReadyForAssets;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    TArray<FString> MissingRequirements;

    FEng_BiomeMetrics()
    {
        BiomeType = EEng_BiomeType::Savanna;
        Status = EEng_BiomeStatus::NotGenerated;
        CurrentActorCount = 0;
        MemoryUsageMB = 0.0f;
        AverageFrameTimeMS = 0.0f;
        bReadyForAssets = false;
        MissingRequirements.Empty();
    }
};

USTRUCT(BlueprintType)
struct FEng_LandscapeSpecs
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    FVector2D TotalSize;

    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    int32 ComponentSizeQuads;

    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    int32 SectionsPerComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    int32 QuadsPerSection;

    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    bool bUseWorldPartition;

    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    int32 StreamingLODLevels;

    FEng_LandscapeSpecs()
    {
        TotalSize = FVector2D(10240.0f, 10240.0f); // 10km²
        ComponentSizeQuads = 255;
        SectionsPerComponent = 2;
        QuadsPerSection = 63;
        bUseWorldPartition = true;
        StreamingLODLevels = 4;
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
    // BIOME ARCHITECTURE MANAGEMENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void InitializeBiomeRequirements();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeRequirements GetBiomeRequirements(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SetBiomeRequirements(EEng_BiomeType BiomeType, const FEng_BiomeRequirements& Requirements);

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    TArray<EEng_BiomeType> GetAllBiomeTypes() const;

    // ========================================================================
    // LANDSCAPE ARCHITECTURE
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void ValidateLandscapeSpecs();

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    FEng_LandscapeSpecs GetLandscapeSpecs() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SetLandscapeSpecs(const FEng_LandscapeSpecs& Specs);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool ValidateWorldPartitionSetup() const;

    // ========================================================================
    // BIOME STATUS MONITORING
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void UpdateBiomeMetrics();

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    FEng_BiomeMetrics GetBiomeMetrics(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    TArray<FEng_BiomeMetrics> GetAllBiomeMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool IsBiomeReadyForAssets(EEng_BiomeType BiomeType) const;

    // ========================================================================
    // ASSET PURCHASE CRITERIA VALIDATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Architecture")
    void ValidateAssetPurchaseCriteria();

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    bool CheckCriterion1_LandscapeExpansion() const;

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    bool CheckCriterion2_AtmosphereStability() const;

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    bool CheckCriterion3_FBXPipeline() const;

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    bool AreAllCriteriaMet() const;

    // ========================================================================
    // PERFORMANCE ARCHITECTURE
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void OptimizeBiomePerformance(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void SetupLODChains();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void ConfigureStreamingLevels();

    // ========================================================================
    // AGENT COORDINATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void RegisterBiomeAgent(int32 AgentID, EEng_BiomeType ResponsibleBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void ValidateAgentBiomeOutput(int32 AgentID, const TArray<FString>& CreatedAssets);

    UFUNCTION(BlueprintPure, Category = "Biome Architecture")
    EEng_BiomeType GetAgentResponsibleBiome(int32 AgentID) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<EEng_BiomeType, FEng_BiomeRequirements> BiomeRequirements;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<EEng_BiomeType, FEng_BiomeMetrics> BiomeMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    FEng_LandscapeSpecs LandscapeSpecs;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    TMap<int32, EEng_BiomeType> AgentBiomeAssignments;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    bool bCriterion1_LandscapeExpanded;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    bool bCriterion2_AtmosphereStable;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    bool bCriterion3_FBXPipelineReady;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Architecture")
    bool bBiomeArchitectureInitialized;

private:
    void InitializeSwampRequirements();
    void InitializeForestRequirements();
    void InitializeSavannaRequirements();
    void InitializeDesertRequirements();
    void InitializeMountainRequirements();
    
    void UpdateBiomeStatus(EEng_BiomeType BiomeType);
    void CalculateBiomePerformance(EEng_BiomeType BiomeType);
    void ValidateBiomeActorCounts();
    
    FVector2D CalculateBiomeWorldPosition(EEng_BiomeType BiomeType) const;
    bool ValidateBiomeSeparation() const;
};

#include "Eng_BiomeArchitecture.generated.h"