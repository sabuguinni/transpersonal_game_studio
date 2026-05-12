#include "StudioDirectorSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

// Studio Director Subsystem Implementation

void UStudioDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Subsystem Initialized - CYCLE 006"));
    
    // Initialize production state
    CurrentPhase = EDir_ProductionPhase::PrototypeDevelopment;
    CurrentCycleNumber = 6;
    CurrentMetrics = FDir_ProductionMetrics();
    CurrentMetrics.CycleStartTime = FDateTime::Now();
    
    // Initialize all 19 agent statuses (including Studio Director)
    InitializeAgentStatuses();
    
    // Set initial milestones
    MilestoneCompletionStatus.Add(TEXT("MinPlayableMap"), true);
    MilestoneCompletionStatus.Add(TEXT("BasicCharacterMovement"), true);
    MilestoneCompletionStatus.Add(TEXT("TerrainGeneration"), false);
    MilestoneCompletionStatus.Add(TEXT("DinosaurActors"), false);
    MilestoneCompletionStatus.Add(TEXT("SurvivalHUD"), false);
    MilestoneCompletionStatus.Add(TEXT("BasicCombat"), false);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Production pipeline ready for CYCLE 006"));
}

void UStudioDirectorSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Subsystem Shutting Down"));
    
    // Generate final production report
    LogProductionStatus();
    
    Super::Deinitialize();
}

bool UStudioDirectorSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Always create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UStudioDirectorSubsystem::StartProductionCycle(int32 CycleNumber)
{
    CurrentCycleNumber = CycleNumber;
    CurrentMetrics.CycleStartTime = FDateTime::Now();
    CurrentMetrics.ActiveAgents = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Starting Production Cycle %d"), CycleNumber);
    
    // Reset all agent statuses to Idle
    for (FDir_AgentTaskInfo& AgentInfo : AgentStatuses)
    {
        AgentInfo.Status = EDir_AgentStatus::Idle;
        AgentInfo.ProgressPercentage = 0.0f;
        AgentInfo.CurrentTask = TEXT("Awaiting Task Assignment");
        AgentInfo.LastUpdateTime = FDateTime::Now();
    }
    
    // Clear previous cycle errors
    ClearCompilationErrors();
    
    UpdateProductionMetrics();
}

void UStudioDirectorSubsystem::EndProductionCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Ending Production Cycle %d"), CurrentCycleNumber);
    
    // Mark all active agents as completed
    for (FDir_AgentTaskInfo& AgentInfo : AgentStatuses)
    {
        if (AgentInfo.Status == EDir_AgentStatus::Working)
        {
            AgentInfo.Status = EDir_AgentStatus::Completed;
            AgentInfo.ProgressPercentage = 100.0f;
        }
    }
    
    UpdateProductionMetrics();
    LogProductionStatus();
}

void UStudioDirectorSubsystem::UpdateAgentStatus(int32 AgentID, const FString& TaskDescription, EDir_AgentStatus NewStatus, float Progress)
{
    if (AgentStatuses.IsValidIndex(AgentID - 1))
    {
        FDir_AgentTaskInfo& AgentInfo = AgentStatuses[AgentID - 1];
        AgentInfo.CurrentTask = TaskDescription;
        AgentInfo.Status = NewStatus;
        AgentInfo.ProgressPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
        AgentInfo.LastUpdateTime = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s - %.1f%% - %s"), 
            AgentID, 
            *AgentInfo.AgentName, 
            *TaskDescription, 
            Progress,
            *UEnum::GetValueAsString(NewStatus));
        
        UpdateProductionMetrics();
    }
}

FDir_ProductionMetrics UStudioDirectorSubsystem::GetProductionMetrics() const
{
    return CurrentMetrics;
}

TArray<FDir_AgentTaskInfo> UStudioDirectorSubsystem::GetAllAgentStatuses() const
{
    return AgentStatuses;
}

void UStudioDirectorSubsystem::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Production Phase changed to %s"), 
        *UEnum::GetValueAsString(NewPhase));
}

EDir_ProductionPhase UStudioDirectorSubsystem::GetCurrentProductionPhase() const
{
    return CurrentPhase;
}

bool UStudioDirectorSubsystem::IsMilestoneComplete(const FString& MilestoneName) const
{
    const bool* CompletionStatus = MilestoneCompletionStatus.Find(MilestoneName);
    return CompletionStatus ? *CompletionStatus : false;
}

