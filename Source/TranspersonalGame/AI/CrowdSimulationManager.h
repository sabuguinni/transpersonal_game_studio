#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd: tribal camps, migration columns, threat scatter

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Scout       UMETA(DisplayName = "Scout"),
    Migrant     UMETA(DisplayName = "Migrant"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead"),
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    TWeakObjectPtr<AActor> AgentActor;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentRole Role = ECrowd_AgentRole::Gatherer;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector FleeTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    TArray<FVector> MigrationWaypoints;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsAlive = true;
};

UCLASS(BlueprintType)
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
    void TriggerThreatResponse(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdTick(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetCrowdActive(bool bActive);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float ThreatScatterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CampRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MigrationSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    bool bCrowdActive;

private:
    UPROPERTY()
    TArray<FCrowd_AgentData> CrowdAgents;
};
