#include "Dir_CycleCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UDir_CycleCoordinator::UDir_CycleCoordinator()
{
    InitializeAgentNames();
}

void UDir_CycleCoordinator::InitializeAgentNames()
{
    AgentNames.Add(1, TEXT("Studio Director"));
    AgentNames.Add(2, TEXT("Engine Architect"));
    AgentNames.Add(3, TEXT("Core Systems Programmer"));
    AgentNames.Add(4, TEXT("Performance Optimizer"));
    AgentNames.Add(5, TEXT("Procedural World Generator"));
    AgentNames.Add(6, TEXT("Environment Artist"));
    AgentNames.Add(7, TEXT("Architecture & Interior Agent"));
    AgentNames.Add(8, TEXT("Lighting & Atmosphere Agent"));
    AgentNames.Add(9, TEXT("Character Artist Agent"));
    AgentNames.Add(10, TEXT("Animation Agent"));
    AgentNames.Add(11, TEXT("NPC Behavior Agent"));
    AgentNames.Add(12, TEXT("Combat & Enemy AI Agent"));
    AgentNames.Add(13, TEXT("Crowd & Traffic Simulation"));
    AgentNames.Add(14, TEXT("Quest & Mission Designer"));
    AgentNames.Add(15, TEXT("Narrative & Dialogue Agent"));
    AgentNames.Add(16, TEXT("Audio Agent"));
    AgentNames.Add(17, TEXT("VFX Agent"));
    AgentNames.Add(18, TEXT("QA & Testing Agent"));
    AgentNames.Add(19, TEXT("Integration & Build Agent"));
}

void UDir_CycleCoordinator::InitializeCycle(const FString& CycleID)
{
    CurrentCycle.CycleID = CycleID;
    AgentTasks.Empty();
    
    // Update current map status
    UpdateMapStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("Director: Initialized cycle %s"), *CycleID);
}

void UDir_CycleCoordinator::AssignAgentTask(int32 AgentNumber, const FString& TaskDescription, const FString& Priority, const TArray<int32>& Dependencies)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.AgentName = AgentNames.Contains(AgentNumber) ? AgentNames[AgentNumber] : FString::Printf(TEXT("Agent %d"), AgentNumber);
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Dependencies = Dependencies;
    NewTask.bCompleted = false;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Director: Assigned task to Agent %d (%s): %s"), AgentNumber, *NewTask.AgentName, *TaskDescription);
}

void UDir_CycleCoordinator::CompleteAgentTask(int32 AgentNumber, const FString& DeliverableDescription)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && !Task.bCompleted)
        {
            Task.bCompleted = true;
            Task.ExpectedDeliverable = DeliverableDescription;
            UE_LOG(LogTemp, Warning, TEXT("Director: Agent %d completed task: %s"), AgentNumber, *DeliverableDescription);
            break;
        }
    }
}

FDir_CycleStatus UDir_CycleCoordinator::GetCurrentCycleStatus() const
{
    return CurrentCycle;
}

TArray<FDir_AgentTask> UDir_CycleCoordinator::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    return PendingTasks;
}

TArray<FDir_AgentTask> UDir_CycleCoordinator::GetReadyTasks() const
{
    TArray<FDir_AgentTask> ReadyTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bCompleted && CheckDependenciesComplete(Task.Dependencies))
        {
            ReadyTasks.Add(Task);
        }
    }
    
    return ReadyTasks;
}

bool UDir_CycleCoordinator::IsAgentReady(int32 AgentNumber) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && !Task.bCompleted)
        {
            return CheckDependenciesComplete(Task.Dependencies);
        }
    }
    
    return true; // No pending tasks for this agent
}

void UDir_CycleCoordinator::UpdateMapStatus()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }
    
    UpdateActorCounts();
    CurrentCycle.CriticalBlockers = GetCriticalBlockers();
}

void UDir_CycleCoordinator::UpdateActorCounts()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }
    
    CurrentCycle.TotalActors = 0;
    CurrentCycle.DinosaurCount = 0;
    CurrentCycle.CharacterCount = 0;
    CurrentCycle.EnvironmentProps = 0;
    CurrentCycle.bHasPlayableCharacter = false;
    CurrentCycle.bHasTerrain = false;
    CurrentCycle.bHasLighting = false;
    
    // Count actors by type
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }
        
        CurrentCycle.TotalActors++;
        
        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Check for dinosaurs
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
            ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
            ActorLabel.Contains(TEXT("dino")))
        {
            CurrentCycle.DinosaurCount++;
        }
        
        // Check for characters
        if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
        {
            CurrentCycle.CharacterCount++;
            CurrentCycle.bHasPlayableCharacter = true;
        }
        
        // Check for environment props
        if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("rock")) || 
            ActorName.Contains(TEXT("bush")) || ActorName.Contains(TEXT("grass")))
        {
            CurrentCycle.EnvironmentProps++;
        }
        
        // Check for terrain
        if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
        {
            CurrentCycle.bHasTerrain = true;
        }
        
        // Check for lighting
        if (ActorName.Contains(TEXT("light")) || ActorName.Contains(TEXT("sun")))
        {
            CurrentCycle.bHasLighting = true;
        }
    }
}

