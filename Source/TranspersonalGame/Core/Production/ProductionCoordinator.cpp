#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentPhase = EDir_ProductionPhase::Prototype;
    CycleID = TEXT("PROD_CYCLE_AUTO_20260614_008");
    
    // Initialize default agent tasks
    FDir_AgentTask EngineTask;
    EngineTask.AgentName = TEXT("Agent_02_Engine");
    EngineTask.TaskDescription = TEXT("Fix compilation errors, ensure all .h files have .cpp implementations");
    EngineTask.Priority = 10;
    AgentTasks.Add(EngineTask);
    
    FDir_AgentTask WorldTask;
    WorldTask.AgentName = TEXT("Agent_05_World");
    WorldTask.TaskDescription = TEXT("Create varied terrain with hills, valleys, rivers - not flat planes");
    WorldTask.Priority = 9;
    AgentTasks.Add(WorldTask);
    
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentName = TEXT("Agent_09_Character");
    CharacterTask.TaskDescription = TEXT("Implement dinosaur collision and basic roaming AI");
    CharacterTask.Priority = 8;
    AgentTasks.Add(CharacterTask);
    
    FDir_AgentTask AnimationTask;
    AnimationTask.AgentName = TEXT("Agent_10_Animation");
    AnimationTask.TaskDescription = TEXT("Add walk/run/idle animations to TranspersonalCharacter");
    AnimationTask.Priority = 7;
    AgentTasks.Add(AnimationTask);
    
    FDir_AgentTask CombatTask;
    CombatTask.AgentName = TEXT("Agent_12_Combat");
    CombatTask.TaskDescription = TEXT("Create survival HUD with health/hunger/thirst bars");
    CombatTask.Priority = 6;
    AgentTasks.Add(CombatTask);
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized for cycle: %s"), *CycleID);
    UpdateProductionMetrics();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update metrics every 5 seconds
    static float MetricsTimer = 0.0f;
    MetricsTimer += DeltaTime;
    if (MetricsTimer >= 5.0f)
    {
        UpdateProductionMetrics();
        MetricsTimer = 0.0f;
    }
}

void AProductionCoordinator::UpdateProductionMetrics()
{
    if (!GetWorld()) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    CurrentMetrics.TotalActors = AllActors.Num();
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.CharacterCount = 0;
    CurrentMetrics.TerrainCount = 0;
    
    TArray<FString> DinoLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
                                  TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Count dinosaurs
        for (const FString& DinoLabel : DinoLabels)
        {
            if (ActorLabel.Contains(DinoLabel))
            {
                CurrentMetrics.DinosaurCount++;
                break;
            }
        }
        
        // Count characters
        if (ActorLabel.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("player")))
        {
            CurrentMetrics.CharacterCount++;
        }
        
        // Count terrain
        if (ActorLabel.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("terrain")))
        {
            CurrentMetrics.TerrainCount++;
        }
    }
    
    // Calculate completion percentage
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bCompleted)
        {
            CompletedTasks++;
        }
    }
    
    CurrentMetrics.CompletionPercentage = AgentTasks.Num() > 0 ? 
        (float)CompletedTasks / (float)AgentTasks.Num() * 100.0f : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Production Metrics - Total: %d, Dinos: %d, Characters: %d, Completion: %.1f%%"), 
           CurrentMetrics.TotalActors, CurrentMetrics.DinosaurCount, 
           CurrentMetrics.CharacterCount, CurrentMetrics.CompletionPercentage);
}

void AProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDesc, int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDesc;
    NewTask.Priority = Priority;
    NewTask.bCompleted = false;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to %s: %s (Priority: %d)"), 
           *AgentName, *TaskDesc, Priority);
}

void AProductionCoordinator::CompleteAgentTask(const FString& AgentName)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName && !Task.bCompleted)
        {
            Task.bCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Task completed by %s: %s"), *AgentName, *Task.TaskDescription);
            break;
        }
    }
}

float AProductionCoordinator::GetOverallProgress() const
{
    return CurrentMetrics.CompletionPercentage;
}

void AProductionCoordinator::AdvanceToNextPhase()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::Prototype;
            break;
        case EDir_ProductionPhase::Prototype:
            CurrentPhase = EDir_ProductionPhase::Alpha;
            break;
        case EDir_ProductionPhase::Alpha:
            CurrentPhase = EDir_ProductionPhase::Beta;
            break;
        case EDir_ProductionPhase::Beta:
            CurrentPhase = EDir_ProductionPhase::Gold;
            break;
        case EDir_ProductionPhase::Gold:
            // Already at final phase
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production phase advanced to: %d"), (int32)CurrentPhase);
}

void AProductionCoordinator::InitializeProductionCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing production cycle: %s"), *CycleID);
    
    // Clear completed tasks for new cycle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.bCompleted = false;
    }
    
    UpdateProductionMetrics();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
            FString::Printf(TEXT("Production Cycle %s Initialized"), *CycleID));
    }
}

void AProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle ID: %s"), *CycleID);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), CurrentMetrics.CharacterCount);
    UE_LOG(LogTemp, Warning, TEXT("Completion: %.1f%%"), CurrentMetrics.CompletionPercentage);
    
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT TASKS ==="));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString Status = Task.bCompleted ? TEXT("COMPLETED") : TEXT("PENDING");
        UE_LOG(LogTemp, Warning, TEXT("%s [%s] Priority %d: %s"), 
               *Task.AgentName, *Status, Task.Priority, *Task.TaskDescription);
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, 
            FString::Printf(TEXT("Production Report Generated - %.1f%% Complete"), 
                          CurrentMetrics.CompletionPercentage));
    }
}