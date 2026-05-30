#include "Director_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentPhase = EDir_ProductionPhase::CoreSystems;
    CurrentCycle = 1;
    CycleDuration = 300.0f; // 5 minutes per cycle
    
    bPlayerControllerReady = false;
    bSurvivalSystemReady = false;
    bDinosaurAIReady = false;
    bMinPlayableMapReady = false;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProduction();
    InitializeBiomeDistribution();
    SetupCriticalPathTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("Director Production Coordinator initialized - Cycle %d"), CurrentCycle);
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update production metrics every 30 seconds
    static float MetricsTimer = 0.0f;
    MetricsTimer += DeltaTime;
    
    if (MetricsTimer >= 30.0f)
    {
        LogProductionMetrics();
        ValidateCriticalPath();
        ValidateBiomeDistribution();
        MetricsTimer = 0.0f;
    }
}

void UDir_ProductionCoordinator::InitializeProduction()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Initialize critical path tasks for minimum playable prototype
    FDir_AgentTask PlayerControllerTask;
    PlayerControllerTask.AgentName = "Agent #9 - Character Artist";
    PlayerControllerTask.TaskDescription = "Implement WASD movement, camera boom, and collision for TranspersonalCharacter";
    PlayerControllerTask.Priority = 10.0f;
    PlayerControllerTask.EstimatedCycles = 2;
    PlayerControllerTask.Status = EDir_AgentStatus::Working;
    AgentTasks.Add(PlayerControllerTask);
    
    FDir_AgentTask SurvivalTask;
    SurvivalTask.AgentName = "Agent #3 - Core Systems";
    SurvivalTask.TaskDescription = "Create SurvivalComponent with Health/Hunger/Thirst/Stamina";
    SurvivalTask.Priority = 9.0f;
    SurvivalTask.EstimatedCycles = 1;
    SurvivalTask.Dependencies.Add("Agent #9 - Character Artist");
    AgentTasks.Add(SurvivalTask);
    
    FDir_AgentTask DinosaurAITask;
    DinosaurAITask.AgentName = "Agent #12 - Combat AI";
    DinosaurAITask.TaskDescription = "Implement DinosaurCombatAIController.cpp with basic BehaviorTree";
    DinosaurAITask.Priority = 8.0f;
    DinosaurAITask.EstimatedCycles = 2;
    AgentTasks.Add(DinosaurAITask);
    
    FDir_AgentTask WorldTask;
    WorldTask.AgentName = "Agent #5 - World Generator";
    WorldTask.TaskDescription = "Enhance MinPlayableMap terrain with height variation and biome distribution";
    WorldTask.Priority = 7.0f;
    WorldTask.EstimatedCycles = 1;
    AgentTasks.Add(WorldTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Production initialized with %d critical tasks"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::InitializeBiomeDistribution()
{
    BiomeDistributions.Empty();
    
    // Savanna (center)
    FDir_BiomeDistribution Savanna;
    Savanna.BiomeName = "Savanna";
    Savanna.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    Savanna.TargetActorCount = 200;
    BiomeDistributions.Add(Savanna);
    
    // Swamp
    FDir_BiomeDistribution Swamp;
    Swamp.BiomeName = "Swamp";
    Swamp.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    Swamp.TargetActorCount = 150;
    BiomeDistributions.Add(Swamp);
    
    // Forest
    FDir_BiomeDistribution Forest;
    Forest.BiomeName = "Forest";
    Forest.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    Forest.TargetActorCount = 250;
    BiomeDistributions.Add(Forest);
    
    // Desert
    FDir_BiomeDistribution Desert;
    Desert.BiomeName = "Desert";
    Desert.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    Desert.TargetActorCount = 100;
    BiomeDistributions.Add(Desert);
    
    // Mountain
    FDir_BiomeDistribution Mountain;
    Mountain.BiomeName = "Mountain";
    Mountain.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    Mountain.TargetActorCount = 120;
    BiomeDistributions.Add(Mountain);
    
    UE_LOG(LogTemp, Warning, TEXT("Biome distribution initialized for %d biomes"), BiomeDistributions.Num());
}

void UDir_ProductionCoordinator::SetupCriticalPathTasks()
{
    CriticalPathTasks.Empty();
    CriticalPathTasks.Add("TranspersonalCharacter movement implementation");
    CriticalPathTasks.Add("SurvivalComponent integration");
    CriticalPathTasks.Add("DinosaurCombatAIController.cpp completion");
    CriticalPathTasks.Add("MinPlayableMap terrain enhancement");
    CriticalPathTasks.Add("Dinosaur mesh distribution across biomes");
}

void UDir_ProductionCoordinator::AdvanceToNextPhase()
{
    if (ValidateCriticalPath())
    {
        int32 CurrentPhaseInt = static_cast<int32>(CurrentPhase);
        CurrentPhaseInt++;
        
        if (CurrentPhaseInt <= static_cast<int32>(EDir_ProductionPhase::Release))
        {
            CurrentPhase = static_cast<EDir_ProductionPhase>(CurrentPhaseInt);
            CurrentCycle++;
            
            UE_LOG(LogTemp, Warning, TEXT("Advanced to Phase: %d, Cycle: %d"), CurrentPhaseInt, CurrentCycle);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot advance phase - critical path validation failed"));
    }
}

void UDir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Assigned task to %s: %s"), *AgentName, *TaskDescription);
}

void UDir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Updated %s status to %d"), *AgentName, static_cast<int32>(NewStatus));
            break;
        }
    }
}

