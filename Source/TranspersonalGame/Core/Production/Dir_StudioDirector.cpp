#include "Dir_StudioDirector.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ADir_StudioDirector::ADir_StudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    ProductionUpdateInterval = 5.0f;
    bAutoManageProduction = true;
    MaxActorsPerLevel = 8000;
    MaxDinosaursPerLevel = 150;

    // Initialize agent tasks for all 19 agents
    AgentTasks.SetNum(19);
    for (int32 i = 0; i < 19; i++)
    {
        FDir_AgentTask& Task = AgentTasks[i];
        Task.AgentID = i + 1;
        
        switch (i + 1)
        {
            case 1: Task.AgentName = TEXT("Studio Director"); break;
            case 2: Task.AgentName = TEXT("Engine Architect"); break;
            case 3: Task.AgentName = TEXT("Core Systems Programmer"); break;
            case 4: Task.AgentName = TEXT("Performance Optimizer"); break;
            case 5: Task.AgentName = TEXT("Procedural World Generator"); break;
            case 6: Task.AgentName = TEXT("Environment Artist"); break;
            case 7: Task.AgentName = TEXT("Architecture & Interior Agent"); break;
            case 8: Task.AgentName = TEXT("Lighting & Atmosphere Agent"); break;
            case 9: Task.AgentName = TEXT("Character Artist Agent"); break;
            case 10: Task.AgentName = TEXT("Animation Agent"); break;
            case 11: Task.AgentName = TEXT("NPC Behavior Agent"); break;
            case 12: Task.AgentName = TEXT("Combat & Enemy AI Agent"); break;
            case 13: Task.AgentName = TEXT("Crowd & Traffic Simulation"); break;
            case 14: Task.AgentName = TEXT("Quest & Mission Designer"); break;
            case 15: Task.AgentName = TEXT("Narrative & Dialogue Agent"); break;
            case 16: Task.AgentName = TEXT("Audio Agent"); break;
            case 17: Task.AgentName = TEXT("VFX Agent"); break;
            case 18: Task.AgentName = TEXT("QA & Testing Agent"); break;
            case 19: Task.AgentName = TEXT("Integration & Build Agent"); break;
            default: Task.AgentName = TEXT("Unknown Agent"); break;
        }
        
        Task.Status = EDir_AgentStatus::Idle;
        Task.CompletionPercentage = 0.0f;
        Task.Priority = 1;
    }

    CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Prototype;
}

void ADir_StudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    
    if (bAutoManageProduction)
    {
        GetWorldTimerManager().SetTimer(ProductionUpdateTimer, this, 
            &ADir_StudioDirector::OnProductionUpdate, ProductionUpdateInterval, true);
    }
}

void ADir_StudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADir_StudioDirector::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing Production Pipeline"));
    
    // Set initial tasks for prototype phase
    AssignTaskToAgent(5, TEXT("Create terrain with hills and valleys"), 10);
    AssignTaskToAgent(6, TEXT("Place vegetation and environmental props"), 8);
    AssignTaskToAgent(9, TEXT("Create dinosaur character actors"), 9);
    AssignTaskToAgent(10, TEXT("Implement dinosaur animations"), 7);
    AssignTaskToAgent(12, TEXT("Add survival HUD elements"), 6);
    AssignTaskToAgent(8, TEXT("Setup lighting and atmosphere"), 5);
    
    UpdateProductionMetrics();
}

void ADir_StudioDirector::UpdateProductionMetrics()
{
    CountLevelActors();
    UpdateProductionPhase();
    
    UE_LOG(LogTemp, Log, TEXT("Production Metrics Updated - Actors: %d, Dinosaurs: %d, Phase: %s"), 
        CurrentMetrics.TotalActorsInLevel, 
        CurrentMetrics.DinosaurCount,
        *UEnum::GetValueAsString(CurrentMetrics.CurrentPhase));
}

