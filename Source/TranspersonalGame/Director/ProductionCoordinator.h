#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Production      UMETA(DisplayName = "Production"),
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
    Gold            UMETA(DisplayName = "Gold Master"),
    PostLaunch      UMETA(DisplayName = "Post Launch")
};

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High            UMETA(DisplayName = "High"),
    Medium          UMETA(DisplayName = "Medium"),
    Low             UMETA(DisplayName = "Low"),
    Maintenance     UMETA(DisplayName = "Maintenance")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTaskData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsBlocking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime Deadline;

    FDir_AgentTaskData()
    {
        AgentID = 0;
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::Medium;
        EstimatedHours = 1.0f;
        bIsBlocking = false;
        Deadline = FDateTime::Now() + FTimespan::FromDays(1);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_BiomeProductionStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> RequiredAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> CompletedAssets;

    FDir_BiomeProductionStatus()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        CompletionPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production phase management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    // Agent task management
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AssignTaskToAgent(int32 AgentID, const FDir_AgentTaskData& TaskData);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void CompleteAgentTask(int32 AgentID, int32 TaskIndex);

    UFUNCTION(BlueprintPure, Category = "Agent Management")
    TArray<FDir_AgentTaskData> GetAgentTasks(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void BlockAgent(int32 AgentID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void UnblockAgent(int32 AgentID);

    // Biome production tracking
    UFUNCTION(BlueprintCallable, Category = "Biome Production")
    void UpdateBiomeStatus(const FString& BiomeName, const FDir_BiomeProductionStatus& Status);

    UFUNCTION(BlueprintPure, Category = "Biome Production")
    FDir_BiomeProductionStatus GetBiomeStatus(const FString& BiomeName) const;

    UFUNCTION(BlueprintPure, Category = "Biome Production")
    TArray<FDir_BiomeProductionStatus> GetAllBiomeStatuses() const;

    // Asset pipeline management
    UFUNCTION(BlueprintCallable, Category = "Asset Pipeline")
    void RegisterAssetRequest(const FString& AssetName, const FString& BiomeName, int32 RequestingAgent);

    UFUNCTION(BlueprintCallable, Category = "Asset Pipeline")
    void CompleteAssetRequest(const FString& AssetName, const FString& AssetPath);

    // Production metrics
    UFUNCTION(BlueprintPure, Category = "Metrics")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintPure, Category = "Metrics")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintPure, Category = "Metrics")
    int32 GetBlockedAgentCount() const;

    // Critical path analysis
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    TArray<int32> GetCriticalPathAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void RecalculateCriticalPath();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Agent Management")
    TMap<int32, TArray<FDir_AgentTaskData>> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Agent Management")
    TMap<int32, bool> BlockedAgents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Agent Management")
    TMap<int32, FString> BlockReasons;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Production")
    TMap<FString, FDir_BiomeProductionStatus> BiomeStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Pipeline")
    TMap<FString, FString> PendingAssetRequests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Pipeline")
    TMap<FString, FString> CompletedAssets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Critical Path")
    TArray<int32> CriticalPath;

private:
    void InitializeBiomeStatuses();
    void ValidateAgentDependencies();
    float CalculateBiomeProgress(const FDir_BiomeProductionStatus& Status) const;
};