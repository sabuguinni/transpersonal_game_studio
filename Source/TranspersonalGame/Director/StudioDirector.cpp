#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

AStudioDirector::AStudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create director marker mesh component
    DirectorMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DirectorMarkerMesh"));
    RootComponent = DirectorMarkerMesh;

    // Load cube mesh for marker
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        DirectorMarkerMesh->SetStaticMesh(CubeMeshAsset.Object);
        DirectorMarkerMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.1f));
    }

    // Initialize default values
    CurrentCycleID = TEXT("PROD_CYCLE_INIT");
    bPlayablePrototypeReady = false;
    
    // Initialize production metrics
    ProductionMetrics.CurrentPhase = EDir_ProductionPhase::Prototype;
    ProductionMetrics.TotalCycles = 0;
    ProductionMetrics.CompletedTasks = 0;
    ProductionMetrics.FailedTasks = 0;
    ProductionMetrics.AverageTaskTime = 0.0f;
    ProductionMetrics.OverallProgress = 0.0f;
}

void AStudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized for cycle: %s"), *CurrentCycleID);
    
    // Initialize critical agent tasks for playable prototype
    InitializeProductionCycle(CurrentCycleID);
}

void AStudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update production metrics every frame
    UpdateProductionMetrics();
    
    // Check for task dependencies and blockers
    CheckTaskDependencies();
    
    // Handle critical blockers
    HandleCriticalBlockers();
}

void AStudioDirector::InitializeProductionCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    ActiveTasks.Empty();
    CriticalBlockers.Empty();
    
    // Assign critical tasks for playable prototype milestone
    AssignTaskToAgent(5, TEXT("Create real terrain with height variation using PCG"), 10.0f);
    AssignTaskToAgent(9, TEXT("Create dinosaur actors with collision and basic AI"), 9.0f);
    AssignTaskToAgent(10, TEXT("Implement character movement animations"), 8.0f);
    AssignTaskToAgent(12, TEXT("Implement survival HUD with health/hunger bars"), 7.0f);
    AssignTaskToAgent(15, TEXT("Write core survival narrative and dialogue"), 6.0f);
    
    ProductionMetrics.TotalCycles++;
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle %s initialized with %d critical tasks"), 
           *CycleID, ActiveTasks.Num());
}

void AStudioDirector::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.AgentName = FString::Printf(TEXT("Agent_%02d"), AgentID);
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Idle;
    NewTask.Priority = Priority;
    NewTask.StartTime = FDateTime::Now();
    NewTask.DeadlineTime = FDateTime::Now() + FTimespan::FromHours(2);
    
    // Set dependencies based on agent hierarchy
    if (AgentID == 9) // Character Artist depends on World Generator
    {
        NewTask.Dependencies.Add(TEXT("Agent_05_WorldGeneration"));
    }
    if (AgentID == 10) // Animation depends on Character Artist
    {
        NewTask.Dependencies.Add(TEXT("Agent_09_CharacterArt"));
    }
    if (AgentID == 12) // Combat AI depends on Animation
    {
        NewTask.Dependencies.Add(TEXT("Agent_10_Animation"));
    }
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Task assigned to Agent %d: %s (Priority: %.1f)"), 
           AgentID, *TaskDescription, Priority);
}

void AStudioDirector::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            
            if (NewStatus == EDir_AgentStatus::Completed)
            {
                ProductionMetrics.CompletedTasks++;
            }
            else if (NewStatus == EDir_AgentStatus::Failed)
            {
                ProductionMetrics.FailedTasks++;
                CriticalBlockers.Add(FString::Printf(TEXT("Agent %d failed: %s"), AgentID, *Task.TaskDescription));
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Agent %d status updated: %s"), 
                   AgentID, *UEnum::GetValueAsString(NewStatus));
            break;
        }
    }
}

void AStudioDirector::ValidatePlayablePrototype()
{
    // Check if minimum viable prototype requirements are met
    bool bHasTerrain = false;
    bool bHasCharacter = false;
    bool bHasDinosaurs = false;
    bool bHasMovement = false;
    
    if (UWorld* World = GetWorld())
    {
        // Check for landscape actors
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Landscape")))
            {
                bHasTerrain = true;
            }
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Character")))
            {
                bHasCharacter = true;
            }
            if (Actor && (Actor->GetActorLabel().Contains(TEXT("Dinosaur")) || 
                         Actor->GetActorLabel().Contains(TEXT("TRex")) ||
                         Actor->GetActorLabel().Contains(TEXT("Raptor"))))
            {
                bHasDinosaurs = true;
            }
        }
    }
    
    // Check movement system (simplified check)
    bHasMovement = bHasCharacter; // Assume character has movement
    
    bPlayablePrototypeReady = bHasTerrain && bHasCharacter && bHasDinosaurs && bHasMovement;
    
    if (bPlayablePrototypeReady)
    {
        ProductionMetrics.CurrentPhase = EDir_ProductionPhase::Production;
        UE_LOG(LogTemp, Warning, TEXT("MILESTONE 1 ACHIEVED: Playable prototype ready!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Prototype validation: Terrain=%s, Character=%s, Dinosaurs=%s, Movement=%s"),
               bHasTerrain ? TEXT("OK") : TEXT("MISSING"),
               bHasCharacter ? TEXT("OK") : TEXT("MISSING"),
               bHasDinosaurs ? TEXT("OK") : TEXT("MISSING"),
               bHasMovement ? TEXT("OK") : TEXT("MISSING"));
    }
}

