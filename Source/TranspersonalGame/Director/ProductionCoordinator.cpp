#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize production state
    CurrentPhase = EDir_ProductionPhase::Prototype;
    CurrentCycle = 9;
    CycleID = TEXT("PROD_CYCLE_AUTO_20260611_009");

    // Initialize production metrics
    ProductionMetrics = FDir_ProductionMetrics();
    ProductionMetrics.LastUpdateTime = FDateTime::Now().ToString();
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Starting Cycle %d - %s"), CurrentCycle, *CycleID);
    
    // Initialize agent coordination system
    InitializeAgentTasks();
    
    // Run initial production assessment
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized with %d agents in %s phase"), 
           AgentTasks.Num(), *UEnum::GetValueAsString(CurrentPhase));
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update production metrics every 5 seconds
    static float MetricsUpdateTimer = 0.0f;
    MetricsUpdateTimer += DeltaTime;
    
    if (MetricsUpdateTimer >= 5.0f)
    {
        UpdateProductionMetrics();
        MetricsUpdateTimer = 0.0f;
    }
}

void AProductionCoordinator::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: No valid world for metrics update"));
        return;
    }

    // Reset counters
    ProductionMetrics.TotalActors = 0;
    ProductionMetrics.CharacterActors = 0;
    ProductionMetrics.DinosaurActors = 0;
    ProductionMetrics.TerrainActors = 0;

    // Count all actors by type
    CountActorsByType();

    // Calculate completion percentage based on minimum viable prototype requirements
    float CompletionScore = 0.0f;
    
    // Character system (25% of MVP)
    if (ProductionMetrics.CharacterActors > 0)
    {
        CompletionScore += 25.0f;
    }
    
    // Dinosaur content (25% of MVP)
    if (ProductionMetrics.DinosaurActors >= 3)
    {
        CompletionScore += 25.0f;
    }
    else if (ProductionMetrics.DinosaurActors > 0)
    {
        CompletionScore += (ProductionMetrics.DinosaurActors / 3.0f) * 25.0f;
    }
    
    // Terrain system (25% of MVP)
    if (ProductionMetrics.TerrainActors > 0)
    {
        CompletionScore += 25.0f;
    }
    
    // Basic world setup (25% of MVP) - lights, sky, etc.
    if (ProductionMetrics.TotalActors >= 10)
    {
        CompletionScore += 25.0f;
    }
    else if (ProductionMetrics.TotalActors > 0)
    {
        CompletionScore += (ProductionMetrics.TotalActors / 10.0f) * 25.0f;
    }

    ProductionMetrics.CompletionPercentage = FMath::Clamp(CompletionScore, 0.0f, 100.0f);
    ProductionMetrics.LastUpdateTime = FDateTime::Now().ToString();

    // Log production status
    LogProductionStatus();

    // Validate MVP requirements
    ValidateMinimumViablePrototype();
}

void AProductionCoordinator::CountActorsByType()
{
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }

        ProductionMetrics.TotalActors++;

        FString ActorName = Actor->GetName().ToLower();
        FString ClassName = Actor->GetClass()->GetName().ToLower();

        // Count character actors
        if (Actor->IsA<ACharacter>() || ClassName.Contains(TEXT("character")) || ClassName.Contains(TEXT("transpersonal")))
        {
            ProductionMetrics.CharacterActors++;
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Found character actor - %s"), *Actor->GetName());
        }

        // Count dinosaur actors
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
            ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("dinosaur")) ||
            ClassName.Contains(TEXT("dinosaur")))
        {
            ProductionMetrics.DinosaurActors++;
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Found dinosaur actor - %s"), *Actor->GetName());
        }

        // Count terrain/landscape actors
        if (Actor->IsA<ALandscape>() || ClassName.Contains(TEXT("landscape")) || ClassName.Contains(TEXT("terrain")))
        {
            ProductionMetrics.TerrainActors++;
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Found terrain actor - %s"), *Actor->GetName());
        }
    }
}

