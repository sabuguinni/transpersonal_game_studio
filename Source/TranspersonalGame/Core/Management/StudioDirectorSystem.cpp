#include "StudioDirectorSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    CurrentCycleID = TEXT("");
    CycleStartTime = 0.0f;
}

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Initialized"));
    
    // Initialize agent chain and production tracking
    InitializeAgentChain();
    InitializeProductionCycle();
}

void UStudioDirectorSystem::Deinitialize()
{
    // Log final production metrics
    FDir_ProductionMetrics FinalMetrics = GetProductionMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Shutdown - Final Metrics: %d actors, %d errors"), 
           FinalMetrics.TotalActorsInLevel, FinalMetrics.CompilationErrors);
    
    Super::Deinitialize();
}

void UStudioDirectorSystem::InitializeProductionCycle()
{
    // Generate cycle ID with timestamp
    FDateTime Now = FDateTime::Now();
    CurrentCycleID = FString::Printf(TEXT("PROD_CYCLE_%s"), *Now.ToString(TEXT("%Y%m%d_%H%M")));
    CycleStartTime = FPlatformTime::Seconds();
    
    // Clear previous cycle data
    PendingTasks.Empty();
    CompletedTasks.Empty();
    
    // Update initial metrics
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle Initialized: %s"), *CurrentCycleID);
    LogProductionEvent(TEXT("CYCLE_START"), CurrentCycleID);
}

void UStudioDirectorSystem::QueueAgentTask(const FString& AgentName, const FString& TaskDescription, 
                                          EDir_AgentPriority Priority, const TArray<FString>& Deliverables)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.RequiredDeliverables = Deliverables;
    NewTask.EstimatedDuration = 300.0f; // 5 minutes default
    NewTask.bIsBlocking = (Priority == EDir_AgentPriority::Critical);
    
    // Insert based on priority
    int32 InsertIndex = PendingTasks.Num();
    for (int32 i = 0; i < PendingTasks.Num(); i++)
    {
        if (PendingTasks[i].Priority > Priority)
        {
            InsertIndex = i;
            break;
        }
    }
    
    PendingTasks.Insert(NewTask, InsertIndex);
    
    UE_LOG(LogTemp, Warning, TEXT("Queued task for %s: %s"), *AgentName, *TaskDescription);
    LogProductionEvent(TEXT("TASK_QUEUED"), FString::Printf(TEXT("%s: %s"), *AgentName, *TaskDescription));
}

FDir_ProductionMetrics UStudioDirectorSystem::GetProductionMetrics() const
{
    return CurrentMetrics;
}

void UStudioDirectorSystem::CompleteAgentTask(const FString& AgentName, bool bSuccess)
{
    // Find and move task from pending to completed
    for (int32 i = PendingTasks.Num() - 1; i >= 0; i--)
    {
        if (PendingTasks[i].AgentName == AgentName)
        {
            FDir_AgentTask CompletedTask = PendingTasks[i];
            PendingTasks.RemoveAt(i);
            CompletedTasks.Add(CompletedTask);
            
            UE_LOG(LogTemp, Warning, TEXT("Agent %s completed task: %s"), 
                   *AgentName, bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
            
            LogProductionEvent(TEXT("TASK_COMPLETED"), 
                             FString::Printf(TEXT("%s: %s"), *AgentName, bSuccess ? TEXT("SUCCESS") : TEXT("FAILED")));
            break;
        }
    }
    
    // Update metrics after task completion
    UpdateProductionMetrics();
}

bool UStudioDirectorSystem::IsMinPlayableMapReady() const
{
    if (UWorld* World = GetWorld())
    {
        // Check for essential components
        TArray<AActor*> PlayerStarts;
        UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
        
        TArray<AActor*> Landscapes;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Landscapes);
        
        // Minimum requirements for playable map
        bool bHasPlayerStart = PlayerStarts.Num() > 0;
        bool bHasLandscape = Landscapes.Num() > 0;
        bool bHasLighting = World->GetDefaultBrush() != nullptr;
        
        return bHasPlayerStart && bHasLandscape && bHasLighting;
    }
    
    return false;
}

