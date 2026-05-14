#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AStudioDirectorSystem::AStudioDirectorSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create command center mesh component
    CommandCenterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CommandCenterMesh"));
    CommandCenterMesh->SetupAttachment(RootComponent);

    // Load a basic cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CommandCenterMesh->SetStaticMesh(CubeMeshAsset.Object);
        CommandCenterMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.5f));
    }

    // Initialize default values
    bAutoCoordination = true;
    CoordinationUpdateInterval = 5.0f;
    LastCoordinationUpdate = 0.0f;

    // Initialize production metrics
    ProductionMetrics = FDir_ProductionMetrics();
}

void AStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgents();
    
    if (bAutoCoordination)
    {
        StartCoordinationCycle();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized with %d agents"), AgentTasks.Num());
}

void AStudioDirectorSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastCoordinationUpdate += DeltaTime;
    
    if (bAutoCoordination && LastCoordinationUpdate >= CoordinationUpdateInterval)
    {
        CoordinationTick();
        LastCoordinationUpdate = 0.0f;
    }
}

void AStudioDirectorSystem::InitializeAgents()
{
    AgentTasks.Empty();
    
    // Initialize all 19 agents
    TArray<EDir_AgentType> AgentTypes = {
        EDir_AgentType::StudioDirector,
        EDir_AgentType::EngineArchitect,
        EDir_AgentType::CoreSystems,
        EDir_AgentType::Performance,
        EDir_AgentType::WorldGeneration,
        EDir_AgentType::EnvironmentArt,
        EDir_AgentType::Architecture,
        EDir_AgentType::Lighting,
        EDir_AgentType::CharacterArt,
        EDir_AgentType::Animation,
        EDir_AgentType::NPCBehavior,
        EDir_AgentType::CombatAI,
        EDir_AgentType::CrowdSimulation,
        EDir_AgentType::QuestDesign,
        EDir_AgentType::Narrative,
        EDir_AgentType::Audio,
        EDir_AgentType::VFX,
        EDir_AgentType::QA,
        EDir_AgentType::Integration
    };
    
    for (int32 i = 0; i < AgentTypes.Num(); ++i)
    {
        FDir_AgentTaskInfo NewAgent;
        NewAgent.AgentID = i + 1;
        NewAgent.AgentType = AgentTypes[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CurrentTask = TEXT("Awaiting task assignment");
        NewAgent.TaskProgress = 0.0f;
        NewAgent.LastOutput = TEXT("");
        NewAgent.CycleTime = 0.0f;
        
        AgentTasks.Add(NewAgent);
    }
    
    ProductionMetrics.ActiveAgents = AgentTasks.Num();
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d agents in Studio Director System"), AgentTasks.Num());
}

void AStudioDirectorSystem::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    for (FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.AgentID == AgentID)
        {
            Agent.Status = NewStatus;
            Agent.CurrentTask = TaskDescription;
            
            if (NewStatus == EDir_AgentStatus::Completed)
            {
                Agent.TaskProgress = 100.0f;
                ProductionMetrics.CompletedTasks++;
            }
            else if (NewStatus == EDir_AgentStatus::Working)
            {
                Agent.TaskProgress = 50.0f;
            }
            
            UE_LOG(LogTemp, Log, TEXT("Agent #%d status updated: %s - %s"), 
                   AgentID, 
                   *UEnum::GetValueAsString(NewStatus),
                   *TaskDescription);
            break;
        }
    }
}

void AStudioDirectorSystem::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription)
{
    UpdateAgentStatus(AgentID, EDir_AgentStatus::Working, TaskDescription);
}

FDir_AgentTaskInfo AStudioDirectorSystem::GetAgentInfo(int32 AgentID) const
{
    for (const FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.AgentID == AgentID)
        {
            return Agent;
        }
    }
    
    return FDir_AgentTaskInfo();
}

TArray<FDir_AgentTaskInfo> AStudioDirectorSystem::GetAllAgentInfo() const
{
    return AgentTasks;
}

