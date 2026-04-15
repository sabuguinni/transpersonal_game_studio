#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "../../SharedTypes.h"
#include "StudioDirectorManager.generated.h"

// Studio Director Agent Coordination System
// Manages the complete 19-agent production pipeline
// Ensures Miguel's creative vision is preserved through all stages

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector      UMETA(DisplayName = "Studio Director"),
    EngineArchitect     UMETA(DisplayName = "Engine Architect"),
    CoreSystems         UMETA(DisplayName = "Core Systems"),
    Performance         UMETA(DisplayName = "Performance"),
    WorldGenerator      UMETA(DisplayName = "World Generator"),
    EnvironmentArt      UMETA(DisplayName = "Environment Art"),
    Architecture        UMETA(DisplayName = "Architecture"),
    Lighting            UMETA(DisplayName = "Lighting"),
    CharacterArt        UMETA(DisplayName = "Character Art"),
    Animation           UMETA(DisplayName = "Animation"),
    NPCBehavior         UMETA(DisplayName = "NPC Behavior"),
    CombatAI            UMETA(DisplayName = "Combat AI"),
    CrowdSimulation     UMETA(DisplayName = "Crowd Simulation"),
    QuestDesign         UMETA(DisplayName = "Quest Design"),
    Narrative           UMETA(DisplayName = "Narrative"),
    Audio               UMETA(DisplayName = "Audio"),
    VFX                 UMETA(DisplayName = "VFX"),
    QA                  UMETA(DisplayName = "QA"),
    Integration         UMETA(DisplayName = "Integration")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction       UMETA(DisplayName = "Pre-Production"),
    TechnicalFoundation UMETA(DisplayName = "Technical Foundation"),
    WorldBuilding       UMETA(DisplayName = "World Building"),
    CharacterSystems    UMETA(DisplayName = "Character Systems"),
    GameplaySystems     UMETA(DisplayName = "Gameplay Systems"),
    ContentCreation     UMETA(DisplayName = "Content Creation"),
    Polish              UMETA(DisplayName = "Polish"),
    QualityAssurance    UMETA(DisplayName = "Quality Assurance"),
    Integration         UMETA(DisplayName = "Integration"),
    Release             UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Critical            UMETA(DisplayName = "Critical"),
    High                UMETA(DisplayName = "High"),
    Medium              UMETA(DisplayName = "Medium"),
    Low                 UMETA(DisplayName = "Low"),
    Background          UMETA(DisplayName = "Background")
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
    EDir_AgentType AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_ProductionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsBlocking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString CreativeVisionNote;

    FDir_AgentTask()
    {
        TaskName = TEXT("");
        Description = TEXT("");
        AssignedAgent = EDir_AgentType::StudioDirector;
        Priority = EDir_TaskPriority::Medium;
        Phase = EDir_ProductionPhase::PreProduction;
        EstimatedHours = 0.0f;
        CompletionPercentage = 0.0f;
        bIsBlocking = false;
        CreativeVisionNote = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockingIssues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float QualityScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bQAApproved;

    FDir_ProductionMetrics()
    {
        TotalTasks = 0;
        CompletedTasks = 0;
        BlockingIssues = 0;
        OverallProgress = 0.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
        QualityScore = 0.0f;
        bQAApproved = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorManager : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Production Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentProductionPhase;

    // Creative Vision Protection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creative Vision")
    FString MiguelVisionStatement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creative Vision")
    TArray<FString> CoreCreativePrinciples;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creative Vision")
    float VisionAlignmentThreshold;

public:
    virtual void Tick(float DeltaTime) override;

    // Task Management Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateNewTask(const FString& TaskName, const FString& Description, 
                      EDir_AgentType AssignedAgent, EDir_TaskPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateTaskProgress(const FString& TaskName, float NewProgress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteTask(const FString& TaskName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockTask(const FString& TaskName, const FString& Reason);

    // Agent Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void DispatchTaskToAgent(EDir_AgentType Agent, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksForAgent(EDir_AgentType Agent);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(EDir_AgentType Agent);

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsPhaseComplete(EDir_ProductionPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidatePhaseRequirements(EDir_ProductionPhase Phase);

    // Creative Vision Protection
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateCreativeAlignment(const FString& ProposedFeature);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetCreativeVision(const FString& VisionStatement);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AddCreativePrinciple(const FString& Principle);

    // Quality Assurance Integration
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RequestQAApproval(const FString& Feature);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void HandleQABlock(const FString& Issue);

    // Metrics and Reporting
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics CalculateCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProgressReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportToMiguel(const FString& Status);

    // Emergency Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void HandleCriticalIssue(const FString& Issue, EDir_AgentType AffectedAgent);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitiateRollback(const FString& Reason);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void InitializeProductionPipeline();

private:
    void SetupAgentDependencies();
    void ValidateTaskDependencies();
    void UpdateProductionMetrics();
    bool CheckForBlockingIssues();
};