bool UStudioDirectorSystem::ValidateProjectState() const
{
    if (UWorld* World = GetWorld())
    {
        // Check actor count is reasonable
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        bool bActorCountValid = AllActors.Num() > 0 && AllActors.Num() < 50000; // Reasonable range
        bool bWorldValid = World->GetName().Contains(TEXT("MinPlayableMap"));
        
        return bActorCountValid && bWorldValid;
    }
    
    return false;
}

FString UStudioDirectorSystem::GetNextAgent(const FString& CurrentAgent) const
{
    int32 CurrentIndex = AgentChain.Find(CurrentAgent);
    if (CurrentIndex != INDEX_NONE && CurrentIndex < AgentChain.Num() - 1)
    {
        return AgentChain[CurrentIndex + 1];
    }
    
    // Return first agent if at end or not found
    return AgentChain.Num() > 0 ? AgentChain[0] : TEXT("Engine Architect");
}

void UStudioDirectorSystem::EmergencyStopProduction(const FString& Reason)
{
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY PRODUCTION STOP: %s"), *Reason);
    
    // Clear all pending tasks
    PendingTasks.Empty();
    
    // Log emergency stop
    LogProductionEvent(TEXT("EMERGENCY_STOP"), Reason);
    
    // Update metrics to reflect emergency state
    CurrentMetrics.CompilationErrors = 999;
    CurrentMetrics.bMinPlayableMapReady = false;
}

void UStudioDirectorSystem::UpdateProductionMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Count total actors
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentMetrics.TotalActorsInLevel = AllActors.Num();
        
        // Update timing
        CurrentMetrics.CycleExecutionTime = FPlatformTime::Seconds() - CycleStartTime;
        
        // Update agent count
        CurrentMetrics.ActiveAgentCount = PendingTasks.Num();
        
        // Check map readiness
        CurrentMetrics.bMinPlayableMapReady = IsMinPlayableMapReady();
        
        // Compilation errors would be set externally
        // CurrentMetrics.CompilationErrors remains as previously set
    }
}

void UStudioDirectorSystem::InitializeAgentChain()
{
    AgentChain.Empty();
    AgentChain.Add(TEXT("Studio Director"));
    AgentChain.Add(TEXT("Engine Architect"));
    AgentChain.Add(TEXT("Core Systems Programmer"));
    AgentChain.Add(TEXT("Performance Optimizer"));
    AgentChain.Add(TEXT("Procedural World Generator"));
    AgentChain.Add(TEXT("Environment Artist"));
    AgentChain.Add(TEXT("Architecture & Interior Agent"));
    AgentChain.Add(TEXT("Lighting & Atmosphere Agent"));
    AgentChain.Add(TEXT("Character Artist Agent"));
    AgentChain.Add(TEXT("Animation Agent"));
    AgentChain.Add(TEXT("NPC Behavior Agent"));
    AgentChain.Add(TEXT("Combat & Enemy AI Agent"));
    AgentChain.Add(TEXT("Crowd & Traffic Simulation"));
    AgentChain.Add(TEXT("Quest & Mission Designer"));
    AgentChain.Add(TEXT("Narrative & Dialogue Agent"));
    AgentChain.Add(TEXT("Audio Agent"));
    AgentChain.Add(TEXT("VFX Agent"));
    AgentChain.Add(TEXT("QA & Testing Agent"));
    AgentChain.Add(TEXT("Integration & Build Agent"));
    
    UE_LOG(LogTemp, Warning, TEXT("Agent chain initialized with %d agents"), AgentChain.Num());
}

void UStudioDirectorSystem::LogProductionEvent(const FString& Event, const FString& Details)
{
    FDateTime Now = FDateTime::Now();
    FString LogEntry = FString::Printf(TEXT("[%s] %s: %s"), 
                                      *Now.ToString(TEXT("%H:%M:%S")), 
                                      *Event, 
                                      *Details);
    
    UE_LOG(LogTemp, Warning, TEXT("PRODUCTION LOG: %s"), *LogEntry);
    
    // In a full implementation, this would write to a production log file
    // For now, we use UE_LOG for immediate visibility
}