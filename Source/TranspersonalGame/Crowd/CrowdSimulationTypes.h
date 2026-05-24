#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationTypes.generated.h"

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Walking UMETA(DisplayName = "Walking"),
    Running UMETA(DisplayName = "Running"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Gathering UMETA(DisplayName = "Gathering"),
    Following UMETA(DisplayName = "Following")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High UMETA(DisplayName = "High Detail"),
    Medium UMETA(DisplayName = "Medium Detail"),
    Low UMETA(DisplayName = "Low Detail"),
    Culled UMETA(DisplayName = "Culled")
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorType : uint8
{
    Wandering UMETA(DisplayName = "Wandering"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Gathering UMETA(DisplayName = "Gathering"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Following UMETA(DisplayName = "Following")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_AgentState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_LODLevel LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector TargetLocation;

    FCrowd_AgentData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        State = ECrowd_AgentState::Idle;
        LODLevel = ECrowd_LODLevel::High;
        Speed = 200.0f;
        Health = 100.0f;
        AgentID = -1;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathfindingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsPathValid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AcceptanceRadius;

    FCrowd_PathfindingData()
    {
        CurrentWaypointIndex = 0;
        PathfindingRadius = 100.0f;
        bIsPathValid = false;
        AcceptanceRadius = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxHighDetailAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxMediumDetailAgents;

    FCrowd_LODSettings()
    {
        HighDetailDistance = 500.0f;
        MediumDetailDistance = 1000.0f;
        LowDetailDistance = 2000.0f;
        CullDistance = 3000.0f;
        MaxHighDetailAgents = 50;
        MaxMediumDetailAgents = 200;
    }
};

#include "CrowdSimulationTypes.generated.h"