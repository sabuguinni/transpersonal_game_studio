#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentPhase = EDir_ProductionPhase::Prototype;
    LastMetricsUpdate = 0.0f;
    
    // Initialize agent status tracking
    AgentStatus.Add(EDir_AgentType::StudioDirector, false);
    AgentStatus.Add(EDir_AgentType::EngineArchitect, false);
    AgentStatus.Add(EDir_AgentType::CoreSystems, false);
    AgentStatus.Add(EDir_AgentType::Performance, false);
    AgentStatus.Add(EDir_AgentType::WorldGenerator, false);
    AgentStatus.Add(EDir_AgentType::Environment, false);
    AgentStatus.Add(EDir_AgentType::Architecture, false);
    AgentStatus.Add(EDir_AgentType::Lighting, false);
    AgentStatus.Add(EDir_AgentType::Character, false);
    AgentStatus.Add(EDir_AgentType::Animation, false);
    AgentStatus.Add(EDir_AgentType::NPCBehavior, false);
    AgentStatus.Add(EDir_AgentType::CombatAI, false);
    AgentStatus.Add(EDir_AgentType::CrowdSimulation, false);
    AgentStatus.Add(EDir_AgentType::QuestDesign, false);
    AgentStatus.Add(EDir_AgentType::Narrative, false);
    AgentStatus.Add(EDir_AgentType::Audio, false);
    AgentStatus.Add(EDir_AgentType::VFX, false);
    AgentStatus.Add(EDir_AgentType::QA, false);
    AgentStatus.Add(EDir_AgentType::Integration, false);
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentDependencies();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized"));
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastMetricsUpdate += DeltaTime;
    
    // Update metrics every 5 seconds
    if (LastMetricsUpdate >= 5.0f)
    {
        UpdateProductionMetrics();
        LastMetricsUpdate = 0.0f;
    }
    
    // Update task priorities based on current state
    UpdateTaskPriorities();
}

void ADir_ProductionCoordinator::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count all actors in the world
    CurrentMetrics.TotalActors = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.CharacterCount = 0;
    CurrentMetrics.EnvironmentProps = 0;
    
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && IsValid(Actor))
        {
            CurrentMetrics.TotalActors++;
            
            FString ActorName = Actor->GetName().ToLower();
            
            // Count dinosaurs
            if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) ||
                ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
                ActorName.Contains(TEXT("ankylo")) || ActorName.Contains(TEXT("parasauro")))
            {
                CurrentMetrics.DinosaurCount++;
            }
            // Count characters
            else if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
            {
                CurrentMetrics.CharacterCount++;
            }
            // Count environment props
            else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("rock")) ||
                     ActorName.Contains(TEXT("bush")) || ActorName.Contains(TEXT("grass")))
            {
                CurrentMetrics.EnvironmentProps++;
            }
        }
    }
    
    // Calculate production progress
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.PendingTasks = TaskQueue.Num();
    
    for (const FDir_AgentTask& Task : TaskQueue)
    {
        if (Task.bIsCompleted)
        {
            CurrentMetrics.CompletedTasks++;
            CurrentMetrics.PendingTasks--;
        }
    }
    
    if (TaskQueue.Num() > 0)
    {
        CurrentMetrics.ProductionProgress = (float)CurrentMetrics.CompletedTasks / (float)TaskQueue.Num();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Production Metrics - Actors: %d, Dinosaurs: %d, Progress: %.2f%%"), 
           CurrentMetrics.TotalActors, CurrentMetrics.DinosaurCount, CurrentMetrics.ProductionProgress * 100.0f);
}

void ADir_ProductionCoordinator::AssignTaskToAgent(EDir_AgentType AgentType, const FString& TaskDescription, int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentType = AgentType;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;
    
    // Add dependencies based on agent type
    switch (AgentType)
    {
        case EDir_AgentType::CoreSystems:
            NewTask.Dependencies.Add(EDir_AgentType::EngineArchitect);
            break;
        case EDir_AgentType::WorldGenerator:
            NewTask.Dependencies.Add(EDir_AgentType::CoreSystems);
            break;
        case EDir_AgentType::Environment:
            NewTask.Dependencies.Add(EDir_AgentType::WorldGenerator);
            break;
        case EDir_AgentType::Character:
            NewTask.Dependencies.Add(EDir_AgentType::CoreSystems);
            break;
        case EDir_AgentType::Animation:
            NewTask.Dependencies.Add(EDir_AgentType::Character);
            break;
        case EDir_AgentType::NPCBehavior:
            NewTask.Dependencies.Add(EDir_AgentType::Animation);
            break;
        case EDir_AgentType::CombatAI:
            NewTask.Dependencies.Add(EDir_AgentType::NPCBehavior);
            break;
        default:
            break;
    }
    
    TaskQueue.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent %d: %s"), 
           (int32)AgentType, *TaskDescription);
}