void AProductionCoordinator::InitializeAgentTasks()
{
    AgentTasks.Empty();

    // Define all 19 agents and their current tasks for Cycle 009
    TArray<TPair<FString, FString>> AgentDefinitions = {
        {TEXT("Studio Director"), TEXT("Coordinate production pipeline and agent tasks")},
        {TEXT("Engine Architect"), TEXT("Define technical architecture and compilation rules")},
        {TEXT("Core Systems"), TEXT("Implement physics, collision, and core gameplay systems")},
        {TEXT("Performance Optimizer"), TEXT("Ensure 60fps performance and optimization")},
        {TEXT("World Generator"), TEXT("Create procedural terrain and biome systems")},
        {TEXT("Environment Artist"), TEXT("Populate world with vegetation and props")},
        {TEXT("Architecture Agent"), TEXT("Build prehistoric structures and shelters")},
        {TEXT("Lighting Agent"), TEXT("Implement day/night cycle and atmospheric lighting")},
        {TEXT("Character Artist"), TEXT("Create player character and NPC systems")},
        {TEXT("Animation Agent"), TEXT("Implement character animations and IK systems")},
        {TEXT("NPC Behavior"), TEXT("Design NPC AI and behavior trees")},
        {TEXT("Combat & Enemy AI"), TEXT("Create dinosaur AI and combat systems")},
        {TEXT("Crowd Simulation"), TEXT("Implement mass AI for large dinosaur herds")},
        {TEXT("Quest Designer"), TEXT("Create survival missions and objectives")},
        {TEXT("Narrative Designer"), TEXT("Write game lore and survival narrative")},
        {TEXT("Audio Agent"), TEXT("Implement adaptive audio and sound effects")},
        {TEXT("VFX Agent"), TEXT("Create particle effects and visual feedback")},
        {TEXT("QA Testing"), TEXT("Test systems and ensure quality standards")},
        {TEXT("Integration Agent"), TEXT("Integrate all systems into cohesive build")}
    };

    for (int32 i = 0; i < AgentDefinitions.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = i + 1;
        NewTask.AgentName = AgentDefinitions[i].Key;
        NewTask.CurrentTask = AgentDefinitions[i].Value;
        NewTask.Status = (i == 0) ? EDir_AgentStatus::Working : EDir_AgentStatus::Idle;
        NewTask.Progress = 0.0f;
        NewTask.LastOutput = TEXT("Awaiting cycle start");
        
        AgentTasks.Add(NewTask);
    }

    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d agent tasks"), AgentTasks.Num());
}

void AProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            Task.CurrentTask = TaskDescription;
            Task.LastOutput = FDateTime::Now().ToString();
            
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Agent %d (%s) status updated to %s - %s"), 
                   AgentID, *Task.AgentName, *UEnum::GetValueAsString(NewStatus), *TaskDescription);
            break;
        }
    }
}

void AProductionCoordinator::AdvanceToNextPhase()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::Prototype;
            break;
        case EDir_ProductionPhase::Prototype:
            CurrentPhase = EDir_ProductionPhase::Production;
            break;
        case EDir_ProductionPhase::Production:
            CurrentPhase = EDir_ProductionPhase::Polish;
            break;
        case EDir_ProductionPhase::Polish:
            CurrentPhase = EDir_ProductionPhase::Release;
            break;
        case EDir_ProductionPhase::Release:
            // Already at final phase
            break;
    }

    OnProductionPhaseChanged();
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Advanced to phase %s"), 
           *UEnum::GetValueAsString(CurrentPhase));
}

