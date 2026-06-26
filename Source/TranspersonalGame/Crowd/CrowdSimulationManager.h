#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// Crowd Simulation Manager — Agent #13
// Manages prehistoric creature crowd groups, patrol routes,
// herd dynamics and density simulation for MinPlayableMap.
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_GroupType : uint8
{
    ApexPredator    UMETA(DisplayName = "Apex Predator"),
    PackPredator    UMETA(DisplayName = "Pack Predator"),
    HerbivoreHerd   UMETA(DisplayName = "Herbivore Herd"),
    MegaHerbivore   UMETA(DisplayName = "Mega Herbivore"),
    Scavenger       UMETA(DisplayName = "Scavenger"),
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
};

USTRUCT(BlueprintType)
struct FCrowd_GroupDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_GroupType GroupType = ECrowd_GroupType::HerbivoreHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorState CurrentBehavior = ECrowd_BehaviorState::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AlertRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FVector> WaypointPositions;
};

USTRUCT(BlueprintType)
struct FCrowd_SimulationStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 TotalGroupsActive = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 TotalAgentsSimulated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 AlertedGroups = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float SimulationTickRate = 0.25f;
};

UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Group registration
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterGroup(const FCrowd_GroupDefinition& GroupDef);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UnregisterGroup(const FString& GroupID);

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    FCrowd_GroupDefinition GetGroupByID(const FString& GroupID) const;

    // Behavior control
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGroupBehavior(const FString& GroupID, ECrowd_BehaviorState NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void AlertGroupsNearLocation(FVector Location, float AlertRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerFleeResponse(const FString& GroupID, FVector ThreatLocation);

    // Stats
    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    FCrowd_SimulationStats GetSimulationStats() const;

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetActiveGroupCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    TArray<FString> GetAllGroupIDs() const;

protected:
    UPROPERTY()
    TMap<FString, FCrowd_GroupDefinition> ActiveGroups;

    UPROPERTY()
    FCrowd_SimulationStats SimStats;

    FTimerHandle SimulationTickHandle;

    void SimulationTick();
    void UpdateGroupBehavior(FCrowd_GroupDefinition& Group, float DeltaTime);
    void RegisterDefaultGroups();

    FVector GetNextWaypoint(const FCrowd_GroupDefinition& Group) const;
    bool IsPlayerNearGroup(const FCrowd_GroupDefinition& Group, float Radius) const;
};
