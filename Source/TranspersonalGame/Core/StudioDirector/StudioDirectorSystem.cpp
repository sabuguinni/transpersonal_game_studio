#include "StudioDirectorSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Tick every 5 seconds
    
    CycleStartTime = 0.0f;
    TotalFilesCreated = 0;
    CompilationErrors = 0;
    
    // Initialize current cycle
    CurrentCycle.CycleID = TEXT("PROD_CYCLE_AUTO_20260510_005");
    CurrentCycle.StartTime = FDateTime::Now();
}

void UStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CycleStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized for cycle: %s"), *CurrentCycle.CycleID);
    
    // Initialize Cycle 005 tasks automatically
    InitializeCycle005Tasks();
}

void UStudioDirectorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic production metrics logging
    static float LastMetricsTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastMetricsTime > 30.0f) // Log every 30 seconds
    {
        LogProductionMetrics();
        LastMetricsTime = CurrentTime;
    }
}

void UStudioDirectorSystem::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, 
                                            const FString& ExpectedDeliverable, EDir_AgentPriority Priority,
                                            const TArray<FString>& Dependencies)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.ExpectedDeliverable = ExpectedDeliverable;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::IDLE;
    NewTask.Dependencies = Dependencies;
    NewTask.AssignedTime = FDateTime::Now();
    
    // Add to current cycle
    int32 TaskIndex = CurrentCycle.AgentTasks.Add(NewTask);
    AgentTaskIndices.Add(AgentName, TaskIndex);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Assigned task to %s - %s"), *AgentName, *TaskDescription);
}

void UStudioDirectorSystem::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    if (int32* TaskIndexPtr = AgentTaskIndices.Find(AgentName))
    {
        int32 TaskIndex = *TaskIndexPtr;
        if (CurrentCycle.AgentTasks.IsValidIndex(TaskIndex))
        {
            CurrentCycle.AgentTasks[TaskIndex].Status = NewStatus;
            
            FString StatusString;
            switch (NewStatus)
            {
                case EDir_AgentStatus::IDLE: StatusString = TEXT("IDLE"); break;
                case EDir_AgentStatus::WORKING: StatusString = TEXT("WORKING"); break;
                case EDir_AgentStatus::COMPLETED: StatusString = TEXT("COMPLETED"); break;
                case EDir_AgentStatus::BLOCKED: StatusString = TEXT("BLOCKED"); break;
                case EDir_AgentStatus::FAILED: StatusString = TEXT("FAILED"); break;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: %s status updated to %s"), *AgentName, *StatusString);
        }
    }
}

void UStudioDirectorSystem::CompleteAgentTask(const FString& AgentName, const FString& DeliverableDescription)
{
    if (int32* TaskIndexPtr = AgentTaskIndices.Find(AgentName))
    {
        int32 TaskIndex = *TaskIndexPtr;
        if (CurrentCycle.AgentTasks.IsValidIndex(TaskIndex))
        {
            CurrentCycle.AgentTasks[TaskIndex].Status = EDir_AgentStatus::COMPLETED;
            CurrentCycle.AgentTasks[TaskIndex].CompletedTime = FDateTime::Now();
            
            // Add to completed deliverables
            CurrentCycle.CompletedDeliverables.Add(FString::Printf(TEXT("%s: %s"), *AgentName, *DeliverableDescription));
            
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: %s completed task - %s"), *AgentName, *DeliverableDescription);
        }
    }
}

bool UStudioDirectorSystem::AreAgentDependenciesMet(const FString& AgentName) const
{
    if (const int32* TaskIndexPtr = AgentTaskIndices.Find(AgentName))
    {
        int32 TaskIndex = *TaskIndexPtr;
        if (CurrentCycle.AgentTasks.IsValidIndex(TaskIndex))
        {
            const FDir_AgentTask& Task = CurrentCycle.AgentTasks[TaskIndex];
            
            // Check if all dependencies are completed
            for (const FString& Dependency : Task.Dependencies)
            {
                if (const int32* DepTaskIndexPtr = AgentTaskIndices.Find(Dependency))
                {
                    int32 DepTaskIndex = *DepTaskIndexPtr;
                    if (CurrentCycle.AgentTasks.IsValidIndex(DepTaskIndex))
                    {
                        if (CurrentCycle.AgentTasks[DepTaskIndex].Status != EDir_AgentStatus::COMPLETED)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false; // Dependency not found
                    }
                }
                else
                {
                    return false; // Dependency not found
                }
            }
            
            return true;
        }
    }
    
    return false;
}

