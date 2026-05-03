#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UProductionCoordinator::UProductionCoordinator()
{
    // Initialize milestone data
    Milestone1Data.MilestoneName = TEXT("Milestone 1 - Walk Around");
    Milestone1Data.Description = TEXT("Player can walk around with dinosaurs visible in the world");
    Milestone1Data.Status = EDir_MilestoneStatus::InProgress;
    Milestone1Data.TargetDate = FDateTime::Now() + FTimespan::FromDays(2);
}

void UProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing..."));
    
    InitializeMilestone1();
    SetupAgentTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized with %d agent tasks"), AgentTasks.Num());
}

void UProductionCoordinator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Deinitializing..."));
    Super::Deinitialize();
}

void UProductionCoordinator::InitializeMilestone1()
{
    Milestone1Data.Tasks.Empty();
    
    // Define critical tasks for Milestone 1
    TArray<FString> CriticalTasks = {
        TEXT("ThirdPersonCharacter with WASD movement"),
        TEXT("Camera boom + follow camera"),
        TEXT("Landscape with basic terrain"),
        TEXT("Player can walk, run, jump"),
        TEXT("3-5 static dinosaur meshes in world"),
        TEXT("Directional light + sky atmosphere + fog")
    };
    
    for (const FString& TaskDesc : CriticalTasks)
    {
        FDir_AgentTask NewTask;
        NewTask.TaskDescription = TaskDesc;
        NewTask.Status = EDir_MilestoneStatus::InProgress;
        NewTask.Priority = 10.0f; // High priority
        Milestone1Data.Tasks.Add(NewTask);
    }
    
    Milestone1Data.Status = EDir_MilestoneStatus::InProgress;
    Milestone1Data.LastUpdate = FDateTime::Now();
}

void UProductionCoordinator::SetupAgentTasks()
{
    AgentTasks.Empty();
    
    // Agent #02 - Engine Architect
    FDir_AgentTask Task02;
    Task02.AgentID = 2;
    Task02.AgentName = TEXT("Engine Architect");
    Task02.TaskDescription = TEXT("Define core architecture and compilation rules");
    Task02.Status = EDir_MilestoneStatus::InProgress;
    Task02.Priority = 10.0f;
    Task02.Deliverables.Add(TEXT("EngineArchitecture.h"));
    Task02.Deliverables.Add(TEXT("EngineArchitecture.cpp"));
    AgentTasks.Add(Task02);
    
    // Agent #03 - Core Systems
    FDir_AgentTask Task03;
    Task03.AgentID = 3;
    Task03.AgentName = TEXT("Core Systems Programmer");
    Task03.TaskDescription = TEXT("Implement physics, collision, and movement systems");
    Task03.Status = EDir_MilestoneStatus::NotStarted;
    Task03.Priority = 9.0f;
    Task03.Dependencies.Add(TEXT("Agent02"));
    Task03.Deliverables.Add(TEXT("PhysicsCore.cpp"));
    Task03.Deliverables.Add(TEXT("CollisionSystem.cpp"));
    AgentTasks.Add(Task03);
    
    // Agent #05 - World Generator
    FDir_AgentTask Task05;
    Task05.AgentID = 5;
    Task05.AgentName = TEXT("Procedural World Generator");
    Task05.TaskDescription = TEXT("Expand landscape to 200km2 with 5 biomes");
    Task05.Status = EDir_MilestoneStatus::InProgress;
    Task05.Priority = 8.0f;
    Task05.Deliverables.Add(TEXT("LandscapeGenerator.cpp"));
    Task05.Deliverables.Add(TEXT("BiomeSystem.cpp"));
    AgentTasks.Add(Task05);
    
    // Agent #09 - Character Artist
    FDir_AgentTask Task09;
    Task09.AgentID = 9;
    Task09.AgentName = TEXT("Character Artist");
    Task09.TaskDescription = TEXT("Create TranspersonalCharacter with movement"));
    Task09.Status = EDir_MilestoneStatus::InProgress;
    Task09.Priority = 10.0f;
    Task09.Deliverables.Add(TEXT("TranspersonalCharacter.cpp"));
    Task09.Deliverables.Add(TEXT("PlayerController.cpp"));
    AgentTasks.Add(Task09);
    
    // Agent #10 - Animation
    FDir_AgentTask Task10;
    Task10.AgentID = 10;
    Task10.AgentName = TEXT("Animation Agent");
    Task10.TaskDescription = TEXT("Add character animations and IK"));
    Task10.Status = EDir_MilestoneStatus::NotStarted;
    Task10.Priority = 7.0f;
    Task10.Dependencies.Add(TEXT("Agent09"));
    Task10.Deliverables.Add(TEXT("CharacterAnimations.cpp"));
    AgentTasks.Add(Task10);
    
    // Agent #12 - Combat & Enemy AI
    FDir_AgentTask Task12;
    Task12.AgentID = 12;
    Task12.AgentName = TEXT("Combat & Enemy AI"));
    Task12.TaskDescription = TEXT("Create dinosaur actors with basic AI"));
    Task12.Status = EDir_MilestoneStatus::InProgress;
    Task12.Priority = 8.0f;
    Task12.Deliverables.Add(TEXT("DinosaurActor.cpp"));
    Task12.Deliverables.Add(TEXT("DinosaurAI.cpp"));
    AgentTasks.Add(Task12);
    
    // Agent #20 - Build Manager
    FDir_AgentTask Task20;
    Task20.AgentID = 20;
    Task20.AgentName = TEXT("Build Manager");
    Task20.TaskDescription = TEXT("Clean 122 phantom headers and compile project"));
    Task20.Status = EDir_MilestoneStatus::Blocked;
    Task20.Priority = 10.0f;
    Task20.Deliverables.Add(TEXT("Compilation Success"));
    Task20.Deliverables.Add(TEXT("Header Cleanup Report"));
    AgentTasks.Add(Task20);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Setup %d agent tasks"), AgentTasks.Num());
}

