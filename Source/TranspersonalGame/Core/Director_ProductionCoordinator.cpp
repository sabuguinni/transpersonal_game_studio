#include "Director_ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    LastUpdateTime = 0.0f;
    CurrentCriticalTaskIndex = 0;
    TotalActorsInWorld = 0;
    TotalFilesCreated = 0;
    ProductionEfficiency = 0.0f;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomes();
    InitializeAgentStatuses();
    InitializeCriticalPath();
    
    UE_LOG(LogTemp, Warning, TEXT("Director_ProductionCoordinator: Production coordination system initialized"));
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    // Update production metrics every 10 seconds
    if (LastUpdateTime >= 10.0f)
    {
        UpdateProductionMetrics();
        LastUpdateTime = 0.0f;
    }
}

void UDir_ProductionCoordinator::InitializeBiomes()
{
    BiomeCoordinates.Empty();
    
    // Initialize the 5 biomes with correct coordinates from memory
    BiomeCoordinates.Add(FDir_BiomeCoordinates(TEXT("Savana"), FVector(0, 0, 100), 15000));
    BiomeCoordinates.Add(FDir_BiomeCoordinates(TEXT("Pantano"), FVector(-50000, -45000, 100), 15000));
    BiomeCoordinates.Add(FDir_BiomeCoordinates(TEXT("Floresta"), FVector(-45000, 40000, 100), 15000));
    BiomeCoordinates.Add(FDir_BiomeCoordinates(TEXT("Deserto"), FVector(55000, 0, 100), 15000));
    BiomeCoordinates.Add(FDir_BiomeCoordinates(TEXT("Montanha"), FVector(40000, 50000, 100), 15000));
    
    UE_LOG(LogTemp, Warning, TEXT("Director_ProductionCoordinator: 5 biomes initialized"));
}

FVector UDir_ProductionCoordinator::GetRandomLocationInBiome(const FString& BiomeName)
{
    for (const FDir_BiomeCoordinates& Biome : BiomeCoordinates)
    {
        if (Biome.BiomeName == BiomeName)
        {
            float RandomX = UKismetMathLibrary::RandomFloatInRange(-Biome.SpawnRadius, Biome.SpawnRadius);
            float RandomY = UKismetMathLibrary::RandomFloatInRange(-Biome.SpawnRadius, Biome.SpawnRadius);
            
            return FVector(
                Biome.CenterLocation.X + RandomX,
                Biome.CenterLocation.Y + RandomY,
                Biome.CenterLocation.Z
            );
        }
    }
    
    // Fallback to origin if biome not found
    return FVector::ZeroVector;
}

FString UDir_ProductionCoordinator::GetBiomeForDistribution(int32 SpawnIndex)
{
    if (BiomeCoordinates.Num() == 0)
    {
        return TEXT("Savana");
    }
    
    // Distribute evenly across biomes (20% each)
    int32 BiomeIndex = SpawnIndex % BiomeCoordinates.Num();
    return BiomeCoordinates[BiomeIndex].BiomeName;
}

void UDir_ProductionCoordinator::InitializeAgentStatuses()
{
    AgentStatuses.Empty();
    
    // Initialize all 19 agents
    TArray<FString> AgentNames = {
        TEXT("Studio Director"), TEXT("Engine Architect"), TEXT("Core Systems"), 
        TEXT("Performance Optimizer"), TEXT("World Generator"), TEXT("Environment Artist"),
        TEXT("Architecture"), TEXT("Lighting Artist"), TEXT("Character Artist"),
        TEXT("Animation Agent"), TEXT("NPC Behavior"), TEXT("Combat AI"),
        TEXT("Crowd Simulation"), TEXT("Quest Designer"), TEXT("Narrative Agent"),
        TEXT("Audio Agent"), TEXT("VFX Agent"), TEXT("QA Testing"), TEXT("Integration Build")
    };
    
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentTaskStatus NewStatus;
        NewStatus.AgentName = AgentName;
        NewStatus.bTaskCompleted = false;
        NewStatus.CurrentTask = TEXT("Awaiting Assignment");
        NewStatus.FilesCreated = 0;
        NewStatus.UE5CommandsExecuted = 0;
        
        AgentStatuses.Add(NewStatus);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Director_ProductionCoordinator: %d agent statuses initialized"), AgentStatuses.Num());
}

