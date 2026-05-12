#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Central coordination system for the 18-agent development pipeline
 * Manages production flow, agent task assignment, and milestone tracking
 * This is the command center for the entire Transpersonal Game Studio operation
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"), 
    Complete    UMETA(DisplayName = "Complete"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    Architecture    UMETA(DisplayName = "Architecture"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    Characters      UMETA(DisplayName = "Characters"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Polish          UMETA(DisplayName = "Polish"),
    Testing         UMETA(DisplayName = "Testing"),
    Integration     UMETA(DisplayName = "Integration")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        Status = EDir_AgentStatus::Idle;
        ProgressPercentage = 0.0f;
        Dependencies = TEXT("");
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime CompletionDate;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bIsComplete = false;
        TargetDate = FDateTime::Now();
        CompletionDate = FDateTime::MinValue();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TaskBoardMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusDisplay;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bProductionActive;

public:
    // Agent Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CheckMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResolveAgentDependency(int32 AgentNumber);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(const FString& Name, const FString& Description, const TArray<int32>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteMilestone(const FString& MilestoneName);

    // Production Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetProductionStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EmergencyStopProduction();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Studio Director")
    void EditorRefreshTaskBoard();

    UFUNCTION(CallInEditor, Category = "Studio Director")
    void EditorGenerateStatusReport();

private:
    void UpdateStatusDisplay();
    void CheckAgentTimeouts();
    void ProcessAgentDependencies();
    FString FormatStatusText();
};