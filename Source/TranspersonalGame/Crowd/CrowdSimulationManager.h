// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd: tribe members, dinosaur herds, LOD crowd agents

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "CrowdSimulationManager.generated.h"

// --- Enums (global scope, Crowd_ prefix) ---

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    TribeHunter     UMETA(DisplayName = "Tribe Hunter"),
    TribeGatherer   UMETA(DisplayName = "Tribe Gatherer"),
    TribeElder      UMETA(DisplayName = "Tribe Elder"),
    TribeChild      UMETA(DisplayName = "Tribe Child"),
    TribeWarrior    UMETA(DisplayName = "Tribe Warrior"),
    DinosaurHerd    UMETA(DisplayName = "Dinosaur Herd Member"),
    DinosaurSolitary UMETA(DisplayName = "Dinosaur Solitary"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Returning   UMETA(DisplayName = "Returning"),
    Interacting UMETA(DisplayName = "Interacting"),
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full    UMETA(DisplayName = "Full Detail"),
    Medium  UMETA(DisplayName = "Medium Detail"),
    Distant UMETA(DisplayName = "Distant Silhouette"),
    Culled  UMETA(DisplayName = "Culled"),
};

// --- Structs (global scope, FCrowd_ prefix) ---

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::TribeHunter;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::Full;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float CurrentStamina = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MaxStamina = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FearLevel = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector HerdCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float HerdRadius = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AlphaAgentID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    TArray<int32> MemberIDs;
};

// --- Main Subsystem ---

UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // UWorldSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterAgent(UPARAM(ref) FCrowd_AgentData& AgentData);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentData> GetAgentsByType(ECrowd_AgentType AgentType) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetSimulationActive(bool bActive);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TribeSettlementRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdSpreadRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceNear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceFar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval;

private:
    UPROPERTY()
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY()
    TArray<FCrowd_HerdData> ActiveHerds;

    bool bSimulationActive;
    int32 NextAgentID = 0;
    float TimeSinceLastTick;

    void UpdateAgentBehaviors(float DeltaTime);
    void UpdateHerdFormations(float DeltaTime);
    void UpdateLODLevels();
    void UpdateHunterBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateGathererBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateHerdMemberBehavior(FCrowd_AgentData& Agent, float DeltaTime);
};
