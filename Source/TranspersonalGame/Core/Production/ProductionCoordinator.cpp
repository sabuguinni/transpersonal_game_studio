#include "ProductionCoordinator.h"
#include "Components/TextRenderComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create status display
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetWorldSize(100.0f);
    StatusDisplay->SetHorizontalAlignment(EHTA_Center);
    StatusDisplay->SetVerticalAlignment(EVRTA_TextCenter);
    StatusDisplay->SetTextRenderColor(FColor::Cyan);
    StatusDisplay->SetText(FText::FromString(TEXT("PRODUCTION COORDINATOR\nINITIALIZING...")));

    // Initialize default values
    CurrentPhase = EDir_ProductionPhase::CoreSystems;
    bAutoUpdateDisplay = true;
    UpdateInterval = 5.0f;
    
    // Initialize metrics for Cycle 025
    Metrics.CycleNumber = 25;
    Metrics.CompletedTasks = 0;
    Metrics.TotalTasks = 0;
    Metrics.OverallProgress = 0.0f;
    Metrics.FilesCreated = 0;
    Metrics.UE5CommandsExecuted = 0;
    Metrics.bPlayablePrototype = false;
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize Cycle 025 tasks
    InitializeCycle025Tasks();
    
    // Start auto-update timer if enabled
    if (bAutoUpdateDisplay && UpdateInterval > 0.0f)
    {
        GetWorldTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &ADir_ProductionCoordinator::UpdateDisplayTimer,
            UpdateInterval,
            true
        );
    }
    
    // Initial display update
    UpdateStatusText();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized for Cycle 025"));
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update metrics periodically
    UpdateMetrics();
}

void ADir_ProductionCoordinator::AddAgentTask(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.Priority = Priority;
    NewTask.StartTime = FDateTime::Now();
    NewTask.DeadlineTime = FDateTime::Now() + FTimespan::FromHours(24);
    
    AgentTasks.Add(NewTask);
    Metrics.TotalTasks = AgentTasks.Num();
    
    UE_LOG(LogTemp, Log, TEXT("Added task for %s: %s"), *AgentName, *TaskDescription);
    
    if (bAutoUpdateDisplay)
    {
        UpdateStatusText();
    }
}

void ADir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Log, TEXT("Updated %s status to %s"), *AgentName, *GetStatusDisplayName(NewStatus));
            break;
        }
    }
    
    if (bAutoUpdateDisplay)
    {
        UpdateStatusText();
    }
}

void ADir_ProductionCoordinator::CompleteAgentTask(const FString& AgentName, const TArray<FString>& Deliverables)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = EDir_AgentStatus::Completed;
            Task.Deliverables = Deliverables;
            Metrics.CompletedTasks++;
            
            UE_LOG(LogTemp, Warning, TEXT("Completed task for %s with %d deliverables"), *AgentName, Deliverables.Num());
            break;
        }
    }
    
    if (bAutoUpdateDisplay)
    {
        UpdateStatusText();
    }
}

void ADir_ProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Production phase changed to: %s"), *GetPhaseDisplayName());
    
    if (bAutoUpdateDisplay)
    {
        UpdateStatusText();
    }
}

float ADir_ProductionCoordinator::GetOverallProgress() const
{
    if (Metrics.TotalTasks == 0)
    {
        return 0.0f;
    }
    
    return (float)Metrics.CompletedTasks / (float)Metrics.TotalTasks * 100.0f;
}

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetBlockedTasks() const
{
    TArray<FDir_AgentTask> BlockedTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || Task.Status == EDir_AgentStatus::Failed)
        {
            BlockedTasks.Add(Task);
        }
    }
    
    return BlockedTasks;
}

void ADir_ProductionCoordinator::UpdateMetrics()
{
    // Count completed tasks
    int32 CompletedCount = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedCount++;
        }
    }
    
    Metrics.CompletedTasks = CompletedCount;
    Metrics.TotalTasks = AgentTasks.Num();
    Metrics.OverallProgress = GetOverallProgress();
    
    // Check if we have a playable prototype
    // This is a simplified check - in reality would be more complex
    Metrics.bPlayablePrototype = (Metrics.OverallProgress >= 60.0f);
}

