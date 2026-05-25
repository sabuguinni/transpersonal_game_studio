#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AStudioDirectorSystem::AStudioDirectorSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    TotalAgents = 19;
    CurrentCycleID = TEXT("");
}

void AStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentStatuses();
    SetupDefaultTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized with %d agents"), TotalAgents);
}

void AStudioDirectorSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update production metrics every frame
    float OverallProgress = GetOverallProgress();
    if (OverallProgress > 0.0f)
    {
        // Log progress periodically
        static float LastLogTime = 0.0f;
        LastLogTime += DeltaTime;
        if (LastLogTime > 5.0f) // Log every 5 seconds
        {
            UE_LOG(LogTemp, Log, TEXT("Production Progress: %.1f%%"), OverallProgress);
            LastLogTime = 0.0f;
        }
    }
}

void AStudioDirectorSystem::InitializeProductionCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    
    // Reset all tasks
    ActiveTasks.Empty();
    
    // Reset agent statuses
    for (FDir_AgentStatus& Status : AgentStatuses)
    {
        Status.CurrentTask = TEXT("Waiting for assignment");
        Status.CompletionPercentage = 0.0f;
        Status.bActive = false;
    }
    
    SetupDefaultTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle %s initialized"), *CycleID);
}

void AStudioDirectorSystem::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& Priority)
{
    // Create new task
    FDir_ProductionTask NewTask;
    NewTask.TaskName = TaskDescription;
    NewTask.AgentNumber = AgentNumber;
    NewTask.Priority = Priority;
    NewTask.bCompleted = false;
    
    ActiveTasks.Add(NewTask);
    
    // Update agent status
    UpdateAgentStatus(AgentNumber, TaskDescription, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Task assigned to Agent #%d: %s"), AgentNumber, *TaskDescription);
}

void AStudioDirectorSystem::UpdateAgentStatus(int32 AgentNumber, const FString& CurrentTask, float Completion)
{
    for (FDir_AgentStatus& Status : AgentStatuses)
    {
        if (Status.AgentNumber == AgentNumber)
        {
            Status.CurrentTask = CurrentTask;
            Status.CompletionPercentage = FMath::Clamp(Completion, 0.0f, 100.0f);
            Status.bActive = (Completion < 100.0f);
            break;
        }
    }
}

void AStudioDirectorSystem::CompleteTask(const FString& TaskName)
{
    for (FDir_ProductionTask& Task : ActiveTasks)
    {
        if (Task.TaskName == TaskName)
        {
            Task.bCompleted = true;
            UpdateAgentStatus(Task.AgentNumber, TEXT("Task completed"), 100.0f);
            UE_LOG(LogTemp, Warning, TEXT("Task completed: %s"), *TaskName);
            break;
        }
    }
}

TArray<FDir_ProductionTask> AStudioDirectorSystem::GetPendingTasks() const
{
    TArray<FDir_ProductionTask> PendingTasks;
    
    for (const FDir_ProductionTask& Task : ActiveTasks)
    {
        if (!Task.bCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    return PendingTasks;
}

FDir_AgentStatus AStudioDirectorSystem::GetAgentStatus(int32 AgentNumber) const
{
    for (const FDir_AgentStatus& Status : AgentStatuses)
    {
        if (Status.AgentNumber == AgentNumber)
        {
            return Status;
        }
    }
    
    return FDir_AgentStatus(); // Return default if not found
}

float AStudioDirectorSystem::GetOverallProgress() const
{
    if (ActiveTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    for (const FDir_ProductionTask& Task : ActiveTasks)
    {
        if (Task.bCompleted)
        {
            CompletedTasks++;
        }
    }
    
    return (float)CompletedTasks / (float)ActiveTasks.Num() * 100.0f;
}

void AStudioDirectorSystem::SpawnDinosaursInBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("Spawning dinosaurs in biomes - Studio Director override"));
    
    // This function can be called from Blueprint or editor
    // Implementation would spawn dinosaurs using UE5 Python bridge
}

void AStudioDirectorSystem::ValidateMinPlayableMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found"));
        return;
    }
    
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap validation: %d actors found"), ActorCount);
}

void AStudioDirectorSystem::SetupDefaultTasks()
{
    // Priority tasks for playable prototype
    AssignTaskToAgent(2, TEXT("Define core architecture and compilation rules"), TEXT("Critical"));
    AssignTaskToAgent(3, TEXT("Implement character movement and physics"), TEXT("Critical"));
    AssignTaskToAgent(5, TEXT("Generate terrain with height variation"), TEXT("High"));
    AssignTaskToAgent(9, TEXT("Create playable character with survival stats"), TEXT("Critical"));
    AssignTaskToAgent(10, TEXT("Add character animations and IK"), TEXT("High"));
    AssignTaskToAgent(12, TEXT("Implement basic dinosaur AI and combat"), TEXT("High"));
    AssignTaskToAgent(15, TEXT("Write survival game narrative framework"), TEXT("Medium"));
    AssignTaskToAgent(16, TEXT("Add ambient sounds and music"), TEXT("Medium"));
    AssignTaskToAgent(18, TEXT("Test playable prototype"), TEXT("Critical"));
}

void AStudioDirectorSystem::InitializeAgentStatuses()
{
    AgentStatuses.Empty();
    
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),
        TEXT("Engine Architect"),
        TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"),
        TEXT("Procedural World Generator"),
        TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"),
        TEXT("Lighting & Atmosphere Agent"),
        TEXT("Character Artist Agent"),
        TEXT("Animation Agent"),
        TEXT("NPC Behavior Agent"),
        TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"),
        TEXT("Quest & Mission Designer"),
        TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"),
        TEXT("VFX Agent"),
        TEXT("QA & Testing Agent"),
        TEXT("Integration & Build Agent")
    };
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentStatus Status;
        Status.AgentNumber = i + 1;
        Status.AgentName = AgentNames[i];
        Status.CurrentTask = TEXT("Waiting for assignment");
        Status.CompletionPercentage = 0.0f;
        Status.bActive = false;
        
        AgentStatuses.Add(Status);
    }
}