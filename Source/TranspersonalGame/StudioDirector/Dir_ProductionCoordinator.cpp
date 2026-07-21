#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260604_001");
    OverallProjectProgress = 0.0f;
    bProductionSystemActive = false;
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionSystem();
    
    // Set up production monitoring timer
    FTimerHandle ProductionTimer;
    GetWorldTimerManager().SetTimer(ProductionTimer, this, &ADir_ProductionCoordinator::ValidateMilestoneProgress, 30.0f, true);
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bProductionSystemActive)
    {
        UpdateMilestoneProgress();
        CheckTaskDependencies();
    }
}

void ADir_ProductionCoordinator::InitializeProductionSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing Production Coordination System"));
    
    // Create WALK AROUND milestone
    CreateMilestone(TEXT("WALK_AROUND_PROTOTYPE"), EDir_ProductionPhase::Development, true);
    
    // Initialize core agent tasks for Milestone 1
    CreateNewTask(TEXT("Agent_02_Engine_Architect"), TEXT("Define core architecture and compilation rules"), TEXT(""));
    CreateNewTask(TEXT("Agent_03_Core_Systems"), TEXT("Implement physics, collision, and ragdoll systems"), TEXT("Agent_02"));
    CreateNewTask(TEXT("Agent_05_World_Generator"), TEXT("Create terrain with height variation using PCG"), TEXT("Agent_02"));
    CreateNewTask(TEXT("Agent_09_Character_Artist"), TEXT("Create ThirdPersonCharacter with WASD movement"), TEXT("Agent_02,Agent_03"));
    CreateNewTask(TEXT("Agent_10_Animation"), TEXT("Add character animations and camera system"), TEXT("Agent_09"));
    CreateNewTask(TEXT("Agent_12_Combat_AI"), TEXT("Place 3-5 static dinosaur meshes in world"), TEXT("Agent_05"));
    CreateNewTask(TEXT("Agent_08_Lighting"), TEXT("Configure directional light, sky, and fog"), TEXT("Agent_05"));
    
    bProductionSystemActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Production system initialized with %d tasks"), ActiveTasks.Num());
}

void ADir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, float Progress)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            Task.ProgressPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            
            UE_LOG(LogTemp, Log, TEXT("Studio Director: %s status updated to %d, progress: %.1f%%"), 
                   *AgentName, (int32)NewStatus, Progress);
            break;
        }
    }
}

void ADir_ProductionCoordinator::CreateNewTask(const FString& AgentName, const FString& Description, const FString& Dependencies)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = Description;
    NewTask.Status = EDir_AgentStatus::Idle;
    NewTask.Dependencies = Dependencies;
    NewTask.StartTime = FDateTime::Now();
    NewTask.EstimatedCompletion = FDateTime::Now() + FTimespan::FromHours(2);
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Created task for %s: %s"), *AgentName, *Description);
}

void ADir_ProductionCoordinator::ValidateMilestoneProgress()
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        float TotalProgress = 0.0f;
        int32 CompletedTasks = 0;
        
        for (const FDir_AgentTask& Task : ActiveTasks)
        {
            TotalProgress += Task.ProgressPercentage;
            if (Task.Status == EDir_AgentStatus::Complete)
            {
                CompletedTasks++;
            }
        }
        
        if (ActiveTasks.Num() > 0)
        {
            Milestone.CompletionPercentage = TotalProgress / ActiveTasks.Num();
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Milestone '%s' progress: %.1f%% (%d/%d tasks complete)"), 
               *Milestone.MilestoneName, Milestone.CompletionPercentage, CompletedTasks, ActiveTasks.Num());
    }
    
    LogProductionMetrics();
}

FString ADir_ProductionCoordinator::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT - %s ===\n"), *CurrentCycleID);
    
    Report += TEXT("\nMILESTONE STATUS:\n");
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        Report += FString::Printf(TEXT("- %s: %.1f%% complete\n"), 
                                  *Milestone.MilestoneName, Milestone.CompletionPercentage);
    }
    
    Report += TEXT("\nAGENT TASK STATUS:\n");
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        FString StatusStr;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusStr = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusStr = TEXT("WORKING"); break;
            case EDir_AgentStatus::Blocked: StatusStr = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Complete: StatusStr = TEXT("COMPLETE"); break;
            case EDir_AgentStatus::Error: StatusStr = TEXT("ERROR"); break;
        }
        
        Report += FString::Printf(TEXT("- %s: %s (%.1f%%) - %s\n"), 
                                  *Task.AgentName, *StatusStr, Task.ProgressPercentage, *Task.TaskDescription);
    }
    
    return Report;
}

