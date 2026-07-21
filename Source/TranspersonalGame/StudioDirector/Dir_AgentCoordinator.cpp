#include "Dir_AgentCoordinator.h"
#include "Engine/Engine.h"

void UDir_AgentCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentMilestone = EDir_MilestonePhase::WalkAround;
    InitializeMilestoneRequirements();
    
    // Register all 19 agents with their specializations
    RegisterAgent(1, TEXT("Studio Director"), TEXT("Production Coordination"));
    RegisterAgent(2, TEXT("Engine Architect"), TEXT("Technical Architecture"));
    RegisterAgent(3, TEXT("Core Systems Programmer"), TEXT("Physics & Core Systems"));
    RegisterAgent(4, TEXT("Performance Optimizer"), TEXT("Performance & Optimization"));
    RegisterAgent(5, TEXT("Procedural World Generator"), TEXT("Terrain & World Generation"));
    RegisterAgent(6, TEXT("Environment Artist"), TEXT("Environment Art & Props"));
    RegisterAgent(7, TEXT("Architecture & Interior Agent"), TEXT("Buildings & Structures"));
    RegisterAgent(8, TEXT("Lighting & Atmosphere Agent"), TEXT("Lighting & Weather"));
    RegisterAgent(9, TEXT("Character Artist Agent"), TEXT("Character Creation"));
    RegisterAgent(10, TEXT("Animation Agent"), TEXT("Animation Systems"));
    RegisterAgent(11, TEXT("NPC Behavior Agent"), TEXT("NPC AI & Behavior"));
    RegisterAgent(12, TEXT("Combat & Enemy AI Agent"), TEXT("Combat Systems"));
    RegisterAgent(13, TEXT("Crowd & Traffic Simulation"), TEXT("Mass AI Systems"));
    RegisterAgent(14, TEXT("Quest & Mission Designer"), TEXT("Quest Systems"));
    RegisterAgent(15, TEXT("Narrative & Dialogue Agent"), TEXT("Story & Dialogue"));
    RegisterAgent(16, TEXT("Audio Agent"), TEXT("Audio & Music"));
    RegisterAgent(17, TEXT("VFX Agent"), TEXT("Visual Effects"));
    RegisterAgent(18, TEXT("QA & Testing Agent"), TEXT("Quality Assurance"));
    RegisterAgent(19, TEXT("Integration & Build Agent"), TEXT("Build Integration"));
    
    LogProductionEvent(TEXT("Agent Coordinator initialized - 19 agents registered"));
    UE_LOG(LogTemp, Warning, TEXT("Dir_AgentCoordinator: Production pipeline initialized"));
}

void UDir_AgentCoordinator::Deinitialize()
{
    LogProductionEvent(TEXT("Agent Coordinator shutdown"));
    Super::Deinitialize();
}

void UDir_AgentCoordinator::RegisterAgent(int32 AgentID, const FString& AgentName, const FString& Specialization)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.AgentName = AgentName;
    NewTask.CurrentTask = FString::Printf(TEXT("Initializing %s systems"), *Specialization);
    NewTask.Status = EDir_AgentStatus::Idle;
    NewTask.ProgressPercentage = 0.0f;
    NewTask.LastUpdate = FDateTime::Now();
    
    AgentTasks.Add(AgentID, NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Registered Agent %d: %s (%s)"), AgentID, *AgentName, *Specialization);
}

void UDir_AgentCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& CurrentTask, float Progress)
{
    if (FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        Task->Status = NewStatus;
        Task->CurrentTask = CurrentTask;
        Task->ProgressPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
        Task->LastUpdate = FDateTime::Now();
        Task->BlockingIssue = TEXT(""); // Clear blocking issue when status updates
        
        FString StatusString;
        switch (NewStatus)
        {
            case EDir_AgentStatus::Idle: StatusString = TEXT("Idle"); break;
            case EDir_AgentStatus::Working: StatusString = TEXT("Working"); break;
            case EDir_AgentStatus::Blocked: StatusString = TEXT("Blocked"); break;
            case EDir_AgentStatus::Complete: StatusString = TEXT("Complete"); break;
            case EDir_AgentStatus::Failed: StatusString = TEXT("Failed"); break;
        }
        
        LogProductionEvent(FString::Printf(TEXT("Agent %d (%s) status: %s - %s (%.1f%%)"), 
            AgentID, *Task->AgentName, *StatusString, *CurrentTask, Progress), AgentID);
    }
}

void UDir_AgentCoordinator::ReportAgentBlocked(int32 AgentID, const FString& BlockingIssue)
{
    if (FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        Task->Status = EDir_AgentStatus::Blocked;
        Task->BlockingIssue = BlockingIssue;
        Task->LastUpdate = FDateTime::Now();
        
        LogProductionEvent(FString::Printf(TEXT("CRITICAL: Agent %d (%s) BLOCKED - %s"), 
            AgentID, *Task->AgentName, *BlockingIssue), AgentID);
        
        UE_LOG(LogTemp, Error, TEXT("Agent %d BLOCKED: %s"), AgentID, *BlockingIssue);
    }
}

