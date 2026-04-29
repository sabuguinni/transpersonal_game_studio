#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "../SharedTypes.h"
#include "CrowdSimulationManager.generated.h"

// Forward declarations
class ACrowdAgent;
class UCrowdBehaviorComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Destination = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Speed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float SeparationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float AlignmentRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CohesionRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 GroupID = 0;

    FCrowd_AgentData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Destination = FVector::ZeroVector;
        Speed = 150.0f;
        MaxSpeed = 300.0f;
        SeparationRadius = 100.0f;
        AlignmentRadius = 200.0f;
        CohesionRadius = 300.0f;
        bIsActive = true;
        GroupID = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AvoidanceWeight = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeekWeight = 2.0f;

    FCrowd_FlockingParameters()
    {
        SeparationWeight = 2.0f;
        AlignmentWeight = 1.0f;
        CohesionWeight = 1.0f;
        AvoidanceWeight = 3.0f;
        SeekWeight = 2.0f;
    }
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Flocking    UMETA(DisplayName = "Flocking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Following   UMETA(DisplayName = "Following"),
    Hunting     UMETA(DisplayName = "Hunting")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 SpawnCrowdAgent(const FVector& Location, int32 GroupID = 0);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RemoveCrowdAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetAgentDestination(int32 AgentID, const FVector& Destination);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGroupDestination(int32 GroupID, const FVector& Destination);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetFlockingParameters(const FCrowd_FlockingParameters& NewParameters);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FCrowd_AgentData> GetAllAgentData() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveAgentCount() const;

    // Behavior management
    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetAgentBehavior(int32 AgentID, ECrowd_BehaviorState NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetGroupBehavior(int32 GroupID, ECrowd_BehaviorState NewBehavior);

    // Dinosaur-specific crowd behaviors
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void CreateDinosaurHerd(const FVector& CenterLocation, int32 HerdSize = 5, float HerdRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void CreateRaptorPack(const FVector& CenterLocation, int32 PackSize = 3);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void TriggerHerdPanic(const FVector& ThreatLocation, float PanicRadius = 1000.0f);

protected:
    // Agent data storage
    UPROPERTY()
    TArray<FCrowd_AgentData> CrowdAgents;

    UPROPERTY()
    TMap<int32, AActor*> AgentActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    FCrowd_FlockingParameters FlockingParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float CullingDistance = 5000.0f;

    // Internal state
    int32 NextAgentID;
    float LastUpdateTime;
    bool bIsInitialized;

    // Core flocking algorithms
    FVector CalculateSeparation(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors);
    FVector CalculateAlignment(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors);
    FVector CalculateCohesion(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors);
    FVector CalculateSeek(const FCrowd_AgentData& Agent, const FVector& Target);
    FVector CalculateAvoidance(const FCrowd_AgentData& Agent);

    // Utility functions
    TArray<FCrowd_AgentData> GetNeighbors(const FCrowd_AgentData& Agent, float Radius);
    void UpdateAgentPosition(FCrowd_AgentData& Agent, float DeltaTime);
    bool IsValidLocation(const FVector& Location);
    AActor* SpawnAgentActor(const FVector& Location);
};