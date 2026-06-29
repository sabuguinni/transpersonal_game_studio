#pragma once

#include "CoreMinimal.h"
#include "CrowdBehaviorTypes.generated.h"

// ============================================================
// Crowd Behavior Types — Agent #13 Crowd & Traffic Simulation
// Cycle: PROD_CYCLE_AUTO_20260629_010
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Wandering       UMETA(DisplayName = "Wandering"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Stampeding      UMETA(DisplayName = "Stampeding"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECrowd_HerdType : uint8
{
    Herbivore_Small     UMETA(DisplayName = "Small Herbivore"),
    Herbivore_Large     UMETA(DisplayName = "Large Herbivore"),
    Omnivore            UMETA(DisplayName = "Omnivore"),
    Scavenger           UMETA(DisplayName = "Scavenger"),
    Primitive_Human     UMETA(DisplayName = "Primitive Human Tribe")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Panic       UMETA(DisplayName = "Panic - Full Stampede")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_HerdType HerdType = ECrowd_HerdType::Herbivore_Small;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float FleeSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float SeparationRadius = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_ThreatLevel ThreatLevel = ECrowd_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 HerdGroupID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float Health = 100.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 GroupID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdType HerdType = ECrowd_HerdType::Herbivore_Small;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<int32> MemberAgentIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 LeaderAgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector GroupCentroid = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_ThreatLevel GroupThreatLevel = ECrowd_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsStampeding = false;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float TriggerRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector StampedeDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 AffectedGroupID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float TimeRemaining = 0.0f;
};
