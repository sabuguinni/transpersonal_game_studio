#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    Hunter    UMETA(DisplayName = "Hunter"),
    Gatherer  UMETA(DisplayName = "Gatherer"),
    Scout     UMETA(DisplayName = "Scout"),
    Elder     UMETA(DisplayName = "Elder"),
    Child     UMETA(DisplayName = "Child"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle      UMETA(DisplayName = "Idle"),
    Wandering UMETA(DisplayName = "Wandering"),
    Fleeing   UMETA(DisplayName = "Fleeing"),
    Gathering UMETA(DisplayName = "Gathering"),
    Hunting   UMETA(DisplayName = "Hunting"),
    Resting   UMETA(DisplayName = "Resting"),
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Energy;

    FCrowd_AgentData()
        : Location(FVector::ZeroVector)
        , Role(ECrowd_AgentRole::Hunter)
        , State(ECrowd_AgentState::Idle)
        , Fear(0.0f)
        , Energy(1.0f)
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float WanderRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveAgentCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_AgentData> AgentPool;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnCrowdAgents(int32 Count, FVector CenterLocation, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeEvent(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateAgentStates(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetAgentRole(int32 AgentIndex, ECrowd_AgentRole NewRole);

private:
    float TimeSinceLastUpdate;

    void UpdateSingleAgent(FCrowd_AgentData& Agent, float DeltaTime);
    FVector GetWanderTarget(const FCrowd_AgentData& Agent) const;
    FVector GetFleeDirection(const FCrowd_AgentData& Agent, FVector ThreatLocation) const;
};
