#include "Dir_AgentCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "EngineUtils.h"

UDir_AgentCoordinator::UDir_AgentCoordinator()
{
    CurrentPhase = EDir_ProductionPhase::PrototypeDevelopment;
    InitializeAgentDependencies();
}

void UDir_AgentCoordinator::InitializeProductionCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing Production Cycle"));
    
    ActiveTasks.Empty();
    
    // Critical Milestone 1 tasks - WALK AROUND prototype
    AssignTaskToAgent(EDir_AgentType::CoreSystems, TEXT("Implement ThirdPersonCharacter with WASD movement"), EDir_TaskPriority::Critical);
    AssignTaskToAgent(EDir_AgentType::WorldGenerator, TEXT("Create landscape with height variation"), EDir_TaskPriority::Critical);
    AssignTaskToAgent(EDir_AgentType::CharacterArtist, TEXT("Setup player character with camera boom"), EDir_TaskPriority::Critical);
    AssignTaskToAgent(EDir_AgentType::EnvironmentArtist, TEXT("Place 3-5 static dinosaur meshes in world"), EDir_TaskPriority::Critical);
    AssignTaskToAgent(EDir_AgentType::Lighting, TEXT("Configure directional light + sky atmosphere"), EDir_TaskPriority::Critical);
    
    // Secondary tasks for enhanced gameplay
    AssignTaskToAgent(EDir_AgentType::Animation, TEXT("Setup basic character animations (walk, run, jump)"), EDir_TaskPriority::High);
    AssignTaskToAgent(EDir_AgentType::NPCBehavior, TEXT("Add basic dinosaur collision and visibility"), EDir_TaskPriority::High);
    AssignTaskToAgent(EDir_AgentType::Performance, TEXT("Ensure 60fps on PC with current actor count"), EDir_TaskPriority::High);
    
    UpdateProductionMetrics();
}

void UDir_AgentCoordinator::AssignTaskToAgent(EDir_AgentType Agent, const FString& TaskDescription, EDir_TaskPriority Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AssignedAgent = Agent;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;
    
    // Set dependencies based on agent type
    switch(Agent)
    {
        case EDir_AgentType::CharacterArtist:
            NewTask.Dependencies.Add(EDir_AgentType::CoreSystems);
            break;
        case EDir_AgentType::Animation:
            NewTask.Dependencies.Add(EDir_AgentType::CharacterArtist);
            break;
        case EDir_AgentType::EnvironmentArtist:
            NewTask.Dependencies.Add(EDir_AgentType::WorldGenerator);
            break;
        case EDir_AgentType::Lighting:
            NewTask.Dependencies.Add(EDir_AgentType::EnvironmentArtist);
            break;
        default:
            // No dependencies for core systems
            break;
    }
    
    ActiveTasks.Add(NewTask);
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent %d: %s"), (int32)Agent, *TaskDescription);
}

void UDir_AgentCoordinator::MarkTaskCompleted(EDir_AgentType Agent, const FString& TaskDescription)
{
    for(FDir_AgentTask& Task : ActiveTasks)
    {
        if(Task.AssignedAgent == Agent && Task.TaskDescription.Contains(TaskDescription))
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Task completed by Agent %d: %s"), (int32)Agent, *TaskDescription);
            break;
        }
    }
    
    UpdateProductionMetrics();
}

FDir_ProductionMetrics UDir_AgentCoordinator::GetCurrentMetrics()
{
    UpdateProductionMetrics();
    return CurrentMetrics;
}

TArray<FDir_AgentTask> UDir_AgentCoordinator::GetPendingTasksForAgent(EDir_AgentType Agent)
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for(const FDir_AgentTask& Task : ActiveTasks)
    {
        if(Task.AssignedAgent == Agent && !Task.bIsCompleted)
        {
            // Check if dependencies are met
            if(ValidateAgentDependencies(Agent))
            {
                PendingTasks.Add(Task);
            }
        }
    }
    
    return PendingTasks;
}

void UDir_AgentCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Production phase changed to: %d"), (int32)NewPhase);
}

bool UDir_AgentCoordinator::ValidateAgentDependencies(EDir_AgentType Agent)
{
    // Find tasks for this agent
    for(const FDir_AgentTask& Task : ActiveTasks)
    {
        if(Task.AssignedAgent == Agent && !Task.bIsCompleted)
        {
            // Check if all dependencies are completed
            for(EDir_AgentType Dependency : Task.Dependencies)
            {
                bool bDependencyMet = false;
                for(const FDir_AgentTask& DepTask : ActiveTasks)
                {
                    if(DepTask.AssignedAgent == Dependency && DepTask.bIsCompleted)
                    {
                        bDependencyMet = true;
                        break;
                    }
                }
                
                if(!bDependencyMet)
                {
                    return false;
                }
            }
        }
    }
    
    return true;
}

