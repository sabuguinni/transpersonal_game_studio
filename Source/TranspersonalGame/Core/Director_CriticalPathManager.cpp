#include "Director_CriticalPathManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDir_CriticalPathManager::UDir_CriticalPathManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    PlayablePrototypeProgress = 0.0f;
    bMinimalPlayableReached = false;
    bEnforceBiomeDistribution = true;
}

void UDir_CriticalPathManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCriticalPath();
    InitializeBiomeTargets();
    
    UE_LOG(LogTemp, Warning, TEXT("Director_CriticalPathManager: Production pipeline initialized"));
}

void UDir_CriticalPathManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePlayableProgress();
    
    // Log status every 30 seconds
    static float LogTimer = 0.0f;
    LogTimer += DeltaTime;
    if (LogTimer >= 30.0f)
    {
        LogCriticalPathStatus();
        LogBiomeDistribution();
        LogTimer = 0.0f;
    }
}

void UDir_CriticalPathManager::InitializeCriticalPath()
{
    CriticalPathTasks.Empty();
    AgentStatusMap.Empty();
    
    SetupDefaultCriticalPath();
    
    UE_LOG(LogTemp, Warning, TEXT("Critical Path initialized with %d tasks"), CriticalPathTasks.Num());
}

void UDir_CriticalPathManager::SetupDefaultCriticalPath()
{
    // MILESTONE 1: Minimal Playable Prototype
    FDir_CriticalPathTask Task1;
    Task1.TaskName = TEXT("Character Movement System");
    Task1.AgentID = 3; // Core Systems
    Task1.Priority = EDir_CriticalPathPriority::Critical;
    Task1.Status = EDir_AgentStatus::Working;
    Task1.EstimatedCycles = 2.0f;
    Task1.DeliverablePath = TEXT("Source/TranspersonalGame/Core/GameFramework/TranspersonalCharacter.cpp");
    Task1.bIsPlayable = true;
    CriticalPathTasks.Add(Task1);
    
    FDir_CriticalPathTask Task2;
    Task2.TaskName = TEXT("Survival Component");
    Task2.AgentID = 3; // Core Systems
    Task2.Priority = EDir_CriticalPathPriority::Critical;
    Task2.Status = EDir_AgentStatus::Idle;
    Task2.EstimatedCycles = 1.0f;
    Task2.Dependencies.Add(0); // Depends on Character Movement
    Task2.DeliverablePath = TEXT("Source/TranspersonalGame/Core/SurvivalComponent.cpp");
    Task2.bIsPlayable = true;
    CriticalPathTasks.Add(Task2);
    
    FDir_CriticalPathTask Task3;
    Task3.TaskName = TEXT("T-Rex AI Controller");
    Task3.AgentID = 12; // Combat AI
    Task3.Priority = EDir_CriticalPathPriority::Critical;
    Task3.Status = EDir_AgentStatus::Idle;
    Task3.EstimatedCycles = 2.0f;
    Task3.DeliverablePath = TEXT("Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp");
    Task3.bIsPlayable = true;
    CriticalPathTasks.Add(Task3);
    
    FDir_CriticalPathTask Task4;
    Task4.TaskName = TEXT("Dinosaur Spawning System");
    Task4.AgentID = 5; // World Generator
    Task4.Priority = EDir_CriticalPathPriority::Critical;
    Task4.Status = EDir_AgentStatus::Idle;
    Task4.EstimatedCycles = 1.0f;
    Task4.DeliverablePath = TEXT("MinPlayableMap");
    Task4.bIsPlayable = true;
    CriticalPathTasks.Add(Task4);
    
    FDir_CriticalPathTask Task5;
    Task5.TaskName = TEXT("Survival HUD");
    Task5.AgentID = 14; // Quest Designer (UI)
    Task5.Priority = EDir_CriticalPathPriority::High;
    Task5.Status = EDir_AgentStatus::Idle;
    Task5.EstimatedCycles = 1.0f;
    Task5.Dependencies.Add(1); // Depends on Survival Component
    Task5.DeliverablePath = TEXT("Source/TranspersonalGame/UI/SurvivalHUD.cpp");
    Task5.bIsPlayable = true;
    CriticalPathTasks.Add(Task5);
    
    // Initialize agent status map
    for (int32 i = 1; i <= 19; ++i)
    {
        AgentStatusMap.Add(i, EDir_AgentStatus::Idle);
    }
}

void UDir_CriticalPathManager::UpdateTaskStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    AgentStatusMap.FindOrAdd(AgentID) = NewStatus;
    
    // Update tasks assigned to this agent
    for (FDir_CriticalPathTask& Task : CriticalPathTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent %d status updated to %d"), AgentID, (int32)NewStatus);
}

