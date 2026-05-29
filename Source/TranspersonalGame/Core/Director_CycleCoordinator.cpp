#include "Director_CycleCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UDir_CycleCoordinator::UDir_CycleCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    bCycleActive = false;
    CycleStartTime = 0.0f;
    CurrentCycleNumber = 0;
    
    // Initialize critical path agents (blocking agents that affect the entire pipeline)
    CriticalPathAgents = {2, 3, 5, 9, 12, 18, 19}; // Engine Architect, Core Systems, World Generator, Character Artist, Combat AI, QA, Integration
    
    // Define minimum requirements for a playable prototype
    PlayablePrototypeRequirements = {
        TEXT("TranspersonalCharacter with WASD movement"),
        TEXT("Camera boom and follow camera"),
        TEXT("Landscape with terrain variation"),
        TEXT("Player can walk, run, jump"),
        TEXT("At least 3 dinosaur meshes in world"),
        TEXT("Directional light and sky atmosphere"),
        TEXT("Survival HUD with health/hunger/thirst bars"),
        TEXT("Basic collision detection"),
        TEXT("Game compiles without errors")
    };
}

void UDir_CycleCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Director Cycle Coordinator initialized"));
    InitializeAgentTasks();
}

void UDir_CycleCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bCycleActive)
    {
        UpdateCycleMetrics();
        ValidateCriticalPath();
        CheckPlayablePrototypeStatus();
    }
}

void UDir_CycleCoordinator::InitializeCycle(int32 CycleNumber)
{
    CurrentCycleNumber = CycleNumber;
    bCycleActive = true;
    CycleStartTime = GetWorld()->GetTimeSeconds();
    
    // Reset metrics
    CurrentMetrics = FDir_CycleMetrics();
    CurrentMetrics.CycleNumber = CycleNumber;
    CurrentMetrics.CurrentPhase = EDir_CyclePhase::Planning;
    CurrentMetrics.TotalTasks = AgentTasks.Num();
    
    // Reset all agent statuses
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.CompletedDeliverables.Empty();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cycle %d initialized - Focus: Playable Prototype"), CycleNumber);
    LogCycleProgress();
}

void UDir_CycleCoordinator::AdvanceToNextPhase()
{
    if (!ValidatePhaseCompletion())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot advance phase - current phase incomplete"));
        return;
    }
    
    int32 CurrentPhaseInt = static_cast<int32>(CurrentMetrics.CurrentPhase);
    CurrentPhaseInt++;
    
    if (CurrentPhaseInt >= static_cast<int32>(EDir_CyclePhase::Completion))
    {
        CurrentMetrics.CurrentPhase = EDir_CyclePhase::Completion;
        bCycleActive = false;
        GenerateCycleReport();
    }
    else
    {
        CurrentMetrics.CurrentPhase = static_cast<EDir_CyclePhase>(CurrentPhaseInt);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced to phase: %d"), CurrentPhaseInt);
}

bool UDir_CycleCoordinator::ValidatePhaseCompletion()
{
    int32 CompletedInPhase = 0;
    int32 RequiredInPhase = 0;
    
    // Count completed tasks relevant to current phase
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bIsBlocking || CriticalPathAgents.Contains(Task.AgentID))
        {
            RequiredInPhase++;
            if (Task.Status == EDir_AgentStatus::Completed)
            {
                CompletedInPhase++;
            }
        }
    }
    
    float CompletionRatio = RequiredInPhase > 0 ? static_cast<float>(CompletedInPhase) / RequiredInPhase : 1.0f;
    return CompletionRatio >= 0.8f; // 80% of critical tasks must be complete
}

void UDir_CycleCoordinator::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const TArray<FString>& Deliverables, float Priority)
{
    // Find existing task or create new one
    FDir_AgentTask* ExistingTask = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& Task) {
        return Task.AgentID == AgentID;
    });
    
    if (ExistingTask)
    {
        ExistingTask->TaskDescription = TaskDescription;
        ExistingTask->RequiredDeliverables = Deliverables;
        ExistingTask->Priority = Priority;
        ExistingTask->Status = EDir_AgentStatus::Working;
    }
    else
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = AgentID;
        NewTask.AgentName = GetAgentName(AgentID);
        NewTask.TaskDescription = TaskDescription;
        NewTask.RequiredDeliverables = Deliverables;
        NewTask.Priority = Priority;
        NewTask.Status = EDir_AgentStatus::Working;
        NewTask.bIsBlocking = CriticalPathAgents.Contains(AgentID);
        
        AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Task assigned to Agent %d: %s"), AgentID, *TaskDescription);
}

