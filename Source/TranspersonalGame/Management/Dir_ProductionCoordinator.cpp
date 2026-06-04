#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create coordinator mesh component
    CoordinatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoordinatorMesh"));
    CoordinatorMesh->SetupAttachment(RootComponent);

    // Set default values
    bAutoUpdateMetrics = true;
    MetricsUpdateInterval = 5.0f;

    // Initialize production metrics
    ProductionMetrics = FDir_ProductionMetrics();
    
    SetupCoordinatorMesh();
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();

    // Initialize agent tasks
    InitializeAgentTasks();

    // Start metrics update timer if auto-update is enabled
    if (bAutoUpdateMetrics)
    {
        GetWorldTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &ADir_ProductionCoordinator::UpdateProductionMetrics,
            MetricsUpdateInterval,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized with %d agent tasks"), AgentTasks.Num());
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update mesh material based on overall progress
    UpdateMeshMaterial();
}

void ADir_ProductionCoordinator::InitializeAgentTasks()
{
    AgentTasks.Empty();

    // Define the 19-agent production chain
    TArray<TPair<FString, EDir_MilestoneType>> AgentDefinitions = {
        {TEXT("Agent01_StudioDirector"), EDir_MilestoneType::Architecture},
        {TEXT("Agent02_EngineArchitect"), EDir_MilestoneType::Architecture},
        {TEXT("Agent03_CoreSystems"), EDir_MilestoneType::CoreSystems},
        {TEXT("Agent04_PerformanceOptimizer"), EDir_MilestoneType::CoreSystems},
        {TEXT("Agent05_WorldGenerator"), EDir_MilestoneType::WorldGeneration},
        {TEXT("Agent06_EnvironmentArtist"), EDir_MilestoneType::WorldGeneration},
        {TEXT("Agent07_Architecture"), EDir_MilestoneType::WorldGeneration},
        {TEXT("Agent08_Lighting"), EDir_MilestoneType::WorldGeneration},
        {TEXT("Agent09_CharacterArtist"), EDir_MilestoneType::Character},
        {TEXT("Agent10_Animation"), EDir_MilestoneType::Animation},
        {TEXT("Agent11_NPCBehavior"), EDir_MilestoneType::AI},
        {TEXT("Agent12_CombatAI"), EDir_MilestoneType::AI},
        {TEXT("Agent13_CrowdSimulation"), EDir_MilestoneType::AI},
        {TEXT("Agent14_QuestDesigner"), EDir_MilestoneType::Narrative},
        {TEXT("Agent15_Narrative"), EDir_MilestoneType::Narrative},
        {TEXT("Agent16_Audio"), EDir_MilestoneType::CoreSystems},
        {TEXT("Agent17_VFX"), EDir_MilestoneType::CoreSystems},
        {TEXT("Agent18_QA"), EDir_MilestoneType::QA},
        {TEXT("Agent19_Integration"), EDir_MilestoneType::QA}
    };

    // Create tasks for each agent
    for (int32 i = 0; i < AgentDefinitions.Num(); ++i)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentDefinitions[i].Key;
        NewTask.MilestoneType = AgentDefinitions[i].Value;
        NewTask.Status = EDir_AgentStatus::Pending;
        NewTask.Priority = 1.0f + (float)i * 0.1f; // Sequential priority
        NewTask.TaskDescription = FString::Printf(TEXT("Core implementation for %s"), *NewTask.AgentName);

        // Set dependencies based on agent chain
        if (i > 0)
        {
            NewTask.Dependencies.Add(AgentDefinitions[i - 1].Key);
        }

        AgentTasks.Add(NewTask);
    }

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agent tasks"), AgentTasks.Num());
}

void ADir_ProductionCoordinator::UpdateAgentTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Updated %s status to %d"), *AgentName, (int32)NewStatus);
            break;
        }
    }

    // Update metrics after status change
    UpdateProductionMetrics();
}

void ADir_ProductionCoordinator::AddAgentTask(const FDir_AgentTask& NewTask)
{
    AgentTasks.Add(NewTask);
    UpdateProductionMetrics();
}

FDir_AgentTask ADir_ProductionCoordinator::GetAgentTask(const FString& AgentName)
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            return Task;
        }
    }
    return FDir_AgentTask(); // Return default task if not found
}

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetTasksByStatus(EDir_AgentStatus Status)
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

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetTasksByMilestone(EDir_MilestoneType MilestoneType)
{
    TArray<FDir_AgentTask> FilteredTasks;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.MilestoneType == MilestoneType)
        {
            FilteredTasks.Add(Task);
        }
    }
    return FilteredTasks;
}

