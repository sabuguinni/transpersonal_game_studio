#include "Director_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    OverallProgress = 0.0f;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    InitializeProductionPipeline();
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateCriticalPath();
    CalculateOverallProgress();
}

void UDir_ProductionCoordinator::InitializeProductionPipeline()
{
    // Initialize all 19 agents
    AgentTasks.Empty();
    
    // Agent #1 - Studio Director (Self)
    FDir_AgentTaskInfo DirectorTask;
    DirectorTask.AgentID = 1;
    DirectorTask.AgentName = TEXT("Studio Director");
    DirectorTask.Status = EDir_AgentStatus::Working;
    DirectorTask.CurrentTask = TEXT("Coordinate production pipeline and manage agent workflow");
    DirectorTask.CompletionPercentage = 25.0f;
    AgentTasks.Add(DirectorTask);

    // Agent #2 - Engine Architect
    FDir_AgentTaskInfo ArchitectTask;
    ArchitectTask.AgentID = 2;
    ArchitectTask.AgentName = TEXT("Engine Architect");
    ArchitectTask.Status = EDir_AgentStatus::Idle;
    ArchitectTask.CurrentTask = TEXT("Define core architecture and technical standards");
    ArchitectTask.Dependencies.Add(TEXT("Studio Director initialization"));
    AgentTasks.Add(ArchitectTask);

    // Agent #3 - Core Systems Programmer
    FDir_AgentTaskInfo CoreTask;
    CoreTask.AgentID = 3;
    CoreTask.AgentName = TEXT("Core Systems Programmer");
    CoreTask.Status = EDir_AgentStatus::Idle;
    CoreTask.CurrentTask = TEXT("Implement physics, collision, and core systems");
    CoreTask.Dependencies.Add(TEXT("Engine Architecture complete"));
    AgentTasks.Add(CoreTask);

    // Agent #4 - Performance Optimizer
    FDir_AgentTaskInfo PerfTask;
    PerfTask.AgentID = 4;
    PerfTask.AgentName = TEXT("Performance Optimizer");
    PerfTask.Status = EDir_AgentStatus::Idle;
    PerfTask.CurrentTask = TEXT("Optimize systems for 60fps PC / 30fps console"));
    PerfTask.Dependencies.Add(TEXT("Core Systems implementation"));
    AgentTasks.Add(PerfTask);

    // Agent #5 - Procedural World Generator
    FDir_AgentTaskInfo WorldTask;
    WorldTask.AgentID = 5;
    WorldTask.AgentName = TEXT("Procedural World Generator");
    WorldTask.Status = EDir_AgentStatus::Idle;
    WorldTask.CurrentTask = TEXT("Generate terrain, biomes, and world structure"));
    WorldTask.Dependencies.Add(TEXT("Performance optimization baseline"));
    AgentTasks.Add(WorldTask);

    // Continue with remaining agents...
    SetupAgentDependencies();
    SetupMilestones();

    UE_LOG(LogTemp, Warning, TEXT("Production Pipeline Initialized with %d agents"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::SetupAgentDependencies()
{
    // Agent #6 - Environment Artist
    FDir_AgentTaskInfo EnvTask;
    EnvTask.AgentID = 6;
    EnvTask.AgentName = TEXT("Environment Artist");
    EnvTask.CurrentTask = TEXT("Populate world with vegetation, rocks, and materials");
    EnvTask.Dependencies.Add(TEXT("World generation complete"));
    AgentTasks.Add(EnvTask);

    // Agent #7 - Architecture & Interior
    FDir_AgentTaskInfo ArchInteriorTask;
    ArchInteriorTask.AgentID = 7;
    ArchInteriorTask.AgentName = TEXT("Architecture & Interior Agent");
    ArchInteriorTask.CurrentTask = TEXT("Build prehistoric structures and interiors"));
    ArchInteriorTask.Dependencies.Add(TEXT("Environment art base"));
    AgentTasks.Add(ArchInteriorTask);

    // Agent #8 - Lighting & Atmosphere
    FDir_AgentTaskInfo LightingTask;
    LightingTask.AgentID = 8;
    LightingTask.AgentName = TEXT("Lighting & Atmosphere Agent");
    LightingTask.CurrentTask = TEXT("Implement day/night cycle and atmospheric lighting"));
    LightingTask.Dependencies.Add(TEXT("Architecture complete"));
    AgentTasks.Add(LightingTask);

    // Agent #9 - Character Artist
    FDir_AgentTaskInfo CharTask;
    CharTask.AgentID = 9;
    CharTask.AgentName = TEXT("Character Artist Agent");
    CharTask.CurrentTask = TEXT("Create playable characters and NPCs using MetaHuman"));
    CharTask.Dependencies.Add(TEXT("Lighting system established"));
    AgentTasks.Add(CharTask);

    // Agent #10 - Animation Agent
    FDir_AgentTaskInfo AnimTask;
    AnimTask.AgentID = 10;
    AnimTask.AgentName = TEXT("Animation Agent");
    AnimTask.CurrentTask = TEXT("Implement Motion Matching and IK systems"));
    AnimTask.Dependencies.Add(TEXT("Character models complete"));
    AgentTasks.Add(AnimTask);

    // Continue with remaining agents #11-#19...
}

