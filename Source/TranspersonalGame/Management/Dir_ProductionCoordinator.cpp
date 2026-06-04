#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    ProductionUpdateInterval = 5.0f;
    LastProductionUpdate = FDateTime::Now();
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultAgents();
    
    // Create initial milestone for playable prototype
    TArray<FString> PrototypeAgents;
    PrototypeAgents.Add(TEXT("Engine Architect"));
    PrototypeAgents.Add(TEXT("Core Systems"));
    PrototypeAgents.Add(TEXT("World Generator"));
    PrototypeAgents.Add(TEXT("Character Artist"));
    PrototypeAgents.Add(TEXT("Animation"));
    
    CreateMilestone(TEXT("Playable Prototype"), PrototypeAgents, FDateTime::Now() + FTimespan::FromDays(2));
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized with %d agents"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    FDateTime CurrentTime = FDateTime::Now();
    if ((CurrentTime - LastProductionUpdate).GetTotalSeconds() >= ProductionUpdateInterval)
    {
        UpdateProductionMetrics();
        LastProductionUpdate = CurrentTime;
    }
}

void UDir_ProductionCoordinator::RegisterAgent(const FString& AgentName, const FString& InitialTask)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = InitialTask;
    NewTask.Status = EDir_AgentStatus::Idle;
    NewTask.ProgressPercentage = 0.0f;
    
    AgentTasks.Add(NewTask);
    
    LogProductionEvent(FString::Printf(TEXT("Agent %s registered with task: %s"), *AgentName, *InitialTask));
}

void UDir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, float Progress)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            Task.ProgressPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            
            FString StatusText;
            switch (NewStatus)
            {
                case EDir_AgentStatus::Idle: StatusText = TEXT("Idle"); break;
                case EDir_AgentStatus::Working: StatusText = TEXT("Working"); break;
                case EDir_AgentStatus::Completed: StatusText = TEXT("Completed"); break;
                case EDir_AgentStatus::Blocked: StatusText = TEXT("Blocked"); break;
                case EDir_AgentStatus::Failed: StatusText = TEXT("Failed"); break;
            }
            
            LogProductionEvent(FString::Printf(TEXT("Agent %s status: %s (%.1f%%)"), *AgentName, *StatusText, Progress));
            break;
        }
    }
}

void UDir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.TaskDescription = TaskDescription;
            Task.Status = EDir_AgentStatus::Working;
            Task.ProgressPercentage = 0.0f;
            
            LogProductionEvent(FString::Printf(TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription));
            break;
        }
    }
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetAllAgentTasks() const
{
    return AgentTasks;
}

void UDir_ProductionCoordinator::CreateMilestone(const FString& MilestoneName, const TArray<FString>& RequiredAgents, const FDateTime& Deadline)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.MilestoneName = MilestoneName;
    NewMilestone.RequiredAgents = RequiredAgents;
    NewMilestone.bIsCompleted = false;
    NewMilestone.Deadline = Deadline;
    
    ProductionMilestones.Add(NewMilestone);
    
    LogProductionEvent(FString::Printf(TEXT("Milestone created: %s (requires %d agents)"), *MilestoneName, RequiredAgents.Num()));
}

void UDir_ProductionCoordinator::CheckMilestoneCompletion(const FString& MilestoneName)
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName && !Milestone.bIsCompleted)
        {
            bool bAllAgentsCompleted = true;
            
            for (const FString& RequiredAgent : Milestone.RequiredAgents)
            {
                bool bAgentCompleted = false;
                for (const FDir_AgentTask& Task : AgentTasks)
                {
                    if (Task.AgentName == RequiredAgent && Task.Status == EDir_AgentStatus::Completed)
                    {
                        bAgentCompleted = true;
                        break;
                    }
                }
                
                if (!bAgentCompleted)
                {
                    bAllAgentsCompleted = false;
                    break;
                }
            }
            
            if (bAllAgentsCompleted)
            {
                Milestone.bIsCompleted = true;
                LogProductionEvent(FString::Printf(TEXT("MILESTONE COMPLETED: %s"), *MilestoneName));
            }
            break;
        }
    }
}

TArray<FDir_ProductionMilestone> UDir_ProductionCoordinator::GetActiveMilestones() const
{
    TArray<FDir_ProductionMilestone> ActiveMilestones;
    
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (!Milestone.bIsCompleted)
        {
            ActiveMilestones.Add(Milestone);
        }
    }
    
    return ActiveMilestones;
}

float UDir_ProductionCoordinator::GetOverallProgress() const
{
    if (AgentTasks.Num() == 0)
        return 0.0f;
    
    float TotalProgress = 0.0f;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TotalProgress += Task.ProgressPercentage;
    }
    
    return TotalProgress / AgentTasks.Num();
}

int32 UDir_ProductionCoordinator::GetActiveAgentCount() const
{
    int32 ActiveCount = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            ActiveCount++;
        }
    }
    
    return ActiveCount;
}

