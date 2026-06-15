#include "Dir_StudioDirector.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"

ADir_StudioDirector::ADir_StudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create director visualization mesh
    DirectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DirectorMesh"));
    RootComponent = DirectorMesh;

    // Load cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        DirectorMesh->SetStaticMesh(CubeMesh.Object);
        DirectorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }

    // Initialize production state
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    CycleTimer = 0.0f;
    CurrentCycle = 0;

    // Initialize metrics
    CurrentMetrics.TotalActors = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.EnvironmentProps = 0;
    CurrentMetrics.LightingActors = 0;
    CurrentMetrics.OverallProgress = 0.0f;
    CurrentMetrics.bPlayablePrototype = false;
}

void ADir_StudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionCycle();
    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized - Production Cycle started"));
}

void ADir_StudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CycleTimer += DeltaTime;

    // Update metrics every 5 seconds
    if (FMath::Fmod(CycleTimer, 5.0f) < DeltaTime)
    {
        UpdateMetrics();
        LogProductionStatus();
    }

    // Advance cycle every 30 seconds (simulating agent cycles)
    if (CycleTimer >= 30.0f)
    {
        CurrentCycle++;
        CycleTimer = 0.0f;
        GenerateNextCycleTasks();
        CoordinateAgentSequence();
    }
}

void ADir_StudioDirector::InitializeProductionCycle()
{
    AgentTasks.Empty();

    // Initialize tasks for all 19 agents
    AssignTaskToAgent(1, "Studio Director - Coordinate production cycle", 10.0f);
    AssignTaskToAgent(2, "Engine Architect - Define core architecture", 9.0f);
    AssignTaskToAgent(3, "Core Systems - Implement physics and collision", 8.0f);
    AssignTaskToAgent(4, "Performance Optimizer - Ensure 60fps target", 7.0f);
    AssignTaskToAgent(5, "Procedural World - Generate terrain and biomes", 9.0f);
    AssignTaskToAgent(6, "Environment Artist - Populate world with props", 8.0f);
    AssignTaskToAgent(7, "Architecture Agent - Build prehistoric structures", 6.0f);
    AssignTaskToAgent(8, "Lighting Agent - Setup day/night cycle", 7.0f);
    AssignTaskToAgent(9, "Character Artist - Create human and dinosaur models", 9.0f);
    AssignTaskToAgent(10, "Animation Agent - Implement motion matching", 8.0f);
    AssignTaskToAgent(11, "NPC Behavior - Program dinosaur AI", 8.0f);
    AssignTaskToAgent(12, "Combat AI - Implement tactical combat", 7.0f);
    AssignTaskToAgent(13, "Crowd Simulation - Mass AI for herds", 6.0f);
    AssignTaskToAgent(14, "Quest Designer - Create survival missions", 5.0f);
    AssignTaskToAgent(15, "Narrative Agent - Write game lore", 4.0f);
    AssignTaskToAgent(16, "Audio Agent - Implement adaptive audio", 6.0f);
    AssignTaskToAgent(17, "VFX Agent - Create particle effects", 5.0f);
    AssignTaskToAgent(18, "QA Testing - Validate all systems", 8.0f);
    AssignTaskToAgent(19, "Integration Agent - Build final version", 9.0f);

    UE_LOG(LogTemp, Warning, TEXT("Production cycle initialized with %d agent tasks"), AgentTasks.Num());
}

void ADir_StudioDirector::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bCompleted = false;
    NewTask.CompletionPercentage = 0.0f;

    AgentTasks.Add(NewTask);
}

void ADir_StudioDirector::UpdateAgentProgress(int32 AgentID, float CompletionPercentage, bool bCompleted)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
            Task.bCompleted = bCompleted;
            break;
        }
    }
}

void ADir_StudioDirector::AnalyzeCurrentState()
{
    UpdateMetrics();

    // Determine if playable prototype is ready
    bool bHasCharacter = CurrentMetrics.TotalActors > 0;
    bool bHasTerrain = CurrentMetrics.EnvironmentProps > 10;
    bool bHasDinosaurs = CurrentMetrics.DinosaurCount > 3;
    bool bHasLighting = CurrentMetrics.LightingActors > 0;

    CurrentMetrics.bPlayablePrototype = bHasCharacter && bHasTerrain && bHasDinosaurs && bHasLighting;

    // Calculate overall progress
    float CompletedTasks = 0.0f;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        CompletedTasks += Task.CompletionPercentage / 100.0f;
    }
    CurrentMetrics.OverallProgress = (CompletedTasks / AgentTasks.Num()) * 100.0f;
}