bool UDir_CriticalPathManager::CanAgentProceed(int32 AgentID)
{
    // Check if agent is blocked
    if (BlockedAgents.Contains(AgentID))
    {
        return false;
    }
    
    // Check if agent's tasks have satisfied dependencies
    for (const FDir_CriticalPathTask& Task : CriticalPathTasks)
    {
        if (Task.AgentID == AgentID && Task.Status == EDir_AgentStatus::Idle)
        {
            return CheckTaskDependencies(Task);
        }
    }
    
    return true;
}

bool UDir_CriticalPathManager::CheckTaskDependencies(const FDir_CriticalPathTask& Task)
{
    for (int32 DepIndex : Task.Dependencies)
    {
        if (DepIndex >= 0 && DepIndex < CriticalPathTasks.Num())
        {
            const FDir_CriticalPathTask& DepTask = CriticalPathTasks[DepIndex];
            if (DepTask.Status != EDir_AgentStatus::Complete)
            {
                return false;
            }
        }
    }
    return true;
}

float UDir_CriticalPathManager::CalculatePlayableProgress()
{
    int32 CompletedPlayableTasks = 0;
    int32 TotalPlayableTasks = 0;
    
    for (const FDir_CriticalPathTask& Task : CriticalPathTasks)
    {
        if (Task.bIsPlayable)
        {
            TotalPlayableTasks++;
            if (Task.Status == EDir_AgentStatus::Complete)
            {
                CompletedPlayableTasks++;
            }
        }
    }
    
    return TotalPlayableTasks > 0 ? (float)CompletedPlayableTasks / (float)TotalPlayableTasks * 100.0f : 0.0f;
}

void UDir_CriticalPathManager::UpdatePlayableProgress()
{
    PlayablePrototypeProgress = CalculatePlayableProgress();
    
    if (PlayablePrototypeProgress >= 80.0f && !bMinimalPlayableReached)
    {
        bMinimalPlayableReached = true;
        UE_LOG(LogTemp, Warning, TEXT("MILESTONE REACHED: Minimal Playable Prototype at %.1f%% completion"), PlayablePrototypeProgress);
    }
}

void UDir_CriticalPathManager::InitializeBiomeTargets()
{
    BiomeTargets.Empty();
    SetupDefaultBiomeTargets();
}

void UDir_CriticalPathManager::SetupDefaultBiomeTargets()
{
    // Savana (0,0)
    FDir_BiomeDistributionTarget Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    Savana.DistributionPercentage = 20.0f;
    BiomeTargets.Add(Savana);
    
    // Pantano (-50000,-45000)
    FDir_BiomeDistributionTarget Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    Pantano.DistributionPercentage = 20.0f;
    BiomeTargets.Add(Pantano);
    
    // Floresta (-45000,40000)
    FDir_BiomeDistributionTarget Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    Floresta.DistributionPercentage = 20.0f;
    BiomeTargets.Add(Floresta);
    
    // Deserto (55000,0)
    FDir_BiomeDistributionTarget Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    Deserto.DistributionPercentage = 20.0f;
    BiomeTargets.Add(Deserto);
    
    // Montanha (40000,50000)
    FDir_BiomeDistributionTarget Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 100.0f);
    Montanha.DistributionPercentage = 20.0f;
    BiomeTargets.Add(Montanha);
    
    UE_LOG(LogTemp, Warning, TEXT("Biome targets initialized: %d biomes"), BiomeTargets.Num());
}

FVector UDir_CriticalPathManager::GetNextSpawnLocation(const FString& BiomeName)
{
    for (FDir_BiomeDistributionTarget& Biome : BiomeTargets)
    {
        if (Biome.BiomeName == BiomeName)
        {
            // Random offset within biome radius (15000 units)
            float OffsetX = FMath::RandRange(-15000.0f, 15000.0f);
            float OffsetY = FMath::RandRange(-15000.0f, 15000.0f);
            
            return Biome.CenterLocation + FVector(OffsetX, OffsetY, 0.0f);
        }
    }
    
    // Default to Savana if biome not found
    return FVector(0.0f, 0.0f, 100.0f);
}

void UDir_CriticalPathManager::UpdateBiomeActorCount(const FString& BiomeName, int32 Delta)
{
    for (FDir_BiomeDistributionTarget& Biome : BiomeTargets)
    {
        if (Biome.BiomeName == BiomeName)
        {
            Biome.CurrentActorCount += Delta;
            break;
        }
    }
}

