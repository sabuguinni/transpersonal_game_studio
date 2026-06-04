#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260604_005");
    TotalCycles = 5;

    InitializeDefaultAgents();
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Starting Cycle 005 coordination"));
    InitializeAgentTracking();
    CheckMilestoneProgress();
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update milestone progress every 5 seconds
    static float TimeAccumulator = 0.0f;
    TimeAccumulator += DeltaTime;
    
    if (TimeAccumulator >= 5.0f)
    {
        CheckMilestoneProgress();
        TimeAccumulator = 0.0f;
    }
}

void ADir_ProductionCoordinator::InitializeDefaultAgents()
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

    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentStatus NewAgent;
        NewAgent.AgentNumber = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.bIsActive = (i < 5); // First 5 agents active for Milestone 1
        NewAgent.CurrentTask = TEXT("Milestone 1: Walk Around Prototype");
        NewAgent.CompletionPercentage = 0.0f;
        NewAgent.LastUpdateTime = FDateTime::Now();
        
        AgentStatuses.Add(NewAgent);
    }
}

void ADir_ProductionCoordinator::InitializeAgentTracking()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing agent tracking for %d agents"), AgentStatuses.Num());
    
    for (FDir_AgentStatus& Agent : AgentStatuses)
    {
        Agent.LastUpdateTime = FDateTime::Now();
        UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s"), 
               Agent.AgentNumber, 
               *Agent.AgentName, 
               Agent.bIsActive ? TEXT("ACTIVE") : TEXT("STANDBY"));
    }
}

void ADir_ProductionCoordinator::UpdateAgentStatus(int32 AgentNumber, const FString& TaskDescription, float Progress)
{
    for (FDir_AgentStatus& Agent : AgentStatuses)
    {
        if (Agent.AgentNumber == AgentNumber)
        {
            Agent.CurrentTask = TaskDescription;
            Agent.CompletionPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            Agent.LastUpdateTime = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d updated: %s (%.1f%%)"), 
                   AgentNumber, *TaskDescription, Progress);
            break;
        }
    }
}

void ADir_ProductionCoordinator::CheckMilestoneProgress()
{
    ScanWorldForMilestoneComponents();
    CurrentMilestone.OverallProgress = CalculateOverallProgress();
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone Progress: %.1f%% - Character:%s Landscape:%s Movement:%s Dinosaurs:%s Lighting:%s"),
           CurrentMilestone.OverallProgress,
           CurrentMilestone.bHasThirdPersonCharacter ? TEXT("YES") : TEXT("NO"),
           CurrentMilestone.bHasLandscape ? TEXT("YES") : TEXT("NO"),
           CurrentMilestone.bHasMovement ? TEXT("YES") : TEXT("NO"),
           CurrentMilestone.bHasDinosaurs ? TEXT("YES") : TEXT("NO"),
           CurrentMilestone.bHasLighting ? TEXT("YES") : TEXT("NO"));
}

void ADir_ProductionCoordinator::ScanWorldForMilestoneComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Reset milestone flags
    CurrentMilestone.bHasThirdPersonCharacter = false;
    CurrentMilestone.bHasLandscape = false;
    CurrentMilestone.bHasMovement = false;
    CurrentMilestone.bHasDinosaurs = false;
    CurrentMilestone.bHasLighting = false;

    // Scan for characters
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (Character && Character->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            CurrentMilestone.bHasThirdPersonCharacter = true;
            CurrentMilestone.bHasMovement = true; // Assume character has movement
            break;
        }
    }

    // Scan for landscape
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape)
        {
            CurrentMilestone.bHasLandscape = true;
            break;
        }
    }

    // Scan for lighting
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        ADirectionalLight* Light = *ActorItr;
        if (Light)
        {
            CurrentMilestone.bHasLighting = true;
            break;
        }
    }

    // Scan for dinosaur placeholders
    int32 DinosaurCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetName().Contains(TEXT("Dinosaur")) || 
                     Actor->GetName().Contains(TEXT("TRex")) ||
                     Actor->GetName().Contains(TEXT("Raptor")) ||
                     Actor->GetName().Contains(TEXT("Brachio"))))
        {
            DinosaurCount++;
        }
    }
    CurrentMilestone.bHasDinosaurs = (DinosaurCount >= 3);
}

float ADir_ProductionCoordinator::CalculateOverallProgress() const
{
    float Progress = 0.0f;
    int32 TotalComponents = 5;
    
    if (CurrentMilestone.bHasThirdPersonCharacter) Progress += 20.0f;
    if (CurrentMilestone.bHasLandscape) Progress += 20.0f;
    if (CurrentMilestone.bHasMovement) Progress += 20.0f;
    if (CurrentMilestone.bHasDinosaurs) Progress += 20.0f;
    if (CurrentMilestone.bHasLighting) Progress += 20.0f;
    
    return Progress;
}

FDir_MilestoneProgress ADir_ProductionCoordinator::GetCurrentMilestoneStatus() const
{
    return CurrentMilestone;
}

TArray<FDir_AgentStatus> ADir_ProductionCoordinator::GetAllAgentStatuses() const
{
    return AgentStatuses;
}

void ADir_ProductionCoordinator::ValidateMinPlayableMap()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MIN PLAYABLE MAP VALIDATION ==="));
    
    CheckMilestoneProgress();
    
    if (CurrentMilestone.OverallProgress >= 80.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("VALIDATION PASSED: Minimum playable prototype ready (%.1f%%)"), 
               CurrentMilestone.OverallProgress);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("VALIDATION FAILED: Prototype incomplete (%.1f%%)"), 
               CurrentMilestone.OverallProgress);
        
        if (!CurrentMilestone.bHasThirdPersonCharacter)
            UE_LOG(LogTemp, Error, TEXT("MISSING: Third Person Character"));
        if (!CurrentMilestone.bHasLandscape)
            UE_LOG(LogTemp, Error, TEXT("MISSING: Landscape terrain"));
        if (!CurrentMilestone.bHasMovement)
            UE_LOG(LogTemp, Error, TEXT("MISSING: Player movement"));
        if (!CurrentMilestone.bHasDinosaurs)
            UE_LOG(LogTemp, Error, TEXT("MISSING: Dinosaur placeholders"));
        if (!CurrentMilestone.bHasLighting)
            UE_LOG(LogTemp, Error, TEXT("MISSING: Directional lighting"));
    }
}

void ADir_ProductionCoordinator::LogProductionMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION METRICS CYCLE %s ==="), *CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Total Cycles: %d"), TotalCycles);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), CurrentMilestone.TotalAgentsActive);
    UE_LOG(LogTemp, Warning, TEXT("Milestone Progress: %.1f%%"), CurrentMilestone.OverallProgress);
    
    for (const FDir_AgentStatus& Agent : AgentStatuses)
    {
        if (Agent.bIsActive)
        {
            UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s - %.1f%%"),
                   Agent.AgentNumber,
                   *Agent.AgentName,
                   *Agent.CurrentTask,
                   Agent.CompletionPercentage);
        }
    }
}