#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Crowd_SharedTypes.generated.h"

/**
 * Crowd simulation density levels for performance scaling
 */
UENUM(BlueprintType)
enum class ECrowd_DensityLevel : uint8
{
    None        UMETA(DisplayName = "No Crowd"),
    Low         UMETA(DisplayName = "Low Density (100-500 agents)"),
    Medium      UMETA(DisplayName = "Medium Density (500-2000 agents)"),
    High        UMETA(DisplayName = "High Density (2000-10000 agents)"),
    Extreme     UMETA(DisplayName = "Extreme Density (10000+ agents)")
};

/**
 * Crowd agent behavior states
 */
UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Following   UMETA(DisplayName = "Following"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Resting     UMETA(DisplayName = "Resting")
};

/**
 * Crowd formation types for organized movement
 */
UENUM(BlueprintType)
enum class ECrowd_FormationType : uint8
{
    None        UMETA(DisplayName = "No Formation"),
    Line        UMETA(DisplayName = "Line Formation"),
    Circle      UMETA(DisplayName = "Circle Formation"),
    Wedge       UMETA(DisplayName = "Wedge Formation"),
    Scatter     UMETA(DisplayName = "Scattered Formation"),
    Cluster     UMETA(DisplayName = "Clustered Formation")
};

/**
 * LOD levels for crowd rendering optimization
 */
UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    FullDetail  UMETA(DisplayName = "Full Detail (0-50m)"),
    Medium      UMETA(DisplayName = "Medium Detail (50-200m)"),
    Low         UMETA(DisplayName = "Low Detail (200-500m)"),
    Impostor    UMETA(DisplayName = "Impostor (500m+)"),
    Hidden      UMETA(DisplayName = "Hidden/Culled")
};

/**
 * Crowd spawn configuration data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfig
{
    GENERATED_BODY()

    /** Number of agents to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 AgentCount = 100;

    /** Spawn radius around the spawn point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnRadius = 500.0f;

    /** Initial density level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowd_DensityLevel DensityLevel = ECrowd_DensityLevel::Medium;

    /** Initial agent state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowd_AgentState InitialState = ECrowd_AgentState::Walking;

    /** Formation type for organized spawning */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowd_FormationType FormationType = ECrowd_FormationType::Scatter;

    FCrowd_SpawnConfig()
    {
        AgentCount = 100;
        SpawnRadius = 500.0f;
        DensityLevel = ECrowd_DensityLevel::Medium;
        InitialState = ECrowd_AgentState::Walking;
        FormationType = ECrowd_FormationType::Scatter;
    }
};

/**
 * Crowd agent performance settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PerformanceSettings
{
    GENERATED_BODY()

    /** Maximum number of agents to simulate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAgents = 50000;

    /** Distance for LOD level 0 (full detail) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD0Distance = 50.0f;

    /** Distance for LOD level 1 (medium detail) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD1Distance = 200.0f;

    /** Distance for LOD level 2 (low detail) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD2Distance = 500.0f;

    /** Distance for LOD level 3 (impostor) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD3Distance = 1000.0f;

    /** Update frequency for distant agents (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DistantUpdateRate = 5.0f;

    /** Enable GPU-based simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseGPUSimulation = true;

    FCrowd_PerformanceSettings()
    {
        MaxAgents = 50000;
        LOD0Distance = 50.0f;
        LOD1Distance = 200.0f;
        LOD2Distance = 500.0f;
        LOD3Distance = 1000.0f;
        DistantUpdateRate = 5.0f;
        bUseGPUSimulation = true;
    }
};

/**
 * Crowd pathfinding configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingConfig
{
    GENERATED_BODY()

    /** Maximum pathfinding distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MaxPathDistance = 2000.0f;

    /** Agent avoidance radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AvoidanceRadius = 100.0f;

    /** Separation force strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float SeparationStrength = 1.0f;

    /** Alignment force strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AlignmentStrength = 0.5f;

    /** Cohesion force strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float CohesionStrength = 0.3f;

    /** Maximum movement speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MaxSpeed = 300.0f;

    /** Enable flow field pathfinding */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bUseFlowFields = true;

    FCrowd_PathfindingConfig()
    {
        MaxPathDistance = 2000.0f;
        AvoidanceRadius = 100.0f;
        SeparationStrength = 1.0f;
        AlignmentStrength = 0.5f;
        CohesionStrength = 0.3f;
        MaxSpeed = 300.0f;
        bUseFlowFields = true;
    }
};