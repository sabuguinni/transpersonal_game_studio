#include "Director_CriticalPathExecutor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UDir_CriticalPathExecutor::UDir_CriticalPathExecutor()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentCycleID = 4;
    CycleBudgetUsed = 25.30f;
    MaxCycleBudget = 75.0f;
}

void UDir_CriticalPathExecutor::BeginPlay()
{
    Super::BeginPlay();
    
    SetupDefaultBiomes();
    SetupDefaultAgents();
    SetupCriticalMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("Director Critical Path Executor initialized for Cycle %d"), CurrentCycleID);
}

void UDir_CriticalPathExecutor::SetupDefaultBiomes()
{
    BiomeConfigs.Empty();
    
    // Biome distribution from memory - 5 biomes with equal 20% distribution
    FDir_BiomeSpawnConfig Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    Savana.SpawnRadius = 15000.0f;
    Savana.MaxActorsPerBiome = 200;
    BiomeConfigs.Add(Savana);
    
    FDir_BiomeSpawnConfig Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    Pantano.SpawnRadius = 15000.0f;
    Pantano.MaxActorsPerBiome = 200;
    BiomeConfigs.Add(Pantano);
    
    FDir_BiomeSpawnConfig Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    Floresta.SpawnRadius = 15000.0f;
    Floresta.MaxActorsPerBiome = 200;
    BiomeConfigs.Add(Floresta);
    
    FDir_BiomeSpawnConfig Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    Deserto.SpawnRadius = 15000.0f;
    Deserto.MaxActorsPerBiome = 200;
    BiomeConfigs.Add(Deserto);
    
    FDir_BiomeSpawnConfig Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 100.0f);
    Montanha.SpawnRadius = 15000.0f;
    Montanha.MaxActorsPerBiome = 200;
    BiomeConfigs.Add(Montanha);
}

void UDir_CriticalPathExecutor::SetupDefaultAgents()
{
    AgentStatuses.Empty();
    
    // Initialize all 19 agents with current status
    TArray<FString> AgentNames = {
        TEXT("Studio Director"), TEXT("Engine Architect"), TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"), TEXT("World Generator"), TEXT("Environment Artist"),
        TEXT("Architecture Agent"), TEXT("Lighting Artist"), TEXT("Character Artist"),
        TEXT("Animation Agent"), TEXT("NPC Behavior Agent"), TEXT("Combat AI Agent"),
        TEXT("Crowd Simulation"), TEXT("Quest Designer"), TEXT("Narrative Designer"),
        TEXT("Audio Agent"), TEXT("VFX Agent"), TEXT("QA Testing"), TEXT("Integration Agent")
    };
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentExecutionStatus Status;
        Status.AgentID = i + 1;
        Status.AgentName = AgentNames[i];
        Status.bHasExecutedThisCycle = (i < 9); // Agents 1-9 have excellent track record
        Status.FilesCreated = (i < 9) ? 8 : 2; // Agents 1-9 create more files
        Status.UE5CommandsExecuted = (i < 9) ? 4 : 1;
        Status.CycleCompletionTime = (i < 9) ? 120.0f : 180.0f;
        AgentStatuses.Add(Status);
    }
}

void UDir_CriticalPathExecutor::SetupCriticalMilestones()
{
    CriticalMilestones.Empty();
    
    // Milestone 1: Walk Around Prototype
    FDir_CriticalPathMilestone WalkAround;
    WalkAround.MilestoneName = TEXT("Walk Around Prototype");
    WalkAround.RequiredAgents = {1, 2, 3, 5, 9, 10}; // Director, Architect, Core, World, Character, Animation
    WalkAround.bIsCompleted = false;
    WalkAround.Priority = 10.0f;
    WalkAround.BlockingReason = TEXT("Need playable character with WASD movement");
    CriticalMilestones.Add(WalkAround);
    
    // Milestone 2: Dinosaur AI
    FDir_CriticalPathMilestone DinosaurAI;
    DinosaurAI.MilestoneName = TEXT("Basic Dinosaur AI");
    DinosaurAI.RequiredAgents = {11, 12}; // NPC Behavior, Combat AI
    DinosaurAI.bIsCompleted = false;
    DinosaurAI.Priority = 9.0f;
    DinosaurAI.BlockingReason = TEXT("T-Rex needs basic pursue behavior");
    CriticalMilestones.Add(DinosaurAI);
    
    // Milestone 3: Survival System
    FDir_CriticalPathMilestone Survival;
    Survival.MilestoneName = TEXT("Survival HUD");
    Survival.RequiredAgents = {3, 9}; // Core Systems, Character
    Survival.bIsCompleted = false;
    Survival.Priority = 8.0f;
    Survival.BlockingReason = TEXT("Need health/hunger/thirst UI");
    CriticalMilestones.Add(Survival);
}

