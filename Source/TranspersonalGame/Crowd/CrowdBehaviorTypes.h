#pragma once

#include "CoreMinimal.h"
#include "CrowdBehaviorTypes.generated.h"

// ============================================================
// Crowd Behavior Types — Agent #13 Crowd & Traffic Simulation
// Shared enums and structs for the Mass AI crowd system
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Resting     UMETA(DisplayName = "Resting"),
    Alerting    UMETA(DisplayName = "Alerting"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECrowd_HerdRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),
    Scout       UMETA(DisplayName = "Scout"),
    Follower    UMETA(DisplayName = "Follower"),
    Juvenile    UMETA(DisplayName = "Juvenile"),
    Elder       UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ECrowd_StampedePhase : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Triggering  UMETA(DisplayName = "Triggering"),
    Propagating UMETA(DisplayName = "Propagating"),
    FullPanic   UMETA(DisplayName = "Full Panic"),
    Dissipating UMETA(DisplayName = "Dissipating")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full        UMETA(DisplayName = "Full — <50m"),
    Medium      UMETA(DisplayName = "Medium — 50-200m"),
    Low         UMETA(DisplayName = "Low — 200-500m"),
    Ghost       UMETA(DisplayName = "Ghost — >500m")
};

// ---------------------------------------------------------------
// Per-agent runtime data (lightweight, used in Mass Entity)
// ---------------------------------------------------------------
USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State = ECrowd_AgentState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdRole Role = ECrowd_HerdRole::Follower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel LOD = ECrowd_LODLevel::Full;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float PanicLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float LastStateChangeTime = 0.0f;
};

// ---------------------------------------------------------------
// Herd descriptor — one per herd group
// ---------------------------------------------------------------
USTRUCT(BlueprintType)
struct FCrowd_HerdDescriptor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString SpeciesName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector CentroidLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_StampedePhase StampedePhase = ECrowd_StampedePhase::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_ThreatLevel ThreatLevel = ECrowd_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MigrationProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsGrazing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AverageSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FVector> WaypointPath;
};

// ---------------------------------------------------------------
// Stampede event data — broadcast to all nearby agents
// ---------------------------------------------------------------
USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Origin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float PanicIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector FleeDirection = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 TriggerHerdID = -1;
};

// ---------------------------------------------------------------
// LOD configuration for crowd rendering
// ---------------------------------------------------------------
USTRUCT(BlueprintType)
struct FCrowd_LODConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float FullDetailRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float MediumDetailRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LowDetailRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 MaxFullDetailAgents = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 MaxMediumDetailAgents = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 MaxLowDetailAgents = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 MaxGhostAgents = 50000;
};
