#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrowdGroupConfig.generated.h"

// ============================================================
// Agent #13 — Crowd & Traffic Simulation
// CrowdGroupConfig.h — Prehistoric crowd group configuration
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_GroupType : uint8
{
    ApexPredator    UMETA(DisplayName = "Apex Predator"),
    PackHunter      UMETA(DisplayName = "Pack Hunter"),
    HerbivoreHerd   UMETA(DisplayName = "Herbivore Herd"),
    Scavenger       UMETA(DisplayName = "Scavenger"),
    Solitary        UMETA(DisplayName = "Solitary")
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Resting     UMETA(DisplayName = "Resting"),
    Migrating   UMETA(DisplayName = "Migrating")
};

USTRUCT(BlueprintType)
struct FCrowd_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    float WaitTimeSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    ECrowd_BehaviorState BehaviorAtWaypoint = ECrowd_BehaviorState::Idle;
};

USTRUCT(BlueprintType)
struct FCrowd_GroupConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FName GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    ECrowd_GroupType GroupType = ECrowd_GroupType::HerbivoreHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 GroupSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeDistanceFromPlayer = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AggroDistanceFromPlayer = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SpreadRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    TArray<FCrowd_WaypointData> PatrolWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    ECrowd_BehaviorState CurrentState = ECrowd_BehaviorState::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bIsPlayerThreat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TerritoryRadius = 3000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdGroupConfig : public UObject
{
    GENERATED_BODY()

public:
    UCrowdGroupConfig();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FCrowd_GroupConfig Config;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetGroupState(ECrowd_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    ECrowd_BehaviorState GetGroupState() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    bool IsPlayerThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void AddWaypoint(FVector Location, float WaitTime, ECrowd_BehaviorState BehaviorAtPoint);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetWaypointCount() const;
};
