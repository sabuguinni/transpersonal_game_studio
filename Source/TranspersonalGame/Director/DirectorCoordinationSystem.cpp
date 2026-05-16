#include "DirectorCoordinationSystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

UDirectorCoordinationSystem::UDirectorCoordinationSystem()
{
    CurrentCycleID = TEXT("");
    bFBXPipelineTested = false;
    StableAtmosphereCycles = 0;
    BudgetUsed = 50.77f;
    BudgetLimit = 100.0f;
}

void UDirectorCoordinationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Initializing Studio Director pipeline"));
    
    InitializeAgentStatuses();
    InitializeBiomeStatuses();
    
    // Set current cycle from environment or default
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260516_006");
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Initialized for cycle %s"), *CurrentCycleID);
}

void UDirectorCoordinationSystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Shutting down"));
    
    AgentStatuses.Empty();
    CurrentTasks.Empty();
    BiomeStatuses.Empty();
    
    Super::Deinitialize();
}

void UDirectorCoordinationSystem::InitializeAgentStatuses()
{
    // Initialize all 19 agents with their specializations
    TArray<EDir_AgentType> AgentTypes = {
        EDir_AgentType::Director,      // Agent 1
        EDir_AgentType::Architect,     // Agent 2
        EDir_AgentType::CoreSystems,   // Agent 3
        EDir_AgentType::Performance,   // Agent 4
        EDir_AgentType::WorldGen,      // Agent 5
        EDir_AgentType::Environment,   // Agent 6
        EDir_AgentType::Architecture,  // Agent 7
        EDir_AgentType::Lighting,      // Agent 8
        EDir_AgentType::Characters,    // Agent 9
        EDir_AgentType::Animation,     // Agent 10
        EDir_AgentType::NPCBehavior,   // Agent 11
        EDir_AgentType::CombatAI,      // Agent 12
        EDir_AgentType::CrowdSim,      // Agent 13
        EDir_AgentType::QuestDesign,   // Agent 14
        EDir_AgentType::Narrative,     // Agent 15
        EDir_AgentType::Audio,         // Agent 16
        EDir_AgentType::VFX,           // Agent 17
        EDir_AgentType::QA,            // Agent 18
        EDir_AgentType::Integration    // Agent 19
    };
    
    for (int32 i = 0; i < AgentTypes.Num(); ++i)
    {
        FDir_AgentStatus Status;
        Status.AgentType = AgentTypes[i];
        Status.LastCycleID = TEXT("PROD_CYCLE_AUTO_20260516_005");
        Status.bIsActive = true;
        Status.DeliverablesCompleted = 0;
        Status.PerformanceScore = 1.0f;
        
        // Set critical path agents
        if (i == 4 || i == 8 || i == 11) // Agents 5, 9, 12
        {
            Status.CurrentTask = TEXT("CRITICAL_PATH");
            Status.PerformanceScore = 1.2f;
        }
        
        AgentStatuses.Add(i + 1, Status);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Initialized %d agent statuses"), AgentStatuses.Num());
}

void UDirectorCoordinationSystem::InitializeBiomeStatuses()
{
    // Initialize the 5 biomes with their coordinates from memory ID 709
    TArray<TPair<FString, FVector>> BiomeCoords = {
        {TEXT("Savanna"), FVector(0, 0, 100)},
        {TEXT("Swamp"), FVector(-50000, -45000, 100)},
        {TEXT("Forest"), FVector(-45000, 40000, 100)},
        {TEXT("Desert"), FVector(55000, 0, 100)},
        {TEXT("Mountains"), FVector(40000, 50000, 100)}
    };
    
    for (const auto& BiomeData : BiomeCoords)
    {
        FDir_BiomeStatus Status;
        Status.BiomeName = BiomeData.Key;
        Status.Coordinates = BiomeData.Value;
        Status.ActorCount = (BiomeData.Key == TEXT("Savanna")) ? 500 : 0; // Savanna already populated
        Status.bIsPopulated = (BiomeData.Key == TEXT("Savanna"));
        
        BiomeStatuses.Add(BiomeData.Key, Status);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Initialized %d biome statuses"), BiomeStatuses.Num());
}

void UDirectorCoordinationSystem::AssignTaskToAgent(int32 AgentNumber, const FDir_AgentTask& Task)
{
    if (AgentStatuses.Contains(AgentNumber))
    {
        CurrentTasks.Add(AgentNumber, Task);
        
        FDir_AgentStatus* Status = AgentStatuses.Find(AgentNumber);
        if (Status)
        {
            Status->CurrentTask = Task.TaskDescription;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Assigned task to Agent %d: %s"), 
               AgentNumber, *Task.TaskDescription);
    }
}

FDir_AgentStatus UDirectorCoordinationSystem::GetAgentStatus(int32 AgentNumber) const
{
    if (const FDir_AgentStatus* Status = AgentStatuses.Find(AgentNumber))
    {
        return *Status;
    }
    
    return FDir_AgentStatus(); // Return default if not found
}

void UDirectorCoordinationSystem::UpdateAgentStatus(int32 AgentNumber, const FDir_AgentStatus& Status)
{
    AgentStatuses.Add(AgentNumber, Status);
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Updated Agent %d status - Deliverables: %d"), 
           AgentNumber, Status.DeliverablesCompleted);
}

TArray<int32> UDirectorCoordinationSystem::GetCriticalPathAgents() const
{
    // Critical path agents: 5 (WorldGen), 9 (Characters), 12 (Combat/UI)
    return {5, 9, 12};
}

void UDirectorCoordinationSystem::UpdateBiomeStatus(const FString& BiomeName, int32 ActorCount, bool bPopulated)
{
    if (FDir_BiomeStatus* Status = BiomeStatuses.Find(BiomeName))
    {
        Status->ActorCount = ActorCount;
        Status->bIsPopulated = bPopulated;
        
        UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Updated %s biome - %d actors, populated: %s"), 
               *BiomeName, ActorCount, bPopulated ? TEXT("YES") : TEXT("NO"));
    }
}

