#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    ProductionUpdateInterval = 5.0f;
    bAutoResolveBlocks = true;
    LastUpdateTime = 0.0f;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgents();
    InitializeMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized with %d agents"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= ProductionUpdateInterval)
    {
        ValidateAgentDependencies();
        CheckMilestoneCompletion();
        LogProductionMetrics();
        LastUpdateTime = 0.0f;
    }
}

void UDir_ProductionCoordinator::InitializeAgents()
{
    AgentTasks.Empty();
    
    // Agent #1 - Studio Director (this agent)
    FDir_AgentTask Agent1;
    Agent1.AgentNumber = 1;
    Agent1.AgentName = TEXT("Studio Director");
    Agent1.CurrentTask = TEXT("Coordinate production workflow and manage agent dependencies");
    Agent1.Status = EDir_AgentStatus::Working;
    Agent1.ProgressPercent = 100.0f;
    AgentTasks.Add(Agent1);
    
    // Agent #2 - Engine Architect
    FDir_AgentTask Agent2;
    Agent2.AgentNumber = 2;
    Agent2.AgentName = TEXT("Engine Architect");
    Agent2.CurrentTask = TEXT("Define core architecture and technical standards");
    Agent2.Status = EDir_AgentStatus::Idle;
    Agent2.Dependencies.Add(TEXT("Studio Director approval"));
    AgentTasks.Add(Agent2);
    
    // Agent #3 - Core Systems Programmer
    FDir_AgentTask Agent3;
    Agent3.AgentNumber = 3;
    Agent3.AgentName = TEXT("Core Systems Programmer");
    Agent3.CurrentTask = TEXT("Implement physics, collision, and core gameplay systems");
    Agent3.Status = EDir_AgentStatus::Idle;
    Agent3.Dependencies.Add(TEXT("Engine Architecture complete"));
    AgentTasks.Add(Agent3);
    
    // Agent #4 - Performance Optimizer
    FDir_AgentTask Agent4;
    Agent4.AgentNumber = 4;
    Agent4.AgentName = TEXT("Performance Optimizer");
    Agent4.CurrentTask = TEXT("Ensure 60fps PC / 30fps console performance targets");
    Agent4.Status = EDir_AgentStatus::Idle;
    Agent4.Dependencies.Add(TEXT("Core Systems implementation"));
    AgentTasks.Add(Agent4);
    
    // Agent #5 - Procedural World Generator
    FDir_AgentTask Agent5;
    Agent5.AgentNumber = 5;
    Agent5.AgentName = TEXT("Procedural World Generator");
    Agent5.CurrentTask = TEXT("Generate terrain, biomes, and geographic structure"));
    Agent5.Status = EDir_AgentStatus::Idle;
    Agent5.Dependencies.Add(TEXT("Core Systems ready"));
    AgentTasks.Add(Agent5);
    
    // Continue with remaining agents...
    for (int32 i = 6; i <= 19; ++i)
    {
        FDir_AgentTask Agent;
        Agent.AgentNumber = i;
        
        switch (i)
        {
            case 6: Agent.AgentName = TEXT("Environment Artist"); break;
            case 7: Agent.AgentName = TEXT("Architecture & Interior Agent"); break;
            case 8: Agent.AgentName = TEXT("Lighting & Atmosphere Agent"); break;
            case 9: Agent.AgentName = TEXT("Character Artist Agent"); break;
            case 10: Agent.AgentName = TEXT("Animation Agent"); break;
            case 11: Agent.AgentName = TEXT("NPC Behavior Agent"); break;
            case 12: Agent.AgentName = TEXT("Combat & Enemy AI Agent"); break;
            case 13: Agent.AgentName = TEXT("Crowd & Traffic Simulation"); break;
            case 14: Agent.AgentName = TEXT("Quest & Mission Designer"); break;
            case 15: Agent.AgentName = TEXT("Narrative & Dialogue Agent"); break;
            case 16: Agent.AgentName = TEXT("Audio Agent"); break;
            case 17: Agent.AgentName = TEXT("VFX Agent"); break;
            case 18: Agent.AgentName = TEXT("QA & Testing Agent"); break;
            case 19: Agent.AgentName = TEXT("Integration & Build Agent"); break;
        }
        
        Agent.Status = EDir_AgentStatus::Idle;
        Agent.CurrentTask = TEXT("Awaiting dependencies");
        AgentTasks.Add(Agent);
    }
}

