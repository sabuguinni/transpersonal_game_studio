#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    ECrowd_BehaviorState BehaviorState;

    FCrowd_AgentData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        MovementSpeed = 150.0f;
        AgentID = -1;
        BehaviorState = ECrowd_BehaviorState::Wandering;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SeparationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float FollowDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    FVector SpawnCenter;

    FCrowd_SpawnParameters()
    {
        MaxAgents = 50;
        SpawnRadius = 200.0f;
        SeparationDistance = 100.0f;
        FollowDistance = 300.0f;
        SpawnCenter = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Crowd management functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(const FCrowd_SpawnParameters& SpawnParams);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FCrowd_AgentData> GetAllAgentData() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdParameters(const FCrowd_SpawnParameters& NewParams);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    FCrowd_SpawnParameters CurrentParameters;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    float LastUpdateTime;

private:
    void UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void ApplyFlockingBehavior(FCrowd_AgentData& Agent);
    void ApplyAvoidanceBehavior(FCrowd_AgentData& Agent);
    FVector CalculateSeparation(const FCrowd_AgentData& Agent);
    FVector CalculateAlignment(const FCrowd_AgentData& Agent);
    FVector CalculateCohesion(const FCrowd_AgentData& Agent);
    bool IsValidSpawnLocation(const FVector& Location);
};