void ADir_ProductionCoordinator::UpdateProductionMetrics()
{
    ProductionMetrics.TotalAgents = AgentTasks.Num();
    ProductionMetrics.CompletedTasks = GetTasksByStatus(EDir_AgentStatus::Complete).Num();
    ProductionMetrics.PendingTasks = GetTasksByStatus(EDir_AgentStatus::Pending).Num();
    ProductionMetrics.BlockedTasks = GetTasksByStatus(EDir_AgentStatus::Blocked).Num();
    ProductionMetrics.ActiveAgents = GetTasksByStatus(EDir_AgentStatus::InProgress).Num();
    ProductionMetrics.OverallProgress = CalculateOverallProgress();
    ProductionMetrics.LastUpdate = FDateTime::Now();

    UE_LOG(LogTemp, Warning, TEXT("Production Metrics - Progress: %.1f%%, Active: %d, Completed: %d, Pending: %d"), 
           ProductionMetrics.OverallProgress, 
           ProductionMetrics.ActiveAgents, 
           ProductionMetrics.CompletedTasks, 
           ProductionMetrics.PendingTasks);
}

float ADir_ProductionCoordinator::CalculateOverallProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedCount = 0;
    int32 InProgressCount = 0;

    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            CompletedCount++;
        }
        else if (Task.Status == EDir_AgentStatus::InProgress)
        {
            InProgressCount++;
        }
    }

    // Complete tasks = 100%, in-progress = 50%
    float TotalProgress = (float)CompletedCount + ((float)InProgressCount * 0.5f);
    return (TotalProgress / (float)AgentTasks.Num()) * 100.0f;
}

bool ADir_ProductionCoordinator::ValidateMilestoneDependencies(EDir_MilestoneType MilestoneType)
{
    TArray<FDir_AgentTask> MilestoneTasks = GetTasksByMilestone(MilestoneType);
    
    for (const FDir_AgentTask& Task : MilestoneTasks)
    {
        // Check if all dependencies are complete
        for (const FString& Dependency : Task.Dependencies)
        {
            FDir_AgentTask DepTask = GetAgentTask(Dependency);
            if (DepTask.Status != EDir_AgentStatus::Complete)
            {
                return false;
            }
        }
    }
    
    return true;
}

TArray<FString> ADir_ProductionCoordinator::GetBlockedAgents()
{
    TArray<FString> BlockedAgents;
    TArray<FDir_AgentTask> BlockedTasks = GetTasksByStatus(EDir_AgentStatus::Blocked);
    
    for (const FDir_AgentTask& Task : BlockedTasks)
    {
        BlockedAgents.Add(Task.AgentName);
    }
    
    return BlockedAgents;
}

void ADir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Agents: %d"), ProductionMetrics.TotalAgents);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), ProductionMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), ProductionMetrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), ProductionMetrics.ActiveAgents);
    UE_LOG(LogTemp, Warning, TEXT("Pending Tasks: %d"), ProductionMetrics.PendingTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked Tasks: %d"), ProductionMetrics.BlockedTasks);

    // Log blocked agents
    TArray<FString> BlockedAgents = GetBlockedAgents();
    if (BlockedAgents.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Blocked Agents:"));
        for (const FString& Agent : BlockedAgents)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Agent);
        }
    }
}

void ADir_ProductionCoordinator::RefreshCoordinatorData()
{
    InitializeAgentTasks();
    UpdateProductionMetrics();
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator data refreshed"));
}

void ADir_ProductionCoordinator::SetupCoordinatorMesh()
{
    // Try to set a default mesh (cube)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CoordinatorMesh->SetStaticMesh(CubeMeshAsset.Object);
        CoordinatorMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.5f));
    }
}

void ADir_ProductionCoordinator::UpdateMeshMaterial()
{
    if (!CoordinatorMesh)
    {
        return;
    }

    // Create dynamic material instance if needed
    UMaterialInstanceDynamic* DynamicMaterial = CoordinatorMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (DynamicMaterial)
    {
        // Set color based on overall progress
        FLinearColor ProgressColor;
        float Progress = ProductionMetrics.OverallProgress / 100.0f;
        
        if (Progress < 0.3f)
        {
            ProgressColor = FLinearColor::Red; // Low progress
        }
        else if (Progress < 0.7f)
        {
            ProgressColor = FLinearColor::Yellow; // Medium progress
        }
        else
        {
            ProgressColor = FLinearColor::Green; // High progress
        }

        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), ProgressColor);
    }
}

FLinearColor ADir_ProductionCoordinator::GetStatusColor(EDir_AgentStatus Status)
{
    switch (Status)
    {
        case EDir_AgentStatus::Pending:
            return FLinearColor::Gray;
        case EDir_AgentStatus::InProgress:
            return FLinearColor::Yellow;
        case EDir_AgentStatus::Complete:
            return FLinearColor::Green;
        case EDir_AgentStatus::Blocked:
            return FLinearColor::Red;
        case EDir_AgentStatus::Failed:
            return FLinearColor(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
        default:
            return FLinearColor::White;
    }
}