void UDir_ProductionCoordinator::InitializeMilestones()
{
    Milestones.Empty();
    
    // Milestone 1: Walk Around Prototype
    FDir_MilestoneProgress Milestone1;
    Milestone1.MilestoneType = EDir_MilestoneType::WalkAround;
    Milestone1.MilestoneName = TEXT("Walk Around Prototype");
    Milestone1.CompletionPercent = 25.0f; // Partially complete
    Milestone1.RequiredAgents = {1, 2, 3, 9, 10}; // Director, Architect, Core, Character, Animation
    Milestone1.RequiredDeliverables.Add(TEXT("ThirdPersonCharacter with WASD movement"));
    Milestone1.RequiredDeliverables.Add(TEXT("Basic terrain with height variation"));
    Milestone1.RequiredDeliverables.Add(TEXT("Camera system with follow camera"));
    Milestone1.RequiredDeliverables.Add(TEXT("Player can walk, run, jump"));
    Milestones.Add(Milestone1);
    
    // Milestone 2: Basic Survival
    FDir_MilestoneProgress Milestone2;
    Milestone2.MilestoneType = EDir_MilestoneType::BasicSurvival;
    Milestone2.MilestoneName = TEXT("Basic Survival Systems");
    Milestone2.CompletionPercent = 0.0f;
    Milestone2.RequiredAgents = {3, 11, 12, 14}; // Core, NPC, Combat, Quest
    Milestone2.RequiredDeliverables.Add(TEXT("Health, hunger, thirst, stamina systems"));
    Milestone2.RequiredDeliverables.Add(TEXT("Basic crafting system"));
    Milestone2.RequiredDeliverables.Add(TEXT("Day/night cycle"));
    Milestones.Add(Milestone2);
    
    // Milestone 3: Dinosaur AI
    FDir_MilestoneProgress Milestone3;
    Milestone3.MilestoneType = EDir_MilestoneType::DinosaurAI;
    Milestone3.MilestoneName = TEXT("Dinosaur AI Implementation");
    Milestone3.CompletionPercent = 0.0f;
    Milestone3.RequiredAgents = {11, 12, 13}; // NPC, Combat, Crowd
    Milestone3.RequiredDeliverables.Add(TEXT("Dinosaur behavior trees"));
    Milestone3.RequiredDeliverables.Add(TEXT("Territorial AI system"));
    Milestone3.RequiredDeliverables.Add(TEXT("Pack hunting mechanics"));
    Milestones.Add(Milestone3);
    
    // Milestone 4: World Generation
    FDir_MilestoneProgress Milestone4;
    Milestone4.MilestoneType = EDir_MilestoneType::WorldGeneration;
    Milestone4.MilestoneName = TEXT("Procedural World Generation");
    Milestone4.CompletionPercent = 0.0f;
    Milestone4.RequiredAgents = {5, 6, 7, 8}; // World, Environment, Architecture, Lighting
    Milestone4.RequiredDeliverables.Add(TEXT("PCG terrain generation"));
    Milestone4.RequiredDeliverables.Add(TEXT("Biome system"));
    Milestone4.RequiredDeliverables.Add(TEXT("Weather system"));
    Milestones.Add(Milestone4);
    
    // Milestone 5: Full Prototype
    FDir_MilestoneProgress Milestone5;
    Milestone5.MilestoneType = EDir_MilestoneType::FullPrototype;
    Milestone5.MilestoneName = TEXT("Full Playable Prototype");
    Milestone5.CompletionPercent = 0.0f;
    Milestone5.RequiredAgents = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    Milestone5.RequiredDeliverables.Add(TEXT("Complete survival gameplay loop"));
    Milestone5.RequiredDeliverables.Add(TEXT("Full dinosaur ecosystem"));
    Milestone5.RequiredDeliverables.Add(TEXT("Polished audio and VFX"));
    Milestones.Add(Milestone5);
}

