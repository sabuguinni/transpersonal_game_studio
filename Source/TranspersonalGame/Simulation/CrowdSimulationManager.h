#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../Consciousness/ConsciousnessComponent.h"
#include "CrowdSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct FCrowdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Destination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialInfluence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    FCrowdAgent()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Destination = FVector::ZeroVector;
        ConsciousnessLevel = 1.0f;
        EmotionalState = 0.5f;
        SocialInfluence = 1.0f;
        AgentID = -1;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FTrafficNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> ConnectedNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrafficDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessInfluence;

    FTrafficNode()
    {
        Location = FVector::ZeroVector;
        TrafficDensity = 1.0f;
        ConsciousnessInfluence = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Crowd Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowdAgent> CrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float CrowdDensityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float ConsciousnessInfluenceRadius;

    // Traffic Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Simulation")
    TArray<FTrafficNode> TrafficNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Simulation")
    float TrafficFlowRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Simulation")
    float ConsciousnessTrafficModifier;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float GlobalConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessSpreadRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float EmotionalContagionStrength;

public:
    // Crowd Simulation Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgent(FVector Location, FVector Destination);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RemoveCrowdAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FVector CalculateFlockingForce(const FCrowdAgent& Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FVector CalculateAvoidanceForce(const FCrowdAgent& Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FVector CalculateSeekForce(const FCrowdAgent& Agent);

    // Traffic Simulation Functions
    UFUNCTION(BlueprintCallable, Category = "Traffic Simulation")
    void InitializeTrafficNetwork();

    UFUNCTION(BlueprintCallable, Category = "Traffic Simulation")
    void UpdateTrafficFlow(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Traffic Simulation")
    TArray<int32> FindOptimalPath(int32 StartNode, int32 EndNode);

    UFUNCTION(BlueprintCallable, Category = "Traffic Simulation")
    void ModifyTrafficByConsciousness(float ConsciousnessLevel);

    // Consciousness Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateConsciousnessInfluence(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void PropagateEmotionalState(const FCrowdAgent& SourceAgent);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    float CalculateLocalConsciousnessLevel(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void OnPlayerConsciousnessChanged(float NewLevel);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Simulation")
    void SetSimulationParameters(int32 NewMaxAgents, float NewDensityRadius);

    UFUNCTION(BlueprintCallable, Category = "Simulation")
    TArray<FCrowdAgent> GetNearbyAgents(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Simulation")
    void ClearAllAgents();

private:
    // Internal simulation functions
    void UpdateAgentMovement(FCrowdAgent& Agent, float DeltaTime);
    void HandleAgentCollisions();
    void UpdateAgentDestinations();
    FVector GetRandomDestination();
    bool IsLocationValid(FVector Location);
    void OptimizePerformance();

    // Consciousness-driven behavior modifiers
    float GetConsciousnessMovementModifier(float ConsciousnessLevel);
    float GetEmotionalInfluenceStrength(float EmotionalState);
    FVector GetConsciousnessDirectionalBias(FVector Location);
};