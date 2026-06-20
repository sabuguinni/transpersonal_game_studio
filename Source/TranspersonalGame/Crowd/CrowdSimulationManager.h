// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribe crowd simulation — Mass AI primitive layer

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — ECrowd_ prefix (unique across project)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Sentinel    UMETA(DisplayName = "Sentinel"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child")
};

UENUM(BlueprintType)
enum class ECrowd_AgentLOD : uint8
{
    Full        UMETA(DisplayName = "Full Simulation"),
    Reduced     UMETA(DisplayName = "Reduced Simulation"),
    Minimal     UMETA(DisplayName = "Minimal / Frozen")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatType : uint8
{
    None            UMETA(DisplayName = "None"),
    Predator        UMETA(DisplayName = "Predator Dinosaur"),
    Fire            UMETA(DisplayName = "Fire"),
    StrangerHuman   UMETA(DisplayName = "Stranger Human"),
    Earthquake      UMETA(DisplayName = "Earthquake")
};

// ============================================================
// STRUCTS — FCrowd_ prefix
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::Gatherer;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentLOD CurrentLOD = ECrowd_AgentLOD::Full;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_ThreatType CurrentThreat = ECrowd_ThreatType::None;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 120.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float WanderRadius = 400.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FacingAngle = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bAlerted = false;
};

// ============================================================
// UCrowdSimulationManager — World Subsystem
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Tick (manual — called from GameMode or tick delegate)
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void Tick(float DeltaTime);

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterAgent(UPARAM(ref) FCrowd_AgentData& AgentData);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterAgent(int32 AgentID);

    // Threat system
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void AlertNearbyAgents(const FVector& ThreatLocation, float AlertRadius, ECrowd_ThreatType ThreatType);

    // Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentData> GetAgentsInRadius(const FVector& Center, float Radius) const;

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentUpdateRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceClose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceMid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval;

private:
    UPROPERTY()
    TArray<FCrowd_AgentData> ActiveAgents;

    bool bSimulationActive;
    int32 NextAgentID = 0;
    float CurrentTickAccumulator;

    void UpdateAgentLOD(FCrowd_AgentData& Agent, const FVector& PlayerLocation);
    void UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void SimulateGathererBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void SimulateHunterBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void SimulateSentinelBehavior(FCrowd_AgentData& Agent, float DeltaTime);
};