void AProductionCoordinator::RunProductionDiagnostics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION DIAGNOSTICS - CYCLE %d ==="), CurrentCycle);
    
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *UEnum::GetValueAsString(CurrentPhase));
    UE_LOG(LogTemp, Warning, TEXT("Completion: %.1f%%"), ProductionMetrics.CompletionPercentage);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), ProductionMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), ProductionMetrics.CharacterActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), ProductionMetrics.DinosaurActors);
    UE_LOG(LogTemp, Warning, TEXT("Terrain: %d"), ProductionMetrics.TerrainActors);
    
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT STATUS ==="));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent %d (%s): %s - %s"), 
               Task.AgentID, *Task.AgentName, *UEnum::GetValueAsString(Task.Status), *Task.CurrentTask);
    }
}

void AProductionCoordinator::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("PRODUCTION REPORT - CYCLE %d\n"), CurrentCycle);
    Report += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(CurrentPhase));
    Report += FString::Printf(TEXT("Completion: %.1f%%\n"), ProductionMetrics.CompletionPercentage);
    Report += FString::Printf(TEXT("Last Update: %s\n\n"), *ProductionMetrics.LastUpdateTime);
    
    Report += TEXT("ASSET COUNTS:\n");
    Report += FString::Printf(TEXT("- Total Actors: %d\n"), ProductionMetrics.TotalActors);
    Report += FString::Printf(TEXT("- Character Actors: %d\n"), ProductionMetrics.CharacterActors);
    Report += FString::Printf(TEXT("- Dinosaur Actors: %d\n"), ProductionMetrics.DinosaurActors);
    Report += FString::Printf(TEXT("- Terrain Actors: %d\n\n"), ProductionMetrics.TerrainActors);
    
    Report += TEXT("AGENT STATUS:\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        Report += FString::Printf(TEXT("- Agent %d (%s): %s\n"), 
                                  Task.AgentID, *Task.AgentName, *UEnum::GetValueAsString(Task.Status));
    }

    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                                         FString::Printf(TEXT("Production Report Generated - %.1f%% Complete"), 
                                                         ProductionMetrics.CompletionPercentage));
    }
}

void AProductionCoordinator::OnProductionPhaseChanged()
{
    // Reset all agent statuses when phase changes
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.Progress = 0.0f;
    }
    
    // Update current agent (Studio Director) to working
    if (AgentTasks.Num() > 0)
    {
        AgentTasks[0].Status = EDir_AgentStatus::Working;
    }
}

void AProductionCoordinator::ValidateMinimumViablePrototype()
{
    bool bMVPReady = true;
    TArray<FString> MissingElements;

    if (ProductionMetrics.CharacterActors == 0)
    {
        bMVPReady = false;
        MissingElements.Add(TEXT("Character System"));
    }

    if (ProductionMetrics.DinosaurActors < 3)
    {
        bMVPReady = false;
        MissingElements.Add(FString::Printf(TEXT("Dinosaur Content (%d/3)"), ProductionMetrics.DinosaurActors));
    }

    if (ProductionMetrics.TerrainActors == 0)
    {
        bMVPReady = false;
        MissingElements.Add(TEXT("Terrain System"));
    }

    if (ProductionMetrics.TotalActors < 10)
    {
        bMVPReady = false;
        MissingElements.Add(FString::Printf(TEXT("Basic World Setup (%d/10 actors)"), ProductionMetrics.TotalActors));
    }

    if (!bMVPReady)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: MVP NOT READY - Missing: %s"), 
               *FString::Join(MissingElements, TEXT(", ")));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: MVP READY - All core systems present"));
    }
}

void AProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Metrics updated - %.1f%% complete, %d total actors"), 
           ProductionMetrics.CompletionPercentage, ProductionMetrics.TotalActors);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                                         FString::Printf(TEXT("Production: %.1f%% | Actors: %d | Characters: %d | Dinosaurs: %d"), 
                                                         ProductionMetrics.CompletionPercentage,
                                                         ProductionMetrics.TotalActors,
                                                         ProductionMetrics.CharacterActors,
                                                         ProductionMetrics.DinosaurActors));
    }
}