#include "StudioDirectorCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UStudioDirectorCoordinator::UStudioDirectorCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    LastValidationTime = 0.0f;
    CurrentCycleNumber = 0;
    bProductionHalted = false;
}

void UStudioDirectorCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Coordinator initialized"));
    InitializeProductionPipeline();
}

void UStudioDirectorCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bProductionHalted)
    {
        LastValidationTime += DeltaTime;
        
        // Validate build health every 30 seconds
        if (LastValidationTime >= 30.0f)
        {
            ValidateBuildHealth();
            LastValidationTime = 0.0f;
        }
    }
}

void UStudioDirectorCoordinator::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing 19-agent production pipeline"));
    
    // Clear existing tasks
    ActiveTasks.Empty();
    ProductionMilestones.Empty();
    
    // Initialize Milestone 1: "Walk Around" prototype
    FDir_MilestoneInfo WalkAroundMilestone;
    WalkAroundMilestone.Name = TEXT("Milestone_1_WalkAround");
    WalkAroundMilestone.Description = TEXT("Minimum viable playable prototype with character movement and dinosaurs");
    WalkAroundMilestone.TargetCycle = 20;
    WalkAroundMilestone.CurrentProgress = 0.0f;
    WalkAroundMilestone.Priority = EDir_TaskPriority::Critical;
    WalkAroundMilestone.bCompleted = false;
    ProductionMilestones.Add(WalkAroundMilestone);
    
    // Initialize build status
    CurrentBuildStatus.bCompilationSuccessful = true;
    CurrentBuildStatus.ActorCount = GetTotalActorCount();
    CurrentBuildStatus.DinosaurCount = GetDinosaurActors().Num();
    CurrentBuildStatus.LastBuildTime = FDateTime::Now().ToString();
    CurrentBuildStatus.MemoryUsagePercent = 0.0f;
    
    LogProductionStatus();
}

void UStudioDirectorCoordinator::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_TaskPriority Priority)
{
    FDir_AgentTaskInfo NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.AssignedTime = FDateTime::Now().ToString();
    NewTask.bCompleted = false;
    NewTask.EstimatedCycles = 1;
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%d: %s"), AgentNumber, *TaskDescription);
}

void UStudioDirectorCoordinator::UpdateMilestoneProgress(const FString& MilestoneName, float ProgressPercent)
{
    for (FDir_MilestoneInfo& Milestone : ProductionMilestones)
    {
        if (Milestone.Name == MilestoneName)
        {
            Milestone.CurrentProgress = FMath::Clamp(ProgressPercent, 0.0f, 100.0f);
            
            if (Milestone.CurrentProgress >= 100.0f)
            {
                Milestone.bCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Milestone completed: %s"), *MilestoneName);
            }
            
            break;
        }
    }
}

bool UStudioDirectorCoordinator::ValidateMinPlayablePrototype()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating minimum playable prototype..."));
    
    // Check for character movement
    bool bHasPlayerCharacter = false;
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
        {
            APawn* Pawn = *ActorItr;
            if (Pawn && Pawn->IsA<APawn>())
            {
                bHasPlayerCharacter = true;
                break;
            }
        }
    }
    
    // Check for dinosaurs
    TArray<AActor*> DinosaurActors = GetDinosaurActors();
    bool bHasDinosaurs = DinosaurActors.Num() >= 3;
    
    // Check for terrain (non-flat landscape)
    bool bHasTerrain = GetTotalActorCount() > 100; // Simplified check
    
    bool bPrototypeValid = bHasPlayerCharacter && bHasDinosaurs && bHasTerrain;
    
    UE_LOG(LogTemp, Warning, TEXT("Prototype validation: Character=%s, Dinosaurs=%s (%d), Terrain=%s"), 
           bHasPlayerCharacter ? TEXT("OK") : TEXT("MISSING"),
           bHasDinosaurs ? TEXT("OK") : TEXT("MISSING"), DinosaurActors.Num(),
           bHasTerrain ? TEXT("OK") : TEXT("MISSING"));
    
    return bPrototypeValid;
}

