// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribal crowd simulation subsystem

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Working     UMETA(DisplayName = "Working"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Guard       UMETA(DisplayName = "Guard"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child")
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FName AgentID;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState CurrentState;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentRole Role;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector WanderTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float ThreatDistance;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed;

    FCrowd_AgentData()
        : AgentID(NAME_None)
        , CurrentState(ECrowd_AgentState::Idle)
        , Role(ECrowd_AgentRole::Gatherer)
        , CurrentLocation(FVector::ZeroVector)
        , WanderTarget(FVector::ZeroVector)
        , ThreatDistance(0.0f)
        , MoveSpeed(150.0f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_CampData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CenterLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Radius;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 PopulationCount;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bUnderThreat;

    FCrowd_CampData()
        : CenterLocation(FVector(900.0f, 100.0f, 50.0f))
        , Radius(600.0f)
        , PopulationCount(0)
        , bUnderThreat(false)
    {}
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent registration
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterAgent(const FCrowd_AgentData& AgentData);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterAgent(const FName& AgentID);

    // Crowd events
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerGatheringEvent(const FVector& GatherLocation);

    // Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    ECrowd_AgentState GetAgentState(const FName& AgentID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    FVector GetRandomCampPosition() const;

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentUpdateRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FVector CampCenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CampRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PatrolRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bCrowdActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_AgentData> RegisteredAgents;

private:
    FTimerHandle CrowdTickHandle;

    void TickCrowdAgents();
    void UpdateAgentBehaviorStates();
};
