#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

DEFINE_LOG_CATEGORY(LogProductionCoordinator);

UProductionCoordinator::UProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    // Initialize production metrics
    ProductionMetrics.TotalActors = 0;
    ProductionMetrics.DinosaurActors = 0;
    ProductionMetrics.TerrainActors = 0;
    ProductionMetrics.PlayerCharacters = 0;
    ProductionMetrics.CompletionPercentage = 0.0f;
    
    // Initialize agent task priorities for Cycle 027
    InitializeAgentTasks();
}

void UProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogProductionCoordinator, Warning, TEXT("Production Coordinator activated for Cycle 027"));
    
    // Perform initial production assessment
    AssessProductionState();
    
    // Set up agent coordination
    CoordinateAgentTasks();
}

void UProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic production monitoring
    AssessProductionState();
    MonitorAgentProgress();
}

void UProductionCoordinator::InitializeAgentTasks()
{
    AgentTasks.Empty();
    
    // Define critical tasks for playable prototype (Cycle 027)
    FDir_AgentTask EngineTask;
    EngineTask.AgentID = "Agent_02_Engine";
    EngineTask.TaskDescription = "Ensure core systems compilation and performance optimization";
    EngineTask.Priority = EDir_TaskPriority::Critical;
    EngineTask.Status = EDir_TaskStatus::Pending;
    EngineTask.EstimatedCycles = 1;
    AgentTasks.Add(EngineTask);
    
    FDir_AgentTask CoreTask;
    CoreTask.AgentID = "Agent_03_Core";
    CoreTask.TaskDescription = "Implement physics and collision for dinosaur interactions";
    CoreTask.Priority = EDir_TaskPriority::High;
    CoreTask.Status = EDir_TaskStatus::Pending;
    CoreTask.EstimatedCycles = 2;
    AgentTasks.Add(CoreTask);
    
    FDir_AgentTask WorldTask;
    WorldTask.AgentID = "Agent_05_World";
    WorldTask.TaskDescription = "Enhance terrain with realistic height variations and biomes";
    WorldTask.Priority = EDir_TaskPriority::High;
    WorldTask.Status = EDir_TaskStatus::Pending;
    WorldTask.EstimatedCycles = 2;
    AgentTasks.Add(WorldTask);
    
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentID = "Agent_09_Character";
    CharacterTask.TaskDescription = "Create dinosaur character blueprints with proper collision";
    CharacterTask.Priority = EDir_TaskPriority::Critical;
    CharacterTask.Status = EDir_TaskStatus::Pending;
    CharacterTask.EstimatedCycles = 2;
    AgentTasks.Add(CharacterTask);
    
    FDir_AgentTask AnimationTask;
    AnimationTask.AgentID = "Agent_10_Animation";
    AnimationTask.TaskDescription = "Add basic locomotion animations for dinosaurs";
    AnimationTask.Priority = EDir_TaskPriority::Medium;
    AnimationTask.Status = EDir_TaskStatus::Pending;
    AnimationTask.EstimatedCycles = 3;
    AgentTasks.Add(AnimationTask);
    
    FDir_AgentTask CombatTask;
    CombatTask.AgentID = "Agent_12_Combat";
    CombatTask.TaskDescription = "Implement survival HUD and basic dinosaur AI behavior";
    CombatTask.Priority = EDir_TaskPriority::Critical;
    CombatTask.Status = EDir_TaskStatus::Pending;
    CombatTask.EstimatedCycles = 2;
    AgentTasks.Add(CombatTask);
    
    FDir_AgentTask NarrativeTask;
    NarrativeTask.AgentID = "Agent_15_Narrative";
    NarrativeTask.TaskDescription = "Define dinosaur species behaviors and world lore";
    NarrativeTask.Priority = EDir_TaskPriority::Medium;
    NarrativeTask.Status = EDir_TaskStatus::Pending;
    NarrativeTask.EstimatedCycles = 1;
    AgentTasks.Add(NarrativeTask);
    
    FDir_AgentTask QATask;
    QATask.AgentID = "Agent_18_QA";
    QATask.TaskDescription = "Test playable prototype functionality";
    QATask.Priority = EDir_TaskPriority::Critical;
    QATask.Status = EDir_TaskStatus::Pending;
    QATask.EstimatedCycles = 1;
    AgentTasks.Add(QATask);
    
    UE_LOG(LogProductionCoordinator, Log, TEXT("Initialized %d agent tasks for Cycle 027"), AgentTasks.Num());
}