void ADir_StudioDirector::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, int32 Priority)
{
    if (AgentID >= 1 && AgentID <= 19)
    {
        int32 Index = AgentID - 1;
        AgentTasks[Index].CurrentTask = TaskDescription;
        AgentTasks[Index].Priority = Priority;
        AgentTasks[Index].Status = EDir_AgentStatus::Working;
        AgentTasks[Index].CompletionPercentage = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent %d (%s): %s"), 
            AgentID, *AgentTasks[Index].AgentName, *TaskDescription);
    }
}

void ADir_StudioDirector::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, float CompletionPercentage)
{
    if (AgentID >= 1 && AgentID <= 19)
    {
        int32 Index = AgentID - 1;
        AgentTasks[Index].Status = NewStatus;
        AgentTasks[Index].CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Agent %d status updated: %s (%.1f%% complete)"), 
            AgentID, *UEnum::GetValueAsString(NewStatus), CompletionPercentage);
    }
}

FDir_AgentTask ADir_StudioDirector::GetAgentTask(int32 AgentID) const
{
    if (AgentID >= 1 && AgentID <= 19)
    {
        return AgentTasks[AgentID - 1];
    }
    return FDir_AgentTask();
}

TArray<FDir_AgentTask> ADir_StudioDirector::GetAllAgentTasks() const
{
    return AgentTasks;
}

void ADir_StudioDirector::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentMetrics.CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Production phase changed to: %s"), 
        *UEnum::GetValueAsString(NewPhase));
}

EDir_ProductionPhase ADir_StudioDirector::GetCurrentProductionPhase() const
{
    return CurrentMetrics.CurrentPhase;
}

void ADir_StudioDirector::EnforceActorLimits()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Count dinosaurs
    int32 DinoCount = 0;
    TArray<AActor*> DinosaurActors;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dino"), ESearchCase::IgnoreCase))
        {
            DinosaurActors.Add(Actor);
            DinoCount++;
        }
    }
    
    // Enforce dinosaur limit
    if (DinoCount > MaxDinosaursPerLevel)
    {
        int32 ToRemove = DinoCount - MaxDinosaursPerLevel;
        for (int32 i = 0; i < ToRemove && i < DinosaurActors.Num(); i++)
        {
            if (DinosaurActors[i])
            {
                DinosaurActors[i]->Destroy();
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("Enforced dinosaur limit: Removed %d dinosaurs"), ToRemove);
    }
    
    // Enforce total actor limit
    if (AllActors.Num() > MaxActorsPerLevel)
    {
        int32 ToRemove = AllActors.Num() - MaxActorsPerLevel;
        for (int32 i = AllActors.Num() - 1; i >= 0 && ToRemove > 0; i--)
        {
            if (AllActors[i] && !AllActors[i]->IsA<ACharacter>() && 
                !AllActors[i]->GetName().Contains(TEXT("Light")) &&
                !AllActors[i]->GetName().Contains(TEXT("PlayerStart")))
            {
                AllActors[i]->Destroy();
                ToRemove--;
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("Enforced actor limit: Removed %d excess actors"), 
            AllActors.Num() - MaxActorsPerLevel - ToRemove);
    }
}

bool ADir_StudioDirector::ValidateProductionState() const
{
    // Check if essential systems are in place
    bool bHasTerrain = CurrentMetrics.TerrainActors > 0;
    bool bHasDinosaurs = CurrentMetrics.DinosaurCount > 0;
    bool bWithinLimits = CurrentMetrics.TotalActorsInLevel <= MaxActorsPerLevel;
    
    return bHasTerrain && bHasDinosaurs && bWithinLimits;
}

void ADir_StudioDirector::DebugPrintProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %s"), 
        *UEnum::GetValueAsString(CurrentMetrics.CurrentPhase));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d/%d"), 
        CurrentMetrics.TotalActorsInLevel, MaxActorsPerLevel);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d/%d"), 
        CurrentMetrics.DinosaurCount, MaxDinosaursPerLevel);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status != EDir_AgentStatus::Idle)
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent %d (%s): %s - %.1f%% complete"), 
                Task.AgentID, *Task.AgentName, *UEnum::GetValueAsString(Task.Status), 
                Task.CompletionPercentage);
        }
    }
}