TArray<FDir_AgentTask> UDir_AgentCoordinator::GetAllAgentTasks() const
{
    TArray<FDir_AgentTask> Tasks;
    for (const auto& TaskPair : AgentTasks)
    {
        Tasks.Add(TaskPair.Value);
    }
    return Tasks;
}

FDir_AgentTask UDir_AgentCoordinator::GetAgentTask(int32 AgentID) const
{
    if (const FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        return *Task;
    }
    return FDir_AgentTask(); // Return default task if not found
}

void UDir_AgentCoordinator::SetCurrentMilestone(EDir_MilestonePhase NewMilestone)
{
    CurrentMilestone = NewMilestone;
    
    FString MilestoneName;
    switch (NewMilestone)
    {
        case EDir_MilestonePhase::WalkAround: MilestoneName = TEXT("Walk Around Prototype"); break;
        case EDir_MilestonePhase::BasicSurvival: MilestoneName = TEXT("Basic Survival Systems"); break;
        case EDir_MilestonePhase::DinosaurEncounter: MilestoneName = TEXT("Dinosaur Encounter"); break;
        case EDir_MilestonePhase::CraftingSystem: MilestoneName = TEXT("Crafting System"); break;
        case EDir_MilestonePhase::FullPrototype: MilestoneName = TEXT("Full Prototype"); break;
    }
    
    LogProductionEvent(FString::Printf(TEXT("MILESTONE CHANGE: Now targeting %s"), *MilestoneName));
}

