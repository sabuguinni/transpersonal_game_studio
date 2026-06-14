#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"

UProductionCoordinator::UProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    CurrentPhase = EDir_ProductionPhase::CoreSystems;
    CycleStartTime = 0.0f;
    CurrentCycleNumber = 20; // Current cycle
}

void UProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionCycle();
    
    // Initialize agent statuses
    AgentStatuses.Empty();
    
    // Add all 19 agents
    TArray<FString> AgentNames = {
        TEXT("Studio Director"), TEXT("Engine Architect"), TEXT("Core Systems"), TEXT("Performance Optimizer"),
        TEXT("Procedural World"), TEXT("Environment Artist"), TEXT("Architecture"), TEXT("Lighting"),
        TEXT("Character Artist"), TEXT("Animation"), TEXT("NPC Behavior"), TEXT("Combat AI"),
        TEXT("Crowd Simulation"), TEXT("Quest Designer"), TEXT("Narrative"), TEXT("Audio"),
        TEXT("VFX"), TEXT("QA Testing"), TEXT("Integration")
    };
    
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentTaskStatus NewStatus;
        NewStatus.AgentName = AgentName;
        NewStatus.CurrentTask = TEXT("Initializing");
        NewStatus.CompletionPercentage = 0.0f;
        NewStatus.bIsBlocked = false;
        AgentStatuses.Add(NewStatus);
    }
}

void UProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateProductionMetrics();
    CheckForCriticalBlockers();
}

void UProductionCoordinator::InitializeProductionCycle()
{
    CycleStartTime = GetWorld()->GetTimeSeconds();
    CurrentCycleNumber++;
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle %d Started - Phase: %s"), 
           CurrentCycleNumber, 
           *UEnum::GetValueAsString(CurrentPhase));
    
    AnalyzeCurrentGameState();
}

void UProductionCoordinator::UpdateAgentStatus(const FString& AgentName, const FString& Task, float Completion, bool bBlocked, const FString& BlockReason)
{
    for (FDir_AgentTaskStatus& Status : AgentStatuses)
    {
        if (Status.AgentName == AgentName)
        {
            Status.CurrentTask = Task;
            Status.CompletionPercentage = FMath::Clamp(Completion, 0.0f, 100.0f);
            Status.bIsBlocked = bBlocked;
            Status.BlockingReason = BlockReason;
            
            UE_LOG(LogTemp, Log, TEXT("Agent %s: %s (%.1f%%)"), 
                   *AgentName, *Task, Status.CompletionPercentage);
            break;
        }
    }
}

void UProductionCoordinator::AnalyzeCurrentGameState()
{
    if (!GetWorld())
    {
        return;
    }
    
    UpdateProductionMetrics();
    
    // Log current state
    UE_LOG(LogTemp, Warning, TEXT("=== GAME STATE ANALYSIS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), CurrentMetrics.CharacterCount);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Actors: %d"), CurrentMetrics.TerrainActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Playable Character: %s"), 
           CurrentMetrics.bHasPlayableCharacter ? TEXT("YES") : TEXT("NO"));
}

void UProductionCoordinator::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Reset metrics
    CurrentMetrics = FDir_ProductionMetrics();
    
    // Count all actors
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        CurrentMetrics.TotalActorsInLevel++;
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Count dinosaurs
        TArray<FString> DinoKeywords = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), TEXT("ankylo"), TEXT("parasauro")};
        for (const FString& Keyword : DinoKeywords)
        {
            if (ActorLabel.Contains(Keyword))
            {
                CurrentMetrics.DinosaurCount++;
                break;
            }
        }
        
        // Count characters
        if (Cast<ACharacter>(Actor) || ActorLabel.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("player")))
        {
            CurrentMetrics.CharacterCount++;
            
            // Check for playable character
            if (ActorLabel.Contains(TEXT("transpersonal")) || ActorLabel.Contains(TEXT("player")))
            {
                CurrentMetrics.bHasPlayableCharacter = true;
            }
        }
        
        // Count terrain
        if (Cast<ALandscape>(Actor) || ActorLabel.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("terrain")))
        {
            CurrentMetrics.TerrainActorCount++;
        }
    }
    
    // Update frame rate (simplified)
    CurrentMetrics.AverageFrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
}

void UProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT CYCLE %d ==="), CurrentCycleNumber);
    
    // Phase status
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %s"), *UEnum::GetValueAsString(CurrentPhase));
    
    // Metrics
    UE_LOG(LogTemp, Warning, TEXT("Game State Metrics:"));
    UE_LOG(LogTemp, Warning, TEXT("- Total Actors: %d"), CurrentMetrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("- Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("- Characters: %d"), CurrentMetrics.CharacterCount);
    UE_LOG(LogTemp, Warning, TEXT("- Terrain: %d"), CurrentMetrics.TerrainActorCount);
    UE_LOG(LogTemp, Warning, TEXT("- Playable Character: %s"), 
           CurrentMetrics.bHasPlayableCharacter ? TEXT("YES") : TEXT("NO"));
    
    // Agent status summary
    int32 CompletedAgents = 0;
    int32 BlockedAgents = 0;
    
    for (const FDir_AgentTaskStatus& Status : AgentStatuses)
    {
        if (Status.CompletionPercentage >= 100.0f)
        {
            CompletedAgents++;
        }
        if (Status.bIsBlocked)
        {
            BlockedAgents++;
            UE_LOG(LogTemp, Error, TEXT("BLOCKED: %s - %s"), *Status.AgentName, *Status.BlockingReason);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Progress: %d/%d Complete, %d Blocked"), 
           CompletedAgents, AgentStatuses.Num(), BlockedAgents);
    
    // Milestone validation
    bool bWalkAround = CheckWalkAroundMilestone();
    bool bDinosaurs = CheckDinosaurPresenceMilestone();
    bool bSurvival = CheckBasicSurvivalMilestone();
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone Status:"));
    UE_LOG(LogTemp, Warning, TEXT("- Walk Around: %s"), bWalkAround ? TEXT("COMPLETE") : TEXT("PENDING"));
    UE_LOG(LogTemp, Warning, TEXT("- Dinosaur Presence: %s"), bDinosaurs ? TEXT("COMPLETE") : TEXT("PENDING"));
    UE_LOG(LogTemp, Warning, TEXT("- Basic Survival: %s"), bSurvival ? TEXT("COMPLETE") : TEXT("PENDING"));
}

bool UProductionCoordinator::ValidateMinimumViablePrototype()
{
    return CheckWalkAroundMilestone() && CheckDinosaurPresenceMilestone() && CheckBasicSurvivalMilestone();
}

bool UProductionCoordinator::CheckWalkAroundMilestone()
{
    // Check for playable character with movement
    return CurrentMetrics.bHasPlayableCharacter && CurrentMetrics.TerrainActorCount > 0;
}

bool UProductionCoordinator::CheckDinosaurPresenceMilestone()
{
    // Need at least 3 dinosaurs in the world
    return CurrentMetrics.DinosaurCount >= 3;
}

bool UProductionCoordinator::CheckBasicSurvivalMilestone()
{
    // Basic check - need character and some environmental elements
    return CurrentMetrics.bHasPlayableCharacter && CurrentMetrics.TotalActorsInLevel > 50;
}

void UProductionCoordinator::AdvanceToNextPhase()
{
    if (ValidateMinimumViablePrototype())
    {
        int32 CurrentPhaseInt = static_cast<int32>(CurrentPhase);
        CurrentPhaseInt++;
        
        if (CurrentPhaseInt < static_cast<int32>(EDir_ProductionPhase::Release))
        {
            CurrentPhase = static_cast<EDir_ProductionPhase>(CurrentPhaseInt);
            UE_LOG(LogTemp, Warning, TEXT("Advanced to Phase: %s"), *UEnum::GetValueAsString(CurrentPhase));
        }
    }
}

void UProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription)
{
    UpdateAgentStatus(AgentName, TaskDescription, 0.0f, false);
}

TArray<FString> UProductionCoordinator::GetBlockedAgents()
{
    TArray<FString> BlockedList;
    
    for (const FDir_AgentTaskStatus& Status : AgentStatuses)
    {
        if (Status.bIsBlocked)
        {
            BlockedList.Add(Status.AgentName);
        }
    }
    
    return BlockedList;
}

void UProductionCoordinator::ResolveAgentBlocker(const FString& AgentName)
{
    for (FDir_AgentTaskStatus& Status : AgentStatuses)
    {
        if (Status.AgentName == AgentName)
        {
            Status.bIsBlocked = false;
            Status.BlockingReason = TEXT("");
            UE_LOG(LogTemp, Log, TEXT("Resolved blocker for agent: %s"), *AgentName);
            break;
        }
    }
}

void UProductionCoordinator::CheckForCriticalBlockers()
{
    int32 BlockedCount = 0;
    
    for (const FDir_AgentTaskStatus& Status : AgentStatuses)
    {
        if (Status.bIsBlocked)
        {
            BlockedCount++;
        }
    }
    
    // If more than 30% of agents are blocked, this is critical
    if (BlockedCount > AgentStatuses.Num() * 0.3f)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: %d/%d agents blocked - Production pipeline stalled"), 
               BlockedCount, AgentStatuses.Num());
    }
}

void UProductionCoordinator::LogProductionState()
{
    GenerateProductionReport();
}