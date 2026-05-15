#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Initialize milestone tracking
    bCharacterMovementComplete = false;
    bTerrainComplete = false;
    bDinosaurActorsComplete = false;
    bLightingComplete = false;
    bSurvivalUIComplete = false;
}

void UStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized"));
    InitializeAgentTasks();
}

void UStudioDirectorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateCycleMetrics();
    CheckMilestone1Progress();
}

void UStudioDirectorSystem::CreateAgentTask(int32 AgentID, const FString& AgentName, const FString& TaskDescription, 
                                          EDir_AgentTaskPriority Priority, const FVector& WorldPosition)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentTaskStatus::Pending;
    NewTask.WorldPosition = WorldPosition;
    
    // Set estimated duration based on priority
    switch (Priority)
    {
        case EDir_AgentTaskPriority::Critical:
            NewTask.EstimatedDuration = 300.0f; // 5 minutes
            break;
        case EDir_AgentTaskPriority::High:
            NewTask.EstimatedDuration = 600.0f; // 10 minutes
            break;
        case EDir_AgentTaskPriority::Medium:
            NewTask.EstimatedDuration = 900.0f; // 15 minutes
            break;
        default:
            NewTask.EstimatedDuration = 1200.0f; // 20 minutes
            break;
    }
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Created task for Agent #%d - %s"), AgentID, *TaskDescription);
}

void UStudioDirectorSystem::UpdateTaskStatus(int32 AgentID, EDir_AgentTaskStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID && Task.Status != EDir_AgentTaskStatus::Completed)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent #%d task status updated to %d"), AgentID, (int32)NewStatus);
            
            if (NewStatus == EDir_AgentTaskStatus::Completed)
            {
                CurrentCycle.CompletedTasks++;
            }
            else if (NewStatus == EDir_AgentTaskStatus::Failed)
            {
                CurrentCycle.FailedTasks++;
            }
            break;
        }
    }
}

TArray<FDir_AgentTask> UStudioDirectorSystem::GetTasksForAgent(int32 AgentID)
{
    TArray<FDir_AgentTask> AgentSpecificTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            AgentSpecificTasks.Add(Task);
        }
    }
    
    return AgentSpecificTasks;
}

TArray<FDir_AgentTask> UStudioDirectorSystem::GetTasksByPriority(EDir_AgentTaskPriority Priority)
{
    TArray<FDir_AgentTask> PriorityTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Priority == Priority)
        {
            PriorityTasks.Add(Task);
        }
    }
    
    return PriorityTasks;
}

TArray<FDir_AgentTask> UStudioDirectorSystem::GetTasksByStatus(EDir_AgentTaskStatus Status)
{
    TArray<FDir_AgentTask> StatusTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == Status)
        {
            StatusTasks.Add(Task);
        }
    }
    
    return StatusTasks;
}

void UStudioDirectorSystem::StartProductionCycle(int32 CycleNumber)
{
    CurrentCycle = FDir_ProductionMetrics();
    CurrentCycle.CycleNumber = CycleNumber;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Starting Production Cycle #%d"), CycleNumber);
    
    // Clear completed tasks from previous cycle
    AgentTasks.RemoveAll([](const FDir_AgentTask& Task) {
        return Task.Status == EDir_AgentTaskStatus::Completed;
    });
    
    AssignCriticalTasks();
}

void UStudioDirectorSystem::EndProductionCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Ending Production Cycle #%d"), CurrentCycle.CycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d, Failed Tasks: %d"), CurrentCycle.CompletedTasks, CurrentCycle.FailedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Files Created: %d, UE5 Commands: %d"), CurrentCycle.FilesCreated, CurrentCycle.UE5CommandsExecuted);
}

FDir_ProductionMetrics UStudioDirectorSystem::GetCurrentCycleMetrics()
{
    return CurrentCycle;
}

