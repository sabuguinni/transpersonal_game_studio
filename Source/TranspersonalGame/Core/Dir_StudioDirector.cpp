#include "Dir_StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ADir_StudioDirector::ADir_StudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;
    
    CurrentCycleID = 0;
    CycleDuration = 1800.0f; // 30 minutes per cycle
    LastCycleTime = 0.0f;
    bEmergencyMode = false;
    FailedAgentCount = 0;
    
    // Initialize milestone tracking
    bWalkAroundMilestone = false;
    bTerrainMilestone = false;
    bDinosaurMilestone = false;
    bAtmosphereMilestone = false;
}

void ADir_StudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentPipeline();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized - 19 agent pipeline active"));
}

void ADir_StudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastCycleTime += DeltaTime;
    
    // Check for cycle completion every 30 minutes
    if (LastCycleTime >= CycleDuration)
    {
        CurrentCycleID++;
        LastCycleTime = 0.0f;
        
        CheckMilestoneCompletion();
        
        // Reset agent statuses for new cycle
        for (FDir_AgentCoordination& Agent : AgentPipeline)
        {
            if (Agent.Status == EDir_AgentStatus::Completed)
            {
                Agent.Status = EDir_AgentStatus::Idle;
                Agent.CompletionPercentage = 0.0f;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Cycle %d completed - resetting pipeline"), CurrentCycleID);
    }
}

void ADir_StudioDirector::InitializeAgentPipeline()
{
    AgentPipeline.Empty();
    
    // Initialize all 19 agents with their dependencies
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
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentCoordination NewAgent;
        NewAgent.AgentID = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CompletionPercentage = 0.0f;
        
        // Set dependencies based on production chain
        if (i == 0) // Studio Director
        {
            NewAgent.Phase = EDir_ProductionPhase::Architecture;
        }
        else if (i == 1) // Engine Architect
        {
            NewAgent.Dependencies.Add(1); // Depends on Studio Director
            NewAgent.Phase = EDir_ProductionPhase::Architecture;
        }
        else if (i >= 2 && i <= 4) // Core Systems, Performance, World Gen
        {
            NewAgent.Dependencies.Add(2); // Depends on Engine Architect
            NewAgent.Phase = EDir_ProductionPhase::CoreSystems;
        }
        else if (i >= 5 && i <= 7) // Environment, Architecture, Lighting
        {
            NewAgent.Dependencies.Add(5); // Depends on World Generator
            NewAgent.Phase = EDir_ProductionPhase::Environment;
        }
        else if (i >= 8 && i <= 12) // Characters, Animation, AI
        {
            NewAgent.Dependencies.Add(8); // Depends on Lighting
            NewAgent.Phase = EDir_ProductionPhase::Characters;
        }
        else if (i >= 13 && i <= 16) // Audio, VFX, etc.
        {
            NewAgent.Dependencies.Add(13); // Depends on AI systems
            NewAgent.Phase = EDir_ProductionPhase::Audio;
        }
        else // QA and Integration
        {
            NewAgent.Dependencies.Add(17); // Depends on content creation
            NewAgent.Phase = EDir_ProductionPhase::QA;
        }
        
        AgentPipeline.Add(NewAgent);
    }
    
    // Initialize production milestones
    ProductionMilestones.Empty();
    
    FDir_ProductionMilestone WalkAroundMilestone;
    WalkAroundMilestone.MilestoneName = TEXT("Walk Around Prototype");
    WalkAroundMilestone.RequiredAgents = {2, 3, 5, 9}; // Engine, Core, World, Character
    WalkAroundMilestone.Priority = 10.0f;
    ProductionMilestones.Add(WalkAroundMilestone);
    
    FDir_ProductionMilestone TerrainMilestone;
    TerrainMilestone.MilestoneName = TEXT("10km2 Terrain with 5 Biomes");
    TerrainMilestone.RequiredAgents = {5, 6, 8}; // World Gen, Environment, Lighting
    TerrainMilestone.Priority = 9.0f;
    ProductionMilestones.Add(TerrainMilestone);
    
    FDir_ProductionMilestone DinosaurMilestone;
    DinosaurMilestone.MilestoneName = TEXT("Dinosaur Actors with AI");
    DinosaurMilestone.RequiredAgents = {9, 10, 12}; // Character, Animation, Combat AI
    DinosaurMilestone.Priority = 8.0f;
    ProductionMilestones.Add(DinosaurMilestone);
}

void ADir_StudioDirector::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentCoordination& Agent : AgentPipeline)
    {
        if (Agent.AgentID == AgentID)
        {
            Agent.Status = NewStatus;
            
            if (NewStatus == EDir_AgentStatus::Failed)
            {
                FailedAgentCount++;
                HandleAgentFailure(AgentID);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Agent %d (%s) status updated to %d"), 
                   AgentID, *Agent.AgentName, (int32)NewStatus);
            break;
        }
    }
}