void UDir_ProductionCoordinator::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.Status = NewStatus;
            Task.CurrentTask = TaskDescription;
            Task.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s) status updated: %s"), 
                AgentNumber, *Task.AgentName, *TaskDescription);
            break;
        }
    }
}

void UDir_ProductionCoordinator::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const TArray<FString>& Dependencies)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.CurrentTask = TaskDescription;
            Task.Dependencies = Dependencies;
            Task.Status = EDir_AgentStatus::Working;
            Task.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%d: %s"), AgentNumber, *TaskDescription);
            break;
        }
    }
}

FDir_AgentTask UDir_ProductionCoordinator::GetAgentStatus(int32 AgentNumber)
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            return Task;
        }
    }
    
    return FDir_AgentTask(); // Return default if not found
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetAllAgentStatuses()
{
    return AgentTasks;
}

void UDir_ProductionCoordinator::UpdateMilestoneProgress(EDir_MilestoneType MilestoneType, float NewProgress)
{
    for (FDir_MilestoneProgress& Milestone : Milestones)
    {
        if (Milestone.MilestoneType == MilestoneType)
        {
            Milestone.CompletionPercent = FMath::Clamp(NewProgress, 0.0f, 100.0f);
            UE_LOG(LogTemp, Warning, TEXT("Milestone '%s' progress: %.1f%%"), 
                *Milestone.MilestoneName, Milestone.CompletionPercent);
            break;
        }
    }
}

FDir_MilestoneProgress UDir_ProductionCoordinator::GetMilestoneProgress(EDir_MilestoneType MilestoneType)
{
    for (const FDir_MilestoneProgress& Milestone : Milestones)
    {
        if (Milestone.MilestoneType == MilestoneType)
        {
            return Milestone;
        }
    }
    
    return FDir_MilestoneProgress(); // Return default if not found
}

TArray<FDir_MilestoneProgress> UDir_ProductionCoordinator::GetAllMilestones()
{
    return Milestones;
}

bool UDir_ProductionCoordinator::CanAgentProceed(int32 AgentNumber)
{
    FDir_AgentTask AgentTask = GetAgentStatus(AgentNumber);
    
    // Check if all dependencies are met
    for (const FString& Dependency : AgentTask.Dependencies)
    {
        // Simple dependency check - in a real system this would be more sophisticated
        if (Dependency.Contains(TEXT("complete")) || Dependency.Contains(TEXT("ready")))
        {
            // Check if the dependency is actually satisfied
            // This is a simplified implementation
            continue;
        }
    }
    
    return AgentTask.Status != EDir_AgentStatus::Blocked;
}

TArray<int32> UDir_ProductionCoordinator::GetBlockedAgents()
{
    TArray<int32> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(Task.AgentNumber);
        }
    }
    
    return BlockedAgents;
}

void UDir_ProductionCoordinator::ResolveAgentBlock(int32 AgentNumber, const FString& Resolution)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && Task.Status == EDir_AgentStatus::Blocked)
        {
            Task.Status = EDir_AgentStatus::Working;
            Task.CurrentTask = Resolution;
            Task.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d unblocked with resolution: %s"), AgentNumber, *Resolution);
            break;
        }
    }
}

TArray<int32> UDir_ProductionCoordinator::GetCriticalPathAgents()
{
    // Critical path for Walk Around milestone
    TArray<int32> CriticalPath = {1, 2, 3, 9, 10}; // Director, Architect, Core, Character, Animation
    return CriticalPath;
}

