#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize production settings
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260502_001");
    DailyBudgetUsed = 0.0f;
    DailyBudgetLimit = 150.0f;
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized for cycle: %s"), *CurrentCycleID);
    
    // Initialize Milestone 1 on startup
    InitializeMilestone1();
    
    // Validate current map state
    ValidateMinPlayableMap();
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Check agent dependencies every 5 seconds
    static float DependencyCheckTimer = 0.0f;
    DependencyCheckTimer += DeltaTime;
    
    if (DependencyCheckTimer >= 5.0f)
    {
        CheckAgentDependencies();
        DependencyCheckTimer = 0.0f;
    }
}

void ADir_ProductionCoordinator::InitializeMilestone1()
{
    FDir_Milestone Milestone1;
    Milestone1.MilestoneName = TEXT("Milestone 1 - Walk Around");
    Milestone1.Description = TEXT("Player can walk around terrain with visible dinosaurs");
    Milestone1.Status = EDir_MilestoneStatus::InProgress;
    
    // Define required tasks for Milestone 1
    TArray<FDir_AgentTask> RequiredTasks;
    
    FDir_AgentTask Task1;
    Task1.AgentName = TEXT("Engine_Architect");
    Task1.TaskDescription = TEXT("Define core architecture and module dependencies");
    Task1.Priority = EDir_AgentPriority::Critical;
    Task1.EstimatedHours = 2.0f;
    RequiredTasks.Add(Task1);
    
    FDir_AgentTask Task2;
    Task2.AgentName = TEXT("Core_Systems");
    Task2.TaskDescription = TEXT("Implement physics and collision for character movement");
    Task2.Priority = EDir_AgentPriority::Critical;
    Task2.EstimatedHours = 3.0f;
    RequiredTasks.Add(Task2);
    
    FDir_AgentTask Task3;
    Task3.AgentName = TEXT("World_Generator");
    Task3.TaskDescription = TEXT("Enhance terrain with height variation and biome zones");
    Task3.Priority = EDir_AgentPriority::High;
    Task3.EstimatedHours = 4.0f;
    RequiredTasks.Add(Task3);
    
    FDir_AgentTask Task4;
    Task4.AgentName = TEXT("Character_Artist");
    Task4.TaskDescription = TEXT("Improve TranspersonalCharacter with proper mesh and animations");
    Task4.Priority = EDir_AgentPriority::High;
    Task4.EstimatedHours = 3.0f;
    RequiredTasks.Add(Task4);
    
    FDir_AgentTask Task5;
    Task5.AgentName = TEXT("NPC_Behavior");
    Task5.TaskDescription = TEXT("Create basic dinosaur AI with territorial behavior");
    Task5.Priority = EDir_AgentPriority::High;
    Task5.EstimatedHours = 4.0f;
    RequiredTasks.Add(Task5);
    
    Milestone1.RequiredTasks = RequiredTasks;
    Milestone1.CompletionPercentage = 0.0f;
    
    Milestones.Add(Milestone1);
    CurrentTasks = RequiredTasks;
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 initialized with %d tasks"), RequiredTasks.Num());
}

void ADir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, EDir_AgentPriority Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.EstimatedHours = 2.0f;
    NewTask.bIsCompleted = false;
    
    CurrentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription);
}

void ADir_ProductionCoordinator::CompleteTask(const FString& AgentName, const FString& TaskDescription)
{
    for (FDir_AgentTask& Task : CurrentTasks)
    {
        if (Task.AgentName == AgentName && Task.TaskDescription == TaskDescription)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Task completed by %s: %s"), *AgentName, *TaskDescription);
            break;
        }
    }
    
    // Update milestone progress
    for (FDir_Milestone& Milestone : Milestones)
    {
        if (Milestone.Status == EDir_MilestoneStatus::InProgress)
        {
            int32 CompletedTasks = 0;
            for (const FDir_AgentTask& Task : Milestone.RequiredTasks)
            {
                if (Task.bIsCompleted)
                {
                    CompletedTasks++;
                }
            }
            
            Milestone.CompletionPercentage = (float)CompletedTasks / (float)Milestone.RequiredTasks.Num() * 100.0f;
            
            if (Milestone.CompletionPercentage >= 100.0f)
            {
                Milestone.Status = EDir_MilestoneStatus::Completed;
                UE_LOG(LogTemp, Warning, TEXT("Milestone completed: %s"), *Milestone.MilestoneName);
            }
        }
    }
}

float ADir_ProductionCoordinator::GetMilestoneProgress(const FString& MilestoneName)
{
    for (const FDir_Milestone& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.CompletionPercentage;
        }
    }
    return 0.0f;
}

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetTasksByPriority(EDir_AgentPriority Priority)
{
    TArray<FDir_AgentTask> FilteredTasks;
    
    for (const FDir_AgentTask& Task : CurrentTasks)
    {
        if (Task.Priority == Priority && !Task.bIsCompleted)
        {
            FilteredTasks.Add(Task);
        }
    }
    
    return FilteredTasks;
}

