#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Director_CriticalPathManager.generated.h"

UENUM(BlueprintType)
enum class EDir_CriticalPathPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical - Blocks Release"),
    High        UMETA(DisplayName = "High - Affects Gameplay"),
    Medium      UMETA(DisplayName = "Medium - Quality Enhancement"),
    Low         UMETA(DisplayName = "Low - Polish")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Complete    UMETA(DisplayName = "Complete"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_CriticalPathTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_CriticalPathPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString DeliverablePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsPlayable;

    FDir_CriticalPathTask()
    {
        TaskName = TEXT("");
        AgentID = 0;
        Priority = EDir_CriticalPathPriority::Medium;
        Status = EDir_AgentStatus::Idle;
        EstimatedCycles = 1.0f;
        bIsPlayable = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeDistributionTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TargetActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 CurrentActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DistributionPercentage;

    FDir_BiomeDistributionTarget()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        TargetActorCount = 0;
        CurrentActorCount = 0;
        DistributionPercentage = 20.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_CriticalPathManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_CriticalPathManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Critical Path Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    TArray<FDir_CriticalPathTask> CriticalPathTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    float PlayablePrototypeProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bMinimalPlayableReached;

    // Biome Distribution Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    TArray<FDir_BiomeDistributionTarget> BiomeTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    bool bEnforceBiomeDistribution;

    // Agent Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TMap<int32, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<int32> BlockedAgents;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void InitializeCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void UpdateTaskStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    bool CanAgentProceed(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    float CalculatePlayableProgress();

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    void InitializeBiomeTargets();

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    FVector GetNextSpawnLocation(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    void UpdateBiomeActorCount(const FString& BiomeName, int32 Delta);

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    bool IsBiomeDistributionBalanced();

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void BlockAgent(int32 AgentID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void UnblockAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<int32> GetReadyAgents();

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCriticalPathStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomeDistribution();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidateProductionPipeline();

private:
    void SetupDefaultCriticalPath();
    void SetupDefaultBiomeTargets();
    bool CheckTaskDependencies(const FDir_CriticalPathTask& Task);
    void UpdatePlayableProgress();
};