#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Failed,
    Blocked
};

UENUM(BlueprintType)
enum class EDir_SystemPriority : uint8
{
    Critical,
    High,
    Medium,
    Low
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    int32 AgentID;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    float ProgressPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentInfo()
    {
        AgentName = TEXT("");
        AgentID = 0;
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        ProgressPercent = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CharacterActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 EnvironmentActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB;

    FDir_SystemMetrics()
    {
        TotalActors = 0;
        CharacterActors = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterAgent(int32 AgentID, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus Status, const FString& Task, float Progress);

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    FDir_AgentInfo GetAgentInfo(int32 AgentID) const;

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    TArray<FDir_AgentInfo> GetAllAgents() const;

    // System Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateSystemMetrics();

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    FDir_SystemMetrics GetSystemMetrics() const;

    // Task Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void QueueTaskForAgent(int32 AgentID, const FString& TaskDescription, EDir_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(int32 AgentID, bool bSuccess);

    // Development Pipeline
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateGameplayReadiness();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void RunSystemDiagnostics();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TMap<int32, FDir_AgentInfo> RegisteredAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDir_SystemMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    bool bSystemInitialized;

private:
    void InitializeAgentChain();
    void ValidateWorldState();
    void CheckCriticalSystems();
};

#include "StudioDirectorSystem.generated.h"