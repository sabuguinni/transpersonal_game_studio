#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bAutoUpdateMetrics = true;
    UpdateInterval = 5.0f;
    LastUpdateTime = 0.0f;
    
    // Set default root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentRegistry();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized with %d agents"), AgentRegistry.Num());
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoUpdateMetrics)
    {
        LastUpdateTime += DeltaTime;
        if (LastUpdateTime >= UpdateInterval)
        {
            UpdateProductionMetrics();
            LastUpdateTime = 0.0f;
        }
    }
}

void AProductionCoordinator::InitializeAgentRegistry()
{
    AgentRegistry.Empty();
    
    // Initialize all 19 production agents
    TArray<FString> AgentNames = {
        TEXT("Engine_Architect"),
        TEXT("Core_Systems"),
        TEXT("Performance_Optimizer"),
        TEXT("World_Generator"),
        TEXT("Environment_Artist"),
        TEXT("Architecture_Agent"),
        TEXT("Lighting_Atmosphere"),
        TEXT("Character_Artist"),
        TEXT("Animation_Agent"),
        TEXT("NPC_Behavior"),
        TEXT("Combat_AI"),
        TEXT("Crowd_Simulation"),
        TEXT("Quest_Designer"),
        TEXT("Narrative_Agent"),
        TEXT("Audio_Agent"),
        TEXT("VFX_Agent"),
        TEXT("QA_Testing"),
        TEXT("Integration_Agent"),
        TEXT("Build_Agent")
    };
    
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentMetrics NewAgent;
        NewAgent.AgentName = AgentName;
        NewAgent.Status = EDir_AgentStatus::Ready;
        NewAgent.CurrentTask = TEXT("Awaiting assignment");
        AgentRegistry.Add(NewAgent);
    }
    
    // Set initial status for active agents
    UpdateAgentStatus(TEXT("Engine_Architect"), EDir_AgentStatus::Active, TEXT("Core systems architecture"));
    UpdateAgentStatus(TEXT("Core_Systems"), EDir_AgentStatus::Active, TEXT("Physics and collision"));
    UpdateAgentStatus(TEXT("World_Generator"), EDir_AgentStatus::Active, TEXT("Terrain generation"));
    UpdateAgentStatus(TEXT("Environment_Artist"), EDir_AgentStatus::Active, TEXT("Vegetation placement"));
    UpdateAgentStatus(TEXT("Character_Artist"), EDir_AgentStatus::Active, TEXT("Player character"));
    UpdateAgentStatus(TEXT("Lighting_Atmosphere"), EDir_AgentStatus::Active, TEXT("Day/night cycle"));
    UpdateAgentStatus(TEXT("Narrative_Agent"), EDir_AgentStatus::Active, TEXT("World lore"));
    UpdateAgentStatus(TEXT("Integration_Agent"), EDir_AgentStatus::Active, TEXT("System coordination"));
}

void AProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, const FString& CurrentTask)
{
    FDir_AgentMetrics* Agent = FindAgentMetrics(AgentName);
    if (Agent)
    {
        Agent->Status = NewStatus;
        Agent->CurrentTask = CurrentTask;
        Agent->LastUpdate = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Agent %s status updated to %s: %s"), 
               *AgentName, 
               *UEnum::GetValueAsString(NewStatus),
               *CurrentTask);
    }
}

void AProductionCoordinator::RecordTaskCompletion(const FString& AgentName, bool bSuccess)
{
    FDir_AgentMetrics* Agent = FindAgentMetrics(AgentName);
    if (Agent)
    {
        if (bSuccess)
        {
            Agent->TasksCompleted++;
        }
        else
        {
            Agent->TasksFailed++;
        }
        
        // Calculate success rate
        int32 TotalTasks = Agent->TasksCompleted + Agent->TasksFailed;
        if (TotalTasks > 0)
        {
            Agent->SuccessRate = (float)Agent->TasksCompleted / TotalTasks * 100.0f;
        }
        
        Agent->LastUpdate = FDateTime::Now();
    }
}

FDir_AgentMetrics AProductionCoordinator::GetAgentMetrics(const FString& AgentName)
{
    FDir_AgentMetrics* Agent = FindAgentMetrics(AgentName);
    if (Agent)
    {
        return *Agent;
    }
    
    return FDir_AgentMetrics(); // Return default metrics if not found
}