void ADir_StudioDirector::AdvanceProductionPhase()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            if (CurrentMetrics.OverallProgress > 20.0f)
            {
                CurrentPhase = EDir_ProductionPhase::CoreSystems;
                UE_LOG(LogTemp, Warning, TEXT("Production advanced to Core Systems phase"));
            }
            break;
        case EDir_ProductionPhase::CoreSystems:
            if (CurrentMetrics.OverallProgress > 40.0f)
            {
                CurrentPhase = EDir_ProductionPhase::ContentCreation;
                UE_LOG(LogTemp, Warning, TEXT("Production advanced to Content Creation phase"));
            }
            break;
        case EDir_ProductionPhase::ContentCreation:
            if (CurrentMetrics.OverallProgress > 70.0f)
            {
                CurrentPhase = EDir_ProductionPhase::Polish;
                UE_LOG(LogTemp, Warning, TEXT("Production advanced to Polish phase"));
            }
            break;
        case EDir_ProductionPhase::Polish:
            if (CurrentMetrics.OverallProgress > 90.0f)
            {
                CurrentPhase = EDir_ProductionPhase::Testing;
                UE_LOG(LogTemp, Warning, TEXT("Production advanced to Testing phase"));
            }
            break;
        case EDir_ProductionPhase::Testing:
            if (CurrentMetrics.bPlayablePrototype)
            {
                CurrentPhase = EDir_ProductionPhase::Release;
                UE_LOG(LogTemp, Warning, TEXT("Production ready for Release!"));
            }
            break;
        default:
            break;
    }
}

TArray<FDir_AgentTask> ADir_StudioDirector::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    return PendingTasks;
}

void ADir_StudioDirector::GenerateNextCycleTasks()
{
    // Update task priorities based on current phase
    for (FDir_AgentTask& Task : AgentTasks)
    {
        switch (CurrentPhase)
        {
            case EDir_ProductionPhase::PreProduction:
                // Prioritize architecture and core systems
                if (Task.AgentID <= 4) Task.Priority += 1.0f;
                break;
            case EDir_ProductionPhase::CoreSystems:
                // Prioritize world generation and environment
                if (Task.AgentID >= 5 && Task.AgentID <= 8) Task.Priority += 1.0f;
                break;
            case EDir_ProductionPhase::ContentCreation:
                // Prioritize characters and gameplay
                if (Task.AgentID >= 9 && Task.AgentID <= 14) Task.Priority += 1.0f;
                break;
            default:
                break;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Generated tasks for cycle %d in phase %d"), 
           CurrentCycle, (int32)CurrentPhase);
}

bool ADir_StudioDirector::IsPlayablePrototypeReady() const
{
    return CurrentMetrics.bPlayablePrototype;
}

void ADir_StudioDirector::UpdateMetrics()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        CurrentMetrics.TotalActors = AllActors.Num();
        CurrentMetrics.DinosaurCount = 0;
        CurrentMetrics.EnvironmentProps = 0;
        CurrentMetrics.LightingActors = 0;

        // Count specific actor types
        for (AActor* Actor : AllActors)
        {
            if (Actor && !Actor->GetName().IsEmpty())
            {
                FString ActorName = Actor->GetName().ToLower();
                
                if (ActorName.Contains("dino") || ActorName.Contains("trex") || 
                    ActorName.Contains("raptor") || ActorName.Contains("brachi"))
                {
                    CurrentMetrics.DinosaurCount++;
                }
                else if (ActorName.Contains("tree") || ActorName.Contains("rock") || 
                         ActorName.Contains("plant") || ActorName.Contains("prop"))
                {
                    CurrentMetrics.EnvironmentProps++;
                }
                else if (ActorName.Contains("light") || ActorName.Contains("sun") || 
                         ActorName.Contains("sky"))
                {
                    CurrentMetrics.LightingActors++;
                }
            }
        }
    }
}

void ADir_StudioDirector::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %d | Phase: %d"), CurrentCycle, (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Environment Props: %d"), CurrentMetrics.EnvironmentProps);
    UE_LOG(LogTemp, Warning, TEXT("Lighting Actors: %d"), CurrentMetrics.LightingActors);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Playable Prototype: %s"), 
           CurrentMetrics.bPlayablePrototype ? TEXT("YES") : TEXT("NO"));
}

void ADir_StudioDirector::ValidateAgentOutputs()
{
    // Validate that agents are producing actual content
    int32 CompletedAgents = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bCompleted || Task.CompletionPercentage > 80.0f)
        {
            CompletedAgents++;
        }
    }

    float CompletionRate = (float)CompletedAgents / AgentTasks.Num();
    if (CompletionRate < 0.3f)
    {
        UE_LOG(LogTemp, Error, TEXT("WARNING: Low agent completion rate: %.1f%%"), 
               CompletionRate * 100.0f);
    }
}

void ADir_StudioDirector::CoordinateAgentSequence()
{
    // Ensure agents work in proper sequence based on dependencies
    AnalyzeCurrentState();
    AdvanceProductionPhase();
    ValidateAgentOutputs();
    
    UE_LOG(LogTemp, Warning, TEXT("Agent coordination complete for cycle %d"), CurrentCycle);
}