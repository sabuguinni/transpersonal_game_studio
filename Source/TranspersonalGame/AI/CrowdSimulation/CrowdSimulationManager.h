#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdSimulationManager.generated.h"

// ECrowd_HerdBehaviour — state machine for individual herd agents
UENUM(BlueprintType)
enum class ECrowd_HerdBehaviour : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Returning   UMETA(DisplayName = "Returning"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Resting     UMETA(DisplayName = "Resting")
};

// FCrowd_AgentState — per-agent runtime state
USTRUCT(BlueprintType)
struct FCrowd_AgentState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    AActor* AgentActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdBehaviour CurrentBehaviour = ECrowd_HerdBehaviour::Grazing;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float AlertLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsLeader = false;
};

/**
 * UCrowdSimulationManager
 * Manages prehistoric herd behaviour for up to 50 agents.
 * Supports grazing clusters, migration lines, watering groups, and flee propagation.
 * Agent #13 — Crowd & Traffic Simulation
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdUpdateRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bCrowdSystemActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float WalkSpeed = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeSpeed = 480.0f;

    // --- Runtime State ---

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_AgentState> ManagedAgents;

    // --- Public API ---

    /** Trigger a flee response in all agents within AlertRadius of ThreatLocation */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerHerdFlee(FVector ThreatLocation, float AlertRadius = 2000.0f);

    /** Returns number of currently active (non-null) agents */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

private:
    void UpdateAgentBehaviour(FCrowd_AgentState& Agent, float DeltaTime);
    void PropagateAlert(const FCrowd_AgentState& SourceAgent, FVector ThreatLocation, float PropagationRadius);
};