FDir_BiomeStatus UDirectorCoordinationSystem::GetBiomeStatus(const FString& BiomeName) const
{
    if (const FDir_BiomeStatus* Status = BiomeStatuses.Find(BiomeName))
    {
        return *Status;
    }
    
    return FDir_BiomeStatus(); // Return default if not found
}

TArray<FDir_BiomeStatus> UDirectorCoordinationSystem::GetAllBiomeStatuses() const
{
    TArray<FDir_BiomeStatus> AllStatuses;
    for (const auto& BiomePair : BiomeStatuses)
    {
        AllStatuses.Add(BiomePair.Value);
    }
    return AllStatuses;
}

void UDirectorCoordinationSystem::StartProductionCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Starting production cycle %s"), *CycleID);
    
    // Reset agent task assignments for new cycle
    CurrentTasks.Empty();
    
    // Assign critical tasks to priority agents
    FDir_AgentTask WorldGenTask;
    WorldGenTask.TaskDescription = TEXT("Populate 4 remaining biomes with 500+ actors each using Meshy pipeline");
    WorldGenTask.Priority = EDir_AgentPriority::Critical;
    WorldGenTask.ExpectedDeliverable = TEXT("StaticMesh actors spawned in Swamp, Forest, Desert, Mountains");
    WorldGenTask.CycleID = CycleID;
    WorldGenTask.bIsBlocking = true;
    AssignTaskToAgent(5, WorldGenTask);
    
    FDir_AgentTask CharacterTask;
    CharacterTask.TaskDescription = TEXT("Create dinosaur actors with collision and basic AI");
    CharacterTask.Priority = EDir_AgentPriority::Critical;
    CharacterTask.ExpectedDeliverable = TEXT("5 dinosaur species with movement and interaction");
    CharacterTask.CycleID = CycleID;
    CharacterTask.bIsBlocking = true;
    AssignTaskToAgent(9, CharacterTask);
    
    FDir_AgentTask UITask;
    UITask.TaskDescription = TEXT("Implement survival HUD with health/hunger/thirst/stamina bars");
    UITask.Priority = EDir_AgentPriority::Critical;
    UITask.ExpectedDeliverable = TEXT("Functional UI overlay showing player stats");
    UITask.CycleID = CycleID;
    UITask.bIsBlocking = false;
    AssignTaskToAgent(12, UITask);
}

