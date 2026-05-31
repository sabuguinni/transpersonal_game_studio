#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Completed UMETA(DisplayName = "Completed"),
    Blocked UMETA(DisplayName = "Blocked"),
    Failed UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_BiomeType : uint8
{
    Savana UMETA(DisplayName = "Savana"),
    Pantano UMETA(DisplayName = "Pantano"),
    Floresta UMETA(DisplayName = "Floresta"),
    Deserto UMETA(DisplayName = "Deserto"),
    Montanha UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct FDir_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EDir_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    FDir_BiomeData()
    {
        BiomeType = EDir_BiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        ActorCount = 0;
        MaxActors = 4000;
        Radius = 20000.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Progress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString LastOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedTimeRemaining;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        Progress = 0.0f;
        LastOutput = TEXT("");
        EstimatedTimeRemaining = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 EnvironmentActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 LightingActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float MemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 PendingTasks;

    FDir_ProductionMetrics()
    {
        TotalActorsInWorld = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        LightingActors = 0;
        FrameRate = 60.0f;
        MemoryUsage = 0.0f;
        CompletedTasks = 0;
        PendingTasks = 0;
    }
};

/**
 * Production Coordinator - Studio Director's central command system
 * Manages the 19-agent pipeline, biome distribution, and production metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Production Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_BiomeData> BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bProductionActive;

    // Agent Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentTask GetAgentTask(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetActiveAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsAgentBlocked(int32 AgentID);

    // Biome Management Functions
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FVector GetBiomeSpawnLocation(EDir_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool CanSpawnInBiome(EDir_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void UpdateBiomeActorCount(EDir_BiomeType BiomeType, int32 NewCount);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EDir_BiomeType GetLeastPopulatedBiome();

    // Production Metrics Functions
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    bool IsActorLimitExceeded();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void CleanupExcessActors();

    // Critical Path Management
    UFUNCTION(BlueprintCallable, Category = "CriticalPath")
    void StartNewCycle();

    UFUNCTION(BlueprintCallable, Category = "CriticalPath")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "CriticalPath")
    float GetCycleProgress();

    UFUNCTION(BlueprintCallable, Category = "CriticalPath")
    TArray<int32> GetBlockedAgents();

    // Emergency Functions
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void EmergencyStop();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ForceAgentReset(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ClearAllBiomes();

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateWorldState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void GenerateProductionReport();

private:
    // Internal helper functions
    void ValidateBiomeDistribution();
    void CheckAgentDependencies();
    void UpdateCycleTimer();
    FString GetAgentNameByID(int32 AgentID);
    void LogBiomeStatus();
};