void UDir_ProductionCoordinator::SetupMilestones()
{
    Milestones.Empty();

    // Milestone 1: Playable Prototype
    FDir_MilestoneInfo PlayablePrototype;
    PlayablePrototype.MilestoneName = TEXT("Playable Prototype");
    PlayablePrototype.Phase = EDir_ProductionPhase::CoreSystems;
    PlayablePrototype.RequiredAgents = {1, 2, 3, 9}; // Director, Architect, Core, Character
    PlayablePrototype.Deliverables.Add(TEXT("Character movement with WASD"));
    PlayablePrototype.Deliverables.Add(TEXT("Basic terrain with collision"));
    PlayablePrototype.Deliverables.Add(TEXT("Camera system"));
    PlayablePrototype.Deliverables.Add(TEXT("5+ dinosaur actors in world"));
    PlayablePrototype.TargetDate = FDateTime::Now() + FTimespan::FromDays(2);
    Milestones.Add(PlayablePrototype);

    // Milestone 2: Survival Systems
    FDir_MilestoneInfo SurvivalSystems;
    SurvivalSystems.MilestoneName = TEXT("Survival Systems");
    SurvivalSystems.Phase = EDir_ProductionPhase::GameplayMechanics;
    SurvivalSystems.RequiredAgents = {3, 12, 14}; // Core, Combat AI, Quest Designer
    SurvivalSystems.Deliverables.Add(TEXT("Health/Hunger/Thirst/Stamina systems"));
    SurvivalSystems.Deliverables.Add(TEXT("Basic crafting"));
    SurvivalSystems.Deliverables.Add(TEXT("Dinosaur AI behavior"));
    SurvivalSystems.TargetDate = FDateTime::Now() + FTimespan::FromDays(7);
    Milestones.Add(SurvivalSystems);

    // Milestone 3: World Population
    FDir_MilestoneInfo WorldPopulation;
    WorldPopulation.MilestoneName = TEXT("World Population");
    WorldPopulation.Phase = EDir_ProductionPhase::WorldGeneration;
    WorldPopulation.RequiredAgents = {5, 6, 7, 8, 13}; // World Gen, Environment, Architecture, Lighting, Crowd
    WorldPopulation.Deliverables.Add(TEXT("3 distinct biomes"));
    WorldPopulation.Deliverables.Add(TEXT("Dynamic weather"));
    WorldPopulation.Deliverables.Add(TEXT("Populated ecosystem"));
    WorldPopulation.TargetDate = FDateTime::Now() + FTimespan::FromDays(14);
    Milestones.Add(WorldPopulation);
}

void UDir_ProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    for (FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.AgentID == AgentID)
        {
            Agent.Status = NewStatus;
            if (!TaskDescription.IsEmpty())
            {
                Agent.CurrentTask = TaskDescription;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s) status updated to %d: %s"), 
                   AgentID, *Agent.AgentName, (int32)NewStatus, *TaskDescription);
            break;
        }
    }
}

bool UDir_ProductionCoordinator::CanAgentProceed(int32 AgentID)
{
    for (const FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.AgentID == AgentID)
        {
            return CheckDependenciesMet(AgentID);
        }
    }
    return false;
}

bool UDir_ProductionCoordinator::CheckDependenciesMet(int32 AgentID)
{
    // For now, simplified dependency check
    // In full implementation, would check actual dependency completion
    return true;
}

TArray<int32> UDir_ProductionCoordinator::GetBlockedAgents()
{
    TArray<int32> BlockedAgents;
    
    for (const FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(Agent.AgentID);
        }
    }
    
    return BlockedAgents;
}

