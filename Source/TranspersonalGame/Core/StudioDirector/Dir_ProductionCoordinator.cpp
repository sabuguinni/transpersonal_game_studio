#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    ProductionUpdateInterval = 5.0f;
    bAutoAssignTasks = true;
    bEnableProductionLogging = true;
    LastUpdateTime = 0.0f;
    
    // Initialize Milestone 1 status
    Milestone1Status.MilestoneName = TEXT("Milestone 1 - Playable Prototype");
    Milestone1Status.bIsCriticalPath = true;
    Milestone1Status.CompletionPercentage = 0.0f;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized - Cycle 009"));
    
    InitializeAgentTasks();
    InitializeMilestone1Tasks();
    
    if (bEnableProductionLogging)
    {
        LogProductionStatus();
    }
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= ProductionUpdateInterval)
    {
        UpdateMilestoneProgress();
        UpdateAgentTaskPriorities();
        CheckMilestoneBlockers();
        
        if (bEnableProductionLogging)
        {
            LogProductionStatus();
        }
        
        LastUpdateTime = 0.0f;
    }
}

void UDir_ProductionCoordinator::InitializeAgentTasks()
{
    ActiveTasks.Empty();
    
    // Critical Milestone 1 tasks for key agents
    AssignTaskToAgent(2, TEXT("Define UE5.5 architecture and compilation rules"), TEXT("Architecture document + working build system"), 10.0f);
    AssignTaskToAgent(3, TEXT("Implement core physics and collision systems"), TEXT("Working physics components"), 9.0f);
    AssignTaskToAgent(5, TEXT("Generate realistic terrain with height variation"), TEXT("Landscape with hills, valleys, and proper collision"), 9.0f);
    AssignTaskToAgent(6, TEXT("Populate world with trees, rocks, and environment props"), TEXT("Visible environment assets in MinPlayableMap"), 8.0f);
    AssignTaskToAgent(8, TEXT("Setup proper lighting and atmosphere"), TEXT("Day/night cycle with proper shadows"), 7.0f);
    AssignTaskToAgent(9, TEXT("Create realistic dinosaur character models"), TEXT("5+ dinosaur actors with proper meshes"), 9.0f);
    AssignTaskToAgent(10, TEXT("Implement character movement and animations"), TEXT("Smooth character movement with animations"), 8.0f);
    AssignTaskToAgent(11, TEXT("Basic dinosaur AI and behavior"), TEXT("Dinosaurs that move and react to player"), 7.0f);
    AssignTaskToAgent(12, TEXT("Combat and survival HUD"), TEXT("Health/hunger/thirst bars visible"), 6.0f);
    AssignTaskToAgent(15, TEXT("Core narrative and game lore"), TEXT("Game bible and character backstory"), 5.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agent tasks for Milestone 1"), ActiveTasks.Num());
}

void UDir_ProductionCoordinator::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const FString& ExpectedDeliverable, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.TaskDescription = TaskDescription;
    NewTask.ExpectedDeliverable = ExpectedDeliverable;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;
    
    // Set agent names
    switch (AgentID)
    {
        case 2: NewTask.AgentName = TEXT("Engine Architect"); break;
        case 3: NewTask.AgentName = TEXT("Core Systems Programmer"); break;
        case 5: NewTask.AgentName = TEXT("Procedural World Generator"); break;
        case 6: NewTask.AgentName = TEXT("Environment Artist"); break;
        case 8: NewTask.AgentName = TEXT("Lighting & Atmosphere"); break;
        case 9: NewTask.AgentName = TEXT("Character Artist"); break;
        case 10: NewTask.AgentName = TEXT("Animation Agent"); break;
        case 11: NewTask.AgentName = TEXT("NPC Behavior Agent"); break;
        case 12: NewTask.AgentName = TEXT("Combat & Enemy AI"); break;
        case 15: NewTask.AgentName = TEXT("Narrative & Dialogue"); break;
        default: NewTask.AgentName = FString::Printf(TEXT("Agent %d"), AgentID); break;
    }
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Assigned task to Agent %d (%s): %s"), AgentID, *NewTask.AgentName, *TaskDescription);
}

