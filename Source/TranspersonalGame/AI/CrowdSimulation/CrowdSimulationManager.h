#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribe crowd simulation system

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Guard       UMETA(DisplayName = "Guard"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Working     UMETA(DisplayName = "Working"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    TWeakObjectPtr<AActor> AgentActor;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_AgentRole Role = ECrowd_AgentRole::Gatherer;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 LODLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float WanderRadius = 400.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterCrowdAgent(AActor* Agent, ECrowd_AgentRole Role);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterCrowdAgent(AActor* Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentData> GetAgentsByRole(ECrowd_AgentRole Role) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerCrowdFlee(const FVector& ThreatLocation, float FleeRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerCrowdAlert(const FVector& AlertLocation, float AlertRadius);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentUpdateRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceClose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceMedium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceFar;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    bool bCrowdSimulationActive;

private:
    UPROPERTY()
    TArray<FCrowd_AgentData> ActiveAgents;
};
