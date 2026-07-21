#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

AStudioDirectorSystem::AStudioDirectorSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    CurrentCycleID = 0;
    MetricsUpdateInterval = 5.0f;
    LastMetricsUpdate = 0.0f;
    bAgentChainActive = false;
    CurrentActiveAgent = 1;
    AgentTimeoutThreshold = 300.0f;
    LastAgentStartTime = 0.0f;
    bEmergencyMode = false;

    // Initialize production metrics
    CurrentMetrics = FDir_ProductionMetrics();
}

void AStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized - Ready for agent coordination"));
    
    // Start initial metrics update
    UpdateProductionMetrics();
    
    // Log startup status
    LogCurrentStatus();
}

void AStudioDirectorSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update metrics periodically
    if (GetWorld()->GetTimeSeconds() - LastMetricsUpdate > MetricsUpdateInterval)
    {
        UpdateProductionMetrics();
        LastMetricsUpdate = GetWorld()->GetTimeSeconds();
    }
    
    // Check agent timeouts if chain is active
    if (bAgentChainActive)
    {
        UpdateAgentTimeouts();
    }
    
    // Validate production limits
    CheckProductionLimits();
}

void AStudioDirectorSystem::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const FString& Priority, float EstimatedHours)
{
    FDir_AgentTaskData NewTask;
    NewTask.AgentID = AgentID;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.EstimatedHours = EstimatedHours;
    NewTask.bCompleted = false;
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%d: %s"), AgentID, *TaskDescription);
}

void AStudioDirectorSystem::CompleteAgentTask(int32 AgentID, const FString& TaskDescription)
{
    for (int32 i = ActiveTasks.Num() - 1; i >= 0; i--)
    {
        if (ActiveTasks[i].AgentID == AgentID && ActiveTasks[i].TaskDescription == TaskDescription)
        {
            ActiveTasks[i].bCompleted = true;
            CompletedTasks.Add(ActiveTasks[i]);
            ActiveTasks.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Task completed by Agent #%d: %s"), AgentID, *TaskDescription);
            break;
        }
    }
}

TArray<FDir_AgentTaskData> AStudioDirectorSystem::GetTasksForAgent(int32 AgentID)
{
    TArray<FDir_AgentTaskData> AgentTasks;
    
    for (const FDir_AgentTaskData& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

int32 AStudioDirectorSystem::GetTotalActiveTasks()
{
    return ActiveTasks.Num();
}

void AStudioDirectorSystem::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.TotalActorsInMap = AllActors.Num();
    
    // Count dinosaur actors
    int32 DinoCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")) || 
            Actor->GetName().Contains(TEXT("TRex")) ||
            Actor->GetName().Contains(TEXT("Veloci")) ||
            Actor->GetName().Contains(TEXT("Tricera")) ||
            Actor->GetName().Contains(TEXT("Brachi")))
        {
            DinoCount++;
        }
    }
    CurrentMetrics.DinosaurCount = DinoCount;
    
    // Count environment props
    int32 PropsCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && (Actor->GetName().Contains(TEXT("Tree")) || 
                     Actor->GetName().Contains(TEXT("Rock")) ||
                     Actor->GetName().Contains(TEXT("Bush")) ||
                     Actor->GetName().Contains(TEXT("Grass"))))
        {
            PropsCount++;
        }
    }
    CurrentMetrics.EnvironmentPropsCount = PropsCount;
    
    // Get frame rate (simplified)
    CurrentMetrics.FrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
    
    // Memory usage (placeholder - would need platform-specific implementation)
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.TotalActorsInMap * 0.1f; // Rough estimate
    
    UE_LOG(LogTemp, Log, TEXT("Production Metrics Updated - Actors: %d, Dinos: %d, Props: %d, FPS: %.1f"), 
           CurrentMetrics.TotalActorsInMap, CurrentMetrics.DinosaurCount, 
           CurrentMetrics.EnvironmentPropsCount, CurrentMetrics.FrameRate);
}

FDir_ProductionMetrics AStudioDirectorSystem::GetCurrentMetrics()
{
    return CurrentMetrics;
}

bool AStudioDirectorSystem::IsProductionOnTrack()
{
    // Check production limits from brain memories
    bool bActorLimitOK = CurrentMetrics.TotalActorsInMap <= 8000;
    bool bDinoLimitOK = CurrentMetrics.DinosaurCount <= 150;
    bool bFrameRateOK = CurrentMetrics.FrameRate >= 30.0f;
    
    return bActorLimitOK && bDinoLimitOK && bFrameRateOK;
}