void UProductionCoordinator::UpdateAgentTask(int32 AgentID, const FString& TaskDescription, EDir_MilestoneStatus Status, const TArray<FString>& Deliverables)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.TaskDescription = TaskDescription;
            Task.Status = Status;
            Task.Deliverables = Deliverables;
            Task.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Updated Agent %d - %s"), AgentID, *TaskDescription);
            
            CalculateMilestoneProgress();
            UpdateCriticalPath();
            return;
        }
    }
    
    // If agent not found, create new task
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.AgentName = GetAgentName(AgentID);
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = Status;
    NewTask.Deliverables = Deliverables;
    NewTask.LastUpdate = FDateTime::Now();
    AgentTasks.Add(NewTask);
    
    CalculateMilestoneProgress();
}

float UProductionCoordinator::GetMilestone1Progress() const
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_MilestoneStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    return (float)CompletedTasks / (float)AgentTasks.Num() * 100.0f;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetBlockedTasks() const
{
    TArray<FDir_AgentTask> BlockedTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_MilestoneStatus::Blocked)
        {
            BlockedTasks.Add(Task);
        }
    }
    
    return BlockedTasks;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetHighPriorityTasks() const
{
    TArray<FDir_AgentTask> HighPriorityTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Priority >= 8.0f && Task.Status != EDir_MilestoneStatus::Completed)
        {
            HighPriorityTasks.Add(Task);
        }
    }
    
    return HighPriorityTasks;
}

FString UProductionCoordinator::GenerateProductionReport() const
{
    FString Report = TEXT("=== PRODUCTION REPORT - MILESTONE 1 ===\n");
    Report += FString::Printf(TEXT("Progress: %.1f%%\n"), GetMilestone1Progress());
    Report += FString::Printf(TEXT("Total Tasks: %d\n"), AgentTasks.Num());
    
    int32 CompletedCount = 0;
    int32 InProgressCount = 0;
    int32 BlockedCount = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_MilestoneStatus::Completed:
                CompletedCount++;
                break;
            case EDir_MilestoneStatus::InProgress:
                InProgressCount++;
                break;
            case EDir_MilestoneStatus::Blocked:
                BlockedCount++;
                break;
        }
    }
    
    Report += FString::Printf(TEXT("Completed: %d | In Progress: %d | Blocked: %d\n"), CompletedCount, InProgressCount, BlockedCount);
    
    Report += TEXT("\n=== HIGH PRIORITY TASKS ===\n");
    TArray<FDir_AgentTask> HighPriority = GetHighPriorityTasks();
    for (const FDir_AgentTask& Task : HighPriority)
    {
        Report += FString::Printf(TEXT("Agent %d (%s): %s [Priority: %.1f]\n"), 
            Task.AgentID, *Task.AgentName, *Task.TaskDescription, Task.Priority);
    }
    
    Report += TEXT("\n=== BLOCKED TASKS ===\n");
    TArray<FDir_AgentTask> Blocked = GetBlockedTasks();
    for (const FDir_AgentTask& Task : Blocked)
    {
        Report += FString::Printf(TEXT("Agent %d (%s): %s\n"), 
            Task.AgentID, *Task.AgentName, *Task.TaskDescription);
    }
    
    return Report;
}

bool UProductionCoordinator::IsAgentReady(int32 AgentID) const
{
    return CheckAgentDependencies(AgentID);
}