void AStudioDirectorSystem::UpdateProductionMetrics()
{
    ProductionMetrics.TotalCycles++;
    
    int32 WorkingAgents = 0;
    float TotalCycleTime = 0.0f;
    
    for (const FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.Status == EDir_AgentStatus::Working || Agent.Status == EDir_AgentStatus::Completed)
        {
            WorkingAgents++;
        }
        TotalCycleTime += Agent.CycleTime;
    }
    
    ProductionMetrics.ActiveAgents = WorkingAgents;
    
    if (AgentTasks.Num() > 0)
    {
        ProductionMetrics.AverageCycleTime = TotalCycleTime / AgentTasks.Num();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Production metrics updated: Cycle %d, Active agents: %d, Avg time: %.2fs"), 
           ProductionMetrics.TotalCycles, 
           ProductionMetrics.ActiveAgents, 
           ProductionMetrics.AverageCycleTime);
}

FDir_ProductionMetrics AStudioDirectorSystem::GetProductionMetrics() const
{
    return ProductionMetrics;
}

void AStudioDirectorSystem::StartCoordinationCycle()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            CoordinationTimerHandle,
            this,
            &AStudioDirectorSystem::CoordinationTick,
            CoordinationUpdateInterval,
            true
        );
        
        bAutoCoordination = true;
        UE_LOG(LogTemp, Warning, TEXT("Studio Director coordination cycle started"));
    }
}

void AStudioDirectorSystem::StopCoordinationCycle()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(CoordinationTimerHandle);
    }
    
    bAutoCoordination = false;
    UE_LOG(LogTemp, Warning, TEXT("Studio Director coordination cycle stopped"));
}

bool AStudioDirectorSystem::IsCoordinationActive() const
{
    return bAutoCoordination;
}

void AStudioDirectorSystem::LogAgentStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR AGENT STATUS REPORT ==="));
    
    for (const FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s - %s (%.1f%%)"), 
               Agent.AgentID,
               *UEnum::GetValueAsString(Agent.AgentType),
               *UEnum::GetValueAsString(Agent.Status),
               *Agent.CurrentTask,
               Agent.TaskProgress);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END AGENT STATUS REPORT ==="));
}

void AStudioDirectorSystem::ResetAllAgents()
{
    for (FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        Agent.Status = EDir_AgentStatus::Idle;
        Agent.CurrentTask = TEXT("Reset - awaiting new assignment");
        Agent.TaskProgress = 0.0f;
        Agent.CycleTime = 0.0f;
    }
    
    ProductionMetrics = FDir_ProductionMetrics();
    ProductionMetrics.ActiveAgents = AgentTasks.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("All agents reset to idle state"));
}

void AStudioDirectorSystem::CoordinationTick()
{
    ValidateAgentChain();
    UpdateProductionMetrics();
    
    // Check for blocked agents
    for (const FDir_AgentTaskInfo& Agent : AgentTasks)
    {
        if (Agent.Status == EDir_AgentStatus::Blocked)
        {
            HandleAgentBlocking(Agent.AgentID);
        }
    }
}

void AStudioDirectorSystem::ValidateAgentChain()
{
    // Ensure agents follow the proper chain sequence
    bool bChainValid = true;
    
    for (int32 i = 1; i < AgentTasks.Num(); ++i)
    {
        const FDir_AgentTaskInfo& CurrentAgent = AgentTasks[i];
        const FDir_AgentTaskInfo& PreviousAgent = AgentTasks[i - 1];
        
        // If current agent is working but previous is not completed, there's a chain issue
        if (CurrentAgent.Status == EDir_AgentStatus::Working && 
            PreviousAgent.Status != EDir_AgentStatus::Completed)
        {
            bChainValid = false;
            UE_LOG(LogTemp, Warning, TEXT("Chain validation failed: Agent #%d working before Agent #%d completed"), 
                   CurrentAgent.AgentID, PreviousAgent.AgentID);
        }
    }
    
    if (bChainValid)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Agent chain validation passed"));
    }
}

void AStudioDirectorSystem::HandleAgentBlocking(int32 BlockedAgentID)
{
    UE_LOG(LogTemp, Error, TEXT("Handling blocking for Agent #%d"), BlockedAgentID);
    
    // Reset the blocked agent and notify
    UpdateAgentStatus(BlockedAgentID, EDir_AgentStatus::Idle, TEXT("Recovered from blocking state"));
    
    // Could implement more sophisticated recovery logic here
}