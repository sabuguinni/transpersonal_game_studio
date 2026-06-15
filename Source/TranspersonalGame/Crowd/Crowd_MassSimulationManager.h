#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Crowd_MassSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GroupID;

    FCrowd_AgentData()
    {
        Location = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        AgentType = ECrowd_AgentType::Hunter;
        Health = 100.0f;
        Stamina = 100.0f;
        GroupID = -1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHighDetailAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxMediumDetailAgents;

    FCrowd_LODSettings()
    {
        HighDetailDistance = 500.0f;
        MediumDetailDistance = 1500.0f;
        LowDetailDistance = 3000.0f;
        MaxHighDetailAgents = 50;
        MaxMediumDetailAgents = 200;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    FCrowd_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    int32 MaxActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float CullingDistance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Simulation")
    int32 CurrentAgentCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Simulation")
    float LastUpdateTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SpawnAgentGroup(FVector Location, ECrowd_AgentType AgentType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void UpdateAgentLOD(FCrowd_AgentData& Agent, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void ProcessAgentMovement(FCrowd_AgentData& Agent, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void CullDistantAgents();

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    TArray<FCrowd_AgentData> GetNearbyAgents(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SetMaxActiveAgents(int32 NewMax);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    int32 GetActiveAgentCount() const;

private:
    void InitializeSimulation();
    void UpdateSimulation(float DeltaTime);
    void OptimizePerformance();
    
    FVector GetPlayerLocation() const;
    bool ShouldUpdateAgent(const FCrowd_AgentData& Agent, float DeltaTime) const;
    void ApplyFlockingBehavior(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents);
};