void UDir_ProductionCoordinator::AdvanceToNextPhase()
{
    if (ValidatePhaseCompletion(CurrentPhase))
    {
        CurrentPhase = static_cast<EDir_ProductionPhase>(static_cast<int32>(CurrentPhase) + 1);
        UE_LOG(LogTemp, Warning, TEXT("Advanced to production phase: %d"), (int32)CurrentPhase);
    }
}

bool UDir_ProductionCoordinator::ValidatePhaseCompletion(EDir_ProductionPhase Phase)
{
    // Check if all required milestones for this phase are complete
    for (const FDir_MilestoneInfo& Milestone : Milestones)
    {
        if (Milestone.Phase == Phase && !Milestone.bIsComplete)
        {
            return false;
        }
    }
    return true;
}

FString UDir_ProductionCoordinator::GetProductionStatusReport()
{
    FString Report = TEXT("=== PRODUCTION STATUS REPORT ===\n");
    Report += FString::Printf(TEXT("Current Phase: %d\n"), (int32)CurrentPhase);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n\n"), OverallProgress);
    
    Report += TEXT("AGENT STATUS:\n");
    for (const FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        Report += FString::Printf(TEXT("#%d %s: %d - %s (%.1f%%)\n"), 
                                  Agent.AgentID, *Agent.AgentName, (int32)Agent.Status, 
                                  *Agent.CurrentTask, Agent.CompletionPercentage);
    }
    
    Report += TEXT("\nMILESTONES:\n");
    for (const FDir_MilestoneInfo& Milestone : Milestones)
    {
        FString Status = Milestone.bIsComplete ? TEXT("COMPLETE") : TEXT("PENDING");
        Report += FString::Printf(TEXT("%s: %s\n"), *Milestone.MilestoneName, *Status);
    }
    
    return Report;
}

void UDir_ProductionCoordinator::CompleteMilestone(const FString& MilestoneName)
{
    for (FDir_MilestoneInfo& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.bIsComplete = true;
            UE_LOG(LogTemp, Warning, TEXT("Milestone completed: %s"), *MilestoneName);
            break;
        }
    }
}

bool UDir_ProductionCoordinator::IsMilestoneComplete(const FString& MilestoneName)
{
    for (const FDir_MilestoneInfo& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.bIsComplete;
        }
    }
    return false;
}

TArray<FString> UDir_ProductionCoordinator::GetPendingMilestones()
{
    TArray<FString> PendingMilestones;
    
    for (const FDir_MilestoneInfo& Milestone : Milestones)
    {
        if (!Milestone.bIsComplete)
        {
            PendingMilestones.Add(Milestone.MilestoneName);
        }
    }
    
    return PendingMilestones;
}

void UDir_ProductionCoordinator::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const TArray<FString>& Dependencies)
{
    for (FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.AgentID == AgentID)
        {
            Agent.CurrentTask = TaskDescription;
            Agent.Dependencies = Dependencies;
            Agent.Status = EDir_AgentStatus::Working;
            break;
        }
    }
}

void UDir_ProductionCoordinator::RegisterAgentOutput(int32 AgentID, const FString& OutputDescription)
{
    for (FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.AgentID == AgentID)
        {
            Agent.Outputs.Add(OutputDescription);
            break;
        }
    }
}

TArray<FString> UDir_ProductionCoordinator::GetAgentOutputs(int32 AgentID)
{
    for (const FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.AgentID == AgentID)
        {
            return Agent.Outputs;
        }
    }
    return TArray<FString>();
}

void UDir_ProductionCoordinator::TriggerQualityGate(EDir_ProductionPhase Phase)
{
    UE_LOG(LogTemp, Warning, TEXT("Quality gate triggered for phase: %d"), (int32)Phase);
    // Implementation would trigger Agent #18 (QA & Testing) validation
}

void UDir_ProductionCoordinator::UpdateCriticalPath()
{
    // Update critical path analysis
    // Implementation would calculate which agents are on the critical path
}

void UDir_ProductionCoordinator::CalculateOverallProgress()
{
    float TotalProgress = 0.0f;
    int32 AgentCount = AgentTasks.Num();
    
    for (const FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        TotalProgress += Agent.CompletionPercentage;
    }
    
    if (AgentCount > 0)
    {
        OverallProgress = TotalProgress / AgentCount;
    }
}