void UDir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, const FString& Task, int32 Files, int32 Commands)
{
    for (FDir_AgentTaskStatus& Status : AgentStatuses)
    {
        if (Status.AgentName == AgentName)
        {
            Status.CurrentTask = Task;
            Status.FilesCreated += Files;
            Status.UE5CommandsExecuted += Commands;
            Status.bTaskCompleted = (Files > 0 || Commands > 0);
            
            UE_LOG(LogTemp, Warning, TEXT("Director_ProductionCoordinator: Updated %s - Task: %s, Files: %d, Commands: %d"), 
                *AgentName, *Task, Status.FilesCreated, Status.UE5CommandsExecuted);
            break;
        }
    }
}

bool UDir_ProductionCoordinator::IsAgentReady(const FString& AgentName)
{
    for (const FDir_AgentTaskStatus& Status : AgentStatuses)
    {
        if (Status.AgentName == AgentName)
        {
            return Status.bTaskCompleted;
        }
    }
    return false;
}

void UDir_ProductionCoordinator::InitializeCriticalPath()
{
    CriticalPathTasks.Empty();
    
    // Critical path for playable prototype
    CriticalPathTasks.Add(TEXT("Player Movement System"));
    CriticalPathTasks.Add(TEXT("Survival Component"));
    CriticalPathTasks.Add(TEXT("Terrain Generation"));
    CriticalPathTasks.Add(TEXT("Dinosaur AI Basic"));
    CriticalPathTasks.Add(TEXT("Combat System"));
    CriticalPathTasks.Add(TEXT("Survival HUD"));
    CriticalPathTasks.Add(TEXT("Audio Integration"));
    CriticalPathTasks.Add(TEXT("Performance Optimization"));
    CriticalPathTasks.Add(TEXT("QA Testing"));
    CriticalPathTasks.Add(TEXT("Build Integration"));
    
    CurrentCriticalTaskIndex = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Director_ProductionCoordinator: Critical path with %d tasks initialized"), CriticalPathTasks.Num());
}

FString UDir_ProductionCoordinator::GetNextCriticalTask()
{
    if (CurrentCriticalTaskIndex < CriticalPathTasks.Num())
    {
        return CriticalPathTasks[CurrentCriticalTaskIndex];
    }
    
    return TEXT("All Critical Tasks Completed");
}

void UDir_ProductionCoordinator::MarkTaskCompleted(const FString& TaskName)
{
    for (int32 i = 0; i < CriticalPathTasks.Num(); i++)
    {
        if (CriticalPathTasks[i] == TaskName && i == CurrentCriticalTaskIndex)
        {
            CurrentCriticalTaskIndex++;
            UE_LOG(LogTemp, Warning, TEXT("Director_ProductionCoordinator: Critical task completed: %s"), *TaskName);
            break;
        }
    }
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    // Count total actors in world
    if (UWorld* World = GetWorld())
    {
        TotalActorsInWorld = World->GetActorCount();
    }
    
    // Calculate total files created by all agents
    TotalFilesCreated = 0;
    int32 TotalCommands = 0;
    
    for (const FDir_AgentTaskStatus& Status : AgentStatuses)
    {
        TotalFilesCreated += Status.FilesCreated;
        TotalCommands += Status.UE5CommandsExecuted;
    }
    
    // Calculate production efficiency (completed tasks / total tasks)
    float CompletedTasks = static_cast<float>(CurrentCriticalTaskIndex);
    float TotalTasks = static_cast<float>(CriticalPathTasks.Num());
    ProductionEfficiency = (TotalTasks > 0) ? (CompletedTasks / TotalTasks) * 100.0f : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Director_ProductionCoordinator: Metrics - Actors: %d, Files: %d, Commands: %d, Efficiency: %.1f%%"), 
        TotalActorsInWorld, TotalFilesCreated, TotalCommands, ProductionEfficiency);
}

void UDir_ProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Critical Task: %s"), *GetNextCriticalTask());
    UE_LOG(LogTemp, Warning, TEXT("Production Efficiency: %.1f%%"), ProductionEfficiency);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors in World: %d"), TotalActorsInWorld);
    UE_LOG(LogTemp, Warning, TEXT("Total Files Created: %d"), TotalFilesCreated);
    
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT STATUS ==="));
    for (const FDir_AgentTaskStatus& Status : AgentStatuses)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s (Files: %d, Commands: %d)"), 
            *Status.AgentName, *Status.CurrentTask, Status.FilesCreated, Status.UE5CommandsExecuted);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME DISTRIBUTION ==="));
    for (const FDir_BiomeCoordinates& Biome : BiomeCoordinates)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Center(%s), Radius: %d"), 
            *Biome.BiomeName, *Biome.CenterLocation.ToString(), Biome.SpawnRadius);
    }
}