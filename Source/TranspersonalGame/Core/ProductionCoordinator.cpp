#include "ProductionCoordinator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create coordinator mesh (cube for visibility)
    CoordinatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoordinatorMesh"));
    CoordinatorMesh->SetupAttachment(RootComponent);
    
    // Set default cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CoordinatorMesh->SetStaticMesh(CubeMeshAsset.Object);
    }

    // Scale the mesh
    CoordinatorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));

    // Create status display text
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    StatusDisplay->SetText(FText::FromString(TEXT("Production Coordinator\nCycle 020")));
    StatusDisplay->SetTextRenderColor(FColor::White);
    StatusDisplay->SetWorldSize(48.0f);
    StatusDisplay->SetHorizontalAlignment(EHTA_Center);

    // Initialize production data
    CurrentCycle = TEXT("PROD_CYCLE_AUTO_20260611_004");
    bAutoUpdateMetrics = true;
    
    ProductionMetrics.CurrentPhase = EDir_ProductionPhase::CoreSystems;
    ProductionMetrics.CurrentMilestone = TEXT("Playable Prototype - Walk Around");
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize Cycle 020 tasks on begin play
    InitializeCycle020Tasks();
    UpdateStatusText();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Cycle 020 initialized with %d tasks"), AgentTasks.Num());
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoUpdateMetrics)
    {
        ProductionMetrics = CalculateMetrics();
        
        // Update status display every few seconds
        static float UpdateTimer = 0.0f;
        UpdateTimer += DeltaTime;
        if (UpdateTimer >= 5.0f)
        {
            UpdateStatusText();
            UpdateTimer = 0.0f;
        }
    }
}

void AProductionCoordinator::AssignTask(const FString& AgentName, const FString& TaskDescription, int32 Priority, const FString& Deliverables)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.ExpectedDeliverables = Deliverables;
    NewTask.Status = EDir_AgentStatus::Pending;
    NewTask.AssignedTime = FDateTime::Now();

    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Assigned task to %s - %s"), *AgentName, *TaskDescription);
}

void AProductionCoordinator::UpdateTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            if (NewStatus == EDir_AgentStatus::Completed)
            {
                Task.CompletionTime = FDateTime::Now();
            }
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Updated %s status to %d"), *AgentName, (int32)NewStatus);
}

void AProductionCoordinator::CompleteTask(const FString& AgentName)
{
    UpdateTaskStatus(AgentName, EDir_AgentStatus::Completed);
}

FDir_ProductionMetrics AProductionCoordinator::CalculateMetrics()
{
    FDir_ProductionMetrics Metrics = ProductionMetrics;
    
    Metrics.TotalTasks = AgentTasks.Num();
    Metrics.CompletedTasks = 0;
    Metrics.BlockedTasks = 0;

    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            Metrics.CompletedTasks++;
        }
        else if (Task.Status == EDir_AgentStatus::Blocked)
        {
            Metrics.BlockedTasks++;
        }
    }

    if (Metrics.TotalTasks > 0)
    {
        Metrics.CompletionPercentage = (float)Metrics.CompletedTasks / (float)Metrics.TotalTasks * 100.0f;
    }

    return Metrics;
}

TArray<FDir_AgentTask> AProductionCoordinator::GetTasksByStatus(EDir_AgentStatus Status)
{
    TArray<FDir_AgentTask> FilteredTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == Status)
        {
            FilteredTasks.Add(Task);
        }
    }
    
    return FilteredTasks;
}

TArray<FDir_AgentTask> AProductionCoordinator::GetHighPriorityTasks()
{
    TArray<FDir_AgentTask> HighPriorityTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Priority >= 8 && Task.Status != EDir_AgentStatus::Completed)
        {
            HighPriorityTasks.Add(Task);
        }
    }
    
    // Sort by priority (highest first)
    HighPriorityTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    return HighPriorityTasks;
}

void AProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    ProductionMetrics.CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Phase changed to %d"), (int32)NewPhase);
}

void AProductionCoordinator::GenerateProductionReport()
{
    FDir_ProductionMetrics CurrentMetrics = CalculateMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - %s ==="), *CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Total Tasks: %d"), CurrentMetrics.TotalTasks);
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d"), CurrentMetrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked: %d"), CurrentMetrics.BlockedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Completion: %.1f%%"), CurrentMetrics.CompletionPercentage);
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentMetrics.CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Milestone: %s"), *CurrentMetrics.CurrentMilestone);
    
    // Log high priority pending tasks
    TArray<FDir_AgentTask> HighPriorityTasks = GetHighPriorityTasks();
    if (HighPriorityTasks.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== HIGH PRIORITY PENDING TASKS ==="));
        for (const FDir_AgentTask& Task : HighPriorityTasks)
        {
            UE_LOG(LogTemp, Warning, TEXT("%s: %s (Priority %d)"), *Task.AgentName, *Task.TaskDescription, Task.Priority);
        }
    }
}

void AProductionCoordinator::InitializeCycle020Tasks()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Critical path tasks for playable prototype
    AssignTask(TEXT("Agent02_EngineArchitect"), TEXT("Validate core systems architecture and fix compilation errors"), 10, TEXT("Fixed .h/.cpp files, compilation success"));
    AssignTask(TEXT("Agent03_CoreSystems"), TEXT("Implement physics and collision for character and dinosaurs"), 9, TEXT("PhysicsCore.cpp, CollisionSystem.cpp"));
    AssignTask(TEXT("Agent05_WorldGenerator"), TEXT("Generate varied terrain biomes with PCG"), 9, TEXT("TerrainGenerator.cpp, BiomeSystem.cpp"));
    AssignTask(TEXT("Agent06_EnvironmentArtist"), TEXT("Populate world with prehistoric vegetation and props"), 8, TEXT("VegetationSystem.cpp, PropPlacer.cpp"));
    AssignTask(TEXT("Agent09_CharacterArtist"), TEXT("Create diverse primitive human character variants"), 8, TEXT("CharacterVariants.cpp, MetaHumanIntegration.cpp"));
    AssignTask(TEXT("Agent10_Animation"), TEXT("Implement Motion Matching for fluid character movement"), 9, TEXT("MotionMatchingSystem.cpp, AnimationBlueprints"));
    AssignTask(TEXT("Agent11_NPCBehavior"), TEXT("Program dinosaur behavior trees and daily routines"), 8, TEXT("DinosaurBehaviorTree.cpp, NPCRoutines.cpp"));
    AssignTask(TEXT("Agent12_CombatAI"), TEXT("Create tactical combat AI for dinosaur encounters"), 9, TEXT("CombatAI.cpp, TacticalBehavior.cpp"));
    AssignTask(TEXT("Agent14_QuestDesigner"), TEXT("Design survival objectives and progression system"), 7, TEXT("SurvivalQuests.cpp, ProgressionSystem.cpp"));
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d tasks for Cycle 020"), AgentTasks.Num());
}

void AProductionCoordinator::RefreshStatusDisplay()
{
    UpdateStatusText();
}

void AProductionCoordinator::UpdateStatusText()
{
    FDir_ProductionMetrics CurrentMetrics = CalculateMetrics();
    
    FString StatusText = FString::Printf(TEXT("Production Coordinator\n%s\nCompletion: %.1f%%\nTasks: %d/%d\nPhase: %s"),
        *CurrentCycle,
        CurrentMetrics.CompletionPercentage,
        CurrentMetrics.CompletedTasks,
        CurrentMetrics.TotalTasks,
        *CurrentMetrics.CurrentMilestone
    );
    
    StatusDisplay->SetText(FText::FromString(StatusText));
}

void AProductionCoordinator::LogProductionStatus()
{
    GenerateProductionReport();
}