void UProductionCoordinator::AssessProductionState()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    ProductionMetrics.TotalActors = AllActors.Num();
    ProductionMetrics.DinosaurActors = 0;
    ProductionMetrics.TerrainActors = 0;
    ProductionMetrics.PlayerCharacters = 0;
    
    // Categorize actors for production assessment
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        FString ClassName = Actor->GetClass()->GetName().ToLower();
        
        // Count dinosaur actors
        if (ActorName.Contains(TEXT("rex")) || ActorName.Contains(TEXT("raptor")) || 
            ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("dino")) ||
            ClassName.Contains(TEXT("dinosaur")))
        {
            ProductionMetrics.DinosaurActors++;
        }
        
        // Count terrain actors
        if (ClassName.Contains(TEXT("landscape")) || ClassName.Contains(TEXT("terrain")))
        {
            ProductionMetrics.TerrainActors++;
        }
        
        // Count player characters
        if (ClassName.Contains(TEXT("transpersonalcharacter")) || ClassName.Contains(TEXT("playercharacter")))
        {
            ProductionMetrics.PlayerCharacters++;
        }
    }
    
    // Calculate completion percentage based on minimum viable prototype requirements
    float CompletionScore = 0.0f;
    
    // Player character exists (25%)
    if (ProductionMetrics.PlayerCharacters > 0)
    {
        CompletionScore += 25.0f;
    }
    
    // Terrain exists (25%)
    if (ProductionMetrics.TerrainActors > 0)
    {
        CompletionScore += 25.0f;
    }
    
    // Dinosaurs exist (25%)
    if (ProductionMetrics.DinosaurActors >= 3)
    {
        CompletionScore += 25.0f;
    }
    else if (ProductionMetrics.DinosaurActors > 0)
    {
        CompletionScore += 12.5f;
    }
    
    // Basic world population (25%)
    if (ProductionMetrics.TotalActors >= 20)
    {
        CompletionScore += 25.0f;
    }
    else if (ProductionMetrics.TotalActors >= 10)
    {
        CompletionScore += 12.5f;
    }
    
    ProductionMetrics.CompletionPercentage = CompletionScore;
    
    UE_LOG(LogProductionCoordinator, Log, TEXT("Production State: %d total actors, %d dinosaurs, %d terrain, %d players - %.1f%% complete"),
           ProductionMetrics.TotalActors, ProductionMetrics.DinosaurActors, ProductionMetrics.TerrainActors, 
           ProductionMetrics.PlayerCharacters, ProductionMetrics.CompletionPercentage);
}

void UProductionCoordinator::CoordinateAgentTasks()
{
    UE_LOG(LogProductionCoordinator, Warning, TEXT("=== STUDIO DIRECTOR AGENT COORDINATION CYCLE 027 ==="));
    
    // Log current production metrics
    UE_LOG(LogProductionCoordinator, Warning, TEXT("Current Production State:"));
    UE_LOG(LogProductionCoordinator, Warning, TEXT("- Total Actors: %d"), ProductionMetrics.TotalActors);
    UE_LOG(LogProductionCoordinator, Warning, TEXT("- Dinosaur Actors: %d"), ProductionMetrics.DinosaurActors);
    UE_LOG(LogProductionCoordinator, Warning, TEXT("- Terrain Actors: %d"), ProductionMetrics.TerrainActors);
    UE_LOG(LogProductionCoordinator, Warning, TEXT("- Player Characters: %d"), ProductionMetrics.PlayerCharacters);
    UE_LOG(LogProductionCoordinator, Warning, TEXT("- Completion: %.1f%%"), ProductionMetrics.CompletionPercentage);
    
    // Prioritize tasks based on current state
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_TaskStatus::Pending)
        {
            UE_LOG(LogProductionCoordinator, Warning, TEXT("PRIORITY TASK - %s: %s"), 
                   *Task.AgentID, *Task.TaskDescription);
        }
    }
    
    // Issue specific directives based on production gaps
    if (ProductionMetrics.DinosaurActors < 3)
    {
        UE_LOG(LogProductionCoordinator, Error, TEXT("CRITICAL: Insufficient dinosaur actors! Agent #9 must create dinosaur blueprints immediately!"));
    }
    
    if (ProductionMetrics.TerrainActors == 0)
    {
        UE_LOG(LogProductionCoordinator, Error, TEXT("CRITICAL: No terrain detected! Agent #5 must enhance landscape immediately!"));
    }
    
    if (ProductionMetrics.CompletionPercentage < 50.0f)
    {
        UE_LOG(LogProductionCoordinator, Error, TEXT("CRITICAL: Playable prototype below 50%% completion! All agents must focus on core deliverables!"));
    }
}

void UProductionCoordinator::MonitorAgentProgress()
{
    // Update task statuses based on production state changes
    for (FDir_AgentTask& Task : AgentTasks)
    {
        // Simple heuristic-based progress tracking
        if (Task.AgentID == "Agent_09_Character" && ProductionMetrics.DinosaurActors >= 3)
        {
            Task.Status = EDir_TaskStatus::Completed;
        }
        else if (Task.AgentID == "Agent_05_World" && ProductionMetrics.TerrainActors > 0)
        {
            Task.Status = EDir_TaskStatus::InProgress;
        }
        else if (Task.AgentID == "Agent_12_Combat" && ProductionMetrics.PlayerCharacters > 0)
        {
            Task.Status = EDir_TaskStatus::InProgress;
        }
    }
}

FDir_ProductionMetrics UProductionCoordinator::GetProductionMetrics() const
{
    return ProductionMetrics;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetAgentTasks() const
{
    return AgentTasks;
}

void UProductionCoordinator::UpdateTaskStatus(const FString& AgentID, EDir_TaskStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            UE_LOG(LogProductionCoordinator, Log, TEXT("Updated task status for %s to %s"), 
                   *AgentID, *UEnum::GetValueAsString(NewStatus));
            break;
        }
    }
}

void UProductionCoordinator::AddAgentTask(const FDir_AgentTask& NewTask)
{
    AgentTasks.Add(NewTask);
    UE_LOG(LogProductionCoordinator, Log, TEXT("Added new task for agent %s: %s"), 
           *NewTask.AgentID, *NewTask.TaskDescription);
}