#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AStudioDirector::AStudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create command center mesh component
    CommandCenterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CommandCenterMesh"));
    CommandCenterMesh->SetupAttachment(RootComponent);

    // Initialize default values
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260516_007");
    TotalActorsInWorld = 0;
    ProductionProgress = 0.0f;

    SetupCommandCenterMesh();
}

void AStudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomes();
    CreateInitialTasks();
    RefreshWorldStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director activated for cycle: %s"), *CurrentCycleID);
}

void AStudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update production progress every 5 seconds
    static float ProgressUpdateTimer = 0.0f;
    ProgressUpdateTimer += DeltaTime;
    
    if (ProgressUpdateTimer >= 5.0f)
    {
        UpdateProductionProgress();
        ProgressUpdateTimer = 0.0f;
    }
}

void AStudioDirector::SetupCommandCenterMesh()
{
    // Try to find a suitable mesh for the command center
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CommandCenterMesh->SetStaticMesh(CubeMeshAsset.Object);
        CommandCenterMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.5f));
    }
}

void AStudioDirector::CreateAgentTask(int32 AgentNumber, const FString& AgentName, const FString& TaskDescription, 
                                    EDir_BiomeType AssignedBiome, FVector TargetLocation, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.AssignedBiome = AssignedBiome;
    NewTask.TargetLocation = TargetLocation;
    NewTask.Priority = Priority;
    NewTask.CreatedTime = FDateTime::Now();
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director: Created task for Agent #%d - %s"), AgentNumber, *TaskDescription);
}

void AStudioDirector::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Log, TEXT("Studio Director: Agent #%d status updated to %d"), AgentNumber, (int32)NewStatus);
            break;
        }
    }
}

TArray<FDir_AgentTask> AStudioDirector::GetTasksByAgent(int32 AgentNumber)
{
    TArray<FDir_AgentTask> AgentTasks;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            AgentTasks.Add(Task);
        }
    }
    return AgentTasks;
}

TArray<FDir_AgentTask> AStudioDirector::GetTasksByBiome(EDir_BiomeType BiomeType)
{
    TArray<FDir_AgentTask> BiomeTasks;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AssignedBiome == BiomeType)
        {
            BiomeTasks.Add(Task);
        }
    }
    return BiomeTasks;
}

void AStudioDirector::InitializeBiomes()
{
    BiomeStatuses.Empty();
    
    // Define biome coordinates based on memory ID 709
    TArray<TPair<EDir_BiomeType, FVector>> BiomeCoords = {
        {EDir_BiomeType::Savanna, FVector(0, 0, 100)},
        {EDir_BiomeType::Swamp, FVector(-50000, -45000, 100)},
        {EDir_BiomeType::Forest, FVector(-45000, 40000, 100)},
        {EDir_BiomeType::Desert, FVector(55000, 0, 100)},
        {EDir_BiomeType::Mountains, FVector(40000, 50000, 100)}
    };
    
    for (const auto& BiomeCoord : BiomeCoords)
    {
        FDir_BiomeStatus BiomeStatus;
        BiomeStatus.BiomeType = BiomeCoord.Key;
        BiomeStatus.CenterLocation = BiomeCoord.Value;
        BiomeStatus.ActorCount = 0;
        BiomeStatus.CompletionPercentage = 0.0f;
        BiomeStatus.bIsPopulated = false;
        
        // Assign agents to biomes
        switch (BiomeCoord.Key)
        {
            case EDir_BiomeType::Savanna:
                BiomeStatus.AssignedAgents = {5, 6, 8, 9}; // WorldGen, Environment, Lighting, Character
                break;
            case EDir_BiomeType::Swamp:
                BiomeStatus.AssignedAgents = {6, 7}; // Environment, Architecture
                break;
            case EDir_BiomeType::Forest:
                BiomeStatus.AssignedAgents = {6, 9}; // Environment, Character
                break;
            case EDir_BiomeType::Desert:
                BiomeStatus.AssignedAgents = {5, 8}; // WorldGen, Lighting
                break;
            case EDir_BiomeType::Mountains:
                BiomeStatus.AssignedAgents = {7, 8}; // Architecture, Lighting
                break;
        }
        
        BiomeStatuses.Add(BiomeStatus);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initialized %d biomes"), BiomeStatuses.Num());
}

void AStudioDirector::UpdateBiomeActorCount(EDir_BiomeType BiomeType, int32 NewActorCount)
{
    for (FDir_BiomeStatus& BiomeStatus : BiomeStatuses)
    {
        if (BiomeStatus.BiomeType == BiomeType)
        {
            BiomeStatus.ActorCount = NewActorCount;
            BiomeStatus.CompletionPercentage = FMath::Clamp(NewActorCount / 500.0f, 0.0f, 1.0f);
            BiomeStatus.bIsPopulated = (NewActorCount >= 500);
            break;
        }
    }
}

FDir_BiomeStatus AStudioDirector::GetBiomeStatus(EDir_BiomeType BiomeType)
{
    for (const FDir_BiomeStatus& BiomeStatus : BiomeStatuses)
    {
        if (BiomeStatus.BiomeType == BiomeType)
        {
            return BiomeStatus;
        }
    }
    return FDir_BiomeStatus(); // Return default if not found
}

bool AStudioDirector::IsBiomePopulated(EDir_BiomeType BiomeType, int32 MinimumActors)
{
    FDir_BiomeStatus BiomeStatus = GetBiomeStatus(BiomeType);
    return BiomeStatus.ActorCount >= MinimumActors;
}

void AStudioDirector::UpdateProductionProgress()
{
    if (UWorld* World = GetWorld())
    {
        TotalActorsInWorld = 0;
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            TotalActorsInWorld++;
        }
        
        // Update biome actor counts
        for (FDir_BiomeStatus& BiomeStatus : BiomeStatuses)
        {
            int32 BiomeActorCount = 0;
            FVector BiomeCenter = BiomeStatus.CenterLocation;
            
            for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
            {
                AActor* Actor = *ActorIterator;
                if (Actor && FVector::Dist(Actor->GetActorLocation(), BiomeCenter) < 10000.0f)
                {
                    BiomeActorCount++;
                }
            }
            
            UpdateBiomeActorCount(BiomeStatus.BiomeType, BiomeActorCount);
        }
        
        ProductionProgress = CalculateOverallProgress();
    }
}