void UDir_CycleCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& Task) {
        return Task.AgentID == AgentID;
    });
    
    if (Task)
    {
        Task->Status = NewStatus;
        UE_LOG(LogTemp, Log, TEXT("Agent %d status updated to: %d"), AgentID, static_cast<int32>(NewStatus));
    }
}

void UDir_CycleCoordinator::MarkDeliverableComplete(int32 AgentID, const FString& Deliverable)
{
    FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& Task) {
        return Task.AgentID == AgentID;
    });
    
    if (Task && !Task->CompletedDeliverables.Contains(Deliverable))
    {
        Task->CompletedDeliverables.Add(Deliverable);
        CurrentMetrics.FilesCreated++;
        
        // Check if all deliverables are complete
        if (Task->CompletedDeliverables.Num() >= Task->RequiredDeliverables.Num())
        {
            Task->Status = EDir_AgentStatus::Completed;
            CurrentMetrics.CompletedTasks++;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Agent %d completed deliverable: %s"), AgentID, *Deliverable);
    }
}

TArray<int32> UDir_CycleCoordinator::GetCriticalPathAgents()
{
    return CriticalPathAgents;
}

bool UDir_CycleCoordinator::IsAgentBlocking(int32 AgentID)
{
    const FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& Task) {
        return Task.AgentID == AgentID;
    });
    
    if (Task)
    {
        return Task->bIsBlocking && (Task->Status == EDir_AgentStatus::Failed || Task->Status == EDir_AgentStatus::Blocked);
    }
    
    return false;
}

void UDir_CycleCoordinator::ResolveBlockingIssue(int32 AgentID, const FString& Resolution)
{
    FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& Task) {
        return Task.AgentID == AgentID;
    });
    
    if (Task && (Task->Status == EDir_AgentStatus::Blocked || Task->Status == EDir_AgentStatus::Failed))
    {
        Task->Status = EDir_AgentStatus::Working;
        UE_LOG(LogTemp, Warning, TEXT("Resolved blocking issue for Agent %d: %s"), AgentID, *Resolution);
    }
}

bool UDir_CycleCoordinator::ValidatePlayablePrototype()
{
    return CalculatePrototypeCompleteness() >= 0.8f; // 80% complete
}

TArray<FString> UDir_CycleCoordinator::GetMissingPrototypeElements()
{
    TArray<FString> MissingElements;
    
    // This would be expanded to check actual game state
    for (const FString& Requirement : PlayablePrototypeRequirements)
    {
        // Placeholder logic - in real implementation, check actual game objects
        bool bRequirementMet = FMath::RandBool(); // Random for now
        
        if (!bRequirementMet)
        {
            MissingElements.Add(Requirement);
        }
    }
    
    return MissingElements;
}

float UDir_CycleCoordinator::CalculatePrototypeCompleteness()
{
    TArray<FString> MissingElements = GetMissingPrototypeElements();
    int32 CompletedElements = PlayablePrototypeRequirements.Num() - MissingElements.Num();
    
    return PlayablePrototypeRequirements.Num() > 0 ? 
        static_cast<float>(CompletedElements) / PlayablePrototypeRequirements.Num() : 0.0f;
}

FDir_CycleMetrics UDir_CycleCoordinator::GetCurrentCycleMetrics()
{
    UpdateCycleMetrics();
    return CurrentMetrics;
}

void UDir_CycleCoordinator::GenerateCycleReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CYCLE %d COMPLETION REPORT ==="), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Tasks Completed: %d/%d"), CurrentMetrics.CompletedTasks, CurrentMetrics.TotalTasks);
    UE_LOG(LogTemp, Warning, TEXT("Files Created: %d"), CurrentMetrics.FilesCreated);
    UE_LOG(LogTemp, Warning, TEXT("UE5 Commands: %d"), CurrentMetrics.UE5CommandsExecuted);
    UE_LOG(LogTemp, Warning, TEXT("Prototype Completeness: %.1f%%"), CurrentMetrics.OverallProgress * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Playable Prototype Ready: %s"), CurrentMetrics.bPlayablePrototypeReady ? TEXT("YES") : TEXT("NO"));
    
    // Log missing prototype elements
    TArray<FString> MissingElements = GetMissingPrototypeElements();
    if (MissingElements.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Missing Prototype Elements:"));
        for (const FString& Element : MissingElements)
        {
            UE_LOG(LogTemp, Error, TEXT("- %s"), *Element);
        }
    }
}

void UDir_CycleCoordinator::ExportProductionDashboard()
{
    // This would export metrics to external dashboard
    UE_LOG(LogTemp, Log, TEXT("Production dashboard exported"));
}

void UDir_CycleCoordinator::TriggerEmergencyReset()
{
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY RESET TRIGGERED"));
    
    // Reset all agents to idle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.CompletedDeliverables.Empty();
    }
    
    CurrentMetrics.CurrentPhase = EDir_CyclePhase::Planning;
    PrioritizePlayableElements();
}