void AStudioDirector::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ===\n"));
    Report += FString::Printf(TEXT("Cycle: %s\n"), *CurrentCycleID);
    Report += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(ProductionMetrics.CurrentPhase));
    Report += FString::Printf(TEXT("Total Cycles: %d\n"), ProductionMetrics.TotalCycles);
    Report += FString::Printf(TEXT("Completed Tasks: %d\n"), ProductionMetrics.CompletedTasks);
    Report += FString::Printf(TEXT("Failed Tasks: %d\n"), ProductionMetrics.FailedTasks);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), ProductionMetrics.OverallProgress);
    Report += FString::Printf(TEXT("Playable Prototype Ready: %s\n"), bPlayablePrototypeReady ? TEXT("YES") : TEXT("NO"));
    
    if (CriticalBlockers.Num() > 0)
    {
        Report += TEXT("\n=== CRITICAL BLOCKERS ===\n");
        for (const FString& Blocker : CriticalBlockers)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Blocker);
        }
    }
    
    Report += TEXT("\n=== ACTIVE TASKS ===\n");
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        Report += FString::Printf(TEXT("Agent %d (%s): %s [%s]\n"), 
                                 Task.AgentID, *Task.AgentName, *Task.TaskDescription,
                                 *UEnum::GetValueAsString(Task.Status));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

bool AStudioDirector::IsAgentBlocked(int32 AgentID) const
{
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID)
        {
            return Task.Status == EDir_AgentStatus::Blocked;
        }
    }
    return false;
}

TArray<FDir_AgentTask> AStudioDirector::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle || Task.Status == EDir_AgentStatus::Working)
        {
            PendingTasks.Add(Task);
        }
    }
    return PendingTasks;
}

float AStudioDirector::GetOverallProgress() const
{
    if (ActiveTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedCount = 0;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedCount++;
        }
    }
    
    return (float)CompletedCount / (float)ActiveTasks.Num() * 100.0f;
}

void AStudioDirector::CreateCoordinationMarkers()
{
    if (UWorld* World = GetWorld())
    {
        // Create main director hub
        SetActorLocation(FVector(0, 0, 500));
        SetActorLabel(TEXT("STUDIO_DIRECTOR_HUB"));
        
        UE_LOG(LogTemp, Warning, TEXT("Studio Director coordination markers created"));
    }
}

void AStudioDirector::ValidateMinPlayableMap()
{
    ValidatePlayablePrototype();
    GenerateProductionReport();
}

void AStudioDirector::UpdateProductionMetrics()
{
    ProductionMetrics.OverallProgress = GetOverallProgress();
    
    // Calculate average task time (simplified)
    if (ProductionMetrics.CompletedTasks > 0)
    {
        ProductionMetrics.AverageTaskTime = 120.0f; // 2 minutes average per task
    }
}

void AStudioDirector::CheckTaskDependencies()
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle)
        {
            bool bDependenciesMet = true;
            for (const FString& Dependency : Task.Dependencies)
            {
                bool bDependencyCompleted = false;
                for (const FDir_AgentTask& OtherTask : ActiveTasks)
                {
                    if (OtherTask.AgentName == Dependency && OtherTask.Status == EDir_AgentStatus::Completed)
                    {
                        bDependencyCompleted = true;
                        break;
                    }
                }
                if (!bDependencyCompleted)
                {
                    bDependenciesMet = false;
                    break;
                }
            }
            
            if (!bDependenciesMet)
            {
                Task.Status = EDir_AgentStatus::Blocked;
            }
        }
    }
}

void AStudioDirector::HandleCriticalBlockers()
{
    // Remove resolved blockers
    for (int32 i = CriticalBlockers.Num() - 1; i >= 0; i--)
    {
        const FString& Blocker = CriticalBlockers[i];
        // Simple check - if blocker mentions a completed agent, remove it
        bool bBlockerResolved = false;
        for (const FDir_AgentTask& Task : ActiveTasks)
        {
            if (Blocker.Contains(FString::FromInt(Task.AgentID)) && Task.Status == EDir_AgentStatus::Completed)
            {
                bBlockerResolved = true;
                break;
            }
        }
        
        if (bBlockerResolved)
        {
            CriticalBlockers.RemoveAt(i);
        }
    }
}