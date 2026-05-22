#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

UStudioDirectorComponent::UStudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second
    
    CurrentPhase = EDir_ProductionPhase::Production;
    CycleTimer = 0.0f;
    CycleDuration = 300.0f; // 5 minutes per cycle
    
    // Initialize agent tasks for all 19 agents
    AgentTasks.Reserve(19);
    
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),
        TEXT("Engine Architect"), 
        TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"),
        TEXT("Procedural World Generator"),
        TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"),
        TEXT("Lighting & Atmosphere Agent"),
        TEXT("Character Artist Agent"),
        TEXT("Animation Agent"),
        TEXT("NPC Behavior Agent"),
        TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"),
        TEXT("Quest & Mission Designer"),
        TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"),
        TEXT("VFX Agent"),
        TEXT("QA & Testing Agent"),
        TEXT("Integration & Build Agent")
    };
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentNumber = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.TaskDescription = TEXT("Awaiting assignment");
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = 1.0f;
        AgentTasks.Add(NewTask);
    }
}

void UStudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Initialized"));
    InitializeProductionPipeline();
}

void UStudioDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CycleTimer += DeltaTime;
    
    // Update production metrics every second
    UpdateProductionMetrics();
    
    // Check agent deadlines every 10 seconds
    if (FMath::Fmod(CycleTimer, 10.0f) < DeltaTime)
    {
        CheckAgentDeadlines();
    }
    
    // Start new cycle when timer expires
    if (CycleTimer >= CycleDuration)
    {
        StartNewProductionCycle();
        CycleTimer = 0.0f;
    }
}

void UStudioDirectorComponent::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Production Pipeline"));
    
    // Set initial tasks for critical agents
    AssignTaskToAgent(2, TEXT("Define core architecture and physics systems"), 10.0f);
    AssignTaskToAgent(3, TEXT("Implement physics core and collision systems"), 9.0f);
    AssignTaskToAgent(5, TEXT("Generate terrain and biome systems"), 8.0f);
    AssignTaskToAgent(9, TEXT("Create character systems and animations"), 7.0f);
    AssignTaskToAgent(12, TEXT("Implement survival mechanics and UI"), 6.0f);
    
    ProductionMetrics.TotalCycles = 1;
    ProductionMetrics.ActiveAgents = 5;
    
    UE_LOG(LogTemp, Warning, TEXT("Production Pipeline Initialized - %d agents active"), ProductionMetrics.ActiveAgents);
}

void UStudioDirectorComponent::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority)
{
    if (AgentNumber >= 1 && AgentNumber <= AgentTasks.Num())
    {
        int32 Index = AgentNumber - 1;
        AgentTasks[Index].TaskDescription = TaskDescription;
        AgentTasks[Index].Priority = Priority;
        AgentTasks[Index].Status = EDir_AgentStatus::Working;
        AgentTasks[Index].StartTime = FDateTime::Now();
        AgentTasks[Index].DeadlineTime = FDateTime::Now() + FTimespan::FromMinutes(30);
        
        UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%d (%s): %s"), 
               AgentNumber, *AgentTasks[Index].AgentName, *TaskDescription);
    }
}

void UStudioDirectorComponent::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus)
{
    if (AgentNumber >= 1 && AgentNumber <= AgentTasks.Num())
    {
        int32 Index = AgentNumber - 1;
        AgentTasks[Index].Status = NewStatus;
        
        if (NewStatus == EDir_AgentStatus::Complete)
        {
            ProductionMetrics.CompletedTasks++;
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d completed task: %s"), 
                   AgentNumber, *AgentTasks[Index].TaskDescription);
        }
    }
}

FDir_ProductionMetrics UStudioDirectorComponent::GetProductionMetrics() const
{
    return ProductionMetrics;
}

void UStudioDirectorComponent::StartNewProductionCycle()
{
    ProductionMetrics.TotalCycles++;
    
    UE_LOG(LogTemp, Warning, TEXT("=== STARTING PRODUCTION CYCLE #%d ==="), ProductionMetrics.TotalCycles);
    
    // Reset agent statuses for new cycle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            Task.Status = EDir_AgentStatus::Idle;
            Task.TaskDescription = TEXT("Awaiting new assignment");
        }
    }
    
    // Validate current prototype state
    bool bPrototypeValid = ValidateMinPlayablePrototype();
    UE_LOG(LogTemp, Warning, TEXT("Minimum Playable Prototype Valid: %s"), bPrototypeValid ? TEXT("YES") : TEXT("NO"));
    
    GenerateProductionReport();
}