void UDir_CycleCoordinator::ForcePhaseAdvancement()
{
    UE_LOG(LogTemp, Warning, TEXT("FORCING PHASE ADVANCEMENT"));
    AdvanceToNextPhase();
}

void UDir_CycleCoordinator::PrioritizePlayableElements()
{
    UE_LOG(LogTemp, Warning, TEXT("PRIORITIZING PLAYABLE ELEMENTS"));
    
    // Increase priority for critical agents
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (CriticalPathAgents.Contains(Task.AgentID))
        {
            Task.Priority = 10.0f; // Maximum priority
            Task.bIsBlocking = true;
        }
    }
}

void UDir_CycleCoordinator::InitializeAgentTasks()
{
    // Initialize all 19 agents with their core responsibilities
    TArray<TPair<int32, FString>> AgentDefinitions = {
        {1, TEXT("Studio Director")},
        {2, TEXT("Engine Architect")},
        {3, TEXT("Core Systems Programmer")},
        {4, TEXT("Performance Optimizer")},
        {5, TEXT("Procedural World Generator")},
        {6, TEXT("Environment Artist")},
        {7, TEXT("Architecture & Interior Agent")},
        {8, TEXT("Lighting & Atmosphere Agent")},
        {9, TEXT("Character Artist Agent")},
        {10, TEXT("Animation Agent")},
        {11, TEXT("NPC Behavior Agent")},
        {12, TEXT("Combat & Enemy AI Agent")},
        {13, TEXT("Crowd & Traffic Simulation")},
        {14, TEXT("Quest & Mission Designer")},
        {15, TEXT("Narrative & Dialogue Agent")},
        {16, TEXT("Audio Agent")},
        {17, TEXT("VFX Agent")},
        {18, TEXT("QA & Testing Agent")},
        {19, TEXT("Integration & Build Agent")}
    };
    
    for (const auto& AgentDef : AgentDefinitions)
    {
        FDir_AgentTask Task;
        Task.AgentID = AgentDef.Key;
        Task.AgentName = AgentDef.Value;
        Task.Status = EDir_AgentStatus::Idle;
        Task.bIsBlocking = CriticalPathAgents.Contains(AgentDef.Key);
        AgentTasks.Add(Task);
    }
    
    CurrentMetrics.TotalTasks = AgentTasks.Num();
}

void UDir_CycleCoordinator::UpdateCycleMetrics()
{
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.FilesCreated = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CurrentMetrics.CompletedTasks++;
        }
        CurrentMetrics.FilesCreated += Task.CompletedDeliverables.Num();
    }
    
    CurrentMetrics.OverallProgress = CalculatePrototypeCompleteness();
    CurrentMetrics.bPlayablePrototypeReady = ValidatePlayablePrototype();
}

void UDir_CycleCoordinator::ValidateCriticalPath()
{
    bool bCriticalPathBlocked = false;
    
    for (int32 AgentID : CriticalPathAgents)
    {
        if (IsAgentBlocking(AgentID))
        {
            bCriticalPathBlocked = true;
            UE_LOG(LogTemp, Error, TEXT("CRITICAL PATH BLOCKED by Agent %d"), AgentID);
        }
    }
    
    if (bCriticalPathBlocked)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL PATH BLOCKED - Emergency intervention required"));
    }
}

void UDir_CycleCoordinator::CheckPlayablePrototypeStatus()
{
    static float LastCheck = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check every 10 seconds
    if (CurrentTime - LastCheck >= 10.0f)
    {
        LastCheck = CurrentTime;
        
        float Completeness = CalculatePrototypeCompleteness();
        if (Completeness >= 0.8f && !CurrentMetrics.bPlayablePrototypeReady)
        {
            CurrentMetrics.bPlayablePrototypeReady = true;
            UE_LOG(LogTemp, Warning, TEXT("PLAYABLE PROTOTYPE ACHIEVED! Completeness: %.1f%%"), Completeness * 100.0f);
        }
    }
}

void UDir_CycleCoordinator::LogCycleProgress()
{
    UE_LOG(LogTemp, Log, TEXT("Cycle Progress - Phase: %d, Completed: %d/%d, Prototype: %.1f%%"), 
        static_cast<int32>(CurrentMetrics.CurrentPhase),
        CurrentMetrics.CompletedTasks,
        CurrentMetrics.TotalTasks,
        CurrentMetrics.OverallProgress * 100.0f);
}

FString UDir_CycleCoordinator::GetAgentName(int32 AgentID)
{
    const FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& Task) {
        return Task.AgentID == AgentID;
    });
    
    return Task ? Task->AgentName : FString::Printf(TEXT("Agent %d"), AgentID);
}