#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Alert       UMETA(DisplayName = "Alert"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_StampedeState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentID;

    FCrowd_HerdAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , State(ECrowd_StampedeState::Grazing)
        , FearLevel(0.0f)
        , AgentID(-1)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float FearPropagationRate;

    FCrowd_StampedeConfig()
        : TriggerRadius(500.0f)
        , MaxSpeed(1200.0f)
        , SeparationWeight(1.5f)
        , AlignmentWeight(1.0f)
        , CohesionWeight(0.8f)
        , FearPropagationRate(0.3f)
    {}
};

/**
 * UCrowdStampedeController
 * Manages prehistoric herd stampede behavior using boid-based flocking simulation.
 * Supports up to 50,000 agents via Mass Entity integration.
 */
UCLASS(ClassGroup = "Crowd", BlueprintType, Blueprintable, meta = (DisplayName = "Crowd Stampede Controller"))
class TRANSPERSONALGAME_API ACrowdStampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdStampedeController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FCrowd_StampedeConfig StampedeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FVector HerdOrigin;

    // --- Runtime State ---
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_StampedeState CurrentHerdState;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    float AverageFearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    FVector StampedeDirection;

    // --- Agent Data ---
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agents")
    TArray<FCrowd_HerdAgent> HerdAgents;

    // --- Blueprint Events ---
    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd|Events")
    void OnStampedeTriggered(FVector TriggerLocation, float FearIntensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd|Events")
    void OnHerdCalmed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd|Events")
    void OnAgentSeparated(int32 AgentID, FVector SeparatedLocation);

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void TriggerStampede(FVector ThreatLocation, float FearIntensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void CalmHerd();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void SpawnHerd(int32 AgentCount, FVector Origin, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    float GetHerdFearLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    FVector GetStampedeDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    int32 GetAgentsInRadius(FVector Center, float Radius) const;

    UFUNCTION(CallInEditor, Category = "Crowd|Debug")
    void DebugDrawHerd();

private:
    void UpdateFlocking(float DeltaTime);
    void UpdateFearPropagation(float DeltaTime);
    void UpdateHerdState();
    FVector ComputeSeparation(int32 AgentIndex) const;
    FVector ComputeAlignment(int32 AgentIndex) const;
    FVector ComputeCohesion(int32 AgentIndex) const;
    FVector ComputeFearFlight(int32 AgentIndex) const;

    FVector ThreatPosition;
    bool bStampedeActive;
    float StampedeTimer;
    float CalmdownThreshold;
};
