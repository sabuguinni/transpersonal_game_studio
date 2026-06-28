#pragma once

#include "CoreMinimal.h"
#include "CrowdBehaviorTypes.generated.h"

// === CROWD BEHAVIOR TYPES — Agent #13 Crowd & Traffic Simulation ===
// Prehistoric survival crowd simulation: herds, packs, tribal groups

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    None            UMETA(DisplayName = "None"),
    DinosaurHerd    UMETA(DisplayName = "Dinosaur Herd"),
    RaptorPack      UMETA(DisplayName = "Raptor Pack"),
    TribalGroup     UMETA(DisplayName = "Tribal Group"),
    MigrationHerd   UMETA(DisplayName = "Migration Herd"),
    ScavengerGroup  UMETA(DisplayName = "Scavenger Group"),
    FleeingPrey     UMETA(DisplayName = "Fleeing Prey")
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Resting     UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full        UMETA(DisplayName = "Full Simulation"),
    Medium      UMETA(DisplayName = "Medium LOD"),
    Low         UMETA(DisplayName = "Low LOD"),
    Dormant     UMETA(DisplayName = "Dormant")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Panic       UMETA(DisplayName = "Full Panic")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    ECrowd_AgentType AgentType = ECrowd_AgentType::DinosaurHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 GroupSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float ThreatDetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float StampedeSpeedMultiplier = 2.5f;

    FCrowd_AgentConfig()
        : AgentType(ECrowd_AgentType::DinosaurHerd)
        , GroupSize(10)
        , MoveSpeed(300.0f)
        , FleeSpeed(600.0f)
        , SeparationRadius(150.0f)
        , CohesionRadius(500.0f)
        , ThreatDetectionRadius(1200.0f)
        , StampedeSpeedMultiplier(2.5f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_AgentState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_BehaviorState BehaviorState = ECrowd_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_ThreatLevel ThreatLevel = ECrowd_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::Full;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    float ThreatDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bIsLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 GroupID = -1;

    FCrowd_AgentState()
        : BehaviorState(ECrowd_BehaviorState::Idle)
        , ThreatLevel(ECrowd_ThreatLevel::None)
        , LODLevel(ECrowd_LODLevel::Full)
        , FleeDirection(FVector::ZeroVector)
        , ThreatLocation(FVector::ZeroVector)
        , ThreatDistance(0.0f)
        , bIsLeader(false)
        , GroupID(-1)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_GroupData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    int32 GroupID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    ECrowd_AgentType GroupType = ECrowd_AgentType::DinosaurHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    ECrowd_BehaviorState CurrentState = ECrowd_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    FVector GroupCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    FVector MigrationTarget = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    int32 AgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    float GroupThreatLevel = 0.0f;

    FCrowd_GroupData()
        : GroupID(-1)
        , GroupType(ECrowd_AgentType::DinosaurHerd)
        , CurrentState(ECrowd_BehaviorState::Idle)
        , GroupCenter(FVector::ZeroVector)
        , MigrationTarget(FVector::ZeroVector)
        , AgentCount(0)
        , GroupThreatLevel(0.0f)
    {}
};
