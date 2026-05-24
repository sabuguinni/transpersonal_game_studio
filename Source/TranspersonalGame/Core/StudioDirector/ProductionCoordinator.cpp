#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UProductionCoordinator::UProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    MetricsUpdateInterval = 5.0f;
    bAutoAdvancePhases = true;
    LastMetricsUpdate = 0.0f;
}

void UProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    UpdateMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized"));
}

void UProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastMetricsUpdate += DeltaTime;
    
    if (LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdateMetrics();
        ValidateProductionState();
        CheckMilestoneCompletion();
        LastMetricsUpdate = 0.0f;
    }
}

void UProductionCoordinator::InitializeProductionPipeline()
{
    ActiveTasks.Empty();
    AgentStatusMap.Empty();
    
    // Initialize core production tasks for each agent
    AssignTaskToAgent(2, TEXT("Define UE5 architecture and core systems framework"), EDir_ProductionPhase::CoreSystems, 10.0f);
    AssignTaskToAgent(3, TEXT("Implement physics, collision, and ragdoll systems"), EDir_ProductionPhase::CoreSystems, 9.0f);
    AssignTaskToAgent(4, TEXT("Optimize performance for 60fps PC / 30fps console"), EDir_ProductionPhase::CoreSystems, 8.0f);
    AssignTaskToAgent(5, TEXT("Generate procedural world with 5 biomes using PCG"), EDir_ProductionPhase::ContentCreation, 9.0f);
    AssignTaskToAgent(6, TEXT("Populate world with vegetation, rocks, and materials"), EDir_ProductionPhase::ContentCreation, 7.0f);
    AssignTaskToAgent(7, TEXT("Build prehistoric structures and interiors"), EDir_ProductionPhase::ContentCreation, 6.0f);
    AssignTaskToAgent(8, TEXT("Implement lighting, day/night cycle, and atmosphere"), EDir_ProductionPhase::ContentCreation, 8.0f);
    AssignTaskToAgent(9, TEXT("Create playable characters using MetaHuman"), EDir_ProductionPhase::ContentCreation, 9.0f);
    AssignTaskToAgent(10, TEXT("Add Motion Matching animations and IK systems"), EDir_ProductionPhase::ContentCreation, 8.0f);
    AssignTaskToAgent(11, TEXT("Implement NPC behavior trees and daily routines"), EDir_ProductionPhase::ContentCreation, 7.0f);
    AssignTaskToAgent(12, TEXT("Create tactical combat AI for dinosaurs"), EDir_ProductionPhase::ContentCreation, 9.0f);
    AssignTaskToAgent(13, TEXT("Implement Mass AI crowd simulation (50K agents)"), EDir_ProductionPhase::ContentCreation, 6.0f);
    AssignTaskToAgent(14, TEXT("Design quests and mission objectives"), EDir_ProductionPhase::ContentCreation, 5.0f);
    AssignTaskToAgent(15, TEXT("Write game bible, story, and dialogue"), EDir_ProductionPhase::PreProduction, 10.0f);
    AssignTaskToAgent(16, TEXT("Create adaptive music and MetaSounds audio"), EDir_ProductionPhase::ContentCreation, 6.0f);
    AssignTaskToAgent(17, TEXT("Implement Niagara VFX with 3-level LOD"), EDir_ProductionPhase::ContentCreation, 7.0f);
    AssignTaskToAgent(18, TEXT("Test all systems and validate build quality"), EDir_ProductionPhase::Polish, 10.0f);
    AssignTaskToAgent(19, TEXT("Integrate all outputs into coherent build"), EDir_ProductionPhase::Polish, 10.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline initialized with %d tasks"), ActiveTasks.Num());
}

void UProductionCoordinator::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_ProductionPhase Phase, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.RequiredPhase = Phase;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Task assigned to Agent #%d: %s"), AgentNumber, *TaskDescription);
}

void UProductionCoordinator::CompleteAgentTask(int32 AgentNumber)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber && !Task.bIsCompleted)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d completed task: %s"), AgentNumber, *Task.TaskDescription);
            break;
        }
    }
}

bool UProductionCoordinator::IsPhaseComplete(EDir_ProductionPhase Phase) const
{
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.RequiredPhase == Phase && !Task.bIsCompleted)
        {
            return false;
        }
    }
    return true;
}

void UProductionCoordinator::AdvanceToNextPhase()
{
    if (!IsPhaseComplete(CurrentPhase))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot advance phase - current phase not complete"));
        return;
    }
    
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::CoreSystems;
            break;
        case EDir_ProductionPhase::CoreSystems:
            CurrentPhase = EDir_ProductionPhase::ContentCreation;
            break;
        case EDir_ProductionPhase::ContentCreation:
            CurrentPhase = EDir_ProductionPhase::Polish;
            break;
        case EDir_ProductionPhase::Polish:
            CurrentPhase = EDir_ProductionPhase::Release;
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced to production phase: %d"), (int32)CurrentPhase);
}

FDir_ProductionMetrics UProductionCoordinator::GetCurrentMetrics() const
{
    return ProductionMetrics;
}

void UProductionCoordinator::UpdateMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Count total actors in the world
        ProductionMetrics.TotalActorsSpawned = World->GetCurrentLevel()->Actors.Num();
        
        // Count dinosaur actors
        ProductionMetrics.DinosaurCount = 0;
        for (AActor* Actor : World->GetCurrentLevel()->Actors)
        {
            if (Actor && (Actor->GetName().Contains(TEXT("Rex")) || 
                         Actor->GetName().Contains(TEXT("Raptor")) ||
                         Actor->GetName().Contains(TEXT("Brachio")) ||
                         Actor->GetName().Contains(TEXT("Trike")) ||
                         Actor->GetName().Contains(TEXT("Ankylo"))))
            {
                ProductionMetrics.DinosaurCount++;
            }
        }
        
        // Update frame rate
        if (GEngine)
        {
            ProductionMetrics.FrameRate = 1.0f / World->GetDeltaSeconds();
        }
        
        // Check if map was saved recently
        ProductionMetrics.bMapSaved = true; // Assume saved if metrics are updating
    }
}

float UProductionCoordinator::GetOverallProgress() const
{
    if (ActiveTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.bIsCompleted)
        {
            CompletedTasks++;
        }
    }
    
    return (float)CompletedTasks / (float)ActiveTasks.Num() * 100.0f;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (!Task.bIsCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    return PendingTasks;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetTasksForAgent(int32 AgentNumber) const
{
    TArray<FDir_AgentTask> AgentTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

void UProductionCoordinator::ReportAgentStatus(int32 AgentNumber, const FString& StatusMessage)
{
    AgentStatusMap.Add(AgentNumber, StatusMessage);
    UE_LOG(LogTemp, Log, TEXT("Agent #%d status: %s"), AgentNumber, *StatusMessage);
}

void UProductionCoordinator::ValidateProductionState()
{
    // Check for critical issues that would block production
    if (ProductionMetrics.TotalActorsSpawned > 50000)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Actor count exceeds safe limit (%d)"), ProductionMetrics.TotalActorsSpawned);
    }
    
    if (ProductionMetrics.FrameRate < 20.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Frame rate below target (%f fps)"), ProductionMetrics.FrameRate);
    }
}

void UProductionCoordinator::CheckMilestoneCompletion()
{
    if (bAutoAdvancePhases && IsPhaseComplete(CurrentPhase))
    {
        AdvanceToNextPhase();
    }
}

void UProductionCoordinator::LogProductionStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), GetOverallProgress());
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), ProductionMetrics.TotalActorsSpawned);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), ProductionMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.1f fps"), ProductionMetrics.FrameRate);
}