void UStudioDirectorSubsystem::ReportCompilationError(const FString& ErrorMessage, const FString& SourceFile)
{
    FString FormattedError = FString::Printf(TEXT("[%s] %s: %s"), 
        *FDateTime::Now().ToString(), 
        *SourceFile, 
        *ErrorMessage);
    
    CompilationErrors.Add(FormattedError);
    CurrentMetrics.CompilationErrors = CompilationErrors.Num();
    
    UE_LOG(LogTemp, Error, TEXT("Studio Director: Compilation Error - %s"), *FormattedError);
}

void UStudioDirectorSubsystem::ClearCompilationErrors()
{
    CompilationErrors.Empty();
    CurrentMetrics.CompilationErrors = 0;
    UE_LOG(LogTemp, Log, TEXT("Studio Director: Compilation errors cleared"));
}

int32 UStudioDirectorSubsystem::GetCompilationErrorCount() const
{
    return CompilationErrors.Num();
}

void UStudioDirectorSubsystem::DispatchTaskToAgent(int32 AgentID, const FString& TaskDescription, int32 Priority)
{
    if (AgentStatuses.IsValidIndex(AgentID - 1))
    {
        FDir_AgentTaskInfo& AgentInfo = AgentStatuses[AgentID - 1];
        AgentInfo.CurrentTask = TaskDescription;
        AgentInfo.Status = EDir_AgentStatus::Working;
        AgentInfo.ProgressPercentage = 0.0f;
        AgentInfo.LastUpdateTime = FDateTime::Now();
        
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Task dispatched to Agent #%d (%s): %s [Priority: %d]"), 
            AgentID, *AgentInfo.AgentName, *TaskDescription, Priority);
    }
}

bool UStudioDirectorSubsystem::IsAgentBlocked(int32 AgentID) const
{
    if (AgentStatuses.IsValidIndex(AgentID - 1))
    {
        return AgentStatuses[AgentID - 1].Status == EDir_AgentStatus::Blocked;
    }
    return false;
}

void UStudioDirectorSubsystem::UnblockAgent(int32 AgentID)
{
    if (AgentStatuses.IsValidIndex(AgentID - 1))
    {
        FDir_AgentTaskInfo& AgentInfo = AgentStatuses[AgentID - 1];
        if (AgentInfo.Status == EDir_AgentStatus::Blocked)
        {
            AgentInfo.Status = EDir_AgentStatus::Idle;
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent #%d (%s) unblocked"), AgentID, *AgentInfo.AgentName);
        }
    }
}

void UStudioDirectorSubsystem::InitializeAgentStatuses()
{
    AgentStatuses.Empty();
    
    // Initialize all 19 agents
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),
        TEXT("Engine Architect"),
        TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"),
        TEXT("Procedural World Generator"),
        TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"),
        TEXT("Lighting & Atmosphere Agent"),
        TEXT("Character Artist Agent"),
        TEXT("Animation Agent"),
        TEXT("NPC Behavior Agent"),
        TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"),
        TEXT("Quest & Mission Designer"),
        TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"),
        TEXT("VFX Agent"),
        TEXT("QA & Testing Agent"),
        TEXT("Integration & Build Agent")
    };
    
    for (int32 i = 0; i < AgentNames.Num(); ++i)
    {
        FDir_AgentTaskInfo AgentInfo;
        AgentInfo.AgentID = i + 1;
        AgentInfo.AgentName = AgentNames[i];
        AgentInfo.CurrentTask = TEXT("Initialized");
        AgentInfo.Status = EDir_AgentStatus::Idle;
        AgentInfo.ProgressPercentage = 0.0f;
        AgentInfo.LastUpdateTime = FDateTime::Now();
        
        AgentStatuses.Add(AgentInfo);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director: Initialized %d agent statuses"), AgentStatuses.Num());
}

void UStudioDirectorSubsystem::UpdateProductionMetrics()
{
    CurrentMetrics.ActiveAgents = 0;
    float TotalProgress = 0.0f;
    
    for (const FDir_AgentTaskInfo& AgentInfo : AgentStatuses)
    {
        if (AgentInfo.Status == EDir_AgentStatus::Working || AgentInfo.Status == EDir_AgentStatus::Completed)
        {
            CurrentMetrics.ActiveAgents++;
        }
        TotalProgress += AgentInfo.ProgressPercentage;
    }
    
    CurrentMetrics.OverallProgress = AgentStatuses.Num() > 0 ? TotalProgress / AgentStatuses.Num() : 0.0f;
}

