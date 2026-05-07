#include "Dir_StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/TranspersonalGameState.h"

UDir_StudioDirector::UDir_StudioDirector()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 2.0f;
    
    // Initialize director state
    bIsDirectorActive = true;
    DirectorLevel = EDir_DirectorLevel::Active;
    ProductionPhase = EDir_ProductionPhase::PreProduction;
    
    // Initialize production metrics
    TotalCycles = 0;
    SuccessfulCycles = 0;
    FailedCycles = 0;
    AverageCycleTime = 0.0f;
    
    // Initialize quality metrics
    OverallQuality = 0.0f;
    TechnicalCompliance = 0.0f;
    CreativeCompliance = 0.0f;
    
    // Initialize resource metrics
    ActiveAgents = 0;
    ResourceEfficiency = 1.0f;
    SystemLoad = 0.0f;
    
    // Initialize director settings
    MaxAgentsPerCycle = 20;
    CycleTimeLimit = 600.0f; // 10 minutes
    QualityThreshold = 0.75f;
    ComplianceThreshold = 0.8f;
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director initialized"));
}

void UDir_StudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDirector();
}

void UDir_StudioDirector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsDirectorActive)
    {
        UpdateDirector(DeltaTime);
        MonitorProduction();
        ValidateCompliance();
    }
}

void UDir_StudioDirector::InitializeDirector()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing Studio Director system"));
    
    // Reset all production metrics
    TotalCycles = 0;
    SuccessfulCycles = 0;
    FailedCycles = 0;
    AverageCycleTime = 0.0f;
    
    // Initialize agent management
    AgentChain.Empty();
    ActiveTasks.Empty();
    CompletedTasks.Empty();
    
    // Set initial director level
    DirectorLevel = EDir_DirectorLevel::Active;
    ProductionPhase = EDir_ProductionPhase::PreProduction;
    
    // Initialize quality tracking
    OverallQuality = 0.0f;
    TechnicalCompliance = 0.0f;
    CreativeCompliance = 0.0f;
    
    // Initialize resource tracking
    ActiveAgents = 0;
    ResourceEfficiency = 1.0f;
    SystemLoad = 0.0f;
    
    bIsDirectorActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director initialized successfully"));
}

void UDir_StudioDirector::UpdateDirector(float DeltaTime)
{
    // Update production metrics
    UpdateProductionMetrics(DeltaTime);
    
    // Process active tasks
    ProcessActiveTasks();
    
    // Update agent chain status
    UpdateAgentChain();
    
    // Check for production issues
    DetectProductionIssues();
}

void UDir_StudioDirector::UpdateProductionMetrics(float DeltaTime)
{
    // Update average cycle time
    if (TotalCycles > 0)
    {
        float SuccessRate = static_cast<float>(SuccessfulCycles) / TotalCycles;
        ResourceEfficiency = FMath::Clamp(SuccessRate, 0.0f, 1.0f);
    }
    
    // Update system load based on active agents
    if (MaxAgentsPerCycle > 0)
    {
        SystemLoad = static_cast<float>(ActiveAgents) / MaxAgentsPerCycle;
    }
    
    // Calculate overall quality
    OverallQuality = (TechnicalCompliance + CreativeCompliance) * 0.5f;
}

void UDir_StudioDirector::ProcessActiveTasks()
{
    // Process active tasks
    for (int32 i = ActiveTasks.Num() - 1; i >= 0; i--)
    {
        FDir_ProductionTask& Task = ActiveTasks[i];
        
        // Check if task has exceeded time limit
        if (Task.ElapsedTime > CycleTimeLimit)
        {
            UE_LOG(LogTemp, Warning, TEXT("Task %s exceeded time limit"), *Task.TaskName);
            Task.bIsFailed = true;
            FailedCycles++;
        }
        
        // Update task elapsed time
        Task.ElapsedTime += GetWorld()->GetDeltaSeconds();
        
        // Check if task is completed
        if (Task.bIsCompleted || Task.bIsFailed)
        {
            if (Task.bIsCompleted)
            {
                SuccessfulCycles++;
                UE_LOG(LogTemp, Log, TEXT("Task %s completed successfully"), *Task.TaskName);
            }
            
            CompletedTasks.Add(Task);
            ActiveTasks.RemoveAt(i);
            TotalCycles++;
        }
    }
}

void UDir_StudioDirector::UpdateAgentChain()
{
    ActiveAgents = 0;
    
    // Count active agents in chain
    for (const auto& Agent : AgentChain)
    {
        if (Agent.bIsActive)
        {
            ActiveAgents++;
        }
    }
}

void UDir_StudioDirector::DetectProductionIssues()
{
    // Check resource efficiency
    if (ResourceEfficiency < ComplianceThreshold)
    {
        DirectorLevel = EDir_DirectorLevel::Warning;
        UE_LOG(LogTemp, Warning, TEXT("Resource efficiency below threshold: %f"), ResourceEfficiency);
    }
    
    // Check overall quality
    if (OverallQuality < QualityThreshold)
    {
        DirectorLevel = EDir_DirectorLevel::Critical;
        UE_LOG(LogTemp, Error, TEXT("Overall quality below threshold: %f"), OverallQuality);
    }
    
    // Check system load
    if (SystemLoad > 0.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("High system load: %f"), SystemLoad);
    }
    
    // Check failed cycles ratio
    if (TotalCycles > 0)
    {
        float FailureRate = static_cast<float>(FailedCycles) / TotalCycles;
        if (FailureRate > 0.3f) // More than 30% failure rate
        {
            DirectorLevel = EDir_DirectorLevel::Critical;
            UE_LOG(LogTemp, Error, TEXT("High failure rate: %f"), FailureRate);
        }
    }
}

