#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

ADir_StudioDirector::ADir_StudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize production state
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260502_010");
    CurrentAgentID = 1;
    bProductionActive = true;
    BudgetUsed = 85.09f;
    BudgetLimit = 150.0f;

    // Initialize Milestone 1
    Milestone1Progress.MilestoneName = TEXT("Walk Around - Playable Prototype");
    Milestone1Progress.CompletedTasks = 0;
    Milestone1Progress.CompletionPercentage = 0.0f;
    Milestone1Progress.bIsMilestoneComplete = false;
}

void ADir_StudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized for cycle: %s"), *CurrentCycleID);
    
    // Setup Milestone 1 requirements
    SetupMilestone1Requirements();
    
    // Verify current map state
    VerifyMinPlayableMapState();
}

void ADir_StudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update milestone progress every 5 seconds
    static float ProgressUpdateTimer = 0.0f;
    ProgressUpdateTimer += DeltaTime;
    
    if (ProgressUpdateTimer >= 5.0f)
    {
        CalculateMilestoneProgress();
        ProgressUpdateTimer = 0.0f;
    }
}

void ADir_StudioDirector::InitializeMilestone1Tasks()
{
    Milestone1Progress.RequiredTasks.Empty();

    // Agent #1 - Studio Director (this agent)
    FDir_AgentTask Task1;
    Task1.AgentID = 1;
    Task1.AgentName = TEXT("Studio Director");
    Task1.TaskDescription = TEXT("Coordinate production pipeline and clean MinPlayableMap");
    Task1.bIsCompleted = true; // This task is completing now
    Task1.Priority = 10.0f;
    Milestone1Progress.RequiredTasks.Add(Task1);

    // Agent #2 - Engine Architect
    FDir_AgentTask Task2;
    Task2.AgentID = 2;
    Task2.AgentName = TEXT("Engine Architect");
    Task2.TaskDescription = TEXT("Define technical architecture for character movement and world systems");
    Task2.bIsCompleted = false;
    Task2.Priority = 9.0f;
    Task2.Dependencies.Add(TEXT("Studio Director coordination"));
    Milestone1Progress.RequiredTasks.Add(Task2);

    // Agent #3 - Core Systems
    FDir_AgentTask Task3;
    Task3.AgentID = 3;
    Task3.AgentName = TEXT("Core Systems Programmer");
    Task3.TaskDescription = TEXT("Implement physics and collision for character movement");
    Task3.bIsCompleted = false;
    Task3.Priority = 8.0f;
    Task3.Dependencies.Add(TEXT("Engine architecture"));
    Milestone1Progress.RequiredTasks.Add(Task3);

    // Agent #5 - World Generator
    FDir_AgentTask Task5;
    Task5.AgentID = 5;
    Task5.AgentName = TEXT("Procedural World Generator");
    Task5.TaskDescription = TEXT("Create varied terrain with hills and valleys in MinPlayableMap");
    Task5.bIsCompleted = false;
    Task5.Priority = 7.0f;
    Task5.Dependencies.Add(TEXT("Core systems"));
    Milestone1Progress.RequiredTasks.Add(Task5);

    // Agent #9 - Character Artist
    FDir_AgentTask Task9;
    Task9.AgentID = 9;
    Task9.AgentName = TEXT("Character Artist");
    Task9.TaskDescription = TEXT("Create realistic dinosaur models and place them in world");
    Task9.bIsCompleted = false;
    Task9.Priority = 6.0f;
    Task9.Dependencies.Add(TEXT("World generation"));
    Milestone1Progress.RequiredTasks.Add(Task9);

    // Agent #10 - Animation
    FDir_AgentTask Task10;
    Task10.AgentID = 10;
    Task10.AgentName = TEXT("Animation Agent");
    Task10.TaskDescription = TEXT("Implement character movement animations and dinosaur idle animations");
    Task10.bIsCompleted = false;
    Task10.Priority = 5.0f;
    Task10.Dependencies.Add(TEXT("Character models"));
    Milestone1Progress.RequiredTasks.Add(Task10);

    // Agent #12 - Combat & AI
    FDir_AgentTask Task12;
    Task12.AgentID = 12;
    Task12.AgentName = TEXT("Combat & Enemy AI");
    Task12.TaskDescription = TEXT("Create survival HUD with health/hunger/thirst/stamina bars");
    Task12.bIsCompleted = false;
    Task12.Priority = 4.0f;
    Task12.Dependencies.Add(TEXT("Character systems"));
    Milestone1Progress.RequiredTasks.Add(Task12);

    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 tasks initialized: %d tasks"), Milestone1Progress.RequiredTasks.Num());
}

void ADir_StudioDirector::SetupMilestone1Requirements()
{
    InitializeMilestone1Tasks();
    CalculateMilestoneProgress();
}

void ADir_StudioDirector::UpdateTaskProgress(int32 AgentID, const FString& TaskName, bool bCompleted)
{
    for (FDir_AgentTask& Task : Milestone1Progress.RequiredTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.bIsCompleted = bCompleted;
            UE_LOG(LogTemp, Warning, TEXT("Task updated - Agent %d: %s = %s"), 
                   AgentID, *TaskName, bCompleted ? TEXT("COMPLETED") : TEXT("PENDING"));
            break;
        }
    }
    
    CalculateMilestoneProgress();
}

