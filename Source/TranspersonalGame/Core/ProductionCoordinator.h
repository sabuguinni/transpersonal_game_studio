#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Tracks Milestone 1 progress across all 18 agents
 * Manages task distribution and completion verification
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Active      UMETA(DisplayName = "Active"), 
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    Implementation  UMETA(DisplayName = "Implementation"),
    Testing         UMETA(DisplayName = "Testing"),
    Integration     UMETA(DisplayName = "Integration"),
    Complete        UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString CompletionNotes;

    FDir_AgentTask()
    {
        TaskName = TEXT("");
        Description = TEXT("");
        Priority = 0;
        bCompleted = false;
        CompletionNotes = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FDir_AgentTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString LastUpdate;

    FDir_AgentInfo()
    {
        AgentName = TEXT("");
        Role = TEXT("");
        Status = EDir_AgentStatus::Ready;
        CompletionPercentage = 0.0f;
        LastUpdate = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestonePhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentInfo> Agents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CriticalPriorities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString CurrentCycle;

    FDir_MilestoneData()
    {
        MilestoneName = TEXT("Milestone 1 - Walk Around Prototype");
        Phase = EDir_MilestonePhase::Implementation;
        OverallProgress = 0.0f;
        CurrentCycle = TEXT("");
    }
};

/**
 * Production Coordinator Subsystem
 * Central hub for Studio Director to manage all agent coordination
 */
UCLASS()
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(const FString& AgentName, const FString& TaskName, const FString& Notes);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress() const;

    // Coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalPriorities() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentInfo> GetAgentsByStatus(EDir_AgentStatus Status) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentInfo GetAgentInfo(const FString& AgentName) const;

    // Validation and reporting
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateMinPlayableMap() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogCriticalIssue(const FString& Issue, const FString& ResponsibleAgent);

    // Editor tools
    UFUNCTION(CallInEditor, Category = "Production")
    void RefreshAgentStatus();

    UFUNCTION(CallInEditor, Category = "Production")
    void ExportProductionData();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    FDir_MilestoneData CurrentMilestone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FString> CriticalIssues;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    FDateTime LastUpdate;

private:
    void SetupAgentPipeline();
    void ValidateAgentDependencies();
    FDir_AgentInfo* FindAgent(const FString& AgentName);
};