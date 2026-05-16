#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

// Studio Director agent coordination and task management
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    int32 AgentNumber = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    FString TaskDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    EDir_TaskPriority Priority = EDir_TaskPriority::Medium;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    EDir_TaskStatus Status = EDir_TaskStatus::Pending;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    FString CycleId;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    float EstimatedDuration = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        TaskDescription = TEXT("");
        Priority = EDir_TaskPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        CycleId = TEXT("");
        EstimatedDuration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    EDir_BiomeType BiomeType = EDir_BiomeType::Savanna;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    bool bIsPopulated = false;

    UPROPERTY(BlueprintReadWrite, Category = "Director")
    TArray<FString> AssetTypes;

    FDir_BiomeStatus()
    {
        BiomeType = EDir_BiomeType::Savanna;
        WorldLocation = FVector::ZeroVector;
        ActorCount = 0;
        bIsPopulated = false;
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

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_TaskPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateTaskStatus(int32 AgentNumber, const FString& CycleId, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetTaskForAgent(int32 AgentNumber) const;

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterBiomeStatus(EDir_BiomeType BiomeType, FVector Location, int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_BiomeStatus> GetBiomeStatuses() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool AreCriteriaMet() const;

    // Production pipeline
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void InitializeAgentTasks();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FString CurrentCycleId;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    float CycleStartTime = 0.0f;

private:
    void SetupInitialTasks();
    void InitializeBiomeTargets();
    bool ValidateBiomePopulation() const;
    bool ValidateAtmosphere() const;
    bool ValidateFBXPipeline() const;
};