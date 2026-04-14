#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "CrowdSimulationManager.generated.h"

// Forward declarations
class UCrowdAgentComponent;
class ACrowdSpawnPoint;
class ACrowdWaypoint;

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle,
    Moving,
    Gathering,
    Fleeing,
    Following,
    Working
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    LOD0_HighDetail,
    LOD1_MediumDetail,
    LOD2_LowDetail,
    LOD3_Culled
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_AgentState State = ECrowd_AgentState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::LOD0_HighDetail;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Speed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SimulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    int32 MaxAgents = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float UpdateFrequency = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0_Distance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1_Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2_Distance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAgentsPerFrame = 1000;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core simulation functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StopSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PauseSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ResumeSimulation();

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Agents")
    int32 SpawnAgents(int32 Count, FVector SpawnLocation, float SpawnRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void RemoveAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void RemoveAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Agents")
    int32 GetActiveAgentCount() const;

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateAgentLOD(int32 AgentID, const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateAllAgentsLOD(const FVector& ViewerLocation);

    // Pathfinding and movement
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void SetAgentDestination(int32 AgentID, const FVector& Destination);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void SetGroupDestination(const TArray<int32>& AgentIDs, const FVector& Destination);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSimulationFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveAgentsInLOD(ECrowd_LODLevel LODLevel) const;

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetSimulationSettings(const FCrowd_SimulationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    FCrowd_SimulationSettings GetSimulationSettings() const;

protected:
    // Internal simulation functions
    void UpdateSimulation(float DeltaTime);
    void UpdateAgentMovement(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    ECrowd_LODLevel CalculateLODLevel(const FVector& AgentPosition, const FVector& ViewerPosition) const;

    // Mass Entity integration
    void InitializeMassEntity();
    void UpdateMassEntity(float DeltaTime);
    void CleanupMassEntity();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FCrowd_SimulationSettings SimulationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation", meta = (AllowPrivateAccess = "true"))
    bool bIsSimulationRunning = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Simulation", meta = (AllowPrivateAccess = "true"))
    bool bIsSimulationPaused = false;

    // Agent storage
    UPROPERTY()
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY()
    TArray<int32> FreeAgentIDs;

    // Performance tracking
    float LastFrameTime = 0.0f;
    float SimulationFPS = 60.0f;
    int32 NextAgentID = 1;

    // Update timing
    float LastUpdateTime = 0.0f;
    float UpdateInterval = 0.033f; // 30 FPS default
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    ACrowdSimulationManager* GetCrowdManager() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterCrowdManager(ACrowdSimulationManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UnregisterCrowdManager();

private:
    UPROPERTY()
    ACrowdSimulationManager* CrowdManager = nullptr;
};