void ADir_ProductionCoordinator::CompleteAgentTask(EDir_AgentType AgentType)
{
    for (FDir_AgentTask& Task : TaskQueue)
    {
        if (Task.AgentType == AgentType && !Task.bIsCompleted)
        {
            Task.bIsCompleted = true;
            AgentStatus[AgentType] = true;
            
            UE_LOG(LogTemp, Warning, TEXT("Task completed by Agent %d: %s"), 
                   (int32)AgentType, *Task.TaskDescription);
            break;
        }
    }
}

bool ADir_ProductionCoordinator::CanAgentProceed(EDir_AgentType AgentType) const
{
    for (const FDir_AgentTask& Task : TaskQueue)
    {
        if (Task.AgentType == AgentType && !Task.bIsCompleted)
        {
            return CheckDependenciesComplete(Task.Dependencies);
        }
    }
    
    return true; // No pending tasks, can proceed
}

float ADir_ProductionCoordinator::GetOverallProgress() const
{
    return CurrentMetrics.ProductionProgress;
}

void ADir_ProductionCoordinator::AdvanceProductionPhase()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::Prototype;
            break;
        case EDir_ProductionPhase::Prototype:
            if (IsMilestone1Complete())
            {
                CurrentPhase = EDir_ProductionPhase::Production;
            }
            break;
        case EDir_ProductionPhase::Production:
            CurrentPhase = EDir_ProductionPhase::Alpha;
            break;
        case EDir_ProductionPhase::Alpha:
            CurrentPhase = EDir_ProductionPhase::Beta;
            break;
        case EDir_ProductionPhase::Beta:
            CurrentPhase = EDir_ProductionPhase::Release;
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production phase advanced to: %d"), (int32)CurrentPhase);
}

TArray<EDir_AgentType> ADir_ProductionCoordinator::GetCriticalPath() const
{
    TArray<EDir_AgentType> CriticalPath;
    
    // Define critical path for Milestone 1 (Walk Around)
    CriticalPath.Add(EDir_AgentType::EngineArchitect);
    CriticalPath.Add(EDir_AgentType::CoreSystems);
    CriticalPath.Add(EDir_AgentType::Character);
    CriticalPath.Add(EDir_AgentType::Animation);
    CriticalPath.Add(EDir_AgentType::WorldGenerator);
    CriticalPath.Add(EDir_AgentType::Environment);
    CriticalPath.Add(EDir_AgentType::Lighting);
    
    return CriticalPath;
}

void ADir_ProductionCoordinator::ValidateAgentDependencies()
{
    for (const FDir_AgentTask& Task : TaskQueue)
    {
        if (!Task.bIsCompleted && !CheckDependenciesComplete(Task.Dependencies))
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent %d blocked by dependencies"), (int32)Task.AgentType);
        }
    }
}

bool ADir_ProductionCoordinator::IsMilestone1Complete() const
{
    // Milestone 1: Walk Around - requires basic character movement and world
    bool bHasCharacter = CurrentMetrics.CharacterCount > 0;
    bool bHasWorld = CurrentMetrics.TotalActors > 100; // Basic world population
    bool bCriticalAgentsComplete = AgentStatus[EDir_AgentType::Character] && 
                                   AgentStatus[EDir_AgentType::WorldGenerator];
    
    return bHasCharacter && bHasWorld && bCriticalAgentsComplete;
}

void ADir_ProductionCoordinator::ReportMilestoneProgress()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MILESTONE 1 PROGRESS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), CurrentMetrics.CharacterCount);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.2f%%"), CurrentMetrics.ProductionProgress * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Complete: %s"), IsMilestone1Complete() ? TEXT("YES") : TEXT("NO"));
}

void ADir_ProductionCoordinator::InitializeAgentDependencies()
{
    // Initialize critical tasks for Milestone 1
    AssignTaskToAgent(EDir_AgentType::EngineArchitect, TEXT("Define core architecture"), 10);
    AssignTaskToAgent(EDir_AgentType::CoreSystems, TEXT("Implement physics and movement"), 9);
    AssignTaskToAgent(EDir_AgentType::Character, TEXT("Create playable character"), 8);
    AssignTaskToAgent(EDir_AgentType::WorldGenerator, TEXT("Generate basic terrain"), 7);
    AssignTaskToAgent(EDir_AgentType::Environment, TEXT("Populate world with props"), 6);
    AssignTaskToAgent(EDir_AgentType::Lighting, TEXT("Setup lighting and atmosphere"), 5);
    AssignTaskToAgent(EDir_AgentType::Animation, TEXT("Implement character animations"), 4);
}

void ADir_ProductionCoordinator::UpdateTaskPriorities()
{
    // Boost priority of unblocked tasks
    for (FDir_AgentTask& Task : TaskQueue)
    {
        if (!Task.bIsCompleted && CheckDependenciesComplete(Task.Dependencies))
        {
            Task.Priority = FMath::Min(Task.Priority + 1, 10);
        }
    }
}

bool ADir_ProductionCoordinator::CheckDependenciesComplete(const TArray<EDir_AgentType>& Dependencies) const
{
    for (EDir_AgentType Dependency : Dependencies)
    {
        if (!AgentStatus[Dependency])
        {
            return false;
        }
    }
    
    return true;
}