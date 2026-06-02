#include "Dir_ProductionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"

UDir_ProductionManager::UDir_ProductionManager()
{
    CurrentPhase = EDir_ProductionPhase::Production;
    InitializeAgentTasks();
}

void UDir_ProductionManager::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Production Phase changed to: %d"), (int32)NewPhase);
    
    // Adjust agent priorities based on phase
    switch (NewPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            // Focus on core systems and architecture
            for (auto& Task : AgentTasks)
            {
                if (Task.AgentNumber <= 4) // Engine Architect, Core Systems, Performance
                {
                    Task.Priority = EDir_AgentPriority::Critical;
                }
            }
            break;
            
        case EDir_ProductionPhase::Production:
            // Focus on content creation
            for (auto& Task : AgentTasks)
            {
                if (Task.AgentNumber >= 5 && Task.AgentNumber <= 12) // World, Environment, Characters, AI
                {
                    Task.Priority = EDir_AgentPriority::High;
                }
            }
            break;
            
        case EDir_ProductionPhase::Alpha:
            // Focus on polish and QA
            for (auto& Task : AgentTasks)
            {
                if (Task.AgentNumber >= 16) // Audio, VFX, QA, Integration
                {
                    Task.Priority = EDir_AgentPriority::Critical;
                }
            }
            break;
    }
}

void UDir_ProductionManager::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_AgentPriority Priority, float EstimatedHours)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.EstimatedHours = EstimatedHours;
    NewTask.bCompleted = false;
    
    // Set agent name based on number
    switch (AgentNumber)
    {
        case 1: NewTask.AgentName = TEXT("Studio Director"); break;
        case 2: NewTask.AgentName = TEXT("Engine Architect"); break;
        case 3: NewTask.AgentName = TEXT("Core Systems Programmer"); break;
        case 4: NewTask.AgentName = TEXT("Performance Optimizer"); break;
        case 5: NewTask.AgentName = TEXT("Procedural World Generator"); break;
        case 6: NewTask.AgentName = TEXT("Environment Artist"); break;
        case 7: NewTask.AgentName = TEXT("Architecture & Interior Agent"); break;
        case 8: NewTask.AgentName = TEXT("Lighting & Atmosphere Agent"); break;
        case 9: NewTask.AgentName = TEXT("Character Artist Agent"); break;
        case 10: NewTask.AgentName = TEXT("Animation Agent"); break;
        case 11: NewTask.AgentName = TEXT("NPC Behavior Agent"); break;
        case 12: NewTask.AgentName = TEXT("Combat & Enemy AI Agent"); break;
        case 13: NewTask.AgentName = TEXT("Crowd & Traffic Simulation"); break;
        case 14: NewTask.AgentName = TEXT("Quest & Mission Designer"); break;
        case 15: NewTask.AgentName = TEXT("Narrative & Dialogue Agent"); break;
        case 16: NewTask.AgentName = TEXT("Audio Agent"); break;
        case 17: NewTask.AgentName = TEXT("VFX Agent"); break;
        case 18: NewTask.AgentName = TEXT("QA & Testing Agent"); break;
        case 19: NewTask.AgentName = TEXT("Integration & Build Agent"); break;
        default: NewTask.AgentName = TEXT("Unknown Agent"); break;
    }
    
    AgentTasks.Add(NewTask);
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent %d: %s"), AgentNumber, *TaskDescription);
}

void UDir_ProductionManager::CompleteAgentTask(int32 AgentNumber)
{
    for (auto& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && !Task.bCompleted)
        {
            Task.bCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Task completed for Agent %d: %s"), AgentNumber, *Task.TaskDescription);
            break;
        }
    }
    
    UpdateMetrics();
}

TArray<FDir_AgentTask> UDir_ProductionManager::GetPendingTasks()
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const auto& Task : AgentTasks)
    {
        if (!Task.bCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    // Sort by priority
    PendingTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return (int32)A.Priority < (int32)B.Priority;
    });
    
    return PendingTasks;
}

TArray<FDir_AgentTask> UDir_ProductionManager::GetTasksForAgent(int32 AgentNumber)
{
    TArray<FDir_AgentTask> AgentSpecificTasks;
    
    for (const auto& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            AgentSpecificTasks.Add(Task);
        }
    }
    
    return AgentSpecificTasks;
}

FDir_ProductionMetrics UDir_ProductionManager::CalculateProductionMetrics()
{
    FDir_ProductionMetrics Metrics;
    
    // Count actors in the world
    if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        Metrics.TotalActors = 0;
        Metrics.DinosaurCount = 0;
        Metrics.CharacterCount = 0;
        Metrics.TerrainActors = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                Metrics.TotalActors++;
                
                FString ActorName = Actor->GetName().ToLower();
                
                if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
                    ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
                    ActorName.Contains(TEXT("ankylo")) || ActorName.Contains(TEXT("parasauro")))
                {
                    Metrics.DinosaurCount++;
                }
                else if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
                {
                    Metrics.CharacterCount++;
                }
                else if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
                {
                    Metrics.TerrainActors++;
                }
            }
        }
    }
    
    // Calculate completion percentage
    int32 CompletedTasks = 0;
    int32 TotalTasks = AgentTasks.Num();
    
    for (const auto& Task : AgentTasks)
    {
        if (Task.bCompleted)
        {
            CompletedTasks++;
        }
    }
    
    Metrics.CompletionPercentage = TotalTasks > 0 ? (float)CompletedTasks / TotalTasks * 100.0f : 0.0f;
    
    // Count critical issues
    Metrics.CriticalIssues = 0;
    if (Metrics.TotalActors > 8000) Metrics.CriticalIssues++;
    if (Metrics.DinosaurCount > 150) Metrics.CriticalIssues++;
    if (Metrics.DinosaurCount < 10) Metrics.CriticalIssues++;
    
    return Metrics;
}

