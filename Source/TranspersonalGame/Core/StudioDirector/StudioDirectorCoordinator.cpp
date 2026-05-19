#include "StudioDirectorCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"

UStudioDirectorCoordinator::UStudioDirectorCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    CycleDuration = 0.0f;
    LastMetricsUpdate = 0.0f;
    bEmergencyMode = false;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260519_002");
}

void UStudioDirectorCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentPipeline();
    AnalyzeProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Coordinator initialized for cycle: %s"), *CurrentCycleID);
}

void UStudioDirectorCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CycleDuration += DeltaTime;
    
    // Update metrics every 5 seconds
    if (CycleDuration - LastMetricsUpdate > 5.0f)
    {
        UpdateProductionMetrics();
        LastMetricsUpdate = CycleDuration;
        
        // Check if we need emergency prototype mode
        if (!ValidatePlayablePrototype() && CycleDuration > 300.0f) // 5 minutes
        {
            TriggerEmergencyPrototype();
        }
    }
}

void UStudioDirectorCoordinator::InitializeAgentPipeline()
{
    AgentStatuses.Empty();
    InitializeDefaultAgents();
    
    UE_LOG(LogTemp, Log, TEXT("Agent pipeline initialized with %d agents"), AgentStatuses.Num());
}

void UStudioDirectorCoordinator::InitializeDefaultAgents()
{
    // Initialize all 19 agents with their specializations
    TArray<TPair<FString, FString>> AgentDefinitions = {
        {TEXT("Studio Director"), TEXT("Pipeline coordination and vision translation")},
        {TEXT("Engine Architect"), TEXT("Core architecture and technical rules")},
        {TEXT("Core Systems Programmer"), TEXT("Physics, collision, ragdoll systems")},
        {TEXT("Performance Optimizer"), TEXT("60fps PC / 30fps console optimization")},
        {TEXT("Procedural World Generator"), TEXT("Terrain, biomes, PCG systems")},
        {TEXT("Environment Artist"), TEXT("Vegetation, rocks, material population")},
        {TEXT("Architecture & Interior"), TEXT("Buildings and prehistoric structures")},
        {TEXT("Lighting & Atmosphere"), TEXT("Day/night cycle, weather, Lumen")},
        {TEXT("Character Artist"), TEXT("MetaHuman characters and NPCs")},
        {TEXT("Animation Agent"), TEXT("Motion Matching and IK systems")},
        {TEXT("NPC Behavior Agent"), TEXT("Behavior Trees and daily routines")},
        {TEXT("Combat & Enemy AI"), TEXT("Tactical AI and dinosaur behavior")},
        {TEXT("Crowd & Traffic Simulation"), TEXT("Mass AI for 50k+ agents")},
        {TEXT("Quest & Mission Designer"), TEXT("Mission systems and objectives")},
        {TEXT("Narrative & Dialogue"), TEXT("Game Bible and story content")},
        {TEXT("Audio Agent"), TEXT("MetaSounds and adaptive music")},
        {TEXT("VFX Agent"), TEXT("Niagara effects with LOD chains")},
        {TEXT("QA & Testing Agent"), TEXT("Quality assurance and build blocking")},
        {TEXT("Integration & Build"), TEXT("Final integration and reporting")}
    };
    
    for (int32 i = 0; i < AgentDefinitions.Num(); i++)
    {
        FDir_AgentStatus NewAgent;
        NewAgent.AgentNumber = i + 1;
        NewAgent.AgentName = AgentDefinitions[i].Key;
        NewAgent.CurrentTask = AgentDefinitions[i].Value;
        NewAgent.CompletionPercentage = 0.0f;
        NewAgent.bIsActive = (i == 0); // Only Studio Director starts active
        NewAgent.LastOutput = TEXT("Initialized");
        
        AgentStatuses.Add(NewAgent);
    }
}

void UStudioDirectorCoordinator::UpdateAgentStatus(int32 AgentNumber, const FString& Task, float Completion)
{
    for (FDir_AgentStatus& Agent : AgentStatuses)
    {
        if (Agent.AgentNumber == AgentNumber)
        {
            Agent.CurrentTask = Task;
            Agent.CompletionPercentage = FMath::Clamp(Completion, 0.0f, 100.0f);
            Agent.bIsActive = (Completion < 100.0f);
            
            UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s - %.1f%%"), 
                AgentNumber, *Agent.AgentName, *Task, Completion);
            break;
        }
    }
}

void UStudioDirectorCoordinator::AnalyzeProductionMetrics()
{
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION METRICS ANALYSIS ==="));
    UE_LOG(LogTemp, Warning, TEXT("C++ Files: %d | Header Files: %d"), 
        CurrentMetrics.TotalCppFiles, CurrentMetrics.TotalHeaderFiles);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d | Dinosaurs: %d"), 
        CurrentMetrics.ActiveActors, CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Coverage: %.1f%% | Playable: %s"), 
        CurrentMetrics.TerrainCoverage, CurrentMetrics.bPlayablePrototype ? TEXT("YES") : TEXT("NO"));
}

