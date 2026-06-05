#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize production state
    TotalCycles = 0;
    CompletedTasks = 0;
    OverallProgress = 0.0f;
    CurrentMilestone = EDir_ProductionMilestone::Movement;
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the production system
    InitializeMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized - Studio Director active"));
}

void AProductionCoordinator::InitializeMilestones()
{
    ProductionMilestones.Empty();
    
    // Milestone 1: Basic Movement
    FDir_MilestoneData MovementMilestone;
    MovementMilestone.MilestoneType = EDir_ProductionMilestone::Movement;
    MovementMilestone.MilestoneName = TEXT("Basic Movement");
    MovementMilestone.Description = TEXT("Player character with WASD movement, camera, jump");
    MovementMilestone.RequiredAgents = {2, 3, 9}; // Engine Arch, Core Systems, Character Artist
    MovementMilestone.WorldLocation = FVector(1000, 0, 200);
    ProductionMilestones.Add(MovementMilestone);
    
    // Milestone 2: Dinosaur Interaction
    FDir_MilestoneData DinosaurMilestone;
    DinosaurMilestone.MilestoneType = EDir_ProductionMilestone::Dinosaurs;
    DinosaurMilestone.MilestoneName = TEXT("Dinosaur Interaction");
    DinosaurMilestone.Description = TEXT("Dinosaur actors with basic AI and collision");
    DinosaurMilestone.RequiredAgents = {10, 11, 12}; // Animation, NPC Behavior, Combat AI
    DinosaurMilestone.WorldLocation = FVector(2000, 0, 200);
    ProductionMilestones.Add(DinosaurMilestone);
    
    // Milestone 3: Survival Systems
    FDir_MilestoneData SurvivalMilestone;
    SurvivalMilestone.MilestoneType = EDir_ProductionMilestone::Survival;
    SurvivalMilestone.MilestoneName = TEXT("Survival Systems");
    SurvivalMilestone.Description = TEXT("Health, hunger, thirst, stamina HUD and mechanics");
    SurvivalMilestone.RequiredAgents = {14, 15, 16}; // Quest Designer, Narrative, Audio
    SurvivalMilestone.WorldLocation = FVector(3000, 0, 200);
    ProductionMilestones.Add(SurvivalMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("Production milestones initialized: %d milestones"), ProductionMilestones.Num());
}

void AProductionCoordinator::UpdateMilestoneProgress(EDir_ProductionMilestone Milestone, float NewProgress)
{
    for (FDir_MilestoneData& MilestoneData : ProductionMilestones)
    {
        if (MilestoneData.MilestoneType == Milestone)
        {
            MilestoneData.Progress = FMath::Clamp(NewProgress, 0.0f, 100.0f);
            if (MilestoneData.Progress >= 100.0f)
            {
                MilestoneData.bCompleted = true;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Milestone %s progress updated: %.1f%%"), 
                   *MilestoneData.MilestoneName, MilestoneData.Progress);
            break;
        }
    }
    
    // Recalculate overall progress
    OverallProgress = CalculateOverallProgress();
}

bool AProductionCoordinator::IsMilestoneComplete(EDir_ProductionMilestone Milestone) const
{
    for (const FDir_MilestoneData& MilestoneData : ProductionMilestones)
    {
        if (MilestoneData.MilestoneType == Milestone)
        {
            return MilestoneData.bCompleted;
        }
    }
    return false;
}

FDir_MilestoneData AProductionCoordinator::GetMilestoneData(EDir_ProductionMilestone Milestone) const
{
    for (const FDir_MilestoneData& MilestoneData : ProductionMilestones)
    {
        if (MilestoneData.MilestoneType == Milestone)
        {
            return MilestoneData;
        }
    }
    return FDir_MilestoneData();
}

void AProductionCoordinator::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_TaskPriority Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.AgentName = FString::Printf(TEXT("Agent #%02d"), AgentNumber);
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.LastUpdate = FDateTime::Now().ToString();
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%02d: %s"), AgentNumber, *TaskDescription);
}

void AProductionCoordinator::UpdateAgentTaskProgress(int32 AgentNumber, float Progress, const FString& UpdateNote)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && !Task.bCompleted)
        {
            Task.CompletionPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            Task.LastUpdate = FString::Printf(TEXT("%s - %s"), *FDateTime::Now().ToString(), *UpdateNote);
            
            if (Task.CompletionPercentage >= 100.0f)
            {
                Task.bCompleted = true;
                CompletedTasks++;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Agent #%02d task progress: %.1f%% - %s"), 
                   AgentNumber, Progress, *UpdateNote);
            break;
        }
    }
}

