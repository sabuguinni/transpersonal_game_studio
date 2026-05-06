#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Manages agent task distribution, milestone tracking, and production pipeline
 * Critical for coordinating 19 specialized agents in the development chain
 */

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    CRITICAL = 0    UMETA(DisplayName = "Critical"),
    HIGH = 1        UMETA(DisplayName = "High"),
    MEDIUM = 2      UMETA(DisplayName = "Medium"),
    LOW = 3         UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    PENDING = 0     UMETA(DisplayName = "Pending"),
    IN_PROGRESS = 1 UMETA(DisplayName = "In Progress"),
    COMPLETED = 2   UMETA(DisplayName = "Completed"),
    BLOCKED = 3     UMETA(DisplayName = "Blocked"),
    FAILED = 4      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    WALK_AROUND = 0     UMETA(DisplayName = "Walk Around"),
    BASIC_SURVIVAL = 1  UMETA(DisplayName = "Basic Survival"),
    DINOSAUR_AI = 2     UMETA(DisplayName = "Dinosaur AI"),
    FULL_GAMEPLAY = 3   UMETA(DisplayName = "Full Gameplay")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Deliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime Deadline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float CompletionPercentage;

    FDir_AgentTask()
    {
        Priority = EDir_AgentPriority::MEDIUM;
        Status = EDir_TaskStatus::PENDING;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneType MilestoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Blockers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_TaskStatus Status;

    FDir_ProductionMilestone()
    {
        MilestoneType = EDir_MilestoneType::WALK_AROUND;
        CompletionPercentage = 0.0f;
        Status = EDir_TaskStatus::PENDING;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_CompilationStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    bool bCompilationSuccessful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 ErrorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> CriticalErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> MissingCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    FDateTime LastCompilationTime;

    FDir_CompilationStatus()
    {
        bCompilationSuccessful = false;
        ErrorCount = 0;
        WarningCount = 0;
    }
};

/**
 * Production Coordinator Component
 * Manages the entire development pipeline and agent coordination
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionCoordinatorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinatorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void UpdateTaskStatus(const FString& AgentName, EDir_TaskStatus NewStatus, float CompletionPercentage = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    TArray<FDir_AgentTask> GetBlockedTasks();

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void CreateMilestone(const FDir_ProductionMilestone& Milestone);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void UpdateMilestoneProgress(EDir_MilestoneType MilestoneType, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    FDir_ProductionMilestone GetCurrentMilestone();

    // Compilation Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void UpdateCompilationStatus(const FDir_CompilationStatus& Status);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    FDir_CompilationStatus GetLatestCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool IsCompilationBlocking();

    // Production Pipeline Control
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void TriggerEmergencyStop(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void ResumeProduction();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool CanAgentProceed(const FString& AgentName);

    // Reporting and Analytics
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    float GetOverallProjectCompletion();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production Data")
    TMap<FString, FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production Data")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production Data")
    FDir_CompilationStatus CurrentCompilationStatus;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production Control")
    bool bProductionHalted;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production Control")
    FString HaltReason;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production Control")
    FDateTime LastProductionUpdate;

private:
    void InitializeDefaultMilestones();
    void CheckMilestoneDependencies();
    void ValidateAgentDependencies();
    bool AreTaskDependenciesMet(const FDir_AgentTask& Task);
};

/**
 * Production Coordinator Actor
 * Singleton actor that manages the entire production pipeline
 */
UCLASS()
class TRANSPERSONALGAME_API ADir_ProductionCoordinatorActor : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinatorActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    static ADir_ProductionCoordinatorActor* GetProductionCoordinator(UWorld* World);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDir_ProductionCoordinatorComponent* ProductionCoordinatorComponent;

protected:
    static ADir_ProductionCoordinatorActor* Instance;
};

#include "ProductionCoordinator.generated.h"