float UDir_ProductionCoordinator::GetOverallProjectProgress()
{
    float TotalProgress = 0.0f;
    int32 CompletedMilestones = 0;
    
    for (const FDir_MilestoneProgress& Milestone : Milestones)
    {
        TotalProgress += Milestone.CompletionPercent;
        if (Milestone.CompletionPercent >= 100.0f)
        {
            CompletedMilestones++;
        }
    }
    
    return Milestones.Num() > 0 ? TotalProgress / Milestones.Num() : 0.0f;
}

void UDir_ProductionCoordinator::EmergencyResetAllAgents()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.CurrentTask = TEXT("Reset - awaiting new assignment");
        Task.ProgressPercent = 0.0f;
        Task.LastUpdate = FDateTime::Now();
    }
    
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY RESET: All agents reset to idle state"));
}

void UDir_ProductionCoordinator::ForceUnblockAgent(int32 AgentNumber)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.Status = EDir_AgentStatus::Working;
            Task.Dependencies.Empty(); // Clear all dependencies
            Task.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("FORCE UNBLOCK: Agent #%d dependencies cleared"), AgentNumber);
            break;
        }
    }
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    FString Report = TEXT("=== PRODUCTION REPORT ===\n");
    
    // Agent status summary
    int32 IdleAgents = 0, WorkingAgents = 0, CompletedAgents = 0, BlockedAgents = 0, FailedAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: IdleAgents++; break;
            case EDir_AgentStatus::Working: WorkingAgents++; break;
            case EDir_AgentStatus::Completed: CompletedAgents++; break;
            case EDir_AgentStatus::Blocked: BlockedAgents++; break;
            case EDir_AgentStatus::Failed: FailedAgents++; break;
        }
    }
    
    Report += FString::Printf(TEXT("Agent Status: %d Working, %d Completed, %d Idle, %d Blocked, %d Failed\n"), 
        WorkingAgents, CompletedAgents, IdleAgents, BlockedAgents, FailedAgents);
    
    // Milestone progress
    Report += TEXT("\nMilestone Progress:\n");
    for (const FDir_MilestoneProgress& Milestone : Milestones)
    {
        Report += FString::Printf(TEXT("- %s: %.1f%%\n"), *Milestone.MilestoneName, Milestone.CompletionPercent);
    }
    
    Report += FString::Printf(TEXT("\nOverall Project Progress: %.1f%%\n"), GetOverallProjectProgress());
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    // Save to file
    FString FilePath = FPaths::ProjectSavedDir() + TEXT("ProductionReport.txt");
    FFileHelper::SaveStringToFile(Report, *FilePath);
}

void UDir_ProductionCoordinator::ValidateAgentDependencies()
{
    // Check for circular dependencies and update agent status based on dependencies
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working && !CanAgentProceed(Task.AgentNumber))
        {
            Task.Status = EDir_AgentStatus::Blocked;
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d blocked due to unmet dependencies"), Task.AgentNumber);
        }
    }
}

void UDir_ProductionCoordinator::CheckMilestoneCompletion()
{
    for (FDir_MilestoneProgress& Milestone : Milestones)
    {
        // Check if all required agents have completed their tasks
        bool AllAgentsComplete = true;
        for (int32 RequiredAgent : Milestone.RequiredAgents)
        {
            FDir_AgentTask AgentTask = GetAgentStatus(RequiredAgent);
            if (AgentTask.Status != EDir_AgentStatus::Completed)
            {
                AllAgentsComplete = false;
                break;
            }
        }
        
        if (AllAgentsComplete && Milestone.CompletionPercent < 100.0f)
        {
            Milestone.CompletionPercent = 100.0f;
            UE_LOG(LogTemp, Warning, TEXT("Milestone COMPLETED: %s"), *Milestone.MilestoneName);
        }
    }
}

void UDir_ProductionCoordinator::LogProductionMetrics()
{
    float OverallProgress = GetOverallProjectProgress();
    TArray<int32> BlockedAgents = GetBlockedAgents();
    
    UE_LOG(LogTemp, Log, TEXT("Production Metrics - Overall: %.1f%%, Blocked Agents: %d"), 
        OverallProgress, BlockedAgents.Num());
}