void UDir_ProductionCoordinator::MarkTaskCompleted(int32 AgentID, const FString& DeliverableResult)
{
    for (int32 i = 0; i < ActiveTasks.Num(); i++)
    {
        if (ActiveTasks[i].AgentID == AgentID && !ActiveTasks[i].bIsCompleted)
        {
            ActiveTasks[i].bIsCompleted = true;
            CompletedTasks.Add(ActiveTasks[i]);
            
            UE_LOG(LogTemp, Warning, TEXT("Task completed by Agent %d: %s"), AgentID, *DeliverableResult);
            
            // Remove from active tasks
            ActiveTasks.RemoveAt(i);
            break;
        }
    }
    
    UpdateMilestoneProgress();
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetPendingTasks()
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (!Task.bIsCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    // Sort by priority (highest first)
    PendingTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    return PendingTasks;
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetTasksForAgent(int32 AgentID)
{
    TArray<FDir_AgentTask> AgentTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

void UDir_ProductionCoordinator::InitializeMilestone1Tasks()
{
    Milestone1Status.RequiredComponents.Empty();
    Milestone1Status.CompletedComponents.Empty();
    
    // Define required components for Milestone 1
    Milestone1Status.RequiredComponents.Add(TEXT("Terrain with height variation"));
    Milestone1Status.RequiredComponents.Add(TEXT("Player character with movement"));
    Milestone1Status.RequiredComponents.Add(TEXT("5+ dinosaur actors visible"));
    Milestone1Status.RequiredComponents.Add(TEXT("Environment props (trees, rocks)"));
    Milestone1Status.RequiredComponents.Add(TEXT("Proper lighting setup"));
    Milestone1Status.RequiredComponents.Add(TEXT("Basic collision system"));
    Milestone1Status.RequiredComponents.Add(TEXT("Survival HUD elements"));
    Milestone1Status.RequiredComponents.Add(TEXT("Working game mode"));
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 initialized with %d required components"), Milestone1Status.RequiredComponents.Num());
}

FDir_MilestoneStatus UDir_ProductionCoordinator::GetMilestone1Status()
{
    return Milestone1Status;
}

void UDir_ProductionCoordinator::UpdateMilestoneProgress()
{
    if (Milestone1Status.RequiredComponents.Num() == 0)
    {
        return;
    }
    
    // Calculate completion percentage based on completed tasks
    int32 CompletedCount = CompletedTasks.Num();
    int32 TotalTasks = CompletedTasks.Num() + ActiveTasks.Num();
    
    if (TotalTasks > 0)
    {
        Milestone1Status.CompletionPercentage = (float)CompletedCount / (float)TotalTasks * 100.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Milestone 1 Progress: %.1f%% (%d/%d tasks completed)"), 
           Milestone1Status.CompletionPercentage, CompletedCount, TotalTasks);
}

TArray<int32> UDir_ProductionCoordinator::GetCriticalPathAgents()
{
    TArray<int32> CriticalAgents;
    
    // Critical path for Milestone 1
    CriticalAgents.Add(2);  // Engine Architect
    CriticalAgents.Add(3);  // Core Systems
    CriticalAgents.Add(5);  // World Generator
    CriticalAgents.Add(9);  // Character Artist
    CriticalAgents.Add(10); // Animation
    
    return CriticalAgents;
}

void UDir_ProductionCoordinator::EscalateCriticalIssue(int32 AgentID, const FString& IssueDescription)
{
    UE_LOG(LogTemp, Error, TEXT("CRITICAL ISSUE - Agent %d: %s"), AgentID, *IssueDescription);
    
    // In a real system, this would send notifications to the Studio Director
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, 
            FString::Printf(TEXT("CRITICAL: Agent %d - %s"), AgentID, *IssueDescription));
    }
}

FString UDir_ProductionCoordinator::GenerateProductionReport()
{
    FString Report = TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT - CYCLE 009 ===\n\n");
    
    Report += FString::Printf(TEXT("Milestone 1 Progress: %.1f%%\n"), Milestone1Status.CompletionPercentage);
    Report += FString::Printf(TEXT("Active Tasks: %d\n"), ActiveTasks.Num());
    Report += FString::Printf(TEXT("Completed Tasks: %d\n"), CompletedTasks.Num());
    Report += TEXT("\nPENDING HIGH-PRIORITY TASKS:\n");
    
    TArray<FDir_AgentTask> PendingTasks = GetPendingTasks();
    for (int32 i = 0; i < FMath::Min(5, PendingTasks.Num()); i++)
    {
        const FDir_AgentTask& Task = PendingTasks[i];
        Report += FString::Printf(TEXT("- Agent %d (%s): %s [Priority: %.1f]\n"), 
                                 Task.AgentID, *Task.AgentName, *Task.TaskDescription, Task.Priority);
    }
    
    Report += TEXT("\nCRITICAL PATH STATUS:\n");
    TArray<int32> CriticalAgents = GetCriticalPathAgents();
    for (int32 AgentID : CriticalAgents)
    {
        TArray<FDir_AgentTask> AgentTasks = GetTasksForAgent(AgentID);
        Report += FString::Printf(TEXT("- Agent %d: %d pending tasks\n"), AgentID, AgentTasks.Num());
    }
    
    return Report;
}

void UDir_ProductionCoordinator::LogProductionStatus()
{
    FString Report = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void UDir_ProductionCoordinator::UpdateAgentTaskPriorities()
{
    // Boost priority for critical path agents
    TArray<int32> CriticalAgents = GetCriticalPathAgents();
    
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (CriticalAgents.Contains(Task.AgentID))
        {
            Task.Priority = FMath::Max(Task.Priority, 8.0f);
        }
    }
}

void UDir_ProductionCoordinator::CheckMilestoneBlockers()
{
    // Check for tasks that have been pending too long
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Priority >= 9.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("High-priority task pending: Agent %d - %s"), 
                   Task.AgentID, *Task.TaskDescription);
        }
    }
}

void UDir_ProductionCoordinator::ValidateTaskDependencies()
{
    // Ensure dependencies are met before allowing certain tasks
    // This would be expanded with actual dependency logic
    UE_LOG(LogTemp, Log, TEXT("Validating task dependencies..."));
}