void UStudioDirectorSystem::LogAgentAction(int32 AgentID, const FString& Action, bool bSuccess)
{
    if (Action.Contains(TEXT("github_file_write")))
    {
        CurrentCycle.FilesCreated++;
    }
    else if (Action.Contains(TEXT("ue5_execute")))
    {
        CurrentCycle.UE5CommandsExecuted++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director: Agent #%d - %s (%s)"), 
           AgentID, *Action, bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
}

void UStudioDirectorSystem::CheckMilestone1Progress()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Check for character movement
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), Characters);
    bCharacterMovementComplete = Characters.Num() > 0;
    
    // Check for terrain
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Landscape"), Landscapes);
    bTerrainComplete = Landscapes.Num() > 0;
    
    // Check for dinosaur actors
    TArray<AActor*> Dinosaurs;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Dinosaur"), Dinosaurs);
    bDinosaurActorsComplete = Dinosaurs.Num() >= 3;
    
    // Check for lighting
    TArray<AActor*> Lights;
    UGameplayStatics::GetAllActorsOfClass(World, ALight::StaticClass(), Lights);
    bLightingComplete = Lights.Num() > 0;
}

bool UStudioDirectorSystem::IsMilestone1Complete()
{
    return bCharacterMovementComplete && bTerrainComplete && bDinosaurActorsComplete && bLightingComplete;
}

void UStudioDirectorSystem::AssignCriticalTasks()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Agent #5 - World Generation (Critical)
    CreateAgentTask(5, TEXT("World Generator"), 
                   TEXT("Create 10km2 landscape with 5 biomes - Swamp SW, Forest NW, Savanna center, Desert E, Mountains NE"),
                   EDir_AgentTaskPriority::Critical, FVector(0, 0, 0));
    
    // Agent #6 - Environment Art (High)
    CreateAgentTask(6, TEXT("Environment Artist"),
                   TEXT("Populate biomes with vegetation, rocks, and props using FBX pipeline"),
                   EDir_AgentTaskPriority::High, FVector(2000, 0, 0));
    
    // Agent #9 - Character Artist (Critical)
    CreateAgentTask(9, TEXT("Character Artist"),
                   TEXT("Create playable character with MetaHuman and 5 dinosaur actors with collision"),
                   EDir_AgentTaskPriority::Critical, FVector(0, 2000, 0));
    
    // Agent #10 - Animation (High)
    CreateAgentTask(10, TEXT("Animation Agent"),
                   TEXT("Implement character movement animations and dinosaur behavior animations"),
                   EDir_AgentTaskPriority::High, FVector(-2000, 0, 0));
    
    // Agent #12 - Combat & Survival UI (Critical)
    CreateAgentTask(12, TEXT("Combat & Survival UI"),
                   TEXT("Create survival HUD with health/hunger/thirst/stamina bars"),
                   EDir_AgentTaskPriority::Critical, FVector(0, -2000, 0));
    
    // Agent #8 - Lighting (Medium)
    CreateAgentTask(8, TEXT("Lighting & Atmosphere"),
                   TEXT("Implement Cretaceous atmosphere with tropical lighting and sky"),
                   EDir_AgentTaskPriority::Medium, FVector(1000, 1000, 0));
}

void UStudioDirectorSystem::ValidateAgentDependencies()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (!ValidateTaskDependencies(Task))
        {
            Task.Status = EDir_AgentTaskStatus::Blocked;
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Task for Agent #%d blocked due to dependencies"), Task.AgentID);
        }
    }
}

void UStudioDirectorSystem::InitializeAgentTasks()
{
    AssignCriticalTasks();
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initialized %d critical tasks"), AgentTasks.Num());
}

void UStudioDirectorSystem::UpdateCycleMetrics()
{
    // Update execution time
    CurrentCycle.TotalExecutionTime += GetWorld()->GetDeltaSeconds();
}

bool UStudioDirectorSystem::ValidateTaskDependencies(const FDir_AgentTask& Task)
{
    // Check if dependencies are completed
    for (const FString& Dependency : Task.Dependencies)
    {
        bool bDependencyMet = false;
        for (const FDir_AgentTask& OtherTask : AgentTasks)
        {
            if (OtherTask.AgentName == Dependency && OtherTask.Status == EDir_AgentTaskStatus::Completed)
            {
                bDependencyMet = true;
                break;
            }
        }
        
        if (!bDependencyMet)
        {
            return false;
        }
    }
    
    return true;
}