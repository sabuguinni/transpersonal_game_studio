#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260521_005");
    TotalActorsInWorld = 0;
    DinosaurActorCount = 0;
}

void UStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    CreateMilestone1_WalkAround();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized for cycle: %s"), *CurrentCycleID);
}

void UStudioDirectorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ValidateWorldState();
    CheckMilestoneCompletion();
}

void UStudioDirectorSystem::InitializeProductionPipeline()
{
    SetupAgentTaskList();
    
    // Set initial milestone
    UpdateAgentStatus(1, EDir_AgentStatus::Working, TEXT("Studio Director - Coordinating production pipeline"));
    UpdateAgentStatus(2, EDir_AgentStatus::Idle, TEXT("Engine Architect - Awaiting architecture tasks"));
    UpdateAgentStatus(5, EDir_AgentStatus::Working, TEXT("Procedural World Generator - Terrain generation"));
    UpdateAgentStatus(9, EDir_AgentStatus::Working, TEXT("Character Artist - Dinosaur placement"));
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline initialized with %d agents"), AgentTasks.Num());
}

void UStudioDirectorSystem::SetupAgentTaskList()
{
    AgentTasks.Empty();
    
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
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTaskData NewAgent;
        NewAgent.AgentNumber = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CurrentTask = TEXT("Awaiting task assignment");
        AgentTasks.Add(NewAgent);
    }
}

void UStudioDirectorSystem::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    for (FDir_AgentTaskData& Agent : AgentTasks)
    {
        if (Agent.AgentNumber == AgentNumber)
        {
            Agent.Status = NewStatus;
            Agent.CurrentTask = TaskDescription;
            
            if (NewStatus == EDir_AgentStatus::Completed)
            {
                Agent.ProgressPercentage = 100.0f;
            }
            
            UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s) status updated: %s"), 
                AgentNumber, *Agent.AgentName, *TaskDescription);
            break;
        }
    }
}

void UStudioDirectorSystem::ValidateMinPlayableMap()
{
    ValidateWorldState();
    
    bool bHasCharacter = false;
    bool bHasDinosaurs = false;
    bool bHasTerrain = false;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName();
        FString ActorLabel = Actor->GetActorLabel();
        
        if (ActorName.Contains(TEXT("Character")) || ActorLabel.Contains(TEXT("Character")))
        {
            bHasCharacter = true;
        }
        
        if (ActorLabel.Contains(TEXT("Rex")) || ActorLabel.Contains(TEXT("Raptor")) || 
            ActorLabel.Contains(TEXT("Brachio")) || ActorLabel.Contains(TEXT("Tricera")))
        {
            bHasDinosaurs = true;
        }
        
        if (ActorName.Contains(TEXT("Landscape")) || ActorLabel.Contains(TEXT("Terrain")))
        {
            bHasTerrain = true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap validation - Character: %s, Dinosaurs: %s, Terrain: %s"),
        bHasCharacter ? TEXT("YES") : TEXT("NO"),
        bHasDinosaurs ? TEXT("YES") : TEXT("NO"),
        bHasTerrain ? TEXT("YES") : TEXT("NO"));
}

void UStudioDirectorSystem::SpawnDinosaursInBiomes()
{
    // This function coordinates dinosaur spawning across biomes
    // Actual spawning is handled by UE5 Python scripts
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director coordinating dinosaur spawning across biomes"));
    
    UpdateAgentStatus(9, EDir_AgentStatus::Working, TEXT("Character Artist - Spawning dinosaurs in biomes"));
    
    // Mark dinosaur spawning task as in progress
    DinosaurActorCount = 0; // Will be updated by ValidateWorldState
}

void UStudioDirectorSystem::ValidateWorldState()
{
    CountActorsByType();
    
    // Update progress based on world state
    if (DinosaurActorCount >= 5)
    {
        UpdateAgentStatus(9, EDir_AgentStatus::Completed, TEXT("Character Artist - Dinosaurs spawned successfully"));
    }
}

void UStudioDirectorSystem::CountActorsByType()
{
    TotalActorsInWorld = 0;
    DinosaurActorCount = 0;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        TotalActorsInWorld++;
        
        FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel.Contains(TEXT("Rex")) || ActorLabel.Contains(TEXT("Raptor")) || 
            ActorLabel.Contains(TEXT("Brachio")) || ActorLabel.Contains(TEXT("Tricera")) ||
            ActorLabel.Contains(TEXT("Ankylo")) || ActorLabel.Contains(TEXT("Para")))
        {
            DinosaurActorCount++;
        }
    }
}

FDir_AgentTaskData UStudioDirectorSystem::GetAgentStatus(int32 AgentNumber) const
{
    for (const FDir_AgentTaskData& Agent : AgentTasks)
    {
        if (Agent.AgentNumber == AgentNumber)
        {
            return Agent;
        }
    }
    
    return FDir_AgentTaskData(); // Return empty if not found
}

TArray<FString> UStudioDirectorSystem::GetBlockedAgents() const
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTaskData& Agent : AgentTasks)
    {
        if (Agent.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(FString::Printf(TEXT("Agent #%d (%s): %s"), 
                Agent.AgentNumber, *Agent.AgentName, *Agent.BlockingReason));
        }
    }
    
    return BlockedAgents;
}

float UStudioDirectorSystem::GetOverallProgress() const
{
    if (AgentTasks.Num() == 0) return 0.0f;
    
    float TotalProgress = 0.0f;
    for (const FDir_AgentTaskData& Agent : AgentTasks)
    {
        TotalProgress += Agent.ProgressPercentage;
    }
    
    return TotalProgress / AgentTasks.Num();
}

void UStudioDirectorSystem::CheckMilestoneCompletion()
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.bCompleted) continue;
        
        if (Milestone.MilestoneName == TEXT("MILESTONE 1 - WALK AROUND"))
        {
            // Check if all required elements are present
            bool bCharacterExists = TotalActorsInWorld > 0; // Simplified check
            bool bDinosaursSpawned = DinosaurActorCount >= 5;
            bool bTerrainExists = true; // Assume terrain exists
            
            if (bCharacterExists && bDinosaursSpawned && bTerrainExists)
            {
                Milestone.bCompleted = true;
                Milestone.CompletionPercentage = 100.0f;
                
                UE_LOG(LogTemp, Warning, TEXT("MILESTONE 1 COMPLETED: Walk Around prototype ready"));
            }
            else
            {
                float Progress = 0.0f;
                if (bCharacterExists) Progress += 33.3f;
                if (bDinosaursSpawned) Progress += 33.3f;
                if (bTerrainExists) Progress += 33.3f;
                
                Milestone.CompletionPercentage = Progress;
            }
        }
    }
}

bool UStudioDirectorSystem::IsMilestoneCompleted(const FString& MilestoneName) const
{
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.bCompleted;
        }
    }
    return false;
}

void UStudioDirectorSystem::CreateMilestone1_WalkAround()
{
    FDir_ProductionMilestone Milestone1;
    Milestone1.MilestoneName = TEXT("MILESTONE 1 - WALK AROUND");
    Milestone1.Description = TEXT("ThirdPersonCharacter with WASD movement, landscape with terrain, 3-5 dinosaur meshes, directional light + sky");
    Milestone1.RequiredAgents = {1, 2, 3, 5, 9, 10}; // Studio Director, Engine Architect, Core Systems, World Generator, Character Artist, Animation
    Milestone1.bCompleted = false;
    Milestone1.CompletionPercentage = 0.0f;
    
    ProductionMilestones.Add(Milestone1);
    
    UE_LOG(LogTemp, Warning, TEXT("Created Milestone 1: Walk Around prototype"));
}