bool UDir_ProductionCoordinator::ValidateCriticalPath()
{
    // Check if minimum playable prototype requirements are met
    bool bCriticalPathValid = true;
    
    // Validate player controller
    if (!bPlayerControllerReady)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical Path BLOCKED: Player controller not ready"));
        bCriticalPathValid = false;
    }
    
    // Validate survival system
    if (!bSurvivalSystemReady)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical Path BLOCKED: Survival system not ready"));
        bCriticalPathValid = false;
    }
    
    // Validate dinosaur AI
    if (!bDinosaurAIReady)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical Path BLOCKED: Dinosaur AI not ready"));
        bCriticalPathValid = false;
    }
    
    if (bCriticalPathValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Critical Path VALIDATED - Ready for next phase"));
    }
    
    return bCriticalPathValid;
}

FVector UDir_ProductionCoordinator::GetRandomLocationInBiome(const FString& BiomeName)
{
    for (const FDir_BiomeDistribution& Biome : BiomeDistributions)
    {
        if (Biome.BiomeName == BiomeName)
        {
            float RandomX = FMath::RandRange(-Biome.Radius, Biome.Radius);
            float RandomY = FMath::RandRange(-Biome.Radius, Biome.Radius);
            
            return Biome.CenterLocation + FVector(RandomX, RandomY, 100.0f);
        }
    }
    
    return FVector::ZeroVector;
}

void UDir_ProductionCoordinator::UpdateBiomeActorCount(const FString& BiomeName, int32 ActorCount)
{
    for (FDir_BiomeDistribution& Biome : BiomeDistributions)
    {
        if (Biome.BiomeName == BiomeName)
        {
            Biome.CurrentActorCount = ActorCount;
            UE_LOG(LogTemp, Warning, TEXT("Updated %s actor count: %d/%d"), *BiomeName, ActorCount, Biome.TargetActorCount);
            break;
        }
    }
}

bool UDir_ProductionCoordinator::ValidateBiomeDistribution()
{
    bool bDistributionValid = true;
    int32 TotalActors = 0;
    
    for (const FDir_BiomeDistribution& Biome : BiomeDistributions)
    {
        TotalActors += Biome.CurrentActorCount;
        
        float DistributionRatio = static_cast<float>(Biome.CurrentActorCount) / static_cast<float>(Biome.TargetActorCount);
        
        if (DistributionRatio < 0.8f) // Less than 80% of target
        {
            UE_LOG(LogTemp, Warning, TEXT("Biome %s under-populated: %d/%d (%.1f%%)"), 
                   *Biome.BiomeName, Biome.CurrentActorCount, Biome.TargetActorCount, DistributionRatio * 100.0f);
            bDistributionValid = false;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total actors across all biomes: %d"), TotalActors);
    return bDistributionValid;
}

bool UDir_ProductionCoordinator::ValidateMinimumPlayablePrototype()
{
    return bPlayerControllerReady && bSurvivalSystemReady && bDinosaurAIReady && bMinPlayableMapReady;
}

TArray<FString> UDir_ProductionCoordinator::GetBlockingIssues()
{
    TArray<FString> BlockingIssues;
    
    if (!bPlayerControllerReady)
    {
        BlockingIssues.Add("Player controller missing WASD movement or camera");
    }
    
    if (!bSurvivalSystemReady)
    {
        BlockingIssues.Add("SurvivalComponent not implemented or integrated");
    }
    
    if (!bDinosaurAIReady)
    {
        BlockingIssues.Add("DinosaurCombatAIController.cpp missing implementation");
    }
    
    if (!bMinPlayableMapReady)
    {
        BlockingIssues.Add("MinPlayableMap lacks terrain variation or proper lighting");
    }
    
    return BlockingIssues;
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - CYCLE %d ==="), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), static_cast<int32>(CurrentPhase));
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), AgentTasks.Num());
    
    int32 CompletedTasks = 0;
    int32 BlockedTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            CompletedTasks++;
        }
        else if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedTasks++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d, Blocked: %d"), CompletedTasks, BlockedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Critical Path Valid: %s"), ValidateCriticalPath() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Minimum Playable Prototype: %s"), ValidateMinimumPlayablePrototype() ? TEXT("READY") : TEXT("NOT READY"));
}

void UDir_ProductionCoordinator::ValidateAgentDependencies()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        bool bDependenciesMet = true;
        
        for (const FString& Dependency : Task.Dependencies)
        {
            bool bDependencyComplete = false;
            
            for (const FDir_AgentTask& OtherTask : AgentTasks)
            {
                if (OtherTask.AgentName == Dependency && OtherTask.Status == EDir_AgentStatus::Complete)
                {
                    bDependencyComplete = true;
                    break;
                }
            }
            
            if (!bDependencyComplete)
            {
                bDependenciesMet = false;
                break;
            }
        }
        
        if (!bDependenciesMet && Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Blocked;
        }
        else if (bDependenciesMet && Task.Status == EDir_AgentStatus::Blocked)
        {
            Task.Status = EDir_AgentStatus::Working;
        }
    }
}

float UDir_ProductionCoordinator::CalculateProductionProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            CompletedTasks++;
        }
    }
    
    return static_cast<float>(CompletedTasks) / static_cast<float>(AgentTasks.Num());
}

void UDir_ProductionCoordinator::LogProductionMetrics()
{
    float Progress = CalculateProductionProgress();
    ValidateAgentDependencies();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Progress: %.1f%% (%d/%d tasks)"), 
           Progress * 100.0f, 
           AgentTasks.Num() - AgentTasks.FilterByPredicate([](const FDir_AgentTask& Task) { return Task.Status == EDir_AgentStatus::Complete; }).Num(),
           AgentTasks.Num());
}