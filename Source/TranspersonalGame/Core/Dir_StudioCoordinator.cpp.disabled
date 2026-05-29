#include "Dir_StudioCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/TranspersonalGameState.h"

UDir_StudioCoordinator::UDir_StudioCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize coordination state
    bIsCoordinationActive = true;
    CoordinationLevel = EDir_CoordinationLevel::Active;
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    
    // Initialize agent tracking
    ActiveAgentCount = 0;
    CompletedTaskCount = 0;
    PendingTaskCount = 0;
    
    // Initialize performance metrics
    AverageTaskCompletionTime = 0.0f;
    SystemEfficiency = 1.0f;
    ResourceUtilization = 0.5f;
    
    // Initialize quality metrics
    QualityScore = 0.0f;
    ComplianceLevel = 0.0f;
    
    // Initialize coordination settings
    MaxConcurrentAgents = 5;
    TaskTimeoutDuration = 300.0f; // 5 minutes
    QualityThreshold = 0.8f;
    EfficiencyThreshold = 0.7f;
    
    UE_LOG(LogTemp, Log, TEXT("Studio Coordinator initialized"));
}

void UDir_StudioCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCoordination();
}

void UDir_StudioCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsCoordinationActive)
    {
        UpdateCoordination(DeltaTime);
        MonitorAgentPerformance();
        ValidateSystemCompliance();
    }
}

void UDir_StudioCoordinator::InitializeCoordination()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing studio coordination system"));
    
    // Reset all coordination metrics
    ActiveAgentCount = 0;
    CompletedTaskCount = 0;
    PendingTaskCount = 0;
    
    // Initialize agent registry
    AgentRegistry.Empty();
    TaskQueue.Empty();
    CompletedTasks.Empty();
    
    // Set initial coordination level
    CoordinationLevel = EDir_CoordinationLevel::Active;
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    
    // Initialize performance tracking
    AverageTaskCompletionTime = 0.0f;
    SystemEfficiency = 1.0f;
    ResourceUtilization = 0.5f;
    
    bIsCoordinationActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Studio coordination initialized successfully"));
}

void UDir_StudioCoordinator::UpdateCoordination(float DeltaTime)
{
    // Update coordination metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Process task queue
    ProcessTaskQueue();
    
    // Update agent statuses
    UpdateAgentStatuses();
    
    // Check for coordination issues
    DetectCoordinationIssues();
}

void UDir_StudioCoordinator::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update system efficiency based on completed tasks
    if (CompletedTaskCount > 0)
    {
        float CompletionRate = static_cast<float>(CompletedTaskCount) / (CompletedTaskCount + PendingTaskCount);
        SystemEfficiency = FMath::Clamp(CompletionRate, 0.0f, 1.0f);
    }
    
    // Update resource utilization
    if (MaxConcurrentAgents > 0)
    {
        ResourceUtilization = static_cast<float>(ActiveAgentCount) / MaxConcurrentAgents;
    }
    
    // Calculate quality score based on compliance
    QualityScore = ComplianceLevel * SystemEfficiency;
}

void UDir_StudioCoordinator::ProcessTaskQueue()
{
    // Process pending tasks
    for (int32 i = TaskQueue.Num() - 1; i >= 0; i--)
    {
        FDir_TaskInfo& Task = TaskQueue[i];
        
        // Check if task has timed out
        if (Task.ElapsedTime > TaskTimeoutDuration)
        {
            UE_LOG(LogTemp, Warning, TEXT("Task %s timed out"), *Task.TaskName);
            TaskQueue.RemoveAt(i);
            continue;
        }
        
        // Update task elapsed time
        Task.ElapsedTime += GetWorld()->GetDeltaSeconds();
        
        // Check if task is completed
        if (Task.bIsCompleted)
        {
            CompletedTasks.Add(Task);
            TaskQueue.RemoveAt(i);
            CompletedTaskCount++;
            
            UE_LOG(LogTemp, Log, TEXT("Task %s completed"), *Task.TaskName);
        }
    }
    
    PendingTaskCount = TaskQueue.Num();
}

void UDir_StudioCoordinator::UpdateAgentStatuses()
{
    ActiveAgentCount = 0;
    
    // Count active agents
    for (const auto& Agent : AgentRegistry)
    {
        if (Agent.Value.bIsActive)
        {
            ActiveAgentCount++;
        }
    }
}

void UDir_StudioCoordinator::DetectCoordinationIssues()
{
    // Check system efficiency
    if (SystemEfficiency < EfficiencyThreshold)
    {
        CoordinationLevel = EDir_CoordinationLevel::Warning;
        UE_LOG(LogTemp, Warning, TEXT("System efficiency below threshold: %f"), SystemEfficiency);
    }
    
    // Check quality score
    if (QualityScore < QualityThreshold)
    {
        CoordinationLevel = EDir_CoordinationLevel::Critical;
        UE_LOG(LogTemp, Error, TEXT("Quality score below threshold: %f"), QualityScore);
    }
    
    // Check resource utilization
    if (ResourceUtilization > 0.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("High resource utilization: %f"), ResourceUtilization);
    }
}

