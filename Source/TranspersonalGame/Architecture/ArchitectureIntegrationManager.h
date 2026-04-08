// Copyright Transpersonal Game Studio. All Rights Reserved.
// Architecture Integration Manager - Connecting all architectural systems
// Agent #07 - Architecture & Interior Agent
// CYCLE_ID: REQ-20260408-MKT-004

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "ArchitectureTypes.h"
#include "JurassicArchitectureCore.h"
#include "JurassicArchitectureManager.h"
#include "ArchitecturalStorytellingSystem.h"
#include "PCGArchitectureGenerator.h"
#include "PrehistoricStructure.h"
#include "../WorldGeneration/WorldGenerationSubsystem.h"
#include "../EnvironmentArt/EnvironmentArtSubsystem.h"
#include "ArchitectureIntegrationManager.generated.h"

/**
 * @brief Architecture Integration Manager - Transpersonal Game Studio
 * 
 * \"Não és um agente que coloca modelos 3D. És o arqueólogo que cria os artefactos 
 * que os arqueólogos do futuro vão estudar.\" - Architecture Agent Philosophy
 * 
 * Core Philosophy (Stewart Brand + Gaston Bachelard):
 * - Buildings show time in layers - construction, habitation, abandonment, decay
 * - Every interior tells a complete story - who lived there, what happened to them
 * - Architecture is evidence of human adaptation to a hostile prehistoric world
 * - No structure exists without narrative purpose and environmental integration
 * 
 * System Integration Responsibilities:
 * - Coordinate all architectural subsystems into a coherent whole
 * - Integrate with World Generation for biome-appropriate placement
 * - Work with Environment Art for seamless material and prop consistency
 * - Prepare lighting anchor points and atmospheric zones for Lighting Agent (#8)
 * - Provide quest hooks and narrative elements for future agents
 * - Manage performance optimization across all architectural systems
 * 
 * Technical Architecture:
 * - Orchestrates JurassicArchitectureManager for structure placement
 * - Manages ArchitecturalStorytellingSystem for interior narratives
 * - Controls PCGArchitectureGenerator for procedural generation
 * - Coordinates PrehistoricStructure instances for specific buildings
 * - Integrates with UE5 Nanite, Lumen, and World Partition systems
 * 
 * Performance Targets:
 * - 60fps on PC / 30fps on console with full architectural detail
 * - Seamless streaming with World Partition for large worlds
 * - Aggressive LOD chains for structures visible at distance
 * - Memory-efficient instancing for repeated structural elements
 * 
 * @author Architecture & Interior Agent — Agent #7
 * @version 1.0 — March 2026
 */

class UWorldGenerationSubsystem;
class UEnvironmentArtSubsystem;
class AJurassicArchitectureManager;
class UArchitecturalStorytellingSystem;
class UPCGArchitectureGenerator;
class APrehistoricStructure;
class UPCGGraph;
class UPCGComponent;

