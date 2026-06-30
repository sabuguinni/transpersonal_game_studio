#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// Crowd & Traffic Simulation — Agent #13
// Manages herd/pack behavior for prehistoric creatures and
// primitive human groups using lightweight agent simulation.
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    HumanPrimitive      UMETA(DisplayName = "Human Primitive"),
    DinosaurHerbivore   UMETA(DisplayName = "Dinosaur Herbivore"),
    DinosaurCarnivore   UMETA(DisplayName = "Dinosaur Carnivore"),
    DinosaurPack        UMETA(DisplayName = "Dinosaur Pack"),
    MigrationHerd       UMETA(DisplayName = "Migration Herd"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Resting     UMETA(DisplayName = "Resting"),
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentID;

    FCrowd_AgentData()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , AgentType(ECrowd_AgentType::DinosaurHerbivore)
        , State(ECrowd_AgentState::Grazing)
        , Speed(300.0f)
        , FearLevel(0.0f)
        , AgentID(-1)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float StampedeThreshold;

    FCrowd_HerdConfig()
        : HerdSize(20)
        , CohesionRadius(800.0f)
        , SeparationRadius(150.0f)
        , AlignmentRadius(400.0f)
        , FleeThreshold(0.4f)
        , StampedeThreshold(0.8f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Herd Management ----

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnHerd(ECrowd_AgentType AgentType, FVector CenterLocation, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerStampede(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFlee(FVector ThreatLocation, float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateAgentStates(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ClearAllAgents();

    // ---- Flocking (Boids) ----

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector ComputeCohesion(int32 AgentIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector ComputeSeparation(int32 AgentIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector ComputeAlignment(int32 AgentIndex) const;

    // ---- LOD ----

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetLODLevel(int32 Level);

    // ---- Config ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FCrowd_HerdConfig HerdConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SimulationTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bEnableFlocking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bEnableStampede;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceFar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceMid;

private:
    UPROPERTY()
    TArray<FCrowd_AgentData> Agents;

    UPROPERTY()
    int32 NextAgentID;

    UPROPERTY()
    int32 CurrentLODLevel;

    float SimAccumulator;

    void TickAgent(FCrowd_AgentData& Agent, float DeltaTime);
    FVector ClampVelocity(const FVector& Vel, float MaxSpeed) const;
    void SpawnAgentMesh(const FCrowd_AgentData& Agent);
};