void UDir_CriticalPathExecutor::InitializeBiomeDistribution()
{
    if (!ValidateBiomeDistribution())
    {
        UE_LOG(LogTemp, Error, TEXT("Biome distribution validation failed!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Biome distribution initialized with %d biomes"), BiomeConfigs.Num());
    for (const FDir_BiomeSpawnConfig& Biome : BiomeConfigs)
    {
        UE_LOG(LogTemp, Log, TEXT("Biome %s at %s with radius %f"), 
            *Biome.BiomeName, 
            *Biome.CenterLocation.ToString(), 
            Biome.SpawnRadius);
    }
}

FVector UDir_CriticalPathExecutor::GetRandomLocationInBiome(const FString& BiomeName)
{
    for (const FDir_BiomeSpawnConfig& Biome : BiomeConfigs)
    {
        if (Biome.BiomeName == BiomeName)
        {
            float RandomX = FMath::RandRange(-Biome.SpawnRadius, Biome.SpawnRadius);
            float RandomY = FMath::RandRange(-Biome.SpawnRadius, Biome.SpawnRadius);
            return Biome.CenterLocation + FVector(RandomX, RandomY, 0.0f);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Biome %s not found, returning origin"), *BiomeName);
    return FVector::ZeroVector;
}

bool UDir_CriticalPathExecutor::ValidateAgentExecution(int32 AgentID, int32 MinFiles, int32 MinUE5Commands)
{
    if (AgentID < 1 || AgentID > AgentStatuses.Num())
    {
        return false;
    }
    
    const FDir_AgentExecutionStatus& Status = AgentStatuses[AgentID - 1];
    return (Status.FilesCreated >= MinFiles && Status.UE5CommandsExecuted >= MinUE5Commands);
}

void UDir_CriticalPathExecutor::UpdateAgentStatus(int32 AgentID, int32 FilesCreated, int32 CommandsExecuted)
{
    if (AgentID < 1 || AgentID > AgentStatuses.Num())
    {
        return;
    }
    
    FDir_AgentExecutionStatus& Status = AgentStatuses[AgentID - 1];
    Status.FilesCreated = FilesCreated;
    Status.UE5CommandsExecuted = CommandsExecuted;
    Status.bHasExecutedThisCycle = true;
    Status.CycleCompletionTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Agent %d (%s) updated: %d files, %d commands"), 
        AgentID, *Status.AgentName, FilesCreated, CommandsExecuted);
}

bool UDir_CriticalPathExecutor::IsMilestoneBlocked(const FString& MilestoneName)
{
    for (const FDir_CriticalPathMilestone& Milestone : CriticalMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return !Milestone.BlockingReason.IsEmpty();
        }
    }
    return false;
}

TArray<int32> UDir_CriticalPathExecutor::GetBlockingAgents()
{
    TArray<int32> BlockingAgents;
    
    for (const FDir_AgentExecutionStatus& Status : AgentStatuses)
    {
        // Agents 11-15 have timeout issues based on memories
        if (Status.AgentID >= 11 && Status.AgentID <= 15)
        {
            if (Status.FilesCreated < 2 || Status.UE5CommandsExecuted < 1)
            {
                BlockingAgents.Add(Status.AgentID);
            }
        }
        // Other agents need minimum 4 files, 2 UE5 commands
        else if (Status.FilesCreated < 4 || Status.UE5CommandsExecuted < 2)
        {
            BlockingAgents.Add(Status.AgentID);
        }
    }
    
    return BlockingAgents;
}

void UDir_CriticalPathExecutor::ExecuteCriticalPathValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CRITICAL PATH VALIDATION CYCLE %d ==="), CurrentCycleID);
    
    // Check biome distribution
    if (!ValidateBiomeDistribution())
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Biome distribution failed validation!"));
    }
    
    // Check agent performance
    TArray<int32> BlockingAgents = GetBlockingAgents();
    if (BlockingAgents.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: %d agents are blocking production"), BlockingAgents.Num());
        for (int32 AgentID : BlockingAgents)
        {
            UE_LOG(LogTemp, Error, TEXT("  - Agent %d (%s)"), AgentID, *AgentStatuses[AgentID-1].AgentName);
        }
    }
    
    // Check milestones
    for (const FDir_CriticalPathMilestone& Milestone : CriticalMilestones)
    {
        if (!Milestone.bIsCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Milestone '%s' blocked: %s"), 
                *Milestone.MilestoneName, *Milestone.BlockingReason);
        }
    }
    
    LogCriticalPathStatus();
}