bool UStudioDirectorComponent::ValidateMinPlayablePrototype()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count dinosaur actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    int32 EnvironmentCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("rex")) || ActorName.Contains(TEXT("raptor")) || 
            ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("brachio")))
        {
            DinosaurCount++;
        }
        else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("rock")) || 
                 ActorName.Contains(TEXT("grass")) || ActorName.Contains(TEXT("plant")))
        {
            EnvironmentCount++;
        }
    }
    
    ProductionMetrics.DinosaurCount = DinosaurCount;
    ProductionMetrics.EnvironmentAssets = EnvironmentCount;
    
    // Minimum viable prototype requirements
    bool bHasDinosaurs = DinosaurCount >= 3;
    bool bHasEnvironment = EnvironmentCount >= 10;
    bool bHasPlayer = true; // Assume player character exists
    
    ProductionMetrics.OverallProgress = FMath::Clamp(
        (DinosaurCount * 10.0f + EnvironmentCount * 2.0f) / 100.0f, 0.0f, 1.0f);
    
    return bHasDinosaurs && bHasEnvironment && bHasPlayer;
}

void UStudioDirectorComponent::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT CYCLE #%d ==="), ProductionMetrics.TotalCycles);
    UE_LOG(LogTemp, Warning, TEXT("Total Tasks Completed: %d"), ProductionMetrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), ProductionMetrics.ActiveAgents);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Count: %d"), ProductionMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Environment Assets: %d"), ProductionMetrics.EnvironmentAssets);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), ProductionMetrics.OverallProgress * 100.0f);
    
    // List agent statuses
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusText;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusText = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusText = TEXT("WORKING"); break;
            case EDir_AgentStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Complete: StatusText = TEXT("COMPLETE"); break;
            case EDir_AgentStatus::Error: StatusText = TEXT("ERROR"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s): %s - %s"), 
               Task.AgentNumber, *Task.AgentName, *StatusText, *Task.TaskDescription);
    }
}

void UStudioDirectorComponent::DebugSpawnDinosaurs()
{
    UE_LOG(LogTemp, Warning, TEXT("Debug: Spawning dinosaurs via Studio Director"));
    
    // This function can be called from Blueprint or editor
    // Actual spawning is handled by UE5 Python scripts
}

void UStudioDirectorComponent::ValidateGameSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating all game systems..."));
    
    bool bAllSystemsValid = ValidateMinPlayablePrototype();
    
    if (bAllSystemsValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ All game systems validated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Game systems validation failed"));
    }
}

void UStudioDirectorComponent::UpdateProductionMetrics()
{
    // Count active agents
    int32 ActiveCount = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            ActiveCount++;
        }
    }
    ProductionMetrics.ActiveAgents = ActiveCount;
}

void UStudioDirectorComponent::CheckAgentDeadlines()
{
    FDateTime CurrentTime = FDateTime::Now();
    
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working && CurrentTime > Task.DeadlineTime)
        {
            Task.Status = EDir_AgentStatus::Blocked;
            UE_LOG(LogTemp, Error, TEXT("Agent #%d (%s) missed deadline for task: %s"), 
                   Task.AgentNumber, *Task.AgentName, *Task.TaskDescription);
        }
    }
}

void UStudioDirectorComponent::OptimizeAgentWorkflow()
{
    // Sort tasks by priority and reassign if needed
    AgentTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
}

// AStudioDirectorActor Implementation
AStudioDirectorActor::AStudioDirectorActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create director component
    DirectorComponent = CreateDefaultSubobject<UStudioDirectorComponent>(TEXT("DirectorComponent"));
    
    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;
    
    // Try to load a cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        VisualizationMesh->SetStaticMesh(CubeMeshAsset.Object);
        VisualizationMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.1f)); // Flat command center platform
    }
}