void UStudioDirectorCoordinator::UpdateProductionMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Count actors in the world
        CurrentMetrics.ActiveActors = 0;
        CurrentMetrics.DinosaurCount = 0;
        CurrentMetrics.TerrainCoverage = 0.0f;
        
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && !Actor->IsPendingKill())
            {
                CurrentMetrics.ActiveActors++;
                
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("rex")) || ActorName.Contains(TEXT("raptor")) || 
                    ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("dino")))
                {
                    CurrentMetrics.DinosaurCount++;
                }
            }
        }
        
        // Estimate terrain coverage based on landscape actors
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetName().ToLower().Contains(TEXT("landscape")))
            {
                CurrentMetrics.TerrainCoverage += 20.0f; // Each landscape adds 20%
            }
        }
        
        CurrentMetrics.TerrainCoverage = FMath::Clamp(CurrentMetrics.TerrainCoverage, 0.0f, 100.0f);
        
        // Check if we have a playable prototype
        CurrentMetrics.bPlayablePrototype = ValidatePlayablePrototype();
    }
}

bool UStudioDirectorCoordinator::ValidatePlayablePrototype()
{
    // Milestone 1 requirements: Character movement + terrain + dinosaurs + lighting
    bool bHasCharacter = false;
    bool bHasTerrain = (CurrentMetrics.TerrainCoverage > 0.0f);
    bool bHasDinosaurs = (CurrentMetrics.DinosaurCount > 0);
    bool bHasLighting = false;
    
    if (UWorld* World = GetWorld())
    {
        // Check for player character
        for (TActorIterator<ACharacter> CharacterIterator(World); CharacterIterator; ++CharacterIterator)
        {
            if (*CharacterIterator)
            {
                bHasCharacter = true;
                break;
            }
        }
        
        // Check for lighting
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && (Actor->GetName().Contains(TEXT("Light")) || Actor->GetName().Contains(TEXT("Sun"))))
            {
                bHasLighting = true;
                break;
            }
        }
    }
    
    return bHasCharacter && bHasTerrain && bHasDinosaurs && bHasLighting;
}

bool UStudioDirectorCoordinator::CheckMilestone1Completion()
{
    return ValidatePlayablePrototype();
}

TArray<FString> UStudioDirectorCoordinator::GetMissingMilestone1Requirements()
{
    TArray<FString> MissingRequirements;
    
    if (CurrentMetrics.DinosaurCount == 0)
    {
        MissingRequirements.Add(TEXT("Dinosaur actors in world"));
    }
    
    if (CurrentMetrics.TerrainCoverage < 20.0f)
    {
        MissingRequirements.Add(TEXT("Basic terrain/landscape"));
    }
    
    if (CurrentMetrics.ActiveActors < 10)
    {
        MissingRequirements.Add(TEXT("Sufficient world population"));
    }
    
    bool bHasCharacter = false;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<ACharacter> CharacterIterator(World); CharacterIterator; ++CharacterIterator)
        {
            if (*CharacterIterator)
            {
                bHasCharacter = true;
                break;
            }
        }
    }
    
    if (!bHasCharacter)
    {
        MissingRequirements.Add(TEXT("Playable character with movement"));
    }
    
    return MissingRequirements;
}

void UStudioDirectorCoordinator::TriggerEmergencyPrototype()
{
    if (!bEmergencyMode)
    {
        bEmergencyMode = true;
        UE_LOG(LogTemp, Error, TEXT("=== EMERGENCY PROTOTYPE MODE ACTIVATED ==="));
        UE_LOG(LogTemp, Error, TEXT("Milestone 1 not achieved within time limit. Forcing minimal viable prototype."));
        
        // Force all agents to focus on Milestone 1 requirements only
        for (FDir_AgentStatus& Agent : AgentStatuses)
        {
            if (Agent.AgentNumber <= 12) // Core agents only
            {
                Agent.bIsActive = true;
                Agent.CurrentTask = TEXT("EMERGENCY: Milestone 1 requirements only");
            }
        }
    }
}

void UStudioDirectorCoordinator::CoordinateAgentExecution()
{
    // Activate next agent in sequence if current is complete
    for (int32 i = 0; i < AgentStatuses.Num() - 1; i++)
    {
        if (AgentStatuses[i].bIsActive && AgentStatuses[i].CompletionPercentage >= 100.0f)
        {
            AgentStatuses[i].bIsActive = false;
            AgentStatuses[i + 1].bIsActive = true;
            
            UE_LOG(LogTemp, Log, TEXT("Agent #%d completed. Activating Agent #%d"), 
                i + 1, i + 2);
            break;
        }
    }
}

FString UStudioDirectorCoordinator::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ===\n"));
    Report += FString::Printf(TEXT("Cycle: %s | Duration: %.1fs\n"), *CurrentCycleID, CycleDuration);
    Report += FString::Printf(TEXT("Active Actors: %d | Dinosaurs: %d\n"), 
        CurrentMetrics.ActiveActors, CurrentMetrics.DinosaurCount);
    Report += FString::Printf(TEXT("Terrain Coverage: %.1f%%\n"), CurrentMetrics.TerrainCoverage);
    Report += FString::Printf(TEXT("Playable Prototype: %s\n"), 
        CurrentMetrics.bPlayablePrototype ? TEXT("ACHIEVED") : TEXT("PENDING"));
    
    if (bEmergencyMode)
    {
        Report += TEXT("STATUS: EMERGENCY MODE - Focus on Milestone 1\n");
    }
    
    TArray<FString> Missing = GetMissingMilestone1Requirements();
    if (Missing.Num() > 0)
    {
        Report += TEXT("Missing Requirements:\n");
        for (const FString& Requirement : Missing)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Requirement);
        }
    }
    
    return Report;
}