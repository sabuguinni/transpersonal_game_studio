// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Manages prehistoric herd and aerial flock crowd agents

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrowdSimulationManager.generated.h"

// ── Enums (global scope — RULE 1) ──────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_HerdType : uint8
{
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Gallimimus          UMETA(DisplayName = "Gallimimus"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Drinking    UMETA(DisplayName = "Drinking"),
};

// ── Structs (global scope — RULE 1) ────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_HerdAgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    AActor* AgentActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdType HerdType = ECrowd_HerdType::Parasaurolophus;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState AgentState = ECrowd_AgentState::Grazing;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsFleeing = false;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FleeTimer = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_FlockAgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    AActor* AgentActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState AgentState = ECrowd_AgentState::Patrolling;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CircleCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float CircleRadius = 400.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float CircleAngle = 0.0f;
};

// ── Class ──────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UObject
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // Initialization
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void InitializeSimulation(UWorld* InWorld);

    // Agent registration
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterHerdAgent(AActor* Agent, ECrowd_HerdType HerdType);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterFlockAgent(AActor* Agent);

    // Tick — call from GameMode or subsystem
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TickSimulation(float DeltaTime);

    // Events
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius);

    // Stats
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void GetCrowdStats(int32& OutHerdCount, int32& OutFlockCount, bool& OutSimActive) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void StopSimulation();

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxFlockAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdSeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockAltitudeMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockAltitudeMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentMoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SimulationTickRate;

private:
    void TickHerdBehavior(float DeltaTime);
    void TickFlockBehavior(float DeltaTime);

    UPROPERTY()
    TArray<FCrowd_HerdAgentData> HerdAgents;

    UPROPERTY()
    TArray<FCrowd_FlockAgentData> FlockAgents;

    UPROPERTY()
    UWorld* WorldRef = nullptr;

    bool bSimulationActive;
    float AccumulatedTime;
};