void UDir_ProductionManager::UpdateMetrics()
{
    CurrentMetrics = CalculateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Metrics Updated:"));
    UE_LOG(LogTemp, Warning, TEXT("  Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("  Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("  Characters: %d"), CurrentMetrics.CharacterCount);
    UE_LOG(LogTemp, Warning, TEXT("  Completion: %.1f%%"), CurrentMetrics.CompletionPercentage);
    UE_LOG(LogTemp, Warning, TEXT("  Critical Issues: %d"), CurrentMetrics.CriticalIssues);
}

TArray<int32> UDir_ProductionManager::GetCriticalPathAgents()
{
    TArray<int32> CriticalAgents;
    
    // Based on current production phase and dependencies
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CriticalAgents = {2, 3, 4}; // Engine Architect, Core Systems, Performance
            break;
            
        case EDir_ProductionPhase::Production:
            CriticalAgents = {5, 9, 12}; // World Generator, Character Artist, Combat AI
            break;
            
        case EDir_ProductionPhase::Alpha:
            CriticalAgents = {18, 19}; // QA, Integration
            break;
            
        case EDir_ProductionPhase::Beta:
            CriticalAgents = {16, 17, 18}; // Audio, VFX, QA
            break;
            
        case EDir_ProductionPhase::Release:
            CriticalAgents = {19}; // Integration & Build
            break;
    }
    
    return CriticalAgents;
}

bool UDir_ProductionManager::IsAgentBlocked(int32 AgentNumber)
{
    for (const auto& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && Task.Priority == EDir_AgentPriority::Blocked)
        {
            return true;
        }
    }
    
    return false;
}

void UDir_ProductionManager::SetMilestone(const FString& MilestoneName, float TargetCompletion)
{
    Milestones.Add(MilestoneName, TargetCompletion);
    UE_LOG(LogTemp, Warning, TEXT("Milestone set: %s (%.1f%%)"), *MilestoneName, TargetCompletion);
}

float UDir_ProductionManager::GetMilestoneProgress(const FString& MilestoneName)
{
    if (Milestones.Contains(MilestoneName))
    {
        return Milestones[MilestoneName];
    }
    
    return 0.0f;
}

void UDir_ProductionManager::InitializeAgentTasks()
{
    // Initialize critical tasks for current production cycle
    AssignTaskToAgent(5, TEXT("Create realistic terrain with height variation"), EDir_AgentPriority::Critical, 4.0f);
    AssignTaskToAgent(9, TEXT("Spawn dinosaur actors with collision and basic AI"), EDir_AgentPriority::Critical, 6.0f);
    AssignTaskToAgent(12, TEXT("Implement survival HUD with health/hunger bars"), EDir_AgentPriority::High, 3.0f);
    AssignTaskToAgent(3, TEXT("Ensure character movement and physics work correctly"), EDir_AgentPriority::High, 2.0f);
    AssignTaskToAgent(8, TEXT("Set up proper lighting and atmosphere"), EDir_AgentPriority::Medium, 2.0f);
    AssignTaskToAgent(6, TEXT("Add vegetation and environmental props"), EDir_AgentPriority::Medium, 4.0f);
    AssignTaskToAgent(18, TEXT("Test playable prototype functionality"), EDir_AgentPriority::High, 3.0f);
    AssignTaskToAgent(19, TEXT("Integrate all systems into cohesive build"), EDir_AgentPriority::High, 4.0f);
    
    // Set milestones
    SetMilestone(TEXT("Playable Prototype"), 25.0f);
    SetMilestone(TEXT("Alpha Build"), 60.0f);
    SetMilestone(TEXT("Beta Build"), 85.0f);
    SetMilestone(TEXT("Release Candidate"), 100.0f);
}

void UDir_ProductionManager::ValidateTaskDependencies()
{
    // Ensure task dependencies are logical
    for (auto& Task : AgentTasks)
    {
        for (int32 Dependency : Task.Dependencies)
        {
            bool bDependencyExists = false;
            for (const auto& OtherTask : AgentTasks)
            {
                if (OtherTask.AgentNumber == Dependency)
                {
                    bDependencyExists = true;
                    break;
                }
            }
            
            if (!bDependencyExists)
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid dependency: Agent %d depends on non-existent Agent %d"), 
                       Task.AgentNumber, Dependency);
            }
        }
    }
}

int32 UDir_ProductionManager::CountActorsByType(const FString& TypeKeyword)
{
    int32 Count = 0;
    
    if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetName().ToLower().Contains(TypeKeyword.ToLower()))
            {
                Count++;
            }
        }
    }
    
    return Count;
}