void AProductionCoordinator::CompleteAgentTask(int32 AgentNumber)
{
    UpdateAgentTaskProgress(AgentNumber, 100.0f, TEXT("Task completed"));
}

TArray<FDir_AgentTask> AProductionCoordinator::GetTasksForAgent(int32 AgentNumber) const
{
    TArray<FDir_AgentTask> AgentSpecificTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            AgentSpecificTasks.Add(Task);
        }
    }
    
    return AgentSpecificTasks;
}

TArray<FDir_AgentTask> AProductionCoordinator::GetHighPriorityTasks() const
{
    TArray<FDir_AgentTask> HighPriorityTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Priority == EDir_TaskPriority::High || Task.Priority == EDir_TaskPriority::Critical)
        {
            if (!Task.bCompleted)
            {
                HighPriorityTasks.Add(Task);
            }
        }
    }
    
    return HighPriorityTasks;
}

void AProductionCoordinator::IncrementCycleCount()
{
    TotalCycles++;
    UE_LOG(LogTemp, Warning, TEXT("Production cycle count: %d"), TotalCycles);
}

float AProductionCoordinator::CalculateOverallProgress() const
{
    if (ProductionMilestones.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        TotalProgress += Milestone.Progress;
    }
    
    return TotalProgress / ProductionMilestones.Num();
}

FString AProductionCoordinator::GenerateProductionReport() const
{
    FString Report = TEXT("=== PRODUCTION REPORT ===\n");
    Report += FString::Printf(TEXT("Total Cycles: %d\n"), TotalCycles);
    Report += FString::Printf(TEXT("Completed Tasks: %d\n"), CompletedTasks);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n\n"), OverallProgress);
    
    Report += TEXT("MILESTONES:\n");
    for (const FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        FString Status = Milestone.bCompleted ? TEXT("COMPLETE") : TEXT("IN PROGRESS");
        Report += FString::Printf(TEXT("- %s: %.1f%% [%s]\n"), 
                                  *Milestone.MilestoneName, Milestone.Progress, *Status);
    }
    
    Report += TEXT("\nHIGH PRIORITY TASKS:\n");
    TArray<FDir_AgentTask> HighPriority = GetHighPriorityTasks();
    for (const FDir_AgentTask& Task : HighPriority)
    {
        Report += FString::Printf(TEXT("- Agent #%02d: %s (%.1f%%)\n"), 
                                  Task.AgentNumber, *Task.TaskDescription, Task.CompletionPercentage);
    }
    
    return Report;
}

void AProductionCoordinator::CreateMilestoneMarkers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create milestone markers - no world"));
        return;
    }
    
    for (const FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("Milestone_%s"), *Milestone.MilestoneName));
        
        AStaticMeshActor* MarkerActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Milestone.WorldLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (MarkerActor)
        {
            MarkerActor->SetActorLabel(FString::Printf(TEXT("MILESTONE_%s"), *Milestone.MilestoneName.ToUpper()));
            UE_LOG(LogTemp, Warning, TEXT("Created milestone marker: %s"), *Milestone.MilestoneName);
        }
    }
}

void AProductionCoordinator::ValidateMinPlayableMapState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot validate map state - no world"));
        return;
    }
    
    // Count actors by type
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 CharacterActors = 0;
    int32 DinosaurActors = 0;
    int32 EnvironmentActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetName().ToLower();
        
        if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
        {
            CharacterActors++;
        }
        else if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
                 ActorName.Contains(TEXT("brach")) || ActorName.Contains(TEXT("dinosaur")))
        {
            DinosaurActors++;
        }
        else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("rock")) || 
                 ActorName.Contains(TEXT("landscape")))
        {
            EnvironmentActors++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Map validation - Characters: %d, Dinosaurs: %d, Environment: %d"), 
           CharacterActors, DinosaurActors, EnvironmentActors);
}

TArray<int32> AProductionCoordinator::GetNextPriorityAgents() const
{
    TArray<int32> PriorityAgents;
    
    // Based on current milestone, determine priority agents
    switch (CurrentMilestone)
    {
        case EDir_ProductionMilestone::Movement:
            PriorityAgents = {2, 3, 9}; // Engine Arch, Core Systems, Character Artist
            break;
            
        case EDir_ProductionMilestone::Dinosaurs:
            PriorityAgents = {5, 10, 11, 12}; // World Gen, Animation, NPC Behavior, Combat AI
            break;
            
        case EDir_ProductionMilestone::Survival:
            PriorityAgents = {14, 15, 16}; // Quest Designer, Narrative, Audio
            break;
            
        default:
            PriorityAgents = {2, 5, 9, 12}; // Default priority agents
            break;
    }
    
    return PriorityAgents;
}