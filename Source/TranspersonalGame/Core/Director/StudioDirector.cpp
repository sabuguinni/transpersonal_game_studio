#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

UStudioDirector::UStudioDirector()
{
    bEnableAutoCoordination = true;
    CoordinationInterval = 300.0f; // 5 minutes
    MaxConcurrentTasks = 20;
}

void UStudioDirector::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized - Managing game production"));
    
    // Initialize production status
    CurrentStatus = FDir_ProductionStatus();
    
    // Set up automatic coordination
    if (bEnableAutoCoordination)
    {
        GetWorld()->GetTimerManager().SetTimer(
            CoordinationTimerHandle,
            this,
            &UStudioDirector::CoordinateAgents,
            CoordinationInterval,
            true
        );
    }
    
    // Initial production assessment
    UpdateProductionMetrics();
    
    // Set up critical tasks for current cycle
    AssignTaskToAgent(TEXT("Agent_02_Engine_Architect"), 
        TEXT("Fix 122 header files without .cpp implementations"), 
        EDir_Priority::Critical, true);
        
    AssignTaskToAgent(TEXT("Agent_02_Engine_Architect"), 
        TEXT("Resolve compilation errors in UnrealBuildTool"), 
        EDir_Priority::Critical, true);
        
    AssignTaskToAgent(TEXT("Agent_05_World_Generator"), 
        TEXT("Expand MinPlayableMap landscape to 200km2"), 
        EDir_Priority::High, false);
        
    AssignTaskToAgent(TEXT("Agent_09_Character_Artist"), 
        TEXT("Create actual dinosaur static meshes with collision"), 
        EDir_Priority::High, false);
        
    AssignTaskToAgent(TEXT("Agent_12_Combat_AI"), 
        TEXT("Implement survival HUD with health/hunger/thirst/stamina bars"), 
        EDir_Priority::Medium, false);
}

void UStudioDirector::Deinitialize()
{
    if (CoordinationTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(CoordinationTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director shutting down - Final production report generated"));
    GenerateProductionReport();
    
    Super::Deinitialize();
}

void UStudioDirector::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, EDir_Priority Priority, bool bIsBlocking)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bIsBlocking = bIsBlocking;
    NewTask.EstimatedTime = (Priority == EDir_Priority::Critical) ? 120.0f : 60.0f;
    
    ActiveTasks.Add(NewTask);
    
    if (bIsBlocking)
    {
        BlockingIssues.AddUnique(FString::Printf(TEXT("%s: %s"), *AgentName, *TaskDescription));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to %s: %s [Priority: %d]"), 
        *AgentName, *TaskDescription, (int32)Priority);
}

TArray<FDir_AgentTask> UStudioDirector::GetTasksForAgent(const FString& AgentName) const
{
    TArray<FDir_AgentTask> AgentTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName)
        {
            AgentTasks.Add(Task);
        }
    }
    
    // Sort by priority
    AgentTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return (int32)A.Priority > (int32)B.Priority;
    });
    
    return AgentTasks;
}

void UStudioDirector::CompleteTask(const FString& AgentName, const FString& TaskDescription)
{
    for (int32 i = ActiveTasks.Num() - 1; i >= 0; i--)
    {
        if (ActiveTasks[i].AgentName == AgentName && ActiveTasks[i].TaskDescription == TaskDescription)
        {
            FDir_AgentTask CompletedTask = ActiveTasks[i];
            CompletedTasks.Add(CompletedTask);
            ActiveTasks.RemoveAt(i);
            
            // Remove from blocking issues if it was blocking
            if (CompletedTask.bIsBlocking)
            {
                FString BlockingIssue = FString::Printf(TEXT("%s: %s"), *AgentName, *TaskDescription);
                BlockingIssues.Remove(BlockingIssue);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Task completed by %s: %s"), *AgentName, *TaskDescription);
            break;
        }
    }
    
    UpdateProductionMetrics();
}

FDir_ProductionStatus UStudioDirector::GetProductionStatus() const
{
    return CurrentStatus;
}

void UStudioDirector::UpdateProductionMetrics()
{
    // Count actors in current level
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentStatus.TotalActorsInMap = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            CurrentStatus.TotalActorsInMap++;
        }
    }
    
    // Update blocking issues
    CurrentStatus.BlockingIssues = FString::Join(BlockingIssues, TEXT("; "));
    
    // Calculate overall progress
    int32 TotalTasks = ActiveTasks.Num() + CompletedTasks.Num();
    if (TotalTasks > 0)
    {
        CurrentStatus.OverallProgress = (float)CompletedTasks.Num() / (float)TotalTasks * 100.0f;
    }
    
    // Check if we can playtest
    CurrentStatus.bCanPlaytest = (BlockingIssues.Num() == 0) && 
                                 (CurrentStatus.CompilationErrors == 0) && 
                                 (CurrentStatus.TotalActorsInMap > 10);
    
    LogProductionMetrics();
}

bool UStudioDirector::CanAgentProceed(const FString& AgentName) const
{
    // Check if there are blocking tasks for other agents that prevent this agent from proceeding
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.bIsBlocking && Task.AgentName != AgentName)
        {
            // If there's a blocking task for another agent, this agent should wait
            if (Task.Priority == EDir_Priority::Critical)
            {
                UE_LOG(LogTemp, Warning, TEXT("Agent %s blocked by critical task: %s"), 
                    *AgentName, *Task.TaskDescription);
                return false;
            }
        }
    }
    
    return true;
}

