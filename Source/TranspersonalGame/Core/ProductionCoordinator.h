#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordinator
 * Manages the development pipeline and coordinates agent tasks
 * Tracks production status and ensures milestone completion
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime AssignedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime CompletedTime;

    FDir_ProductionTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_Priority::Medium;
        bCompleted = false;
        AssignedTime = FDateTime::Now();
        CompletedTime = FDateTime();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsOperational;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString StatusNotes;

    FDir_SystemStatus()
    {
        SystemName = TEXT("");
        ActorCount = 0;
        bIsOperational = false;
        StatusNotes = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> RemainingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime TargetDate;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now();
    }
};

/**
 * Production Coordinator Component
 * Tracks development progress and coordinates agent activities
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

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTask(const FString& AgentName, const FString& TaskDescription, EDir_Priority Priority = EDir_Priority::Medium);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteTask(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionTask> GetTasksForAgent(const FString& AgentName) const;

    // System Status Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateSystemStatus(const FString& SystemName, int32 ActorCount, bool bIsOperational, const FString& Notes = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_SystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_SystemStatus> GetAllSystemStatuses() const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(const FString& MilestoneName, float CompletionPercentage, const TArray<FString>& CompletedTasks, const TArray<FString>& RemainingTasks);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_MilestoneProgress GetMilestoneProgress(const FString& MilestoneName) const;

    // Level Analysis
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor = true)
    void AnalyzeLevelStatus();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor = true)
    void CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_ProductionTask> ProductionTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_SystemStatus> SystemStatuses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_MilestoneProgress> MilestoneProgresses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float StatusUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoCleanupDuplicates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bGenerateDetailedReports;

private:
    float LastStatusUpdate;
    
    void InitializeDefaultMilestones();
    void CheckCriticalSystems();
    void LogProductionStatus();
};

/**
 * Production Coordinator Actor
 * Singleton actor that manages the entire development pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    static ADir_ProductionCoordinator* GetProductionCoordinator(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Production")
    UDir_ProductionCoordinatorComponent* GetCoordinatorComponent() const { return CoordinatorComponent; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    UDir_ProductionCoordinatorComponent* CoordinatorComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsSingleton;

private:
    static ADir_ProductionCoordinator* SingletonInstance;
};