TArray<int32> UProductionCoordinator::GetNextAgentsToExecute() const
{
    TArray<int32> ReadyAgents;
    
    // Priority order based on critical path
    TArray<int32> PriorityOrder = {20, 2, 3, 5, 9, 12, 10, 8, 15, 14};
    
    for (int32 AgentID : PriorityOrder)
    {
        if (IsAgentReady(AgentID) && !CompletedAgents.Contains(AgentID))
        {
            ReadyAgents.Add(AgentID);
        }
    }
    
    return ReadyAgents;
}

void UProductionCoordinator::MarkAgentCompleted(int32 AgentID, const TArray<FString>& OutputFiles)
{
    CompletedAgents.AddUnique(AgentID);
    
    UpdateAgentTask(AgentID, TEXT("Completed"), EDir_MilestoneStatus::Completed, OutputFiles);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent %d marked as completed with %d output files"), AgentID, OutputFiles.Num());
}

TArray<FString> UProductionCoordinator::GetCriticalPath() const
{
    TArray<FString> CriticalPath;
    CriticalPath.Add(TEXT("Agent #20: Clean headers + Compile"));
    CriticalPath.Add(TEXT("Agent #02: Engine Architecture"));
    CriticalPath.Add(TEXT("Agent #03: Core Systems"));
    CriticalPath.Add(TEXT("Agent #09: Character Movement"));
    CriticalPath.Add(TEXT("Agent #05: Landscape Expansion"));
    CriticalPath.Add(TEXT("Agent #12: Dinosaur Actors"));
    
    return CriticalPath;
}

bool UProductionCoordinator::HasCriticalBlockers() const
{
    TArray<FDir_AgentTask> BlockedTasks = GetBlockedTasks();
    
    for (const FDir_AgentTask& Task : BlockedTasks)
    {
        if (Task.Priority >= 9.0f)
        {
            return true;
        }
    }
    
    return false;
}

void UProductionCoordinator::ValidateCurrentState()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Validating current state..."));
    
    FString ValidationReport = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *ValidationReport);
    
    // Check for critical blockers
    if (HasCriticalBlockers())
    {
        UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: CRITICAL BLOCKERS DETECTED!"));
    }
    
    // Update milestone progress
    CalculateMilestoneProgress();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Validation complete - Progress: %.1f%%"), GetMilestone1Progress());
}

void UProductionCoordinator::GenerateAgentInstructions()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Generating agent instructions..."));
    
    TArray<int32> NextAgents = GetNextAgentsToExecute();
    
    for (int32 AgentID : NextAgents)
    {
        FString AgentName = GetAgentName(AgentID);
        UE_LOG(LogTemp, Warning, TEXT("NEXT AGENT: #%d (%s) - READY FOR EXECUTION"), AgentID, *AgentName);
    }
    
    if (NextAgents.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: No agents ready - check dependencies"));
    }
}

void UProductionCoordinator::CalculateMilestoneProgress()
{
    float Progress = GetMilestone1Progress();
    Milestone1Data.CompletionPercentage = Progress;
    Milestone1Data.LastUpdate = FDateTime::Now();
    
    if (Progress >= 100.0f)
    {
        Milestone1Data.Status = EDir_MilestoneStatus::Completed;
    }
    else if (Progress > 0.0f)
    {
        Milestone1Data.Status = EDir_MilestoneStatus::InProgress;
    }
}

void UProductionCoordinator::UpdateCriticalPath()
{
    CriticalBlockers.Empty();
    
    TArray<FDir_AgentTask> BlockedTasks = GetBlockedTasks();
    for (const FDir_AgentTask& Task : BlockedTasks)
    {
        if (Task.Priority >= 9.0f)
        {
            CriticalBlockers.Add(FString::Printf(TEXT("Agent %d: %s"), Task.AgentID, *Task.TaskDescription));
        }
    }
}

FString UProductionCoordinator::GetAgentName(int32 AgentID) const
{
    switch (AgentID)
    {
        case 2: return TEXT("Engine Architect");
        case 3: return TEXT("Core Systems Programmer");
        case 5: return TEXT("Procedural World Generator");
        case 9: return TEXT("Character Artist");
        case 10: return TEXT("Animation Agent");
        case 12: return TEXT("Combat & Enemy AI");
        case 20: return TEXT("Build Manager");
        default: return FString::Printf(TEXT("Agent #%d"), AgentID);
    }
}

bool UProductionCoordinator::CheckAgentDependencies(int32 AgentID) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            for (const FString& Dependency : Task.Dependencies)
            {
                // Check if dependency is completed
                bool DependencyMet = false;
                for (const FDir_AgentTask& DepTask : AgentTasks)
                {
                    if (DepTask.AgentName == Dependency && DepTask.Status == EDir_MilestoneStatus::Completed)
                    {
                        DependencyMet = true;
                        break;
                    }
                }
                
                if (!DependencyMet)
                {
                    return false;
                }
            }
            return true;
        }
    }
    
    // If agent not found in tasks, assume ready
    return true;
}