void UStudioDirector::ReportCompilationError(const FString& ErrorMessage, const FString& FileName)
{
    FString FullError = FString::Printf(TEXT("%s: %s"), *FileName, *ErrorMessage);
    CompilationErrors.AddUnique(FullError);
    CurrentStatus.CompilationErrors = CompilationErrors.Num();
    
    UE_LOG(LogTemp, Error, TEXT("Compilation error reported: %s"), *FullError);
}

void UStudioDirector::ValidateGameplayReadiness()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Check for essential gameplay elements
    bool bHasPlayerStart = false;
    bool bHasLandscape = false;
    bool bHasGameMode = false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("PlayerStart")))
        {
            bHasPlayerStart = true;
        }
        else if (ClassName.Contains(TEXT("Landscape")))
        {
            bHasLandscape = true;
        }
    }
    
    AGameModeBase* GameMode = UGameplayStatics::GetGameMode(World);
    bHasGameMode = (GameMode != nullptr);
    
    // Update readiness status
    if (!bHasPlayerStart)
    {
        BlockingIssues.AddUnique(TEXT("Missing PlayerStart actor"));
    }
    if (!bHasLandscape)
    {
        BlockingIssues.AddUnique(TEXT("Missing Landscape actor"));
    }
    if (!bHasGameMode)
    {
        BlockingIssues.AddUnique(TEXT("Missing GameMode configuration"));
    }
    
    CurrentStatus.bCanPlaytest = bHasPlayerStart && bHasLandscape && bHasGameMode && 
                                 (CompilationErrors.Num() == 0);
}

void UStudioDirector::CoordinateAgents()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR - AGENT COORDINATION ==="));
    
    UpdateProductionMetrics();
    ValidateGameplayReadiness();
    PrioritizeTasks();
    
    // Log current status
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), ActiveTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), CompletedTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Blocking Issues: %d"), BlockingIssues.Num());
    UE_LOG(LogTemp, Warning, TEXT("Can Playtest: %s"), CurrentStatus.bCanPlaytest ? TEXT("YES") : TEXT("NO"));
    
    // Report high priority tasks
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Priority == EDir_Priority::Critical || Task.bIsBlocking)
        {
            UE_LOG(LogTemp, Error, TEXT("CRITICAL: %s - %s"), *Task.AgentName, *Task.TaskDescription);
        }
    }
}

void UStudioDirector::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR - PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors in Map: %d"), CurrentStatus.TotalActorsInMap);
    UE_LOG(LogTemp, Warning, TEXT("Compilation Errors: %d"), CurrentStatus.CompilationErrors);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentStatus.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Playtest Ready: %s"), CurrentStatus.bCanPlaytest ? TEXT("YES") : TEXT("NO"));
    
    if (BlockingIssues.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BLOCKING ISSUES:"));
        for (const FString& Issue : BlockingIssues)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Issue);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END PRODUCTION REPORT ==="));
}

void UStudioDirector::ScanForCompilationIssues()
{
    // This would typically interface with UnrealBuildTool
    // For now, we'll simulate based on known issues
    CurrentStatus.HeadersWithoutCpp = 122; // Known issue from memory
    
    if (CurrentStatus.HeadersWithoutCpp > 0)
    {
        BlockingIssues.AddUnique(FString::Printf(TEXT("Headers without .cpp: %d"), CurrentStatus.HeadersWithoutCpp));
    }
}

void UStudioDirector::AssessGameplayReadiness()
{
    ValidateGameplayReadiness();
    
    // Additional gameplay checks
    UWorld* World = GetWorld();
    if (World)
    {
        int32 DinosaurCount = 0;
        int32 EnvironmentCount = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            FString ClassName = (*ActorItr)->GetClass()->GetName();
            
            if (ClassName.Contains(TEXT("Dinosaur")) || ClassName.Contains(TEXT("TRex")) || ClassName.Contains(TEXT("Raptor")))
            {
                DinosaurCount++;
            }
            else if (ClassName.Contains(TEXT("Tree")) || ClassName.Contains(TEXT("Rock")) || ClassName.Contains(TEXT("Environment")))
            {
                EnvironmentCount++;
            }
        }
        
        if (DinosaurCount < 5)
        {
            BlockingIssues.AddUnique(TEXT("Insufficient dinosaur actors for gameplay"));
        }
        if (EnvironmentCount < 10)
        {
            BlockingIssues.AddUnique(TEXT("Insufficient environment assets"));
        }
    }
}

void UStudioDirector::PrioritizeTasks()
{
    // Sort active tasks by priority and blocking status
    ActiveTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        if (A.bIsBlocking != B.bIsBlocking)
        {
            return A.bIsBlocking; // Blocking tasks first
        }
        return (int32)A.Priority > (int32)B.Priority; // Then by priority
    });
}

void UStudioDirector::LogProductionMetrics() const
{
    UE_LOG(LogTemp, Log, TEXT("Production Metrics - Actors: %d, Progress: %.1f%%, Errors: %d"), 
        CurrentStatus.TotalActorsInMap, CurrentStatus.OverallProgress, CurrentStatus.CompilationErrors);
}