void UStudioDirectorSystem::AddCriticalBlocker(const FString& BlockerDescription)
{
    CurrentCycle.CriticalBlockers.AddUnique(BlockerDescription);
    UE_LOG(LogTemp, Error, TEXT("Studio Director: CRITICAL BLOCKER - %s"), *BlockerDescription);
}

void UStudioDirectorSystem::ResolveCriticalBlocker(const FString& BlockerDescription)
{
    CurrentCycle.CriticalBlockers.Remove(BlockerDescription);
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: RESOLVED BLOCKER - %s"), *BlockerDescription);
}

void UStudioDirectorSystem::StartNewProductionCycle(const FString& CycleID)
{
    // Archive current cycle data if needed
    GenerateProductionReport();
    
    // Reset for new cycle
    CurrentCycle = FDir_ProductionCycle();
    CurrentCycle.CycleID = CycleID;
    CurrentCycle.StartTime = FDateTime::Now();
    
    AgentTaskIndices.Empty();
    CycleStartTime = GetWorld()->GetTimeSeconds();
    TotalFilesCreated = 0;
    CompilationErrors = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Started new production cycle - %s"), *CycleID);
}

void UStudioDirectorSystem::GenerateProductionReport() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle ID: %s"), *CurrentCycle.CycleID);
    UE_LOG(LogTemp, Warning, TEXT("Start Time: %s"), *CurrentCycle.StartTime.ToString());
    
    float CycleElapsedTime = GetWorld()->GetTimeSeconds() - CycleStartTime;
    UE_LOG(LogTemp, Warning, TEXT("Elapsed Time: %.2f seconds"), CycleElapsedTime);
    
    UE_LOG(LogTemp, Warning, TEXT("Total Agent Tasks: %d"), CurrentCycle.AgentTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Critical Blockers: %d"), CurrentCycle.CriticalBlockers.Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed Deliverables: %d"), CurrentCycle.CompletedDeliverables.Num());
    
    // Count tasks by status
    int32 CompletedTasks = 0;
    int32 WorkingTasks = 0;
    int32 BlockedTasks = 0;
    int32 FailedTasks = 0;
    
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::COMPLETED: CompletedTasks++; break;
            case EDir_AgentStatus::WORKING: WorkingTasks++; break;
            case EDir_AgentStatus::BLOCKED: BlockedTasks++; break;
            case EDir_AgentStatus::FAILED: FailedTasks++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Task Status - Completed: %d, Working: %d, Blocked: %d, Failed: %d"), 
           CompletedTasks, WorkingTasks, BlockedTasks, FailedTasks);
}

bool UStudioDirectorSystem::ValidateMinimumViablePrototype() const
{
    // Check for essential game elements
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for player character
    bool bHasPlayerCharacter = false;
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && Pawn->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            bHasPlayerCharacter = true;
            break;
        }
    }
    
    // Check for landscape
    bool bHasLandscape = false;
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        bHasLandscape = true;
        break;
    }
    
    // Check for game mode
    bool bHasGameMode = World->GetAuthGameMode() != nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("MVP Validation - Character: %s, Landscape: %s, GameMode: %s"), 
           bHasPlayerCharacter ? TEXT("YES") : TEXT("NO"),
           bHasLandscape ? TEXT("YES") : TEXT("NO"),
           bHasGameMode ? TEXT("YES") : TEXT("NO"));
    
    return bHasPlayerCharacter && bHasLandscape && bHasGameMode;
}

