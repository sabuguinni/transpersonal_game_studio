#pragma once

#include "CoreMinimal.h"
#include "CrowdBehaviorTypes.generated.h"

// === CROWD BEHAVIOR TYPES — Agent #13 Crowd Simulation ===
// Prehistoric survival game — tribal crowd agents, stampede herds, predator scatter

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Wandering       UMETA(DisplayName = "Wandering"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Stampeding      UMETA(DisplayName = "Stampeding"),
    Sheltering      UMETA(DisplayName = "Sheltering"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    TribalHuman     UMETA(DisplayName = "Tribal Human"),
    HerbivoreHerd   UMETA(DisplayName = "Herbivore Herd Member"),
    ScavengerBird   UMETA(DisplayName = "Scavenger Bird"),
    SmallMammal     UMETA(DisplayName = "Small Mammal"),
    InsectSwarm     UMETA(DisplayName = "Insect Swarm")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low — Distant predator"),
    Medium      UMETA(DisplayName = "Medium — Predator approaching"),
    High        UMETA(DisplayName = "High — Predator attacking"),
    Critical    UMETA(DisplayName = "Critical — Stampede triggered")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::TribalHuman;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_ThreatLevel ThreatLevel = ECrowd_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector FleeTarget = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float StampedeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdGroupID = 0;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector StampedeDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float TriggerRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeDuration = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float ElapsedTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD0_Distance = 1500.0f;   // Full simulation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD1_Distance = 4000.0f;   // Simplified movement

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD2_Distance = 8000.0f;   // Position only

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float CullDistance = 12000.0f;   // Invisible

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 MaxAgentsLOD0 = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 MaxAgentsLOD1 = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    int32 MaxAgentsLOD2 = 1000;
};