void ADir_StudioDirector::GenerateProductionReport()
{
    UpdateProductionMetrics();
    
    FString Report = FString::Printf(TEXT("PRODUCTION REPORT\n"));
    Report += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(CurrentMetrics.CurrentPhase));
    Report += FString::Printf(TEXT("Progress: %.1f%%\n"), CurrentMetrics.OverallProgress);
    Report += FString::Printf(TEXT("Actors: %d/%d\n"), CurrentMetrics.TotalActorsInLevel, MaxActorsPerLevel);
    Report += FString::Printf(TEXT("Dinosaurs: %d/%d\n"), CurrentMetrics.DinosaurCount, MaxDinosaursPerLevel);
    
    int32 WorkingAgents = 0;
    int32 CompletedAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working) WorkingAgents++;
        if (Task.Status == EDir_AgentStatus::Complete) CompletedAgents++;
    }
    
    Report += FString::Printf(TEXT("Agents Working: %d\n"), WorkingAgents);
    Report += FString::Printf(TEXT("Agents Complete: %d\n"), CompletedAgents);
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void ADir_StudioDirector::OptimizeLevel()
{
    EnforceActorLimits();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Level optimization complete"));
}

void ADir_StudioDirector::OnProductionUpdate()
{
    UpdateProductionMetrics();
    ValidateAgentDependencies();
    
    if (bAutoManageProduction)
    {
        EnforceActorLimits();
    }
}

void ADir_StudioDirector::CountLevelActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    CurrentMetrics.TotalActorsInLevel = AllActors.Num();
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.TerrainActors = 0;
    CurrentMetrics.VegetationCount = 0;
    CurrentMetrics.BuildingCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName().ToLower();
        
        if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("rex")) || 
            ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachi")))
        {
            CurrentMetrics.DinosaurCount++;
        }
        else if (ActorName.Contains(TEXT("terrain")) || ActorName.Contains(TEXT("landscape")))
        {
            CurrentMetrics.TerrainActors++;
        }
        else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("bush")) || 
                 ActorName.Contains(TEXT("grass")) || ActorName.Contains(TEXT("plant")))
        {
            CurrentMetrics.VegetationCount++;
        }
        else if (ActorName.Contains(TEXT("building")) || ActorName.Contains(TEXT("structure")))
        {
            CurrentMetrics.BuildingCount++;
        }
    }
}

void ADir_StudioDirector::ValidateAgentDependencies()
{
    // Check if agents have their dependencies met
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            // Try to unblock agents based on current state
            if (Task.AgentID == 6 && CurrentMetrics.TerrainActors > 0) // Environment Artist
            {
                Task.Status = EDir_AgentStatus::Working;
                UE_LOG(LogTemp, Log, TEXT("Unblocked Agent 6 - terrain is ready"));
            }
            else if (Task.AgentID == 10 && CurrentMetrics.DinosaurCount > 0) // Animation Agent
            {
                Task.Status = EDir_AgentStatus::Working;
                UE_LOG(LogTemp, Log, TEXT("Unblocked Agent 10 - dinosaurs are ready"));
            }
        }
    }
}

void ADir_StudioDirector::UpdateProductionPhase()
{
    // Calculate overall progress based on agent completion
    float TotalProgress = 0.0f;
    int32 ActiveAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status != EDir_AgentStatus::Idle)
        {
            TotalProgress += Task.CompletionPercentage;
            ActiveAgents++;
        }
    }
    
    if (ActiveAgents > 0)
    {
        CurrentMetrics.OverallProgress = TotalProgress / ActiveAgents;
    }
    
    // Update phase based on progress and metrics
    if (CurrentMetrics.OverallProgress >= 80.0f && CurrentMetrics.DinosaurCount >= 5 && 
        CurrentMetrics.TerrainActors > 0)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Alpha;
    }
    else if (CurrentMetrics.OverallProgress >= 50.0f && CurrentMetrics.DinosaurCount >= 3)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Prototype;
    }
}