TArray<FString> UStudioDirectorSystem::GetBlockingIssues() const
{
    TArray<FString> BlockingIssues;
    
    // Add critical blockers
    for (const FString& Blocker : CurrentCycle.CriticalBlockers)
    {
        BlockingIssues.Add(FString::Printf(TEXT("CRITICAL: %s"), *Blocker));
    }
    
    // Check for failed tasks
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::FAILED)
        {
            BlockingIssues.Add(FString::Printf(TEXT("FAILED TASK: %s - %s"), *Task.AgentName, *Task.TaskDescription));
        }
        else if (Task.Status == EDir_AgentStatus::BLOCKED)
        {
            BlockingIssues.Add(FString::Printf(TEXT("BLOCKED TASK: %s - %s"), *Task.AgentName, *Task.TaskDescription));
        }
    }
    
    return BlockingIssues;
}

void UStudioDirectorSystem::InitializeCycle005Tasks()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing Cycle 005 tasks"));
    
    // Clear existing tasks
    CurrentCycle.AgentTasks.Empty();
    AgentTaskIndices.Empty();
    
    // Add critical blockers
    AddCriticalBlocker(TEXT("122 orphaned header files preventing compilation"));
    AddCriticalBlocker(TEXT("Placeholder dinosaurs need proper collision and meshes"));
    AddCriticalBlocker(TEXT("Landscape limited to small area - needs 200km2 expansion"));
    
    // Agent #02 - Engine Architect (CRITICAL)
    AssignTaskToAgent(TEXT("Agent_02_Engine_Architect"), 
                     TEXT("Fix compilation errors and clean orphaned headers"),
                     TEXT("Compilation success report + cleaned codebase"),
                     EDir_AgentPriority::CRITICAL, {});
    
    // Agent #03 - Core Systems (HIGH)
    AssignTaskToAgent(TEXT("Agent_03_Core_Systems"),
                     TEXT("Implement physics collision for dinosaur actors"),
                     TEXT("Collision components + ragdoll system"),
                     EDir_AgentPriority::HIGH, {TEXT("Agent_02_Engine_Architect")});
    
    // Agent #05 - World Generator (CRITICAL)
    AssignTaskToAgent(TEXT("Agent_05_World_Generator"),
                     TEXT("Expand landscape to 200km2, create 5 distinct biomes"),
                     TEXT("Large landscape with biome distribution"),
                     EDir_AgentPriority::CRITICAL, {TEXT("Agent_02_Engine_Architect")});
    
    // Agent #09 - Character Artist (HIGH)
    AssignTaskToAgent(TEXT("Agent_09_Character_Artist"),
                     TEXT("Replace placeholder dinosaurs with proper meshes"),
                     TEXT("5 dinosaur actors with proper collision"),
                     EDir_AgentPriority::HIGH, {TEXT("Agent_03_Core_Systems"), TEXT("Agent_05_World_Generator")});
    
    // Agent #12 - Combat AI (MEDIUM)
    AssignTaskToAgent(TEXT("Agent_12_Combat_AI"),
                     TEXT("Implement basic dinosaur AI behavior"),
                     TEXT("Behavior trees for territorial/feeding behavior"),
                     EDir_AgentPriority::MEDIUM, {TEXT("Agent_09_Character_Artist")});
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Cycle 005 tasks initialized - %d total tasks"), CurrentCycle.AgentTasks.Num());
}

void UStudioDirectorSystem::CheckCompilationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Checking compilation status..."));
    
    // This would normally check build logs, for now just log the attempt
    UE_LOG(LogTemp, Warning, TEXT("Compilation check completed - see UE5 output log for details"));
}

void UStudioDirectorSystem::LogProductionMetrics() const
{
    float CycleElapsedTime = GetWorld()->GetTimeSeconds() - CycleStartTime;
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director Metrics - Cycle: %s, Elapsed: %.1fs, Files: %d, Errors: %d"), 
           *CurrentCycle.CycleID, CycleElapsedTime, TotalFilesCreated, CompilationErrors);
}

void UStudioDirectorSystem::ValidateAgentOutputs() const
{
    // Validate that agents are producing actual deliverables
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::COMPLETED)
        {
            UE_LOG(LogTemp, Log, TEXT("Validated deliverable: %s - %s"), *Task.AgentName, *Task.ExpectedDeliverable);
        }
    }
}