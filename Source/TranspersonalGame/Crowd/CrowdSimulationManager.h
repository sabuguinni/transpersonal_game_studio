// CrowdSimulationManager.h — Agent #13 Crowd & Traffic Simulation
// Transpersonal Game Studio — Prehistoric Survival Game
// Mass AI crowd simulation: up to 50,000 agents with LOD, stampede, flocking

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    DinosaurHerbivore   UMETA(DisplayName = "Dinosaur Herbivore"),
    DinosaurCarnivore   UMETA(DisplayName = "Dinosaur Carnivore"),
    HumanTribal         UMETA(DisplayName = "Human Tribal"),
    SmallAnimal         UMETA(DisplayName = "Small Animal"),
    Bird                UMETA(DisplayName = "Bird")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full    UMETA(DisplayName = "Full Simulation"),
    Medium  UMETA(DisplayName = "Medium LOD"),
    Low     UMETA(DisplayName = "Low LOD"),
    Culled  UMETA(DisplayName = "Culled")
};

// ============================================================
// STRUCTS — must be at global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::DinosaurHerbivore;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float CurrentSpeed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float PanicLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsLeader = false;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::Full;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;
};

// ============================================================
// DELEGATE
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrowd_OnStampedeTriggered, FVector, TriggerLocation, int32, AffectedAgentCount);

// ============================================================
// UCLASS
// ============================================================

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent), DisplayName = "Crowd Simulation Manager")
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceClose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceMedium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceFar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Debug")
    bool bDebugDrawEnabled;

    // ---- Runtime State ----

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 CurrentAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agents")
    TArray<FCrowd_AgentData> ActiveAgents;

    // ---- Delegates ----

    UPROPERTY(BlueprintAssignable, Category = "Crowd|Events")
    FCrowd_OnStampedeTriggered OnStampedeTriggered;

    // ---- Public API ----

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void InitializeCrowdSystem(int32 InitialAgentCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnCrowdAgents(int32 Count, FVector CenterLocation, ECrowd_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerStampede(FVector TriggerLocation, float PanicRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetSimulationActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ClearAllAgents();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdateAgentLOD();
    void UpdateHerdBehavior(float DeltaTime);
    void DrawDebugCrowd();
    float GetMaxSpeedForType(ECrowd_AgentType AgentType) const;
};
