// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival game — Mass AI crowd simulation

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — all prefixed Crowd_ to avoid global namespace clash
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_ActivityZone : uint8
{
    Camp         UMETA(DisplayName = "Camp"),
    Foraging     UMETA(DisplayName = "Foraging"),
    WaterSource  UMETA(DisplayName = "Water Source"),
    Hunting      UMETA(DisplayName = "Hunting"),
    Shelter      UMETA(DisplayName = "Shelter"),
    Unknown      UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    CampWorker   UMETA(DisplayName = "Camp Worker"),
    Sentinel     UMETA(DisplayName = "Sentinel"),
    Forager      UMETA(DisplayName = "Forager"),
    WaterGatherer UMETA(DisplayName = "Water Gatherer"),
    Hunter       UMETA(DisplayName = "Hunter"),
    Elder        UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Moving       UMETA(DisplayName = "Moving"),
    Working      UMETA(DisplayName = "Working"),
    Fleeing      UMETA(DisplayName = "Fleeing"),
    Resting      UMETA(DisplayName = "Resting"),
    Dead         UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Near         UMETA(DisplayName = "Near — Full Simulation"),
    Mid          UMETA(DisplayName = "Mid — Reduced Tick"),
    Far          UMETA(DisplayName = "Far — Position Only"),
    Culled       UMETA(DisplayName = "Culled — Inactive")
};

// ============================================================
// STRUCTS — global scope, prefixed FCrowd_
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FGuid AgentID;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentRole Role = ECrowd_AgentRole::CampWorker;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::Near;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_ActivityZone CurrentZone = ECrowd_ActivityZone::Camp;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float StaminaLevel = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsAlive = true;
};

USTRUCT(BlueprintType)
struct FCrowd_ActivityZoneData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_ActivityZone ZoneType = ECrowd_ActivityZone::Camp;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Radius = 300.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 MaxOccupants = 10;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 CurrentOccupants = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;
};

// ============================================================
// UCrowdSimulationManager — main manager UObject
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UObject
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // --- Lifecycle ---
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeSimulation(UWorld* InWorld);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ShutdownSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TickSimulation(float DeltaTime);

    // --- Zone Management ---
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterActivityZone(ECrowd_ActivityZone ZoneType, FVector Location, float Radius, int32 MaxOccupants);

    // --- Agent Management ---
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgent(ECrowd_AgentRole Role, FVector SpawnLocation, ECrowd_ActivityZone InitialZone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerDangerAlert(FVector DangerSource, float AlertRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateAgentLOD(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Simulation")
    int32 GetAgentCountByRole(ECrowd_AgentRole Role) const;

    // --- Properties ---
    UPROPERTY(BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Simulation")
    int32 CurrentAgentCount;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistanceNear;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistanceMid;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistanceFar;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd Simulation")
    float DangerAlertRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Simulation")
    bool bSimulationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Simulation")
    bool bDangerStateActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Simulation")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Simulation")
    TArray<FCrowd_ActivityZoneData> ActivityZones;

private:
    UPROPERTY()
    UWorld* WorldRef = nullptr;
};
