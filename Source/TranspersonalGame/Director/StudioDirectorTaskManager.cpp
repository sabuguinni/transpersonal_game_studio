#include "StudioDirectorTaskManager.h"
#include "Engine/Engine.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStudioDirector, Log, All);

UStudioDirectorTaskManager::UStudioDirectorTaskManager()
{
    // Initialize the 19-agent pipeline
    InitializeAgentPipeline();
}

void UStudioDirectorTaskManager::InitializeAgentPipeline()
{
    // Clear existing data
    AgentTasks.Empty();
    AgentStatusMap.Empty();
    AgentDeliverables.Empty();

    // Initialize all 19 agents with default status
    for (int32 i = 1; i <= 19; ++i)
    {
        AgentStatusMap.Add(i, EDir_AgentStatus::Idle);
        AgentDeliverables.Add(i, TArray<FString>());
    }

    // Setup critical path dependencies
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),           // #1
        TEXT("Engine Architect"),          // #2
        TEXT("Core Systems Programmer"),   // #3
        TEXT("Performance Optimizer"),     // #4
        TEXT("Procedural World Generator"), // #5
        TEXT("Environment Artist"),        // #6
        TEXT("Architecture & Interior"),   // #7
        TEXT("Lighting & Atmosphere"),     // #8
        TEXT("Character Artist"),          // #9
        TEXT("Animation Agent"),           // #10
        TEXT("NPC Behavior Agent"),        // #11
        TEXT("Combat & Enemy AI"),         // #12
        TEXT("Crowd & Traffic Simulation"), // #13
        TEXT("Quest & Mission Designer"),  // #14
        TEXT("Narrative & Dialogue"),      // #15
        TEXT("Audio Agent"),               // #16
        TEXT("VFX Agent"),                 // #17
        TEXT("QA & Testing Agent"),        // #18
        TEXT("Integration & Build Agent")  // #19
    };

    // Create initial tasks for critical path
    FDir_AgentTask EngineTask;
    EngineTask.AgentID = 2;
    EngineTask.TaskDescription = TEXT("Define UE5 architecture and compilation rules");
    EngineTask.Priority = EDir_TaskPriority::Critical;
    EngineTask.Status = EDir_AgentStatus::Idle;
    EngineTask.Dependencies = {1}; // Depends on Studio Director
    EngineTask.EstimatedTime = 120.0f;
    EngineTask.ExpectedDeliverables = TEXT("Architecture headers, build configuration");
    AgentTasks.Add(EngineTask);

    FDir_AgentTask CoreSystemsTask;
    CoreSystemsTask.AgentID = 3;
    CoreSystemsTask.TaskDescription = TEXT("Implement physics, collision, and core gameplay systems");
    CoreSystemsTask.Priority = EDir_TaskPriority::Critical;
    CoreSystemsTask.Status = EDir_AgentStatus::Idle;
    CoreSystemsTask.Dependencies = {2}; // Depends on Engine Architect
    CoreSystemsTask.EstimatedTime = 180.0f;
    CoreSystemsTask.ExpectedDeliverables = TEXT("Physics components, collision system");
    AgentTasks.Add(CoreSystemsTask);

    FDir_AgentTask WorldGenTask;
    WorldGenTask.AgentID = 5;
    WorldGenTask.TaskDescription = TEXT("Generate playable terrain with biomes");
    WorldGenTask.Priority = EDir_TaskPriority::High;
    WorldGenTask.Status = EDir_AgentStatus::Idle;
    WorldGenTask.Dependencies = {3}; // Depends on Core Systems
    WorldGenTask.EstimatedTime = 150.0f;
    WorldGenTask.ExpectedDeliverables = TEXT("Landscape with Savana, Forest, Desert biomes");
    AgentTasks.Add(WorldGenTask);

    FDir_AgentTask CharacterTask;
    CharacterTask.AgentID = 9;
    CharacterTask.TaskDescription = TEXT("Create playable character with WASD movement");
    CharacterTask.Priority = EDir_TaskPriority::Critical;
    CharacterTask.Status = EDir_AgentStatus::Idle;
    CharacterTask.Dependencies = {3}; // Depends on Core Systems
    CharacterTask.EstimatedTime = 120.0f;
    CharacterTask.ExpectedDeliverables = TEXT("TranspersonalCharacter with movement, camera, survival stats");
    AgentTasks.Add(CharacterTask);

    FDir_AgentTask CombatAITask;
    CombatAITask.AgentID = 12;
    CombatAITask.TaskDescription = TEXT("Implement T-Rex AI with pursuit behavior");
    CombatAITask.Priority = EDir_TaskPriority::High;
    CombatAITask.Status = EDir_AgentStatus::Idle;
    CombatAITask.Dependencies = {9}; // Depends on Character Artist
    CombatAITask.EstimatedTime = 100.0f;
    CombatAITask.ExpectedDeliverables = TEXT("DinosaurCombatAIController.cpp with BehaviorTree");
    AgentTasks.Add(CombatAITask);

    // Setup default milestones
    SetupDefaultMilestones();

    UE_LOG(LogStudioDirector, Log, TEXT("Studio Director pipeline initialized with %d agents"), AgentStatusMap.Num());
}

void UStudioDirectorTaskManager::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, EDir_TaskPriority Priority)
{
    if (!AgentStatusMap.Contains(AgentID))
    {
        UE_LOG(LogStudioDirector, Warning, TEXT("Invalid Agent ID: %d"), AgentID);
        return;
    }

    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Idle;
    
    AgentTasks.Add(NewTask);
    UE_LOG(LogStudioDirector, Log, TEXT("Task assigned to Agent #%d: %s"), AgentID, *TaskDescription);
}