void ADir_ProductionCoordinator::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("PRODUCTION REPORT - CYCLE %d\n"), Metrics.CycleNumber);
    Report += FString::Printf(TEXT("Phase: %s\n"), *GetPhaseDisplayName());
    Report += FString::Printf(TEXT("Progress: %.1f%%\n"), Metrics.OverallProgress);
    Report += FString::Printf(TEXT("Tasks: %d/%d completed\n"), Metrics.CompletedTasks, Metrics.TotalTasks);
    Report += FString::Printf(TEXT("Files Created: %d\n"), Metrics.FilesCreated);
    Report += FString::Printf(TEXT("UE5 Commands: %d\n"), Metrics.UE5CommandsExecuted);
    Report += FString::Printf(TEXT("Playable Prototype: %s\n"), Metrics.bPlayablePrototype ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\nAGENT STATUS:\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        Report += FString::Printf(TEXT("  %s: %s\n"), *Task.AgentName, *GetStatusDisplayName(Task.Status));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void ADir_ProductionCoordinator::InitializeCycle025Tasks()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Add critical tasks for Cycle 025
    AddAgentTask(TEXT("Agent_02_Engine_Architect"), TEXT("Fix compilation errors and finalize engine architecture"), 10.0f);
    AddAgentTask(TEXT("Agent_03_Core_Systems"), TEXT("Implement physics and movement systems"), 9.0f);
    AddAgentTask(TEXT("Agent_05_World_Generator"), TEXT("Create playable terrain with collision"), 8.0f);
    AddAgentTask(TEXT("Agent_09_Character_Artist"), TEXT("Implement functional character with survival stats"), 7.0f);
    AddAgentTask(TEXT("Agent_12_Combat_AI"), TEXT("Basic dinosaur AI and combat system"), 6.0f);
    AddAgentTask(TEXT("Agent_06_Environment_Artist"), TEXT("Populate world with vegetation and props"), 5.0f);
    AddAgentTask(TEXT("Agent_08_Lighting"), TEXT("Implement day/night cycle and atmosphere"), 4.0f);
    AddAgentTask(TEXT("Agent_18_QA"), TEXT("Test playable prototype and report issues"), 3.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d tasks for Cycle 025"), AgentTasks.Num());
}

void ADir_ProductionCoordinator::RefreshStatusDisplay()
{
    UpdateStatusText();
    UE_LOG(LogTemp, Log, TEXT("Status display refreshed"));
}

void ADir_ProductionCoordinator::UpdateDisplayTimer()
{
    UpdateStatusText();
}

void ADir_ProductionCoordinator::UpdateStatusText()
{
    if (!StatusDisplay)
    {
        return;
    }
    
    FString DisplayText = FString::Printf(TEXT("PRODUCTION COORDINATOR\nCYCLE %d\n"), Metrics.CycleNumber);
    DisplayText += FString::Printf(TEXT("PHASE: %s\n"), *GetPhaseDisplayName());
    DisplayText += FString::Printf(TEXT("PROGRESS: %.1f%%\n"), GetOverallProgress());
    DisplayText += FString::Printf(TEXT("TASKS: %d/%d\n"), Metrics.CompletedTasks, Metrics.TotalTasks);
    
    if (Metrics.bPlayablePrototype)
    {
        DisplayText += TEXT("STATUS: PLAYABLE PROTOTYPE READY");
    }
    else
    {
        DisplayText += TEXT("STATUS: BUILDING PROTOTYPE");
    }
    
    StatusDisplay->SetText(FText::FromString(DisplayText));
}

FString ADir_ProductionCoordinator::GetPhaseDisplayName() const
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction: return TEXT("PRE-PRODUCTION");
        case EDir_ProductionPhase::CoreSystems: return TEXT("CORE SYSTEMS");
        case EDir_ProductionPhase::WorldBuilding: return TEXT("WORLD BUILDING");
        case EDir_ProductionPhase::CharacterDev: return TEXT("CHARACTER DEV");
        case EDir_ProductionPhase::GameplayLoop: return TEXT("GAMEPLAY LOOP");
        case EDir_ProductionPhase::Polish: return TEXT("POLISH & QA");
        case EDir_ProductionPhase::Release: return TEXT("RELEASE READY");
        default: return TEXT("UNKNOWN");
    }
}

FString ADir_ProductionCoordinator::GetStatusDisplayName(EDir_AgentStatus Status) const
{
    switch (Status)
    {
        case EDir_AgentStatus::Idle: return TEXT("IDLE");
        case EDir_AgentStatus::Working: return TEXT("WORKING");
        case EDir_AgentStatus::Completed: return TEXT("COMPLETED");
        case EDir_AgentStatus::Blocked: return TEXT("BLOCKED");
        case EDir_AgentStatus::Failed: return TEXT("FAILED");
        default: return TEXT("UNKNOWN");
    }
}