FString UDir_CycleCoordinator::GenerateNextCycleDirective() const
{
    FString Directive = TEXT("NEXT CYCLE PRIORITIES:\n");
    
    if (!CurrentCycle.bHasPlayableCharacter)
    {
        Directive += TEXT("- CRITICAL: Agent #9 must create playable character with movement\n");
    }
    
    if (!CurrentCycle.bHasTerrain)
    {
        Directive += TEXT("- CRITICAL: Agent #5 must generate terrain landscape\n");
    }
    
    if (CurrentCycle.DinosaurCount < 5)
    {
        Directive += TEXT("- HIGH: Agent #12 must spawn dinosaur actors with basic AI\n");
    }
    
    if (!CurrentCycle.bHasLighting)
    {
        Directive += TEXT("- HIGH: Agent #8 must set up lighting and atmosphere\n");
    }
    
    if (CurrentCycle.EnvironmentProps < 50)
    {
        Directive += TEXT("- MEDIUM: Agent #6 must populate environment with props\n");
    }
    
    return Directive;
}

bool UDir_CycleCoordinator::IsMilestone1Complete() const
{
    return CurrentCycle.bHasPlayableCharacter && 
           CurrentCycle.bHasTerrain && 
           CurrentCycle.bHasLighting && 
           CurrentCycle.DinosaurCount >= 3 &&
           CurrentCycle.EnvironmentProps >= 20;
}

TArray<FString> UDir_CycleCoordinator::GetMilestone1Requirements() const
{
    TArray<FString> Requirements;
    
    if (!CurrentCycle.bHasPlayableCharacter)
    {
        Requirements.Add(TEXT("Playable character with WASD movement"));
    }
    
    if (!CurrentCycle.bHasTerrain)
    {
        Requirements.Add(TEXT("Landscape terrain with height variation"));
    }
    
    if (!CurrentCycle.bHasLighting)
    {
        Requirements.Add(TEXT("Directional light and sky atmosphere"));
    }
    
    if (CurrentCycle.DinosaurCount < 3)
    {
        Requirements.Add(FString::Printf(TEXT("At least 3 dinosaur actors (%d/3)"), CurrentCycle.DinosaurCount));
    }
    
    if (CurrentCycle.EnvironmentProps < 20)
    {
        Requirements.Add(FString::Printf(TEXT("At least 20 environment props (%d/20)"), CurrentCycle.EnvironmentProps));
    }
    
    return Requirements;
}

float UDir_CycleCoordinator::GetOverallProgress() const
{
    int32 CompletedTasks = 0;
    int32 TotalTasks = AgentTasks.Num();
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bCompleted)
        {
            CompletedTasks++;
        }
    }
    
    return TotalTasks > 0 ? (float)CompletedTasks / (float)TotalTasks : 0.0f;
}

bool UDir_CycleCoordinator::CheckDependenciesComplete(const TArray<int32>& Dependencies) const
{
    for (int32 DependencyAgent : Dependencies)
    {
        bool bDependencyComplete = false;
        
        for (const FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.AgentNumber == DependencyAgent && Task.bCompleted)
            {
                bDependencyComplete = true;
                break;
            }
        }
        
        if (!bDependencyComplete)
        {
            return false;
        }
    }
    
    return true;
}

FString UDir_CycleCoordinator::GetCriticalBlockers() const
{
    TArray<FString> Blockers;
    
    if (CurrentCycle.TotalActors > 8000)
    {
        Blockers.Add(TEXT("Actor count exceeds 8000 limit"));
    }
    
    if (CurrentCycle.DinosaurCount > 150)
    {
        Blockers.Add(TEXT("Dinosaur count exceeds 150 limit"));
    }
    
    if (!CurrentCycle.bHasPlayableCharacter)
    {
        Blockers.Add(TEXT("No playable character found"));
    }
    
    return FString::Join(Blockers, TEXT("; "));
}