void UDir_StudioCoordinator::MonitorAgentPerformance()
{
    // Monitor individual agent performance
    for (auto& AgentPair : AgentRegistry)
    {
        FDir_AgentInfo& Agent = AgentPair.Value;
        
        // Update agent metrics
        Agent.ElapsedTime += GetWorld()->GetDeltaSeconds();
        
        // Check agent timeout
        if (Agent.ElapsedTime > TaskTimeoutDuration && Agent.bIsActive)
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent %s exceeded timeout"), *Agent.AgentName);
            Agent.bIsActive = false;
        }
    }
}

void UDir_StudioCoordinator::ValidateSystemCompliance()
{
    // Calculate compliance level based on various factors
    float ComplianceScore = 1.0f;
    
    // Reduce compliance for timed out tasks
    if (PendingTaskCount > 0)
    {
        int32 TimedOutTasks = 0;
        for (const auto& Task : TaskQueue)
        {
            if (Task.ElapsedTime > TaskTimeoutDuration)
            {
                TimedOutTasks++;
            }
        }
        
        if (TimedOutTasks > 0)
        {
            ComplianceScore -= static_cast<float>(TimedOutTasks) / PendingTaskCount * 0.5f;
        }
    }
    
    // Reduce compliance for inactive agents
    int32 InactiveAgents = 0;
    for (const auto& Agent : AgentRegistry)
    {
        if (!Agent.Value.bIsActive)
        {
            InactiveAgents++;
        }
    }
    
    if (AgentRegistry.Num() > 0)
    {
        ComplianceScore -= static_cast<float>(InactiveAgents) / AgentRegistry.Num() * 0.3f;
    }
    
    ComplianceLevel = FMath::Clamp(ComplianceScore, 0.0f, 1.0f);
}

bool UDir_StudioCoordinator::RegisterAgent(const FString& AgentName, int32 AgentID)
{
    if (AgentRegistry.Contains(AgentID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent %s already registered"), *AgentName);
        return false;
    }
    
    FDir_AgentInfo NewAgent;
    NewAgent.AgentName = AgentName;
    NewAgent.AgentID = AgentID;
    NewAgent.bIsActive = true;
    NewAgent.ElapsedTime = 0.0f;
    NewAgent.TaskCount = 0;
    NewAgent.CompletedTasks = 0;
    
    AgentRegistry.Add(AgentID, NewAgent);
    
    UE_LOG(LogTemp, Log, TEXT("Agent %s registered successfully"), *AgentName);
    return true;
}

bool UDir_StudioCoordinator::UnregisterAgent(int32 AgentID)
{
    if (!AgentRegistry.Contains(AgentID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent ID %d not found"), AgentID);
        return false;
    }
    
    FString AgentName = AgentRegistry[AgentID].AgentName;
    AgentRegistry.Remove(AgentID);
    
    UE_LOG(LogTemp, Log, TEXT("Agent %s unregistered"), *AgentName);
    return true;
}

void UDir_StudioCoordinator::AddTask(const FString& TaskName, int32 AssignedAgentID, float Priority)
{
    FDir_TaskInfo NewTask;
    NewTask.TaskName = TaskName;
    NewTask.AssignedAgentID = AssignedAgentID;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;
    NewTask.ElapsedTime = 0.0f;
    
    TaskQueue.Add(NewTask);
    PendingTaskCount = TaskQueue.Num();
    
    UE_LOG(LogTemp, Log, TEXT("Task %s added to queue"), *TaskName);
}

void UDir_StudioCoordinator::CompleteTask(const FString& TaskName)
{
    for (auto& Task : TaskQueue)
    {
        if (Task.TaskName == TaskName)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Log, TEXT("Task %s marked as completed"), *TaskName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Task %s not found in queue"), *TaskName);
}

void UDir_StudioCoordinator::SetCoordinationLevel(EDir_CoordinationLevel NewLevel)
{
    CoordinationLevel = NewLevel;
    
    switch (NewLevel)
    {
        case EDir_CoordinationLevel::Inactive:
            bIsCoordinationActive = false;
            break;
        case EDir_CoordinationLevel::Active:
        case EDir_CoordinationLevel::Warning:
        case EDir_CoordinationLevel::Critical:
            bIsCoordinationActive = true;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Coordination level set to %d"), static_cast<int32>(NewLevel));
}

void UDir_StudioCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Log, TEXT("Production phase set to %d"), static_cast<int32>(NewPhase));
}

FDir_CoordinationStatus UDir_StudioCoordinator::GetCoordinationStatus() const
{
    FDir_CoordinationStatus Status;
    Status.bIsActive = bIsCoordinationActive;
    Status.CoordinationLevel = CoordinationLevel;
    Status.CurrentPhase = CurrentPhase;
    Status.ActiveAgentCount = ActiveAgentCount;
    Status.CompletedTaskCount = CompletedTaskCount;
    Status.PendingTaskCount = PendingTaskCount;
    Status.SystemEfficiency = SystemEfficiency;
    Status.QualityScore = QualityScore;
    Status.ResourceUtilization = ResourceUtilization;
    
    return Status;
}