void UDir_AgentCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), CurrentMetrics.CharacterCount);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d/%d"), CurrentMetrics.CompletedTasks, ActiveTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    
    // List pending critical tasks
    UE_LOG(LogTemp, Warning, TEXT("CRITICAL PENDING TASKS:"));
    for(const FDir_AgentTask& Task : ActiveTasks)
    {
        if(!Task.bIsCompleted && Task.Priority == EDir_TaskPriority::Critical)
        {
            UE_LOG(LogTemp, Warning, TEXT("- Agent %d: %s"), (int32)Task.AssignedAgent, *Task.TaskDescription);
        }
    }
}

TArray<EDir_AgentType> UDir_AgentCoordinator::GetCriticalPathAgents()
{
    TArray<EDir_AgentType> CriticalAgents;
    
    // For Milestone 1, critical path is:
    CriticalAgents.Add(EDir_AgentType::CoreSystems);      // Character movement
    CriticalAgents.Add(EDir_AgentType::WorldGenerator);   // Terrain
    CriticalAgents.Add(EDir_AgentType::CharacterArtist);  // Player setup
    CriticalAgents.Add(EDir_AgentType::EnvironmentArtist); // Dinosaur placement
    CriticalAgents.Add(EDir_AgentType::Lighting);         // Visual completion
    
    return CriticalAgents;
}

void UDir_AgentCoordinator::UpdateProductionMetrics()
{
    // Count completed tasks
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.PendingTasks = 0;
    
    for(const FDir_AgentTask& Task : ActiveTasks)
    {
        if(Task.bIsCompleted)
        {
            CurrentMetrics.CompletedTasks++;
        }
        else
        {
            CurrentMetrics.PendingTasks++;
        }
    }
    
    // Calculate progress
    if(ActiveTasks.Num() > 0)
    {
        CurrentMetrics.OverallProgress = (float)CurrentMetrics.CompletedTasks / (float)ActiveTasks.Num() * 100.0f;
    }
    
    // Get level metrics if world exists
    if(UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentMetrics.TotalActorsInLevel = AllActors.Num();
        
        // Count specific types
        CurrentMetrics.DinosaurCount = 0;
        CurrentMetrics.CharacterCount = 0;
        CurrentMetrics.TerrainActorCount = 0;
        
        for(AActor* Actor : AllActors)
        {
            FString ActorName = Actor->GetName().ToLower();
            if(ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("trex")) || 
               ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachi")))
            {
                CurrentMetrics.DinosaurCount++;
            }
            else if(ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
            {
                CurrentMetrics.CharacterCount++;
            }
            else if(ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
            {
                CurrentMetrics.TerrainActorCount++;
            }
        }
    }
}

void UDir_AgentCoordinator::InitializeAgentDependencies()
{
    // Initialize all agents as not ready
    for(int32 i = 1; i <= 19; i++)
    {
        AgentReadyStatus.Add((EDir_AgentType)i, false);
    }
    
    // Studio Director is always ready
    AgentReadyStatus[EDir_AgentType::StudioDirector] = true;
}

void UDir_AgentCoordinator::CalculateOverallProgress()
{
    if(ActiveTasks.Num() == 0)
    {
        CurrentMetrics.OverallProgress = 0.0f;
        return;
    }
    
    float TotalWeight = 0.0f;
    float CompletedWeight = 0.0f;
    
    for(const FDir_AgentTask& Task : ActiveTasks)
    {
        float TaskWeight = 1.0f;
        
        // Critical tasks have higher weight
        if(Task.Priority == EDir_TaskPriority::Critical)
        {
            TaskWeight = 3.0f;
        }
        else if(Task.Priority == EDir_TaskPriority::High)
        {
            TaskWeight = 2.0f;
        }
        
        TotalWeight += TaskWeight;
        
        if(Task.bIsCompleted)
        {
            CompletedWeight += TaskWeight;
        }
    }
    
    CurrentMetrics.OverallProgress = (CompletedWeight / TotalWeight) * 100.0f;
}

bool UDir_AgentCoordinator::CheckAgentPrerequisites(EDir_AgentType Agent)
{
    // Define prerequisite chains
    switch(Agent)
    {
        case EDir_AgentType::CharacterArtist:
            return AgentReadyStatus[EDir_AgentType::CoreSystems];
        case EDir_AgentType::Animation:
            return AgentReadyStatus[EDir_AgentType::CharacterArtist];
        case EDir_AgentType::EnvironmentArtist:
            return AgentReadyStatus[EDir_AgentType::WorldGenerator];
        case EDir_AgentType::Lighting:
            return AgentReadyStatus[EDir_AgentType::EnvironmentArtist];
        case EDir_AgentType::NPCBehavior:
            return AgentReadyStatus[EDir_AgentType::CharacterArtist] && AgentReadyStatus[EDir_AgentType::EnvironmentArtist];
        default:
            return true; // No prerequisites for core agents
    }
}