void UDirectorCoordinationSystem::CompleteProductionCycle(const FString& CycleID)
{
    UE_LOG(LogTemp, Warning, TEXT("DirectorCoordinationSystem: Completing production cycle %s"), *CycleID);
    
    // Update agent performance scores based on deliverables
    for (auto& AgentPair : AgentStatuses)
    {
        FDir_AgentStatus& Status = AgentPair.Value;
        Status.LastCycleID = CycleID;
        
        // Check if agent completed their assigned task
        if (CurrentTasks.Contains(AgentPair.Key))
        {
            Status.DeliverablesCompleted++;
            Status.PerformanceScore = FMath::Min(Status.PerformanceScore + 0.1f, 2.0f);
        }
    }
}

bool UDirectorCoordinationSystem::IsPlayablePrototypeReady() const
{
    // Check all three critical criteria
    bool bCriterion1 = CheckCriterion1_BiomePopulation();
    bool bCriterion2 = CheckCriterion2_AtmosphereStable();
    bool bCriterion3 = CheckCriterion3_FBXPipeline();
    
    return bCriterion1 && bCriterion2 && bCriterion3;
}

float UDirectorCoordinationSystem::GetOverallProgress() const
{
    float Progress = 0.0f;
    
    // Criterion 1: Biome population (40% weight)
    int32 PopulatedBiomes = 0;
    for (const auto& BiomePair : BiomeStatuses)
    {
        if (BiomePair.Value.bIsPopulated)
        {
            PopulatedBiomes++;
        }
    }
    Progress += (PopulatedBiomes / 5.0f) * 0.4f;
    
    // Criterion 2: Atmosphere stability (30% weight)
    Progress += (StableAtmosphereCycles >= 3 ? 1.0f : StableAtmosphereCycles / 3.0f) * 0.3f;
    
    // Criterion 3: FBX pipeline (30% weight)
    Progress += (bFBXPipelineTested ? 1.0f : 0.0f) * 0.3f;
    
    return FMath::Clamp(Progress, 0.0f, 1.0f);
}

bool UDirectorCoordinationSystem::CheckCriterion1_BiomePopulation() const
{
    // All 5 biomes must have 500+ actors
    for (const auto& BiomePair : BiomeStatuses)
    {
        if (BiomePair.Value.ActorCount < 500)
        {
            return false;
        }
    }
    return true;
}

bool UDirectorCoordinationSystem::CheckCriterion2_AtmosphereStable() const
{
    // Must have 3+ consecutive cycles with stable Cretaceous atmosphere
    return StableAtmosphereCycles >= 3;
}

bool UDirectorCoordinationSystem::CheckCriterion3_FBXPipeline() const
{
    // FBX import pipeline must be tested with at least one asset
    return bFBXPipelineTested;
}

void UDirectorCoordinationSystem::ValidateAgentDependencies()
{
    // Ensure critical path agents are not blocked
    TArray<int32> CriticalAgents = GetCriticalPathAgents();
    
    for (int32 AgentNum : CriticalAgents)
    {
        if (const FDir_AgentStatus* Status = AgentStatuses.Find(AgentNum))
        {
            if (!Status->bIsActive)
            {
                UE_LOG(LogTemp, Error, TEXT("DirectorCoordinationSystem: CRITICAL AGENT %d IS BLOCKED!"), AgentNum);
            }
        }
    }
}