void AStudioDirectorSystem::StartAgentChain()
{
    bAgentChainActive = true;
    CurrentActiveAgent = 2; // Start with Engine Architect
    LastAgentStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Chain Started - Current Agent: #%d"), CurrentActiveAgent);
    
    // Assign initial task to Engine Architect
    AssignTaskToAgent(2, "Define technical architecture and establish coding standards", "Critical", 2.0f);
}

void AStudioDirectorSystem::AdvanceToNextAgent()
{
    if (!bAgentChainActive)
    {
        return;
    }
    
    CurrentActiveAgent++;
    LastAgentStartTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentActiveAgent > 19)
    {
        // Chain complete
        bAgentChainActive = false;
        CurrentActiveAgent = 1;
        UE_LOG(LogTemp, Warning, TEXT("Agent Chain Complete - All agents have executed"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Advanced to Agent #%d"), CurrentActiveAgent);
    }
}

void AStudioDirectorSystem::ReportAgentCompletion(int32 AgentID, bool bSuccess)
{
    if (AgentID == CurrentActiveAgent)
    {
        if (bSuccess)
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d completed successfully"), AgentID);
            AdvanceToNextAgent();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Agent #%d failed - Emergency intervention required"), AgentID);
            bEmergencyMode = true;
        }
    }
}

bool AStudioDirectorSystem::IsAgentChainComplete()
{
    return !bAgentChainActive && CurrentActiveAgent == 1;
}

void AStudioDirectorSystem::EmergencyStopAllAgents()
{
    bAgentChainActive = false;
    bEmergencyMode = true;
    CurrentActiveAgent = 1;
    
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY STOP - All agent operations halted"));
}

void AStudioDirectorSystem::RestartAgentChain()
{
    bEmergencyMode = false;
    ActiveTasks.Empty();
    StartAgentChain();
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Chain Restarted after emergency"));
}

void AStudioDirectorSystem::ForceAgentTimeout(int32 AgentID)
{
    UE_LOG(LogTemp, Error, TEXT("Agent #%d timed out - Forcing advancement"), AgentID);
    AdvanceToNextAgent();
}

void AStudioDirectorSystem::LogCurrentStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle ID: %d"), CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), ActiveTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), CompletedTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Agent Chain Active: %s"), bAgentChainActive ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Current Active Agent: #%d"), CurrentActiveAgent);
    UE_LOG(LogTemp, Warning, TEXT("Emergency Mode: %s"), bEmergencyMode ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsInMap);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Count: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Production On Track: %s"), IsProductionOnTrack() ? TEXT("Yes") : TEXT("No"));
}

void AStudioDirectorSystem::ValidateAgentChainIntegrity()
{
    // Check for stuck agents
    if (bAgentChainActive)
    {
        float TimeSinceLastAgent = GetWorld()->GetTimeSeconds() - LastAgentStartTime;
        if (TimeSinceLastAgent > AgentTimeoutThreshold)
        {
            UE_LOG(LogTemp, Error, TEXT("Agent #%d exceeded timeout threshold"), CurrentActiveAgent);
            ForceAgentTimeout(CurrentActiveAgent);
        }
    }
    
    // Validate production limits
    if (!IsProductionOnTrack())
    {
        UE_LOG(LogTemp, Warning, TEXT("Production metrics indicate potential issues"));
    }
}

void AStudioDirectorSystem::UpdateAgentTimeouts()
{
    if (!bAgentChainActive)
    {
        return;
    }
    
    float TimeSinceLastAgent = GetWorld()->GetTimeSeconds() - LastAgentStartTime;
    
    if (TimeSinceLastAgent > AgentTimeoutThreshold)
    {
        UE_LOG(LogTemp, Error, TEXT("Agent #%d timeout detected - Auto-advancing"), CurrentActiveAgent);
        ForceAgentTimeout(CurrentActiveAgent);
    }
}

void AStudioDirectorSystem::CheckProductionLimits()
{
    // Check actor limits from brain memories
    if (CurrentMetrics.TotalActorsInMap > 8000)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Actor count (%d) exceeds limit (8000)"), CurrentMetrics.TotalActorsInMap);
        bEmergencyMode = true;
    }
    
    if (CurrentMetrics.DinosaurCount > 150)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Dinosaur count (%d) exceeds limit (150)"), CurrentMetrics.DinosaurCount);
        bEmergencyMode = true;
    }
}

void AStudioDirectorSystem::ValidateMapState()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Verify MinPlayableMap is loaded
    FString CurrentLevelName = GetWorld()->GetName();
    if (!CurrentLevelName.Contains(TEXT("MinPlayableMap")))
    {
        UE_LOG(LogTemp, Warning, TEXT("Warning: Not in MinPlayableMap - Current level: %s"), *CurrentLevelName);
    }
}