void UStudioDirectorCoordinator::TriggerAgentSequence()
{
    UE_LOG(LogTemp, Warning, TEXT("Triggering agent sequence for cycle %d"), CurrentCycleNumber + 1);
    CurrentCycleNumber++;
    
    // Assign tasks based on current needs
    if (GetDinosaurActors().Num() < 5)
    {
        AssignTaskToAgent(9, TEXT("Spawn additional dinosaur actors in biomes"), EDir_TaskPriority::High);
    }
    
    if (GetTotalActorCount() < 500)
    {
        AssignTaskToAgent(6, TEXT("Populate environment with vegetation and props"), EDir_TaskPriority::Medium);
    }
}

void UStudioDirectorCoordinator::ValidateBuildHealth()
{
    CheckMemoryUsage();
    
    int32 ActorCount = GetTotalActorCount();
    int32 DinosaurCount = GetDinosaurActors().Num();
    
    CurrentBuildStatus.ActorCount = ActorCount;
    CurrentBuildStatus.DinosaurCount = DinosaurCount;
    CurrentBuildStatus.LastBuildTime = FDateTime::Now().ToString();
    
    UE_LOG(LogTemp, Warning, TEXT("Build health: %d actors, %d dinosaurs"), ActorCount, DinosaurCount);
    
    // Check for memory issues
    if (CurrentBuildStatus.MemoryUsagePercent > 85.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("High memory usage detected: %.1f%%"), CurrentBuildStatus.MemoryUsagePercent);
        RequestProductionHalt(TEXT("High memory usage"));
    }
}

int32 UStudioDirectorCoordinator::GetTotalActorCount()
{
    int32 Count = 0;
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            Count++;
        }
    }
    return Count;
}

TArray<AActor*> UStudioDirectorCoordinator::GetDinosaurActors()
{
    TArray<AActor*> DinosaurActors;
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                FString ActorName = Actor->GetName();
                FString ActorLabel = Actor->GetActorLabel();
                
                // Check if actor name/label contains dinosaur keywords
                if (ActorName.Contains(TEXT("Rex")) || ActorName.Contains(TEXT("Raptor")) || 
                    ActorName.Contains(TEXT("Dino")) || ActorName.Contains(TEXT("Brachio")) ||
                    ActorLabel.Contains(TEXT("Rex")) || ActorLabel.Contains(TEXT("Raptor")) ||
                    ActorLabel.Contains(TEXT("Dino")) || ActorLabel.Contains(TEXT("Brachio")))
                {
                    DinosaurActors.Add(Actor);
                }
            }
        }
    }
    return DinosaurActors;
}

void UStudioDirectorCoordinator::HandleAgentTimeout(int32 AgentNumber)
{
    UE_LOG(LogTemp, Error, TEXT("Agent #%d timeout detected"), AgentNumber);
    
    // Mark agent tasks as failed
    for (FDir_AgentTaskInfo& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber && !Task.bCompleted)
        {
            Task.bCompleted = true; // Mark as completed but failed
            UE_LOG(LogTemp, Warning, TEXT("Task marked as failed due to timeout: %s"), *Task.TaskDescription);
        }
    }
}

void UStudioDirectorCoordinator::RequestProductionHalt(const FString& Reason)
{
    bProductionHalted = true;
    UE_LOG(LogTemp, Error, TEXT("PRODUCTION HALT REQUESTED: %s"), *Reason);
}

void UStudioDirectorCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), ActiveTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Milestones: %d"), ProductionMilestones.Num());
    UE_LOG(LogTemp, Warning, TEXT("Current Cycle: %d"), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Production Halted: %s"), bProductionHalted ? TEXT("YES") : TEXT("NO"));
}

void UStudioDirectorCoordinator::CheckMemoryUsage()
{
    // Simplified memory check - in real implementation would use platform-specific APIs
    CurrentBuildStatus.MemoryUsagePercent = 45.0f; // Placeholder value
}

bool UStudioDirectorCoordinator::IsAgentResponsive(int32 AgentNumber)
{
    // Simplified responsiveness check
    return true; // Placeholder - would check actual agent status
}