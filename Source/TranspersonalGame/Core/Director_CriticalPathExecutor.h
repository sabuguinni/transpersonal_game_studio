#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Director_CriticalPathExecutor.generated.h"

USTRUCT(BlueprintType)
struct FDir_BiomeSpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActorsPerBiome;

    FDir_BiomeSpawnConfig()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        SpawnRadius = 15000.0f;
        MaxActorsPerBiome = 100;
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentExecutionStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bHasExecutedThisCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CycleCompletionTime;

    FDir_AgentExecutionStatus()
    {
        AgentID = 0;
        AgentName = TEXT("Unknown");
        bHasExecutedThisCycle = false;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        CycleCompletionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_CriticalPathMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString BlockingReason;

    FDir_CriticalPathMilestone()
    {
        MilestoneName = TEXT("Unknown");
        bIsCompleted = false;
        Priority = 1.0f;
        BlockingReason = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_CriticalPathExecutor : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_CriticalPathExecutor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FDir_BiomeSpawnConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentExecutionStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<FDir_CriticalPathMilestone> CriticalMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleBudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float MaxCycleBudget;

public:
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void InitializeBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    FVector GetRandomLocationInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    bool ValidateAgentExecution(int32 AgentID, int32 MinFiles, int32 MinUE5Commands);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void UpdateAgentStatus(int32 AgentID, int32 FilesCreated, int32 CommandsExecuted);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    bool IsMilestoneBlocked(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    TArray<int32> GetBlockingAgents();

    UFUNCTION(BlueprintCallable, Category = "Critical Path", CallInEditor = true)
    void ExecuteCriticalPathValidation();

    UFUNCTION(BlueprintCallable, Category = "Critical Path", CallInEditor = true)
    void GenerateProductionReport();

private:
    void SetupDefaultBiomes();
    void SetupDefaultAgents();
    void SetupCriticalMilestones();
    bool ValidateBiomeDistribution();
    void LogCriticalPathStatus();
};