bool ADir_ProductionCoordinator::CheckCriticalPathBlocked()
{
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || Task.Status == EDir_AgentStatus::Error)
        {
            UE_LOG(LogTemp, Error, TEXT("Studio Director: CRITICAL PATH BLOCKED - %s: %s"), 
                   *Task.AgentName, *Task.TaskDescription);
            return true;
        }
    }
    return false;
}

void ADir_ProductionCoordinator::DispatchTaskToAgent(int32 AgentNumber, const FString& TaskDescription)
{
    FString AgentName = FString::Printf(TEXT("Agent_%02d"), AgentNumber);
    CreateNewTask(AgentName, TaskDescription, TEXT(""));
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Dispatched task to %s: %s"), *AgentName, *TaskDescription);
}

TArray<FString> ADir_ProductionCoordinator::GetBlockedAgents()
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(Task.AgentName);
        }
    }
    
    return BlockedAgents;
}

void ADir_ProductionCoordinator::ResolveAgentConflict(const FString& Agent1, const FString& Agent2, const FString& Resolution)
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Resolving conflict between %s and %s: %s"), 
           *Agent1, *Agent2, *Resolution);
    
    // Update both agents to working status
    UpdateAgentStatus(Agent1, EDir_AgentStatus::Working, 0.0f);
    UpdateAgentStatus(Agent2, EDir_AgentStatus::Working, 0.0f);
}

void ADir_ProductionCoordinator::CreateMilestone(const FString& Name, EDir_ProductionPhase Phase, bool bCriticalPath)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.MilestoneName = Name;
    NewMilestone.Phase = Phase;
    NewMilestone.bIsCriticalPath = bCriticalPath;
    NewMilestone.CompletionPercentage = 0.0f;
    
    ProductionMilestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Created milestone '%s' in phase %d"), *Name, (int32)Phase);
}

float ADir_ProductionCoordinator::GetMilestoneProgress(const FString& MilestoneName)
{
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.CompletionPercentage;
        }
    }
    return 0.0f;
}

void ADir_ProductionCoordinator::CompleteMilestone(const FString& MilestoneName)
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.CompletionPercentage = 100.0f;
            Milestone.Phase = EDir_ProductionPhase::Complete;
            
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: MILESTONE COMPLETED - %s"), *MilestoneName);
            break;
        }
    }
}

void ADir_ProductionCoordinator::UpdateMilestoneProgress()
{
    // Calculate overall project progress
    float TotalMilestoneProgress = 0.0f;
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        TotalMilestoneProgress += Milestone.CompletionPercentage;
    }
    
    if (ProductionMilestones.Num() > 0)
    {
        OverallProjectProgress = TotalMilestoneProgress / ProductionMilestones.Num();
    }
}

void ADir_ProductionCoordinator::CheckTaskDependencies()
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle && !Task.Dependencies.IsEmpty())
        {
            TArray<FString> Dependencies;
            Task.Dependencies.ParseIntoArray(Dependencies, TEXT(","), true);
            
            bool bAllDependenciesMet = true;
            for (const FString& Dependency : Dependencies)
            {
                bool bDependencyComplete = false;
                for (const FDir_AgentTask& OtherTask : ActiveTasks)
                {
                    if (OtherTask.AgentName == Dependency.TrimStartAndEnd() && 
                        OtherTask.Status == EDir_AgentStatus::Complete)
                    {
                        bDependencyComplete = true;
                        break;
                    }
                }
                
                if (!bDependencyComplete)
                {
                    bAllDependenciesMet = false;
                    break;
                }
            }
            
            if (bAllDependenciesMet)
            {
                Task.Status = EDir_AgentStatus::Working;
                UE_LOG(LogTemp, Log, TEXT("Studio Director: %s dependencies met, task activated"), *Task.AgentName);
            }
        }
    }
}

void ADir_ProductionCoordinator::LogProductionMetrics()
{
    int32 IdleTasks = 0, WorkingTasks = 0, CompleteTasks = 0, BlockedTasks = 0, ErrorTasks = 0;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: IdleTasks++; break;
            case EDir_AgentStatus::Working: WorkingTasks++; break;
            case EDir_AgentStatus::Complete: CompleteTasks++; break;
            case EDir_AgentStatus::Blocked: BlockedTasks++; break;
            case EDir_AgentStatus::Error: ErrorTasks++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Metrics - Idle:%d Working:%d Complete:%d Blocked:%d Error:%d"), 
           IdleTasks, WorkingTasks, CompleteTasks, BlockedTasks, ErrorTasks);
}

FString ADir_ProductionCoordinator::FormatProgressReport()
{
    return GenerateProductionReport();
}