void ADir_StudioDirector::CalculateMilestoneProgress()
{
    int32 CompletedCount = 0;
    int32 TotalTasks = Milestone1Progress.RequiredTasks.Num();
    
    for (const FDir_AgentTask& Task : Milestone1Progress.RequiredTasks)
    {
        if (Task.bIsCompleted)
        {
            CompletedCount++;
        }
    }
    
    Milestone1Progress.CompletedTasks = CompletedCount;
    Milestone1Progress.CompletionPercentage = TotalTasks > 0 ? (float)CompletedCount / (float)TotalTasks * 100.0f : 0.0f;
    Milestone1Progress.bIsMilestoneComplete = (CompletedCount == TotalTasks);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: %d/%d tasks (%.1f%%)"), 
           CompletedCount, TotalTasks, Milestone1Progress.CompletionPercentage);
}

bool ADir_StudioDirector::CanAgentProceed(int32 AgentID) const
{
    // Check if agent's dependencies are met
    for (const FDir_AgentTask& Task : Milestone1Progress.RequiredTasks)
    {
        if (Task.AgentID == AgentID)
        {
            // Check dependencies
            for (const FString& Dependency : Task.Dependencies)
            {
                bool bDependencyMet = false;
                for (const FDir_AgentTask& DepTask : Milestone1Progress.RequiredTasks)
                {
                    if (DepTask.TaskDescription.Contains(Dependency) && DepTask.bIsCompleted)
                    {
                        bDependencyMet = true;
                        break;
                    }
                }
                
                if (!bDependencyMet)
                {
                    return false;
                }
            }
            break;
        }
    }
    
    return true;
}

TArray<FDir_AgentTask> ADir_StudioDirector::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : Milestone1Progress.RequiredTasks)
    {
        if (!Task.bIsCompleted && CanAgentProceed(Task.AgentID))
        {
            PendingTasks.Add(Task);
        }
    }
    
    // Sort by priority (highest first)
    PendingTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    return PendingTasks;
}

FDir_AgentTask ADir_StudioDirector::GetNextCriticalTask() const
{
    TArray<FDir_AgentTask> PendingTasks = GetPendingTasks();
    
    if (PendingTasks.Num() > 0)
    {
        return PendingTasks[0];
    }
    
    return FDir_AgentTask();
}

void ADir_StudioDirector::VerifyMinPlayableMapState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: No world found"));
        return;
    }
    
    // Count key actors
    int32 PlayerStartCount = 0;
    int32 CharacterCount = 0;
    int32 TerrainCount = 0;
    int32 DinosaurCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ClassName = Actor->GetClass()->GetName();
        FString ActorName = Actor->GetName();
        
        if (ClassName.Contains(TEXT("PlayerStart")))
        {
            PlayerStartCount++;
        }
        else if (ClassName.Contains(TEXT("Character")) || ClassName.Contains(TEXT("Transpersonal")))
        {
            CharacterCount++;
        }
        else if (ClassName.Contains(TEXT("Landscape")) || ClassName.Contains(TEXT("Terrain")))
        {
            TerrainCount++;
        }
        else if (ActorName.Contains(TEXT("Rex")) || ActorName.Contains(TEXT("Raptor")) || 
                 ActorName.Contains(TEXT("Brachio")) || ActorName.Contains(TEXT("Dinosaur")))
        {
            DinosaurCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap State: PlayerStarts=%d, Characters=%d, Terrain=%d, Dinosaurs=%d"),
           PlayerStartCount, CharacterCount, TerrainCount, DinosaurCount);
}

bool ADir_StudioDirector::IsPlayablePrototypeReady() const
{
    return Milestone1Progress.bIsMilestoneComplete;
}

void ADir_StudioDirector::StartProductionCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    bProductionActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle started: %s"), *CycleID);
}

void ADir_StudioDirector::CompleteProductionCycle()
{
    bProductionActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle completed: %s"), *CurrentCycleID);
}

void ADir_StudioDirector::EmergencyStop(const FString& Reason)
{
    bProductionActive = false;
    
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY STOP: %s"), *Reason);
}

void ADir_StudioDirector::LogProductionState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION STATE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s"), *CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Active: %s"), bProductionActive ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Budget: $%.2f / $%.2f"), BudgetUsed, BudgetLimit);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1: %.1f%% complete"), Milestone1Progress.CompletionPercentage);
    
    TArray<FDir_AgentTask> PendingTasks = GetPendingTasks();
    UE_LOG(LogTemp, Warning, TEXT("Pending tasks: %d"), PendingTasks.Num());
    
    for (const FDir_AgentTask& Task : PendingTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - Agent %d (%s): %s"), 
               Task.AgentID, *Task.AgentName, *Task.TaskDescription);
    }
}

void ADir_StudioDirector::ResetMilestone1Progress()
{
    Milestone1Progress.CompletedTasks = 0;
    Milestone1Progress.CompletionPercentage = 0.0f;
    Milestone1Progress.bIsMilestoneComplete = false;
    
    for (FDir_AgentTask& Task : Milestone1Progress.RequiredTasks)
    {
        Task.bIsCompleted = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 progress reset"));
}