bool ADir_StudioDirector::CanAgentProceed(int32 AgentID)
{
    if (bEmergencyMode && AgentID > 16)
    {
        return false; // Block late agents in emergency
    }
    
    for (const FDir_AgentCoordination& Agent : AgentPipeline)
    {
        if (Agent.AgentID == AgentID)
        {
            // Check if all dependencies are completed
            for (int32 DepID : Agent.Dependencies)
            {
                bool bDepCompleted = false;
                for (const FDir_AgentCoordination& DepAgent : AgentPipeline)
                {
                    if (DepAgent.AgentID == DepID && DepAgent.Status == EDir_AgentStatus::Completed)
                    {
                        bDepCompleted = true;
                        break;
                    }
                }
                
                if (!bDepCompleted)
                {
                    return false;
                }
            }
            
            return true;
        }
    }
    
    return false;
}

void ADir_StudioDirector::BlockAgent(int32 AgentID, const FString& Reason)
{
    UpdateAgentStatus(AgentID, EDir_AgentStatus::Blocked);
    
    UE_LOG(LogTemp, Warning, TEXT("Agent %d blocked: %s"), AgentID, *Reason);
}

TArray<int32> ADir_StudioDirector::GetReadyAgents()
{
    TArray<int32> ReadyAgents;
    
    for (const FDir_AgentCoordination& Agent : AgentPipeline)
    {
        if (Agent.Status == EDir_AgentStatus::Idle && CanAgentProceed(Agent.AgentID))
        {
            ReadyAgents.Add(Agent.AgentID);
        }
    }
    
    return ReadyAgents;
}

void ADir_StudioDirector::CheckMilestoneCompletion()
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.bIsCompleted)
        {
            continue;
        }
        
        bool bAllAgentsComplete = true;
        for (int32 RequiredAgent : Milestone.RequiredAgents)
        {
            bool bAgentComplete = false;
            for (const FDir_AgentCoordination& Agent : AgentPipeline)
            {
                if (Agent.AgentID == RequiredAgent && Agent.Status == EDir_AgentStatus::Completed)
                {
                    bAgentComplete = true;
                    break;
                }
            }
            
            if (!bAgentComplete)
            {
                bAllAgentsComplete = false;
                break;
            }
        }
        
        if (bAllAgentsComplete)
        {
            Milestone.bIsCompleted = true;
            
            if (Milestone.MilestoneName == TEXT("Walk Around Prototype"))
            {
                bWalkAroundMilestone = true;
                CreateWalkAroundPrototype();
            }
            else if (Milestone.MilestoneName.Contains(TEXT("Terrain")))
            {
                bTerrainMilestone = true;
            }
            else if (Milestone.MilestoneName.Contains(TEXT("Dinosaur")))
            {
                bDinosaurMilestone = true;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Milestone completed: %s"), *Milestone.MilestoneName);
        }
    }
}

void ADir_StudioDirector::CreateWalkAroundPrototype()
{
    ValidateMinPlayableMap();
    
    UE_LOG(LogTemp, Warning, TEXT("Walk Around Prototype milestone achieved!"));
}

void ADir_StudioDirector::ValidateMinPlayableMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count essential actors
    int32 CharacterCount = 0;
    int32 LandscapeCount = 0;
    int32 DinosaurCount = 0;
    int32 LightCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ActorName = Actor->GetClass()->GetName();
        
        if (ActorName.Contains(TEXT("Character")))
        {
            CharacterCount++;
        }
        else if (ActorName.Contains(TEXT("Landscape")))
        {
            LandscapeCount++;
        }
        else if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")))
        {
            DinosaurCount++;
        }
        else if (ActorName.Contains(TEXT("Light")))
        {
            LightCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap validation: Characters=%d, Landscapes=%d, Dinosaurs=%d, Lights=%d"), 
           CharacterCount, LandscapeCount, DinosaurCount, LightCount);
}

void ADir_StudioDirector::CoordinateAssetPipeline()
{
    // Check asset purchase criteria from brain memories
    bool bTerrainReady = bTerrainMilestone;
    bool bAtmosphereReady = bAtmosphereMilestone;
    bool bFBXPipelineReady = false; // To be implemented by agents #6/#7/#9
    
    if (bTerrainReady && bAtmosphereReady && bFBXPipelineReady)
    {
        UE_LOG(LogTemp, Warning, TEXT("Asset purchase criteria met - ready for commercial assets"));
    }
}

bool ADir_StudioDirector::AreAssetCriteriaReady()
{
    return bTerrainMilestone && bAtmosphereMilestone;
}

void ADir_StudioDirector::HandleAgentFailure(int32 AgentID)
{
    if (FailedAgentCount >= 3)
    {
        ActivateEmergencyMode();
    }
    
    // Block dependent agents
    for (FDir_AgentCoordination& Agent : AgentPipeline)
    {
        for (int32 DepID : Agent.Dependencies)
        {
            if (DepID == AgentID)
            {
                BlockAgent(Agent.AgentID, FString::Printf(TEXT("Dependency %d failed"), AgentID));
            }
        }
    }
}

void ADir_StudioDirector::ActivateEmergencyMode()
{
    bEmergencyMode = true;
    
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY MODE ACTIVATED - Multiple agent failures detected"));
    
    // Reduce scope for remaining agents
    for (FDir_AgentCoordination& Agent : AgentPipeline)
    {
        if (Agent.AgentID > 16 && Agent.Status != EDir_AgentStatus::Completed)
        {
            Agent.Status = EDir_AgentStatus::Blocked;
        }
    }
}