void UStudioDirectorTaskManager::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    if (AgentStatusMap.Contains(AgentID))
    {
        AgentStatusMap[AgentID] = NewStatus;
        
        // Update corresponding task status
        for (FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.AgentID == AgentID)
            {
                Task.Status = NewStatus;
                break;
            }
        }
        
        UE_LOG(LogStudioDirector, Log, TEXT("Agent #%d status updated to: %d"), AgentID, (int32)NewStatus);
    }
}

bool UStudioDirectorTaskManager::CanAgentStart(int32 AgentID) const
{
    // Find the agent's current task
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            return AreAgentDependenciesMet(AgentID);
        }
    }
    return true; // No task assigned, can start
}

bool UStudioDirectorTaskManager::AreAgentDependenciesMet(int32 AgentID) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            for (int32 DepAgentID : Task.Dependencies)
            {
                if (AgentStatusMap.Contains(DepAgentID))
                {
                    EDir_AgentStatus DepStatus = AgentStatusMap[DepAgentID];
                    if (DepStatus != EDir_AgentStatus::Completed)
                    {
                        return false;
                    }
                }
            }
            break;
        }
    }
    return true;
}

TArray<FDir_AgentTask> UStudioDirectorTaskManager::GetBlockedTasks() const
{
    TArray<FDir_AgentTask> BlockedTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || !AreAgentDependenciesMet(Task.AgentID))
        {
            BlockedTasks.Add(Task);
        }
    }
    
    return BlockedTasks;
}

void UStudioDirectorTaskManager::CreateMilestone(const FString& MilestoneName, const TArray<int32>& RequiredAgents)
{
    FDir_MilestoneTracker NewMilestone;
    NewMilestone.MilestoneName = MilestoneName;
    NewMilestone.RequiredAgents = RequiredAgents;
    NewMilestone.bIsCompleted = false;
    NewMilestone.CompletionPercentage = 0.0f;
    
    Milestones.Add(NewMilestone);
    UE_LOG(LogStudioDirector, Log, TEXT("Milestone created: %s"), *MilestoneName);
}

float UStudioDirectorTaskManager::GetMilestoneProgress(const FString& MilestoneName) const
{
    for (const FDir_MilestoneTracker& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            int32 CompletedAgents = 0;
            for (int32 AgentID : Milestone.RequiredAgents)
            {
                if (AgentStatusMap.Contains(AgentID) && AgentStatusMap[AgentID] == EDir_AgentStatus::Completed)
                {
                    CompletedAgents++;
                }
            }
            
            return Milestone.RequiredAgents.Num() > 0 ? 
                (float)CompletedAgents / (float)Milestone.RequiredAgents.Num() * 100.0f : 0.0f;
        }
    }
    return 0.0f;
}

bool UStudioDirectorTaskManager::IsMilestoneComplete(const FString& MilestoneName) const
{
    return GetMilestoneProgress(MilestoneName) >= 100.0f;
}

TArray<int32> UStudioDirectorTaskManager::GetCriticalPathAgents() const
{
    // Critical path for minimum viable prototype
    return {2, 3, 5, 9, 12}; // Engine, Core, World, Character, Combat AI
}

void UStudioDirectorTaskManager::ReportAgentDeliverable(int32 AgentID, const FString& DeliverableName)
{
    if (AgentDeliverables.Contains(AgentID))
    {
        AgentDeliverables[AgentID].Add(DeliverableName);
        UE_LOG(LogStudioDirector, Log, TEXT("Agent #%d delivered: %s"), AgentID, *DeliverableName);
    }
}

void UStudioDirectorTaskManager::TriggerEmergencyReallocation()
{
    UE_LOG(LogStudioDirector, Warning, TEXT("Emergency reallocation triggered - reassigning critical tasks"));
    
    // Reset blocked agents to idle
    for (auto& StatusPair : AgentStatusMap)
    {
        if (StatusPair.Value == EDir_AgentStatus::Blocked || StatusPair.Value == EDir_AgentStatus::Failed)
        {
            StatusPair.Value = EDir_AgentStatus::Idle;
        }
    }
}

void UStudioDirectorTaskManager::BlockAgent(int32 AgentID, const FString& Reason)
{
    UpdateAgentStatus(AgentID, EDir_AgentStatus::Blocked);
    UE_LOG(LogStudioDirector, Error, TEXT("Agent #%d blocked: %s"), AgentID, *Reason);
}

void UStudioDirectorTaskManager::SetupDefaultMilestones()
{
    // Milestone 1: Walk Around (Critical)
    CreateMilestone(TEXT("Walk Around"), {2, 3, 5, 9});
    
    // Milestone 2: Dinosaur Encounters (High Priority)
    CreateMilestone(TEXT("Dinosaur Encounters"), {9, 11, 12});
    
    // Milestone 3: Survival Mechanics (Medium Priority)
    CreateMilestone(TEXT("Survival Mechanics"), {3, 9, 14});
    
    // Milestone 4: World Population (Medium Priority)
    CreateMilestone(TEXT("World Population"), {5, 6, 7, 8});
    
    // Milestone 5: Audio/Visual Polish (Low Priority)
    CreateMilestone(TEXT("Audio Visual Polish"), {16, 17, 8});
}

void UStudioDirectorTaskManager::ValidateAgentDependencies()
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        for (int32 DepID : Task.Dependencies)
        {
            if (!AgentStatusMap.Contains(DepID))
            {
                UE_LOG(LogStudioDirector, Error, TEXT("Invalid dependency: Agent #%d depends on non-existent Agent #%d"), 
                    Task.AgentID, DepID);
            }
        }
    }
}