/** Architecture generation phases */
UENUM(BlueprintType)
enum class EArchitectureGenerationPhase : uint8
{
    Planning            UMETA(DisplayName = \"Planning - Site Analysis and Selection\"),
    Foundation          UMETA(DisplayName = \"Foundation - Base Structure Placement\"),
    Construction        UMETA(DisplayName = \"Construction - Building Assembly\"),
    Habitation          UMETA(DisplayName = \"Habitation - Interior Furnishing\"),
    Storytelling        UMETA(DisplayName = \"Storytelling - Narrative Props Placement\"),
    Weathering          UMETA(DisplayName = \"Weathering - Decay and Nature Reclamation\"),
    Integration         UMETA(DisplayName = \"Integration - Environment Art Blending\"),
    Optimization        UMETA(DisplayName = \"Optimization - Performance Tuning\")
};

/** Architecture quality levels for performance scaling */
UENUM(BlueprintType)
enum class EArchitectureQuality : uint8
{
    Minimal             UMETA(DisplayName = \"Minimal - Basic Structures Only\"),
    Low                 UMETA(DisplayName = \"Low - Reduced Detail and Props\"),
    Medium              UMETA(DisplayName = \"Medium - Balanced Quality/Performance\"),
    High                UMETA(DisplayName = \"High - Full Detail and Storytelling\"),
    Cinematic           UMETA(DisplayName = \"Cinematic - Maximum Quality for Screenshots\")
};

/** Architecture generation settings */
USTRUCT(BlueprintType)
struct FArchitectureGenerationSettings
{
    GENERATED_BODY()

    /** Overall generation settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Generation\")
    EArchitectureQuality QualityLevel = EArchitectureQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Generation\", meta = (ClampMin = \"0.0\", ClampMax = \"2.0\"))
    float StructureDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Generation\", meta = (ClampMin = \"0.0\", ClampMax = \"2.0\"))
    float StorytellingDensity = 1.0f;

    /** Biome-specific settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Biome Settings\")
    TMap<EJurassicBiomeType, float> BiomeStructureProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Biome Settings\")
    TMap<EJurassicBiomeType, EConstructionTechnique> PreferredConstructionByBiome;

    /** Settlement clustering */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Settlement\")
    bool bEnableSettlementClusters = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Settlement\", meta = (ClampMin = \"1\", ClampMax = \"10\"))
    int32 MaxStructuresPerCluster = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Settlement\", meta = (ClampMin = \"100.0\", ClampMax = \"2000.0\"))
    float ClusterRadius = 500.0f;

    /** Storytelling configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Storytelling\")
    bool bEnableDetailedInteriors = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Storytelling\")
    bool bEnableQuestHooks = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Storytelling\", meta = (ClampMin = \"0.0\", ClampMax = \"1.0\"))
    float TragedyStoryProbability = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Storytelling\", meta = (ClampMin = \"0.0\", ClampMax = \"1.0\"))
    float SecretAreaProbability = 0.2f;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")
    bool bUseNaniteForStructures = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")
    bool bUseHierarchicalLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\", meta = (ClampMin = \"1000.0\", ClampMax = \"50000.0\"))
    float MaxRenderDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\", meta = (ClampMin = \"1\", ClampMax = \"100\"))
    int32 MaxVisibleStructures = 50;

    /** Integration settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Integration\")
    bool bIntegrateWithEnvironmentArt = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Integration\")
    bool bCreateLightingAnchors = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Integration\")
    bool bGenerateAtmosphericZones = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Integration\")
    bool bCreateNavigationMesh = true;
};

/** Architecture performance statistics */
USTRUCT(BlueprintType)
struct FArchitecturePerformanceStats
{
    GENERATED_BODY()

    /** Structure counts */
    UPROPERTY(BlueprintReadOnly, Category = \"Statistics\")
    int32 TotalStructuresGenerated = 0;

    UPROPERTY(BlueprintReadOnly, Category = \"Statistics\")
    int32 ActiveStructuresRendered = 0;

    UPROPERTY(BlueprintReadOnly, Category = \"Statistics\")
    int32 StructuresCulled = 0;

    /** Performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = \"Performance\")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = \"Performance\")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = \"Performance\")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = \"Performance\")
    int32 TriangleCount = 0;

    /** Generation statistics */
    UPROPERTY(BlueprintReadOnly, Category = \"Generation\")
    float LastGenerationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = \"Generation\")
    int32 SuccessfulPlacements = 0;

    UPROPERTY(BlueprintReadOnly, Category = \"Generation\")
    int32 FailedPlacements = 0;

    /** Quality metrics */
    UPROPERTY(BlueprintReadOnly, Category = \"Quality\")
    float AverageStorytellingScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = \"Quality\")
    float EnvironmentIntegrationScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = \"Quality\")
    float ArchitecturalAuthenticityScore = 0.0f;
};

/** Architecture region data */
USTRUCT(BlueprintType)
struct FArchitectureRegion
{
    GENERATED_BODY()

    /** Region identification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Region\")
    FString RegionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Region\")
    FBox RegionBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Region\")
    EJurassicBiomeType PrimaryBiome = EJurassicBiomeType::TropicalRainforest;

    /** Structures in this region */
    UPROPERTY(BlueprintReadOnly, Category = \"Structures\")
    TArray<TSoftObjectPtr<APrehistoricStructure>> Structures;

    /** Settlement clusters */
    UPROPERTY(BlueprintReadOnly, Category = \"Settlements\")
    TArray<FSettlementCluster> SettlementClusters;

    /** Region narrative */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Story\")
    FString RegionStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Story\")
    ESettlementStage DominantSettlementStage = ESettlementStage::Abandoned;

    /** Performance data */
    UPROPERTY(BlueprintReadOnly, Category = \"Performance\")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = \"Performance\")
    float LastUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = \"Performance\")
    int32 ActiveStructureCount = 0;
};

/**
 * Architecture Integration Manager Class
 * 
 * Central coordination hub for all architectural systems in the game.
 * Orchestrates the creation of believable prehistoric human settlements
 * that tell stories through their construction, habitation, and abandonment.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureIntegrationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UArchitectureIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /** Main architecture generation functions */
    UFUNCTION(BlueprintCallable, Category = \"Architecture\")
    void GenerateArchitecture(const FArchitectureGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = \"Architecture\")
    void RegenerateRegion(const FBox& WorldBounds, const FArchitectureGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = \"Architecture\")
    void ClearArchitecture();

    /** Phase-by-phase generation */
    UFUNCTION(BlueprintCallable, Category = \"Generation Phases\")
    void ExecuteGenerationPhase(EArchitectureGenerationPhase Phase, const FBox& Region);

    UFUNCTION(BlueprintCallable, Category = \"Generation Phases\")
    bool IsPhaseComplete(EArchitectureGenerationPhase Phase, const FBox& Region) const;

    /** Settlement management */
    UFUNCTION(BlueprintCallable, Category = \"Settlements\")
    void CreateSettlementCluster(const FVector& CenterLocation, const FSettlementCluster& ClusterConfig);

    UFUNCTION(BlueprintCallable, Category = \"Settlements\")
    void PopulateSettlement(const FSettlementCluster& Settlement);

    /** Structure management */
    UFUNCTION(BlueprintCallable, Category = \"Structures\")
    APrehistoricStructure* CreateStructure(const FVector& Location, const FStructureTemplate& Template);

    UFUNCTION(BlueprintCallable, Category = \"Structures\")
    void PopulateStructureInterior(APrehistoricStructure* Structure, const FArchitecturalStory& Story);

    /** Integration with other systems */
    UFUNCTION(BlueprintCallable, Category = \"Integration\")
    void IntegrateWithEnvironmentArt();

    UFUNCTION(BlueprintCallable, Category = \"Integration\")
    void CreateLightingAnchors();

    UFUNCTION(BlueprintCallable, Category = \"Integration\")
    void GenerateAtmosphericZones();

    /** Performance monitoring and optimization */
    UFUNCTION(BlueprintCallable, Category = \"Performance\")
    FArchitecturePerformanceStats GetPerformanceStats() const;

    UFUNCTION(BlueprintCallable, Category = \"Performance\")
    void OptimizePerformance(const FArchitectureGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = \"Performance\")
    void UpdateLODSystem();

    /** Region management */
    UFUNCTION(BlueprintCallable, Category = \"Region Management\")
    void LoadRegion(const FString& RegionName);

    UFUNCTION(BlueprintCallable, Category = \"Region Management\")
    void UnloadRegion(const FString& RegionName);

    UFUNCTION(BlueprintCallable, Category = \"Region Management\")
    FArchitectureRegion GetRegionData(const FString& RegionName) const;

    /** Quality and validation */
    UFUNCTION(BlueprintCallable, Category = \"Quality\")
    float ValidateArchitecturalAuthenticity(const FBox& Region) const;

    UFUNCTION(BlueprintCallable, Category = \"Quality\")
    float ValidateStorytellingQuality(const FBox& Region) const;

    UFUNCTION(BlueprintCallable, Category = \"Quality\")
    float ValidateEnvironmentIntegration(const FBox& Region) const;

    /** Debug and visualization */
    UFUNCTION(BlueprintCallable, Category = \"Debug\")
    void VisualizeArchitecturalData(bool bShowStructures, bool bShowStories, bool bShowPerformance);

    UFUNCTION(BlueprintCallable, Category = \"Debug\")
    void GenerateArchitectureReport(const FString& OutputPath);

protected:
    /** Core subsystem references */
    UPROPERTY()
    UWorldGenerationSubsystem* WorldGenerationSubsystem;

    UPROPERTY()
    UEnvironmentArtSubsystem* EnvironmentArtSubsystem;

    /** Architecture system components */
    UPROPERTY()
    AJurassicArchitectureManager* ArchitectureManager;

    UPROPERTY()
    UArchitecturalStorytellingSystem* StorytellingSystem;

    UPROPERTY()
    UPCGArchitectureGenerator* PCGGenerator;

    /** Current generation settings */
    UPROPERTY()
    FArchitectureGenerationSettings CurrentSettings;

    /** Active regions */
    UPROPERTY()
    TMap<FString, FArchitectureRegion> ActiveRegions;

    /** Performance tracking */
    UPROPERTY()
    FArchitecturePerformanceStats PerformanceStats;

    /** Generation state */
    UPROPERTY()
    TMap<EArchitectureGenerationPhase, bool> PhaseCompletionStatus;

    /** PCG Graph for architecture generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"PCG\")
    TSoftObjectPtr<UPCGGraph> ArchitecturePCGGraph;

    /** Structure templates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Templates\")
    TArray<FStructureTemplate> StructureTemplates;

    /** Settlement templates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Templates\")
    TArray<FSettlementCluster> SettlementTemplates;

private:
    /** Internal generation functions */
    void InitializeSubsystems();
    void LoadArchitectureAssets();
    void SetupPCGGraphs();
    
    /** Phase implementation */
    void ExecutePlanningPhase(const FBox& Region);
    void ExecuteFoundationPhase(const FBox& Region);
    void ExecuteConstructionPhase(const FBox& Region);
    void ExecuteHabitationPhase(const FBox& Region);
    void ExecuteStorytellingPhase(const FBox& Region);
    void ExecuteWeatheringPhase(const FBox& Region);
    void ExecuteIntegrationPhase(const FBox& Region);
    void ExecuteOptimizationPhase(const FBox& Region);

    /** Site analysis */
    bool AnalyzeBuildingSite(const FVector& Location, const FStructureTemplate& Template) const;
    EJurassicBiomeType GetBiomeAtLocation(const FVector& Location) const;
    float GetSlopeAtLocation(const FVector& Location) const;
    float GetWaterDistanceAtLocation(const FVector& Location) const;
    bool HasClearBuildingSpace(const FVector& Location, const FVector& RequiredSpace) const;

    /** Performance optimization */
    void UpdateStructureLODs();
    void CullDistantStructures();
    void OptimizeMemoryUsage();
    void UpdatePerformanceStats();

    /** Integration helpers */
    void BlendWithEnvironmentArt(APrehistoricStructure* Structure);
    void CreateStructureLightingAnchors(APrehistoricStructure* Structure);
    void GenerateStructureAtmosphere(APrehistoricStructure* Structure);

    /** Validation helpers */
    bool ValidateStructurePlacement(const FVector& Location, const FStructureTemplate& Template) const;
    bool ValidateSettlementLayout(const FSettlementCluster& Settlement) const;
    float CalculateStorytellingScore(APrehistoricStructure* Structure) const;
    float CalculateAuthenticityScore(APrehistoricStructure* Structure) const;
};