FString UDir_ProductionCoordinator::GenerateProductionReport() const
{
    FString Report = TEXT("=== PRODUCTION REPORT ===\n");
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), GetOverallProgress());
    Report += FString::Printf(TEXT("Active Agents: %d/%d\n"), GetActiveAgentCount(), AgentTasks.Num());
    
    Report += TEXT("\nAgent Status:\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusText;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusText = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusText = TEXT("WORKING"); break;
            case EDir_AgentStatus::Completed: StatusText = TEXT("COMPLETED"); break;
            case EDir_AgentStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Failed: StatusText = TEXT("FAILED"); break;
        }
        
        Report += FString::Printf(TEXT("- %s: %s (%.1f%%) - %s\n"), 
            *Task.AgentName, *StatusText, Task.ProgressPercentage, *Task.TaskDescription);
    }
    
    TArray<FDir_ProductionMilestone> ActiveMilestones = GetActiveMilestones();
    if (ActiveMilestones.Num() > 0)
    {
        Report += TEXT("\nActive Milestones:\n");
        for (const FDir_ProductionMilestone& Milestone : ActiveMilestones)
        {
            FTimespan TimeRemaining = Milestone.Deadline - FDateTime::Now();
            Report += FString::Printf(TEXT("- %s (%.1f days remaining)\n"), 
                *Milestone.MilestoneName, TimeRemaining.GetTotalDays());
        }
    }
    
    return Report;
}

TArray<FString> UDir_ProductionCoordinator::GetBlockedAgents() const
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(Task.AgentName);
        }
    }
    
    return BlockedAgents;
}

void UDir_ProductionCoordinator::ResolveAgentBlockage(const FString& AgentName, const FString& Resolution)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName && Task.Status == EDir_AgentStatus::Blocked)
        {
            Task.Status = EDir_AgentStatus::Working;
            Task.LastOutput = Resolution;
            
            LogProductionEvent(FString::Printf(TEXT("Blockage resolved for %s: %s"), *AgentName, *Resolution));
            break;
        }
    }
}

void UDir_ProductionCoordinator::InitializeDefaultAgents()
{
    // Register all 19 agents in the production pipeline
    RegisterAgent(TEXT("Engine Architect"), TEXT("Define UE5 architecture and core systems"));
    RegisterAgent(TEXT("Core Systems"), TEXT("Implement physics, collision, and destruction"));
    RegisterAgent(TEXT("Performance Optimizer"), TEXT("Ensure 60fps PC / 30fps console performance"));
    RegisterAgent(TEXT("World Generator"), TEXT("Create procedural terrain and biomes"));
    RegisterAgent(TEXT("Environment Artist"), TEXT("Populate world with vegetation and props"));
    RegisterAgent(TEXT("Architecture Designer"), TEXT("Build prehistoric structures and shelters"));
    RegisterAgent(TEXT("Lighting Artist"), TEXT("Implement day/night cycle and atmosphere"));
    RegisterAgent(TEXT("Character Artist"), TEXT("Create player character and NPCs"));
    RegisterAgent(TEXT("Animation Specialist"), TEXT("Add motion matching and IK systems"));
    RegisterAgent(TEXT("NPC Behavior"), TEXT("Implement AI behavior trees and routines"));
    RegisterAgent(TEXT("Combat AI"), TEXT("Create tactical dinosaur combat AI"));
    RegisterAgent(TEXT("Crowd Simulation"), TEXT("Implement mass AI for large groups"));
    RegisterAgent(TEXT("Quest Designer"), TEXT("Convert narrative into playable missions"));
    RegisterAgent(TEXT("Narrative Designer"), TEXT("Write game bible and story content"));
    RegisterAgent(TEXT("Audio Specialist"), TEXT("Create adaptive music and sound effects"));
    RegisterAgent(TEXT("VFX Artist"), TEXT("Implement Niagara particle effects"));
    RegisterAgent(TEXT("QA Tester"), TEXT("Test all systems and validate builds"));
    RegisterAgent(TEXT("Integration Specialist"), TEXT("Integrate all agent outputs"));
    RegisterAgent(TEXT("Build Engineer"), TEXT("Maintain builds and deployment"));
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    // Check milestone progress
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (!Milestone.bIsCompleted)
        {
            CheckMilestoneCompletion(Milestone.MilestoneName);
        }
    }
    
    // Update agent dependencies
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        ValidateAgentDependencies(Task.AgentName);
    }
    
    // Log production status
    if (GetActiveAgentCount() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Production Update: %.1f%% complete, %d agents active"), 
            GetOverallProgress(), GetActiveAgentCount());
    }
}

bool UDir_ProductionCoordinator::ValidateAgentDependencies(const FString& AgentName)
{
    // Implement dependency validation logic
    // For now, return true - can be expanded with specific dependency rules
    return true;
}

void UDir_ProductionCoordinator::LogProductionEvent(const FString& EventDescription)
{
    FDateTime CurrentTime = FDateTime::Now();
    FString LogMessage = FString::Printf(TEXT("[%s] %s"), 
        *CurrentTime.ToString(TEXT("%H:%M:%S")), *EventDescription);
    
    UE_LOG(LogTemp, Warning, TEXT("PRODUCTION: %s"), *LogMessage);
    
    // Could also write to file or send to external logging system
}