void UDir_StudioDirector::MonitorProduction()
{
    // Monitor individual agent performance
    for (auto& Agent : AgentChain)
    {
        // Update agent metrics
        Agent.ElapsedTime += GetWorld()->GetDeltaSeconds();
        
        // Check agent timeout
        if (Agent.ElapsedTime > CycleTimeLimit && Agent.bIsActive)
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent %s exceeded cycle time limit"), *Agent.AgentName);
            Agent.bIsActive = false;
        }
    }
}

void UDir_StudioDirector::ValidateCompliance()
{
    // Calculate technical compliance
    float TechScore = 1.0f;
    
    // Reduce score for failed tasks
    if (TotalCycles > 0)
    {
        float FailureRate = static_cast<float>(FailedCycles) / TotalCycles;
        TechScore -= FailureRate * 0.5f;
    }
    
    // Reduce score for inactive agents
    if (AgentChain.Num() > 0)
    {
        int32 InactiveAgents = 0;
        for (const auto& Agent : AgentChain)
        {
            if (!Agent.bIsActive)
            {
                InactiveAgents++;
            }
        }
        
        float InactiveRate = static_cast<float>(InactiveAgents) / AgentChain.Num();
        TechScore -= InactiveRate * 0.3f;
    }
    
    TechnicalCompliance = FMath::Clamp(TechScore, 0.0f, 1.0f);
    
    // Calculate creative compliance (simplified)
    CreativeCompliance = ResourceEfficiency * 0.8f + (1.0f - SystemLoad) * 0.2f;
    CreativeCompliance = FMath::Clamp(CreativeCompliance, 0.0f, 1.0f);
}

bool UDir_StudioDirector::StartProductionCycle(const FString& CycleName)
{
    if (ActiveTasks.Num() >= MaxAgentsPerCycle)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start cycle %s - maximum active tasks reached"), *CycleName);
        return false;
    }
    
    FDir_ProductionTask NewTask;
    NewTask.TaskName = CycleName;
    NewTask.StartTime = GetWorld()->GetTimeSeconds();
    NewTask.ElapsedTime = 0.0f;
    NewTask.bIsCompleted = false;
    NewTask.bIsFailed = false;
    NewTask.Priority = 1.0f;
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Production cycle %s started"), *CycleName);
    return true;
}

void UDir_StudioDirector::CompleteProductionCycle(const FString& CycleName, bool bSuccess)
{
    for (auto& Task : ActiveTasks)
    {
        if (Task.TaskName == CycleName)
        {
            Task.bIsCompleted = bSuccess;
            Task.bIsFailed = !bSuccess;
            
            UE_LOG(LogTemp, Log, TEXT("Production cycle %s completed with result: %s"), 
                   *CycleName, bSuccess ? TEXT("Success") : TEXT("Failed"));
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle %s not found"), *CycleName);
}

bool UDir_StudioDirector::AddAgentToChain(const FString& AgentName, int32 AgentID, int32 ChainPosition)
{
    // Check if agent already exists
    for (const auto& Agent : AgentChain)
    {
        if (Agent.AgentID == AgentID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent %s already in chain"), *AgentName);
            return false;
        }
    }
    
    FDir_AgentChainInfo NewAgent;
    NewAgent.AgentName = AgentName;
    NewAgent.AgentID = AgentID;
    NewAgent.ChainPosition = ChainPosition;
    NewAgent.bIsActive = true;
    NewAgent.ElapsedTime = 0.0f;
    NewAgent.TasksCompleted = 0;
    NewAgent.TasksFailed = 0;
    
    AgentChain.Add(NewAgent);
    
    // Sort by chain position
    AgentChain.Sort([](const FDir_AgentChainInfo& A, const FDir_AgentChainInfo& B) {
        return A.ChainPosition < B.ChainPosition;
    });
    
    UE_LOG(LogTemp, Log, TEXT("Agent %s added to chain at position %d"), *AgentName, ChainPosition);
    return true;
}

bool UDir_StudioDirector::RemoveAgentFromChain(int32 AgentID)
{
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        if (AgentChain[i].AgentID == AgentID)
        {
            FString AgentName = AgentChain[i].AgentName;
            AgentChain.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("Agent %s removed from chain"), *AgentName);
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent ID %d not found in chain"), AgentID);
    return false;
}

void UDir_StudioDirector::SetDirectorLevel(EDir_DirectorLevel NewLevel)
{
    DirectorLevel = NewLevel;
    
    switch (NewLevel)
    {
        case EDir_DirectorLevel::Inactive:
            bIsDirectorActive = false;
            break;
        case EDir_DirectorLevel::Active:
        case EDir_DirectorLevel::Warning:
        case EDir_DirectorLevel::Critical:
            bIsDirectorActive = true;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Director level set to %d"), static_cast<int32>(NewLevel));
}

void UDir_StudioDirector::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    ProductionPhase = NewPhase;
    UE_LOG(LogTemp, Log, TEXT("Production phase set to %d"), static_cast<int32>(NewPhase));
}

FDir_ProductionStatus UDir_StudioDirector::GetProductionStatus() const
{
    FDir_ProductionStatus Status;
    Status.bIsActive = bIsDirectorActive;
    Status.DirectorLevel = DirectorLevel;
    Status.ProductionPhase = ProductionPhase;
    Status.TotalCycles = TotalCycles;
    Status.SuccessfulCycles = SuccessfulCycles;
    Status.FailedCycles = FailedCycles;
    Status.ActiveAgents = ActiveAgents;
    Status.OverallQuality = OverallQuality;
    Status.ResourceEfficiency = ResourceEfficiency;
    Status.SystemLoad = SystemLoad;
    
    return Status;
}