#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Initialized"));
    
    InitializeAgents();
    bMilestone1Completed = false;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260519_001");
    
    // Update initial metrics
    UpdateProductionMetrics();
}

void UStudioDirectorSystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Shutdown"));
    Super::Deinitialize();
}

void UStudioDirectorSystem::InitializeAgents()
{
    Agents.Empty();
    
    // Initialize all 19 agents with their types and default status
    TArray<EDir_AgentType> AgentTypes = {
        EDir_AgentType::StudioDirector,
        EDir_AgentType::EngineArchitect,
        EDir_AgentType::CoreSystems,
        EDir_AgentType::Performance,
        EDir_AgentType::WorldGenerator,
        EDir_AgentType::EnvironmentArtist,
        EDir_AgentType::Architecture,
        EDir_AgentType::Lighting,
        EDir_AgentType::CharacterArtist,
        EDir_AgentType::Animation,
        EDir_AgentType::NPCBehavior,
        EDir_AgentType::CombatAI,
        EDir_AgentType::CrowdSimulation,
        EDir_AgentType::QuestDesigner,
        EDir_AgentType::Narrative,
        EDir_AgentType::Audio,
        EDir_AgentType::VFX,
        EDir_AgentType::QA,
        EDir_AgentType::Integration
    };
    
    for (int32 i = 0; i < AgentTypes.Num(); i++)
    {
        FDir_AgentInfo NewAgent;
        NewAgent.AgentID = i + 1;
        NewAgent.AgentType = AgentTypes[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CurrentTask = TEXT("Awaiting task assignment");
        NewAgent.ProgressPercentage = 0.0f;
        NewAgent.LastUpdate = FDateTime::Now();
        
        Agents.Add(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agents in the production pipeline"), Agents.Num());
}

void UStudioDirectorSystem::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    if (AgentID < 1 || AgentID > Agents.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid AgentID: %d"), AgentID);
        return;
    }
    
    FDir_AgentInfo& Agent = Agents[AgentID - 1];
    Agent.Status = NewStatus;
    Agent.CurrentTask = TaskDescription;
    Agent.LastUpdate = FDateTime::Now();
    
    // Update progress based on status
    switch (NewStatus)
    {
        case EDir_AgentStatus::Working:
            Agent.ProgressPercentage = 50.0f;
            break;
        case EDir_AgentStatus::Completed:
            Agent.ProgressPercentage = 100.0f;
            break;
        case EDir_AgentStatus::Error:
        case EDir_AgentStatus::Blocked:
            Agent.ProgressPercentage = 0.0f;
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent #%d status updated: %s - %s"), 
           AgentID, 
           *UEnum::GetValueAsString(NewStatus),
           *TaskDescription);
}

FDir_AgentInfo UStudioDirectorSystem::GetAgentInfo(int32 AgentID) const
{
    if (AgentID < 1 || AgentID > Agents.Num())
    {
        return FDir_AgentInfo(); // Return default
    }
    
    return Agents[AgentID - 1];
}

TArray<FDir_AgentInfo> UStudioDirectorSystem::GetAllAgents() const
{
    return Agents;
}

void UStudioDirectorSystem::UpdateProductionMetrics()
{
    CurrentMetrics = FDir_ProductionMetrics();
    
    // Get current world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("No world found for metrics update"));
        return;
    }
    
    // Count all actors
    CurrentMetrics.TotalActorsInLevel = 0;
    CurrentMetrics.DinosaurActors = 0;
    CurrentMetrics.EnvironmentActors = 0;
    CurrentMetrics.CharacterActors = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
            continue;
            
        CurrentMetrics.TotalActorsInLevel++;
        
        FString ActorName = Actor->GetName();
        FString ActorLabel = Actor->GetActorLabel();
        
        // Count dinosaurs
        if (ActorLabel.Contains(TEXT("Rex")) || 
            ActorLabel.Contains(TEXT("Raptor")) || 
            ActorLabel.Contains(TEXT("Triceratops")) ||
            ActorLabel.Contains(TEXT("Brachiosaurus")) ||
            ActorLabel.Contains(TEXT("Dinosaur")))
        {
            CurrentMetrics.DinosaurActors++;
        }
        // Count characters
        else if (Actor->IsA<ACharacter>())
        {
            CurrentMetrics.CharacterActors++;
        }
        // Count environment (static meshes, landscapes, etc.)
        else if (Actor->FindComponentByClass<UStaticMeshComponent>() ||
                 ActorName.Contains(TEXT("Landscape")) ||
                 ActorLabel.Contains(TEXT("Tree")) ||
                 ActorLabel.Contains(TEXT("Rock")))
        {
            CurrentMetrics.EnvironmentActors++;
        }
    }
    
    // Calculate completion percentage based on Milestone 1 requirements
    float CompletionScore = 0.0f;
    
    // Character system (20%)
    if (CurrentMetrics.CharacterActors > 0)
        CompletionScore += 20.0f;
    
    // Dinosaur population (30%)
    if (CurrentMetrics.DinosaurActors >= 5)
        CompletionScore += 30.0f;
    else
        CompletionScore += (CurrentMetrics.DinosaurActors / 5.0f) * 30.0f;
    
    // Environment (30%)
    if (CurrentMetrics.EnvironmentActors >= 20)
        CompletionScore += 30.0f;
    else
        CompletionScore += (CurrentMetrics.EnvironmentActors / 20.0f) * 30.0f;
    
    // Total actors (20%)
    if (CurrentMetrics.TotalActorsInLevel >= 50)
        CompletionScore += 20.0f;
    else
        CompletionScore += (CurrentMetrics.TotalActorsInLevel / 50.0f) * 20.0f;
    
    CurrentMetrics.LevelCompletionPercentage = FMath::Clamp(CompletionScore, 0.0f, 100.0f);
    
    if (CurrentMetrics.LevelCompletionPercentage >= 80.0f)
    {
        CurrentMetrics.CurrentBuildStatus = TEXT("Milestone 1 Ready");
        bMilestone1Completed = true;
    }
    else if (CurrentMetrics.LevelCompletionPercentage >= 50.0f)
    {
        CurrentMetrics.CurrentBuildStatus = TEXT("Good Progress");
    }
    else
    {
        CurrentMetrics.CurrentBuildStatus = TEXT("In Development");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production Metrics Updated: %d total actors, %d dinosaurs, %d environment, %.1f%% complete"),
           CurrentMetrics.TotalActorsInLevel,
           CurrentMetrics.DinosaurActors,
           CurrentMetrics.EnvironmentActors,
           CurrentMetrics.LevelCompletionPercentage);
}