void UDir_CriticalPathExecutor::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT CYCLE %d ==="), CurrentCycleID);
    UE_LOG(LogTemp, Log, TEXT("Budget: $%.2f / $%.2f (%.1f%% used)"), 
        CycleBudgetUsed, MaxCycleBudget, (CycleBudgetUsed / MaxCycleBudget) * 100.0f);
    
    int32 ActiveAgents = 0;
    int32 TotalFiles = 0;
    int32 TotalCommands = 0;
    
    for (const FDir_AgentExecutionStatus& Status : AgentStatuses)
    {
        if (Status.bHasExecutedThisCycle)
        {
            ActiveAgents++;
            TotalFiles += Status.FilesCreated;
            TotalCommands += Status.UE5CommandsExecuted;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Active Agents: %d/19"), ActiveAgents);
    UE_LOG(LogTemp, Log, TEXT("Total Files Created: %d"), TotalFiles);
    UE_LOG(LogTemp, Log, TEXT("Total UE5 Commands: %d"), TotalCommands);
    
    // Performance analysis
    if (ActiveAgents >= 15)
    {
        UE_LOG(LogTemp, Warning, TEXT("EXCELLENT: High agent participation"));
    }
    else if (ActiveAgents >= 10)
    {
        UE_LOG(LogTemp, Warning, TEXT("GOOD: Moderate agent participation"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Low agent participation - production at risk"));
    }
}

bool UDir_CriticalPathExecutor::ValidateBiomeDistribution()
{
    if (BiomeConfigs.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("Expected 5 biomes, found %d"), BiomeConfigs.Num());
        return false;
    }
    
    // Validate each biome has proper configuration
    for (const FDir_BiomeSpawnConfig& Biome : BiomeConfigs)
    {
        if (Biome.SpawnRadius <= 0.0f || Biome.MaxActorsPerBiome <= 0)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid configuration for biome %s"), *Biome.BiomeName);
            return false;
        }
    }
    
    return true;
}

void UDir_CriticalPathExecutor::LogCriticalPathStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CRITICAL PATH STATUS ==="));
    
    // Log high-performing agents (1-9)
    UE_LOG(LogTemp, Log, TEXT("HIGH PERFORMERS (Agents 1-9):"));
    for (int32 i = 0; i < 9 && i < AgentStatuses.Num(); i++)
    {
        const FDir_AgentExecutionStatus& Status = AgentStatuses[i];
        UE_LOG(LogTemp, Log, TEXT("  %s: %d files, %d commands"), 
            *Status.AgentName, Status.FilesCreated, Status.UE5CommandsExecuted);
    }
    
    // Log problematic agents (10-19)
    UE_LOG(LogTemp, Warning, TEXT("NEEDS IMPROVEMENT (Agents 10-19):"));
    for (int32 i = 9; i < AgentStatuses.Num(); i++)
    {
        const FDir_AgentExecutionStatus& Status = AgentStatuses[i];
        if (Status.FilesCreated < 4 || Status.UE5CommandsExecuted < 2)
        {
            UE_LOG(LogTemp, Warning, TEXT("  %s: %d files, %d commands (BELOW TARGET)"), 
                *Status.AgentName, Status.FilesCreated, Status.UE5CommandsExecuted);
        }
    }
}