void UStudioDirectorSubsystem::LogProductionStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION STATUS CYCLE %d ==="), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *UEnum::GetValueAsString(CurrentPhase));
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), CurrentMetrics.ActiveAgents);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Compilation Errors: %d"), CurrentMetrics.CompilationErrors);
    UE_LOG(LogTemp, Warning, TEXT("Cycle Duration: %s"), *FDateTime::Now().Subtract(CurrentMetrics.CycleStartTime).ToString());
    UE_LOG(LogTemp, Warning, TEXT("============================================"));
}

// Studio Director Component Implementation

UStudioDirectorComponent::UStudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second
    
    MonitoringInterval = 30.0f;
    bAutoGenerateReports = true;
    LastMonitoringTime = 0.0f;
    DirectorSubsystem = nullptr;
}

void UStudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDirectorSubsystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Component: Begin Play - Monitoring Active"));
}

void UStudioDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastMonitoringTime += DeltaTime;
    
    if (LastMonitoringTime >= MonitoringInterval)
    {
        MonitorProductionHealth();
        
        if (bAutoGenerateReports)
        {
            GenerateProductionReport();
        }
        
        LastMonitoringTime = 0.0f;
    }
}

void UStudioDirectorComponent::MonitorProductionHealth()
{
    if (!DirectorSubsystem)
    {
        InitializeDirectorSubsystem();
        return;
    }
    
    FDir_ProductionMetrics Metrics = DirectorSubsystem->GetProductionMetrics();
    
    // Check for critical issues
    if (Metrics.CompilationErrors > 5)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: CRITICAL - High compilation error count: %d"), Metrics.CompilationErrors);
    }
    
    if (Metrics.ActiveAgents == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: WARNING - No active agents detected"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director Health Check: %d active agents, %.1f%% progress, %d errors"), 
        Metrics.ActiveAgents, Metrics.OverallProgress, Metrics.CompilationErrors);
}

void UStudioDirectorComponent::GenerateProductionReport()
{
    if (!DirectorSubsystem)
    {
        return;
    }
    
    FDir_ProductionMetrics Metrics = DirectorSubsystem->GetProductionMetrics();
    TArray<FDir_AgentTaskInfo> AgentStatuses = DirectorSubsystem->GetAllAgentStatuses();
    
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), Metrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), Metrics.ActiveAgents);
    
    // Report on each agent
    for (const FDir_AgentTaskInfo& AgentInfo : AgentStatuses)
    {
        if (AgentInfo.Status != EDir_AgentStatus::Idle)
        {
            UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s [%.1f%%]"), 
                AgentInfo.AgentID, 
                *AgentInfo.AgentName, 
                *AgentInfo.CurrentTask, 
                AgentInfo.ProgressPercentage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
}

void UStudioDirectorComponent::CheckSystemIntegrity()
{
    if (!DirectorSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: CRITICAL - Director Subsystem not available"));
        return;
    }
    
    // Check for blocked agents
    TArray<FDir_AgentTaskInfo> AgentStatuses = DirectorSubsystem->GetAllAgentStatuses();
    int32 BlockedCount = 0;
    
    for (const FDir_AgentTaskInfo& AgentInfo : AgentStatuses)
    {
        if (AgentInfo.Status == EDir_AgentStatus::Blocked)
        {
            BlockedCount++;
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent #%d (%s) is BLOCKED"), 
                AgentInfo.AgentID, *AgentInfo.AgentName);
        }
    }
    
    if (BlockedCount > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: System Integrity Issue - %d agents blocked"), BlockedCount);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Studio Director: System Integrity Check PASSED"));
    }
}

void UStudioDirectorComponent::InitializeDirectorSubsystem()
{
    if (UWorld* World = GetWorld())
    {
        DirectorSubsystem = World->GetSubsystem<UStudioDirectorSubsystem>();
        if (DirectorSubsystem)
        {
            UE_LOG(LogTemp, Log, TEXT("Studio Director Component: Connected to Director Subsystem"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Studio Director Component: Failed to connect to Director Subsystem"));
        }
    }
}