void AProductionCoordinator::UpdateProductionMetrics()
{
    CountLevelActors();
    
    // Calculate overall progress based on active systems
    int32 ActiveAgents = 0;
    int32 CompletedAgents = 0;
    
    for (const FDir_AgentMetrics& Agent : AgentRegistry)
    {
        if (Agent.Status == EDir_AgentStatus::Active || Agent.Status == EDir_AgentStatus::Monitoring)
        {
            ActiveAgents++;
            if (Agent.TasksCompleted > 0)
            {
                CompletedAgents++;
            }
        }
    }
    
    if (ActiveAgents > 0)
    {
        ProductionStatus.PrototypeProgress = (float)CompletedAgents / ActiveAgents * 100.0f;
    }
    
    // Update critical systems status
    ProductionStatus.CriticalSystemsOnline.Empty();
    ProductionStatus.PendingSystems.Empty();
    
    if (ProductionStatus.TotalActors > 0)
    {
        ProductionStatus.CriticalSystemsOnline.Add(TEXT("Movement"));
        ProductionStatus.CriticalSystemsOnline.Add(TEXT("Terrain"));
        ProductionStatus.CriticalSystemsOnline.Add(TEXT("Basic_Survival"));
    }
    
    if (ProductionStatus.DinosaurActors == 0)
    {
        ProductionStatus.PendingSystems.Add(TEXT("Dinosaur_AI"));
    }
    
    ProductionStatus.PendingSystems.Add(TEXT("Combat"));
    ProductionStatus.PendingSystems.Add(TEXT("Advanced_Survival"));
}

float AProductionCoordinator::GetOverallProgress()
{
    return ProductionStatus.PrototypeProgress;
}

TArray<FString> AProductionCoordinator::GetActiveAgents()
{
    TArray<FString> ActiveAgents;
    
    for (const FDir_AgentMetrics& Agent : AgentRegistry)
    {
        if (Agent.Status == EDir_AgentStatus::Active)
        {
            ActiveAgents.Add(Agent.AgentName);
        }
    }
    
    return ActiveAgents;
}

TArray<FString> AProductionCoordinator::GetPendingTasks()
{
    TArray<FString> PendingTasks;
    
    for (const FDir_AgentMetrics& Agent : AgentRegistry)
    {
        if (Agent.Status == EDir_AgentStatus::Pending)
        {
            PendingTasks.Add(FString::Printf(TEXT("%s: %s"), *Agent.AgentName, *Agent.CurrentTask));
        }
    }
    
    return PendingTasks;
}

void AProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *UEnum::GetValueAsString(ProductionStatus.CurrentPhase));
    UE_LOG(LogTemp, Warning, TEXT("Progress: %.1f%%"), ProductionStatus.PrototypeProgress);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), ProductionStatus.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), ProductionStatus.DinosaurActors);
    UE_LOG(LogTemp, Warning, TEXT("Environment Actors: %d"), ProductionStatus.EnvironmentActors);
    UE_LOG(LogTemp, Warning, TEXT("Build Status: %s"), *ProductionStatus.BuildStatus);
    
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT STATUS ==="));
    for (const FDir_AgentMetrics& Agent : AgentRegistry)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s (%.1f%% success)"), 
               *Agent.AgentName,
               *UEnum::GetValueAsString(Agent.Status),
               Agent.SuccessRate);
    }
}

void AProductionCoordinator::DebugPrintAllMetrics()
{
    LogProductionStatus();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
            FString::Printf(TEXT("Production Progress: %.1f%% | Actors: %d | Dinosaurs: %d"),
                ProductionStatus.PrototypeProgress,
                ProductionStatus.TotalActors,
                ProductionStatus.DinosaurActors));
    }
}

void AProductionCoordinator::CountLevelActors()
{
    if (UWorld* World = GetWorld())
    {
        ProductionStatus.TotalActors = 0;
        ProductionStatus.DinosaurActors = 0;
        ProductionStatus.EnvironmentActors = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && IsValid(Actor))
            {
                ProductionStatus.TotalActors++;
                
                FString ActorName = Actor->GetActorLabel().ToLower();
                
                // Count dinosaur actors
                TArray<FString> DinoKeywords = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), 
                                               TEXT("brachi"), TEXT("ankylo"), TEXT("parasauro")};
                for (const FString& Keyword : DinoKeywords)
                {
                    if (ActorName.Contains(Keyword))
                    {
                        ProductionStatus.DinosaurActors++;
                        break;
                    }
                }
                
                // Count environment actors
                TArray<FString> EnvKeywords = {TEXT("tree"), TEXT("rock"), TEXT("bush"), 
                                              TEXT("grass"), TEXT("plant"), TEXT("stone")};
                for (const FString& Keyword : EnvKeywords)
                {
                    if (ActorName.Contains(Keyword))
                    {
                        ProductionStatus.EnvironmentActors++;
                        break;
                    }
                }
            }
        }
    }
}

FDir_AgentMetrics* AProductionCoordinator::FindAgentMetrics(const FString& AgentName)
{
    for (FDir_AgentMetrics& Agent : AgentRegistry)
    {
        if (Agent.AgentName.Equals(AgentName, ESearchCase::IgnoreCase))
        {
            return &Agent;
        }
    }
    return nullptr;
}