bool UDir_AgentCoordinator::ValidateMilestoneCompletion(EDir_MilestonePhase Milestone)
{
    if (const TArray<FDir_MilestoneRequirement>* Requirements = MilestoneRequirements.Find(Milestone))
    {
        for (const FDir_MilestoneRequirement& Req : *Requirements)
        {
            if (!Req.bIsComplete)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

TArray<FDir_MilestoneRequirement> UDir_AgentCoordinator::GetMilestoneRequirements(EDir_MilestonePhase Milestone) const
{
    if (const TArray<FDir_MilestoneRequirement>* Requirements = MilestoneRequirements.Find(Milestone))
    {
        return *Requirements;
    }
    return TArray<FDir_MilestoneRequirement>();
}

TArray<int32> UDir_AgentCoordinator::GetCriticalPathAgents() const
{
    TArray<int32> CriticalAgents;
    
    // For Walk Around milestone, critical path is:
    // Agent 5 (World Gen) -> Agent 9 (Character) -> Agent 10 (Animation) -> Agent 12 (Combat/HUD)
    if (CurrentMilestone == EDir_MilestonePhase::WalkAround)
    {
        CriticalAgents.AddUnique(5);  // Procedural World Generator
        CriticalAgents.AddUnique(9);  // Character Artist
        CriticalAgents.AddUnique(10); // Animation Agent
        CriticalAgents.AddUnique(12); // Combat & Enemy AI (for HUD)
    }
    
    return CriticalAgents;
}

TArray<int32> UDir_AgentCoordinator::GetBlockedAgents() const
{
    TArray<int32> BlockedAgents;
    
    for (const auto& TaskPair : AgentTasks)
    {
        if (TaskPair.Value.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(TaskPair.Key);
        }
    }
    
    return BlockedAgents;
}

float UDir_AgentCoordinator::GetOverallProgress() const
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const auto& TaskPair : AgentTasks)
    {
        TotalProgress += TaskPair.Value.ProgressPercentage;
    }
    
    return TotalProgress / AgentTasks.Num();
}

FString UDir_AgentCoordinator::GenerateProductionReport() const
{
    FString Report = TEXT("=== TRANSPERSONAL GAME STUDIO PRODUCTION REPORT ===\n\n");
    
    // Current milestone
    FString MilestoneName;
    switch (CurrentMilestone)
    {
        case EDir_MilestonePhase::WalkAround: MilestoneName = TEXT("Walk Around Prototype"); break;
        case EDir_MilestonePhase::BasicSurvival: MilestoneName = TEXT("Basic Survival Systems"); break;
        case EDir_MilestonePhase::DinosaurEncounter: MilestoneName = TEXT("Dinosaur Encounter"); break;
        case EDir_MilestonePhase::CraftingSystem: MilestoneName = TEXT("Crafting System"); break;
        case EDir_MilestonePhase::FullPrototype: MilestoneName = TEXT("Full Prototype"); break;
    }
    
    Report += FString::Printf(TEXT("Current Milestone: %s\n"), *MilestoneName);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n\n"), GetOverallProgress());
    
    // Agent status summary
    int32 IdleCount = 0, WorkingCount = 0, BlockedCount = 0, CompleteCount = 0, FailedCount = 0;
    
    for (const auto& TaskPair : AgentTasks)
    {
        switch (TaskPair.Value.Status)
        {
            case EDir_AgentStatus::Idle: IdleCount++; break;
            case EDir_AgentStatus::Working: WorkingCount++; break;
            case EDir_AgentStatus::Blocked: BlockedCount++; break;
            case EDir_AgentStatus::Complete: CompleteCount++; break;
            case EDir_AgentStatus::Failed: FailedCount++; break;
        }
    }
    
    Report += TEXT("Agent Status Summary:\n");
    Report += FString::Printf(TEXT("- Working: %d\n"), WorkingCount);
    Report += FString::Printf(TEXT("- Complete: %d\n"), CompleteCount);
    Report += FString::Printf(TEXT("- Idle: %d\n"), IdleCount);
    Report += FString::Printf(TEXT("- Blocked: %d\n"), BlockedCount);
    Report += FString::Printf(TEXT("- Failed: %d\n\n"), FailedCount);
    
    // Critical path agents
    TArray<int32> CriticalAgents = GetCriticalPathAgents();
    if (CriticalAgents.Num() > 0)
    {
        Report += TEXT("Critical Path Agents:\n");
        for (int32 AgentID : CriticalAgents)
        {
            if (const FDir_AgentTask* Task = AgentTasks.Find(AgentID))
            {
                Report += FString::Printf(TEXT("- Agent %d (%s): %.1f%% - %s\n"), 
                    AgentID, *Task->AgentName, Task->ProgressPercentage, *Task->CurrentTask);
            }
        }
        Report += TEXT("\n");
    }
    
    // Blocked agents (critical issues)
    TArray<int32> BlockedAgents = GetBlockedAgents();
    if (BlockedAgents.Num() > 0)
    {
        Report += TEXT("CRITICAL ISSUES - Blocked Agents:\n");
        for (int32 AgentID : BlockedAgents)
        {
            if (const FDir_AgentTask* Task = AgentTasks.Find(AgentID))
            {
                Report += FString::Printf(TEXT("- Agent %d (%s): %s\n"), 
                    AgentID, *Task->AgentName, *Task->BlockingIssue);
            }
        }
        Report += TEXT("\n");
    }
    
    Report += TEXT("=== END REPORT ===");
    return Report;
}

void UDir_AgentCoordinator::LogProductionEvent(const FString& EventDescription, int32 AgentID)
{
    FString Timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
    FString LogEntry;
    
    if (AgentID >= 0)
    {
        LogEntry = FString::Printf(TEXT("[%s] Agent %d: %s"), *Timestamp, AgentID, *EventDescription);
    }
    else
    {
        LogEntry = FString::Printf(TEXT("[%s] SYSTEM: %s"), *Timestamp, *EventDescription);
    }
    
    ProductionLog.Add(LogEntry);
    
    // Keep only last 1000 entries to prevent memory bloat
    if (ProductionLog.Num() > 1000)
    {
        ProductionLog.RemoveAt(0, ProductionLog.Num() - 1000);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Production Event: %s"), *LogEntry);
}

void UDir_AgentCoordinator::InitializeMilestoneRequirements()
{
    // Walk Around Milestone Requirements
    TArray<FDir_MilestoneRequirement> WalkAroundReqs;
    
    FDir_MilestoneRequirement CharacterMovement;
    CharacterMovement.RequirementName = TEXT("Character Movement System");
    CharacterMovement.RequiredAgents = {9, 10}; // Character Artist + Animation
    CharacterMovement.ValidationCriteria = TEXT("Player can walk, run, jump with WASD controls");
    WalkAroundReqs.Add(CharacterMovement);
    
    FDir_MilestoneRequirement TerrainGeneration;
    TerrainGeneration.RequirementName = TEXT("Basic Terrain");
    TerrainGeneration.RequiredAgents = {5}; // World Generator
    TerrainGeneration.ValidationCriteria = TEXT("Landscape with height variation, not flat plane");
    WalkAroundReqs.Add(TerrainGeneration);
    
    FDir_MilestoneRequirement DinosaurPlacement;
    DinosaurPlacement.RequirementName = TEXT("Static Dinosaur Meshes");
    DinosaurPlacement.RequiredAgents = {6, 11}; // Environment + NPC Behavior
    DinosaurPlacement.ValidationCriteria = TEXT("3-5 dinosaur meshes placed in world");
    WalkAroundReqs.Add(DinosaurPlacement);
    
    FDir_MilestoneRequirement BasicLighting;
    BasicLighting.RequirementName = TEXT("Scene Lighting");
    BasicLighting.RequiredAgents = {8}; // Lighting & Atmosphere
    BasicLighting.ValidationCriteria = TEXT("Directional light + sky + fog");
    WalkAroundReqs.Add(BasicLighting);
    
    MilestoneRequirements.Add(EDir_MilestonePhase::WalkAround, WalkAroundReqs);
}