#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "StudioDirectorTaskManager.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    Pending = 0,
    InProgress = 1,
    Completed = 2,
    Failed = 3,
    Blocked = 4
};

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    EngineArchitect = 2,
    CoreSystems = 3,
    PerformanceOptimizer = 4,
    ProceduralWorld = 5,
    EnvironmentArtist = 6,
    Architecture = 7,
    Lighting = 8,
    CharacterArtist = 9,
    Animation = 10,
    NPCBehavior = 11,
    CombatAI = 12,
    CrowdSimulation = 13,
    QuestDesigner = 14,
    Narrative = 15,
    Audio = 16,
    VFX = 17,
    QA = 18,
    Integration = 19
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 TaskID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentType AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CreatedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime DueDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        TaskID = 0;
        AssignedAgent = EDir_AgentType::CoreSystems;
        Priority = EDir_AgentPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        EstimatedHours = 1.0f;
        CreatedAt = FDateTime::Now();
        DueDate = FDateTime::Now() + FTimespan::FromHours(24);
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsPopulated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float PopulationPercentage;

    FDir_BiomeStatus()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        bIsPopulated = false;
        PopulationPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorTaskManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorTaskManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 CreateTask(EDir_AgentType Agent, const FString& Description, EDir_AgentPriority Priority, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool UpdateTaskStatus(int32 TaskID, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksForAgent(EDir_AgentType Agent);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_TaskStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetTaskByID(int32 TaskID);

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeStatus(const FString& BiomeName, const FVector& Location, int32 ActorCount, int32 DinosaurCount);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_BiomeStatus> GetAllBiomeStatuses();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_BiomeStatus GetBiomeStatus(const FString& BiomeName);

    // Prototype Validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetPrototypeValidationReport();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignCriticalTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProjectProgress();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    int32 NextTaskID;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDateTime LastValidationTime;

private:
    void InitializeBiomes();
    void ValidateTaskDependencies();
    bool CheckAgentAvailability(EDir_AgentType Agent);
    void GenerateProductionReport();
};

#include "StudioDirectorTaskManager.generated.h"