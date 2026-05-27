#include "StudioDirectorCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UStudioDirectorCoordinator::UStudioDirectorCoordinator()
{
    bMilestone1Completed = false;
}

void UStudioDirectorCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeAgentList();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Coordinator initialized with %d agents"), AgentNames.Num());
}

void UStudioDirectorCoordinator::Deinitialize()
{
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Coordinator deinitialized"));
}

void UStudioDirectorCoordinator::InitializeAgentList()
{
    AgentNames.Empty();
    AgentNames.Add(TEXT("Studio Director"));
    AgentNames.Add(TEXT("Engine Architect"));
    AgentNames.Add(TEXT("Core Systems Programmer"));
    AgentNames.Add(TEXT("Performance Optimizer"));
    AgentNames.Add(TEXT("Procedural World Generator"));
    AgentNames.Add(TEXT("Environment Artist"));
    AgentNames.Add(TEXT("Architecture & Interior Agent"));
    AgentNames.Add(TEXT("Lighting & Atmosphere Agent"));
    AgentNames.Add(TEXT("Character Artist Agent"));
    AgentNames.Add(TEXT("Animation Agent"));
    AgentNames.Add(TEXT("NPC Behavior Agent"));
    AgentNames.Add(TEXT("Combat & Enemy AI Agent"));
    AgentNames.Add(TEXT("Crowd & Traffic Simulation"));
    AgentNames.Add(TEXT("Quest & Mission Designer"));
    AgentNames.Add(TEXT("Narrative & Dialogue Agent"));
    AgentNames.Add(TEXT("Audio Agent"));
    AgentNames.Add(TEXT("VFX Agent"));
    AgentNames.Add(TEXT("QA & Testing Agent"));
    AgentNames.Add(TEXT("Integration & Build Agent"));
}

void UStudioDirectorCoordinator::StartProductionCycle(const FString& CycleID)
{
    CurrentCycle.CycleID = CycleID;
    CurrentCycle.CycleStartTime = GetWorld()->GetTimeSeconds();
    CurrentCycle.bIsCompleted = false;
    CurrentCycle.AgentTasks.Empty();
    
    // Initialize tasks for all agents
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentName;
        NewTask.Status = EDir_AgentStatus::Idle;
        CurrentCycle.AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle %s started with %d agents"), *CycleID, CurrentCycle.AgentTasks.Num());
}

void UStudioDirectorCoordinator::CompleteProductionCycle()
{
    CurrentCycle.bIsCompleted = true;
    
    // Validate all agent deliverables
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    float CompletionRate = (float)CompletedTasks / (float)CurrentCycle.AgentTasks.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle %s completed. Success rate: %.2f%% (%d/%d agents)"), 
           *CurrentCycle.CycleID, CompletionRate * 100.0f, CompletedTasks, CurrentCycle.AgentTasks.Num());
    
    GenerateProductionReport();
}

void UStudioDirectorCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        Task->TaskDescription = TaskDescription;
        Task->Priority = Priority;
        Task->Status = EDir_AgentStatus::Working;
        
        UE_LOG(LogTemp, Log, TEXT("Task assigned to %s: %s (Priority: %.2f)"), *AgentName, *TaskDescription, Priority);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Agent not found: %s"), *AgentName);
    }
}

void UStudioDirectorCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        Task->Status = NewStatus;
        UE_LOG(LogTemp, Log, TEXT("Agent %s status updated to %d"), *AgentName, (int32)NewStatus);
    }
}

void UStudioDirectorCoordinator::AddAgentDeliverable(const FString& AgentName, const FString& Deliverable)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        Task->Deliverables.Add(Deliverable);
        UE_LOG(LogTemp, Log, TEXT("Deliverable added to %s: %s"), *AgentName, *Deliverable);
    }
}

FDir_AgentTask* UStudioDirectorCoordinator::FindAgentTask(const FString& AgentName)
{
    for (FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            return &Task;
        }
    }
    return nullptr;
}

bool UStudioDirectorCoordinator::ValidatePlayablePrototype()
{
    // Check for Milestone 1 requirements
    bool bHasPlayerCharacter = false;
    bool bHasMovementSystem = false;
    bool bHasDinosaurs = false;
    bool bHasLandscape = false;
    bool bHasLighting = false;
    
    // Validate through agent deliverables
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        if (Task.AgentName == TEXT("Character Artist Agent") && Task.Status == EDir_AgentStatus::Completed)
        {
            bHasPlayerCharacter = true;
        }
        if (Task.AgentName == TEXT("Core Systems Programmer") && Task.Status == EDir_AgentStatus::Completed)
        {
            bHasMovementSystem = true;
        }
        if (Task.AgentName == TEXT("Combat & Enemy AI Agent") && Task.Status == EDir_AgentStatus::Completed)
        {
            bHasDinosaurs = true;
        }
        if (Task.AgentName == TEXT("Procedural World Generator") && Task.Status == EDir_AgentStatus::Completed)
        {
            bHasLandscape = true;
        }
        if (Task.AgentName == TEXT("Lighting & Atmosphere Agent") && Task.Status == EDir_AgentStatus::Completed)
        {
            bHasLighting = true;
        }
    }
    
    bMilestone1Completed = bHasPlayerCharacter && bHasMovementSystem && bHasDinosaurs && bHasLandscape && bHasLighting;
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Validation - Character: %s, Movement: %s, Dinosaurs: %s, Landscape: %s, Lighting: %s"),
           bHasPlayerCharacter ? TEXT("OK") : TEXT("MISSING"),
           bHasMovementSystem ? TEXT("OK") : TEXT("MISSING"),
           bHasDinosaurs ? TEXT("OK") : TEXT("MISSING"),
           bHasLandscape ? TEXT("OK") : TEXT("MISSING"),
           bHasLighting ? TEXT("OK") : TEXT("MISSING"));
    
    return bMilestone1Completed;
}

void UStudioDirectorCoordinator::SetMilestone1Priorities()
{
    // Set high priority tasks for Milestone 1
    AssignTaskToAgent(TEXT("Core Systems Programmer"), TEXT("Implement WASD movement and camera for TranspersonalCharacter"), 10.0f);
    AssignTaskToAgent(TEXT("Character Artist Agent"), TEXT("Create playable character with collision and input"), 9.0f);
    AssignTaskToAgent(TEXT("Combat & Enemy AI Agent"), TEXT("Implement T-Rex AI with basic behavior tree"), 8.0f);
    AssignTaskToAgent(TEXT("Procedural World Generator"), TEXT("Generate terrain with height variation"), 7.0f);
    AssignTaskToAgent(TEXT("Lighting & Atmosphere Agent"), TEXT("Setup directional light, sky atmosphere, and fog"), 6.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 priorities set for critical agents"));
}

void UStudioDirectorCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - CYCLE %s ==="), *CurrentCycle.CycleID);
    
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        FString StatusText;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusText = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusText = TEXT("WORKING"); break;
            case EDir_AgentStatus::Completed: StatusText = TEXT("COMPLETED"); break;
            case EDir_AgentStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Failed: StatusText = TEXT("FAILED"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Agent: %s | Status: %s | Deliverables: %d | Priority: %.1f"),
               *Task.AgentName, *StatusText, Task.Deliverables.Num(), Task.Priority);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END PRODUCTION REPORT ==="));
}