bool UDir_CriticalPathManager::IsBiomeDistributionBalanced()
{
    if (!bEnforceBiomeDistribution) return true;
    
    int32 TotalActors = 0;
    for (const FDir_BiomeDistributionTarget& Biome : BiomeTargets)
    {
        TotalActors += Biome.CurrentActorCount;
    }
    
    if (TotalActors == 0) return true;
    
    // Check if any biome has more than 30% of total actors
    for (const FDir_BiomeDistributionTarget& Biome : BiomeTargets)
    {
        float ActualPercentage = (float)Biome.CurrentActorCount / (float)TotalActors * 100.0f;
        if (ActualPercentage > 30.0f)
        {
            return false;
        }
    }
    
    return true;
}

void UDir_CriticalPathManager::BlockAgent(int32 AgentID, const FString& Reason)
{
    if (!BlockedAgents.Contains(AgentID))
    {
        BlockedAgents.Add(AgentID);
        UpdateTaskStatus(AgentID, EDir_AgentStatus::Blocked);
        UE_LOG(LogTemp, Error, TEXT("Agent %d BLOCKED: %s"), AgentID, *Reason);
    }
}

void UDir_CriticalPathManager::UnblockAgent(int32 AgentID)
{
    BlockedAgents.Remove(AgentID);
    UpdateTaskStatus(AgentID, EDir_AgentStatus::Idle);
    UE_LOG(LogTemp, Warning, TEXT("Agent %d UNBLOCKED"), AgentID);
}

TArray<int32> UDir_CriticalPathManager::GetReadyAgents()
{
    TArray<int32> ReadyAgents;
    
    for (int32 i = 1; i <= 19; ++i)
    {
        if (CanAgentProceed(i))
        {
            ReadyAgents.Add(i);
        }
    }
    
    return ReadyAgents;
}

void UDir_CriticalPathManager::LogCriticalPathStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CRITICAL PATH STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Playable Progress: %.1f%%"), PlayablePrototypeProgress);
    UE_LOG(LogTemp, Warning, TEXT("Minimal Playable Reached: %s"), bMinimalPlayableReached ? TEXT("YES") : TEXT("NO"));
    
    for (int32 i = 0; i < CriticalPathTasks.Num(); ++i)
    {
        const FDir_CriticalPathTask& Task = CriticalPathTasks[i];
        FString StatusStr;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusStr = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusStr = TEXT("WORKING"); break;
            case EDir_AgentStatus::Blocked: StatusStr = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Complete: StatusStr = TEXT("COMPLETE"); break;
            case EDir_AgentStatus::Failed: StatusStr = TEXT("FAILED"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Task %d: %s [Agent %d] - %s"), 
               i, *Task.TaskName, Task.AgentID, *StatusStr);
    }
}

void UDir_CriticalPathManager::LogBiomeDistribution()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME DISTRIBUTION ==="));
    
    int32 TotalActors = 0;
    for (const FDir_BiomeDistributionTarget& Biome : BiomeTargets)
    {
        TotalActors += Biome.CurrentActorCount;
    }
    
    for (const FDir_BiomeDistributionTarget& Biome : BiomeTargets)
    {
        float ActualPercentage = TotalActors > 0 ? (float)Biome.CurrentActorCount / (float)TotalActors * 100.0f : 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("%s: %d actors (%.1f%% - Target: %.1f%%)"), 
               *Biome.BiomeName, Biome.CurrentActorCount, ActualPercentage, Biome.DistributionPercentage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Distribution Balanced: %s"), IsBiomeDistributionBalanced() ? TEXT("YES") : TEXT("NO"));
}

void UDir_CriticalPathManager::ValidateProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION PIPELINE VALIDATION ==="));
    
    // Check critical path completion
    int32 CriticalTasks = 0;
    int32 CompletedCritical = 0;
    
    for (const FDir_CriticalPathTask& Task : CriticalPathTasks)
    {
        if (Task.Priority == EDir_CriticalPathPriority::Critical)
        {
            CriticalTasks++;
            if (Task.Status == EDir_AgentStatus::Complete)
            {
                CompletedCritical++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Critical Tasks: %d/%d completed"), CompletedCritical, CriticalTasks);
    
    // Check blocked agents
    if (BlockedAgents.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BLOCKED AGENTS: %d"), BlockedAgents.Num());
        for (int32 AgentID : BlockedAgents)
        {
            UE_LOG(LogTemp, Error, TEXT("  - Agent %d"), AgentID);
        }
    }
    
    // Check biome distribution
    if (!IsBiomeDistributionBalanced())
    {
        UE_LOG(LogTemp, Error, TEXT("BIOME DISTRIBUTION IMBALANCED - Enforcement Required"));
    }
}