FDir_ProductionMetrics UStudioDirectorSystem::GetProductionMetrics() const
{
    return CurrentMetrics;
}

bool UStudioDirectorSystem::IsMilestone1Complete() const
{
    return bMilestone1Completed;
}

float UStudioDirectorSystem::GetMilestone1Progress() const
{
    return CurrentMetrics.LevelCompletionPercentage;
}

void UStudioDirectorSystem::DispatchTaskToAgent(int32 AgentID, const FString& TaskDescription)
{
    if (AgentID < 1 || AgentID > Agents.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot dispatch task to invalid AgentID: %d"), AgentID);
        return;
    }
    
    UpdateAgentStatus(AgentID, EDir_AgentStatus::Working, TaskDescription);
    
    UE_LOG(LogTemp, Warning, TEXT("Task dispatched to Agent #%d: %s"), AgentID, *TaskDescription);
}

bool UStudioDirectorSystem::CanAgentProceed(int32 AgentID) const
{
    if (AgentID < 1 || AgentID > Agents.Num())
    {
        return false;
    }
    
    // Agent #1 (Studio Director) can always proceed
    if (AgentID == 1)
        return true;
    
    // Agent #2 (Engine Architect) can proceed after Studio Director
    if (AgentID == 2)
    {
        const FDir_AgentInfo& StudioDirector = Agents[0];
        return StudioDirector.Status == EDir_AgentStatus::Completed;
    }
    
    // Other agents need their predecessor to be completed
    if (AgentID > 2)
    {
        const FDir_AgentInfo& PreviousAgent = Agents[AgentID - 2];
        return PreviousAgent.Status == EDir_AgentStatus::Completed;
    }
    
    return false;
}