FString AStudioDirector::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ===\n"));
    Report += FString::Printf(TEXT("Cycle ID: %s\n"), *CurrentCycleID);
    Report += FString::Printf(TEXT("Total Actors in World: %d\n"), TotalActorsInWorld);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n\n"), ProductionProgress * 100.0f);
    
    Report += TEXT("BIOME STATUS:\n");
    for (const FDir_BiomeStatus& BiomeStatus : BiomeStatuses)
    {
        FString BiomeName;
        switch (BiomeStatus.BiomeType)
        {
            case EDir_BiomeType::Savanna: BiomeName = TEXT("Savanna"); break;
            case EDir_BiomeType::Swamp: BiomeName = TEXT("Swamp"); break;
            case EDir_BiomeType::Forest: BiomeName = TEXT("Forest"); break;
            case EDir_BiomeType::Desert: BiomeName = TEXT("Desert"); break;
            case EDir_BiomeType::Mountains: BiomeName = TEXT("Mountains"); break;
        }
        
        Report += FString::Printf(TEXT("- %s: %d actors (%.1f%% complete) %s\n"), 
            *BiomeName, 
            BiomeStatus.ActorCount, 
            BiomeStatus.CompletionPercentage * 100.0f,
            BiomeStatus.bIsPopulated ? TEXT("[POPULATED]") : TEXT("[NEEDS WORK]")
        );
    }
    
    Report += FString::Printf(TEXT("\nACTIVE TASKS: %d\n"), ActiveTasks.Num());
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        FString StatusName;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusName = TEXT("Idle"); break;
            case EDir_AgentStatus::Working: StatusName = TEXT("Working"); break;
            case EDir_AgentStatus::Completed: StatusName = TEXT("Completed"); break;
            case EDir_AgentStatus::Error: StatusName = TEXT("Error"); break;
            case EDir_AgentStatus::Waiting: StatusName = TEXT("Waiting"); break;
        }
        
        Report += FString::Printf(TEXT("- Agent #%d (%s): %s [%s]\n"), 
            Task.AgentNumber, *Task.AgentName, *Task.TaskDescription, *StatusName);
    }
    
    return Report;
}

void AStudioDirector::LogAgentActivity(int32 AgentNumber, const FString& Activity)
{
    UE_LOG(LogTemp, Log, TEXT("Studio Director: Agent #%d - %s"), AgentNumber, *Activity);
}

FVector AStudioDirector::GetBiomeCoordinates(EDir_BiomeType BiomeType)
{
    FDir_BiomeStatus BiomeStatus = GetBiomeStatus(BiomeType);
    return BiomeStatus.CenterLocation;
}

TArray<int32> AStudioDirector::GetAgentsForBiome(EDir_BiomeType BiomeType)
{
    FDir_BiomeStatus BiomeStatus = GetBiomeStatus(BiomeType);
    return BiomeStatus.AssignedAgents;
}

void AStudioDirector::RefreshWorldStatus()
{
    UpdateProductionProgress();
    
    FString Report = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void AStudioDirector::CreateInitialTasks()
{
    // Create priority tasks for the current cycle based on brain memories
    CreateAgentTask(5, TEXT("World Generator"), TEXT("Expand terrain to 10km2 with height variation"), 
                   EDir_BiomeType::Savanna, FVector(0, 0, 0), 10.0f);
    
    CreateAgentTask(6, TEXT("Environment Artist"), TEXT("Populate biomes with 500+ actors each"), 
                   EDir_BiomeType::Swamp, FVector(-50000, -45000, 0), 9.0f);
    
    CreateAgentTask(8, TEXT("Lighting & Atmosphere"), TEXT("Stabilize Cretaceous atmosphere lighting"), 
                   EDir_BiomeType::Savanna, FVector(0, 0, 0), 8.0f);
    
    CreateAgentTask(9, TEXT("Character Artist"), TEXT("Create dinosaur actors with collision"), 
                   EDir_BiomeType::Forest, FVector(-45000, 40000, 0), 9.0f);
    
    CreateAgentTask(12, TEXT("Combat & Enemy AI"), TEXT("Implement survival HUD system"), 
                    EDir_BiomeType::Savanna, FVector(0, 0, 0), 7.0f);
}

float AStudioDirector::CalculateOverallProgress()
{
    if (BiomeStatuses.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_BiomeStatus& BiomeStatus : BiomeStatuses)
    {
        TotalProgress += BiomeStatus.CompletionPercentage;
    }
    
    return TotalProgress / BiomeStatuses.Num();
}