void ADir_ProductionCoordinator::UpdateCycleStatus(const FString& CycleID, float BudgetUsed)
{
    CurrentCycleID = CycleID;
    DailyBudgetUsed += BudgetUsed;
    
    UE_LOG(LogTemp, Warning, TEXT("Cycle %s - Budget used: $%.2f / $%.2f"), 
           *CycleID, DailyBudgetUsed, DailyBudgetLimit);
}

void ADir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s"), *CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Budget: $%.2f / $%.2f"), DailyBudgetUsed, DailyBudgetLimit);
    
    for (const FDir_Milestone& Milestone : Milestones)
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone: %s - %.1f%% complete"), 
               *Milestone.MilestoneName, Milestone.CompletionPercentage);
    }
    
    int32 CriticalTasks = GetTasksByPriority(EDir_AgentPriority::Critical).Num();
    int32 HighTasks = GetTasksByPriority(EDir_AgentPriority::High).Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Pending Tasks - Critical: %d, High: %d"), CriticalTasks, HighTasks);
}

void ADir_ProductionCoordinator::CleanupDuplicateActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<ADirectionalLight*> DirectionalLights;
    TArray<AActor*> SkyAtmospheres;
    TArray<AActor*> SkyLights;
    TArray<AActor*> HeightFogs;
    
    // Find all lighting actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("DirectionalLight")))
        {
            if (ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor))
            {
                DirectionalLights.Add(DirLight);
            }
        }
        else if (ClassName.Contains(TEXT("SkyAtmosphere")))
        {
            SkyAtmospheres.Add(Actor);
        }
        else if (ClassName.Contains(TEXT("SkyLight")))
        {
            SkyLights.Add(Actor);
        }
        else if (ClassName.Contains(TEXT("ExponentialHeightFog")))
        {
            HeightFogs.Add(Actor);
        }
    }
    
    int32 CleanupCount = 0;
    
    // Keep only the first of each type
    for (int32 i = 1; i < DirectionalLights.Num(); i++)
    {
        DirectionalLights[i]->Destroy();
        CleanupCount++;
    }
    
    for (int32 i = 1; i < SkyAtmospheres.Num(); i++)
    {
        SkyAtmospheres[i]->Destroy();
        CleanupCount++;
    }
    
    for (int32 i = 1; i < SkyLights.Num(); i++)
    {
        SkyLights[i]->Destroy();
        CleanupCount++;
    }
    
    for (int32 i = 1; i < HeightFogs.Num(); i++)
    {
        HeightFogs[i]->Destroy();
        CleanupCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cleaned up %d duplicate lighting actors"), CleanupCount);
}

void ADir_ProductionCoordinator::ValidateMinPlayableMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 CharacterCount = 0;
    int32 DinosaurCount = 0;
    int32 TerrainCount = 0;
    int32 TreeCount = 0;
    int32 RockCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        FString ClassName = Actor->GetClass()->GetName().ToLower();
        
        if (ActorName.Contains(TEXT("character")) || ClassName.Contains(TEXT("character")))
        {
            CharacterCount++;
        }
        else if (ActorName.Contains(TEXT("rex")) || ActorName.Contains(TEXT("raptor")) || 
                 ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("dinosaur")))
        {
            DinosaurCount++;
        }
        else if (ClassName.Contains(TEXT("landscape")))
        {
            TerrainCount++;
        }
        else if (ActorName.Contains(TEXT("tree")))
        {
            TreeCount++;
        }
        else if (ActorName.Contains(TEXT("rock")))
        {
            RockCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== MAP VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), CharacterCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Terrain: %d"), TerrainCount);
    UE_LOG(LogTemp, Warning, TEXT("Trees: %d"), TreeCount);
    UE_LOG(LogTemp, Warning, TEXT("Rocks: %d"), RockCount);
}

void ADir_ProductionCoordinator::CheckAgentDependencies()
{
    // Check if critical dependencies are met
    bool bEngineArchitectureReady = false;
    bool bCoreSystemsReady = false;
    
    for (const FDir_AgentTask& Task : CurrentTasks)
    {
        if (Task.AgentName == TEXT("Engine_Architect") && Task.bIsCompleted)
        {
            bEngineArchitectureReady = true;
        }
        else if (Task.AgentName == TEXT("Core_Systems") && Task.bIsCompleted)
        {
            bCoreSystemsReady = true;
        }
    }
    
    // Block dependent agents if prerequisites not met
    if (!bEngineArchitectureReady)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Engine Architecture not ready - blocking dependent agents"));
    }
    
    if (!bCoreSystemsReady && bEngineArchitectureReady)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Core Systems not ready - physics-dependent agents blocked"));
    }
}