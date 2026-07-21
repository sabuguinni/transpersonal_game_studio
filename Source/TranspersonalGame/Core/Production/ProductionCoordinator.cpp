#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentPhase = EDir_ProductionPhase::Prototype;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260614_012");
    AnalysisTimer = 0.0f;
    
    InitializeAgentTasks();
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Perform initial production state analysis
    AnalyzeProductionState();
    LogProductionEvent(TEXT("Production Coordinator initialized"));
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    AnalysisTimer += DeltaTime;
    
    // Update production metrics every 30 seconds
    if (AnalysisTimer >= 30.0f)
    {
        UpdateProductionMetrics();
        AnalysisTimer = 0.0f;
    }
}

void ADir_ProductionCoordinator::InitializeAgentTasks()
{
    AgentTasks.Empty();
    
    // Initialize all 19 agents with their current tasks
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
        FDir_AgentTask NewTask;
        NewTask.AgentID = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.Status = EDir_AgentStatus::Working;
        NewTask.CompletionPercentage = 0.0f;
        
        // Assign specific tasks based on current production phase
        switch (i + 1)
        {
        case 1: // Studio Director
            NewTask.CurrentTask = TEXT("Coordinate agent workflows and production metrics");
            break;
        case 2: // Engine Architect
            NewTask.CurrentTask = TEXT("Optimize core systems architecture");
            break;
        case 5: // World Generator
            NewTask.CurrentTask = TEXT("Enhance terrain and landscape systems");
            break;
        case 9: // Character Artist
            NewTask.CurrentTask = TEXT("Refine character systems and movement");
            break;
        case 12: // Combat AI
            NewTask.CurrentTask = TEXT("Implement dinosaur AI and behavior systems");
            break;
        case 16: // Audio Agent
            NewTask.CurrentTask = TEXT("Integrate environmental audio layers");
            break;
        default:
            NewTask.CurrentTask = TEXT("Continue module development");
            break;
        }
        
        AgentTasks.Add(NewTask);
    }
}

void ADir_ProductionCoordinator::UpdateAgentTask(int32 AgentID, const FString& TaskDescription, EDir_AgentStatus Status, float Completion)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.CurrentTask = TaskDescription;
            Task.Status = Status;
            Task.CompletionPercentage = FMath::Clamp(Completion, 0.0f, 100.0f);
            Task.LastUpdate = FDateTime::Now();
            
            FString LogMessage = FString::Printf(TEXT("Agent %d (%s): %s - %.1f%% complete"), 
                AgentID, *Task.AgentName, *TaskDescription, Completion);
            LogProductionEvent(LogMessage);
            break;
        }
    }
}

void ADir_ProductionCoordinator::AnalyzeProductionState()
{
    if (!GetWorld())
    {
        return;
    }
    
    CurrentMetrics = FDir_ProductionMetrics();
    CurrentMetrics.LastAnalysis = FDateTime::Now();
    
    // Count all actors in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.TotalActors = AllActors.Num();
    
    // Analyze different categories
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Count dinosaurs
        TArray<FString> DinoKeywords = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), 
                                       TEXT("brachi"), TEXT("ankylo"), TEXT("parasauro")};
        for (const FString& Keyword : DinoKeywords)
        {
            if (ActorLabel.Contains(Keyword))
            {
                CurrentMetrics.DinosaurCount++;
                break;
            }
        }
        
        // Count terrain elements
        if (ActorLabel.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("terrain")))
        {
            CurrentMetrics.TerrainElements++;
        }
        
        // Count character systems
        if (ActorLabel.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("player")))
        {
            CurrentMetrics.CharacterSystems++;
        }
        
        // Count lighting systems
        TArray<FString> LightKeywords = {TEXT("light"), TEXT("sun"), TEXT("sky"), TEXT("fog")};
        for (const FString& Keyword : LightKeywords)
        {
            if (ActorLabel.Contains(Keyword))
            {
                CurrentMetrics.LightingSystems++;
                break;
            }
        }
        
        // Count environment props
        if (ActorLabel.Contains(TEXT("tree")) || ActorLabel.Contains(TEXT("rock")))
        {
            CurrentMetrics.EnvironmentProps++;
        }
    }
    
    // Calculate overall completion
    CurrentMetrics.OverallCompletion = CalculateOverallCompletion();
    
    LogProductionEvent(FString::Printf(TEXT("Production Analysis: %d total actors, %.1f%% complete"), 
        CurrentMetrics.TotalActors, CurrentMetrics.OverallCompletion));
}

void ADir_ProductionCoordinator::GenerateAgentPriorities()
{
    // Generate priority tasks based on current metrics
    TArray<FString> Priorities;
    
    if (CurrentMetrics.TerrainElements < 5)
    {
        Priorities.Add(TEXT("Agent 05: Priority - Terrain enhancement needed"));
    }
    
    if (CurrentMetrics.DinosaurCount < 10)
    {
        Priorities.Add(TEXT("Agent 12: Priority - More dinosaur actors required"));
    }
    
    if (CurrentMetrics.CharacterSystems < 3)
    {
        Priorities.Add(TEXT("Agent 09: Priority - Character system expansion"));
    }
    
    if (CurrentMetrics.LightingSystems < 5)
    {
        Priorities.Add(TEXT("Agent 08: Priority - Lighting system completion"));
    }
    
    for (const FString& Priority : Priorities)
    {
        LogProductionEvent(Priority);
    }
}

FString ADir_ProductionCoordinator::GetProductionReport()
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT ===\n"));
    Report += FString::Printf(TEXT("Cycle: %s\n"), *CurrentCycleID);
    Report += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(CurrentPhase));
    Report += FString::Printf(TEXT("Total Actors: %d\n"), CurrentMetrics.TotalActors);
    Report += FString::Printf(TEXT("Dinosaurs: %d\n"), CurrentMetrics.DinosaurCount);
    Report += FString::Printf(TEXT("Terrain Elements: %d\n"), CurrentMetrics.TerrainElements);
    Report += FString::Printf(TEXT("Character Systems: %d\n"), CurrentMetrics.CharacterSystems);
    Report += FString::Printf(TEXT("Overall Completion: %.1f%%\n"), CurrentMetrics.OverallCompletion);
    
    Report += TEXT("\n=== AGENT STATUS ===\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        Report += FString::Printf(TEXT("Agent %02d (%s): %s [%.1f%%]\n"), 
            Task.AgentID, *Task.AgentName, *UEnum::GetValueAsString(Task.Status), Task.CompletionPercentage);
    }
    
    return Report;
}

void ADir_ProductionCoordinator::AdvanceProductionPhase()
{
    if (CurrentMetrics.OverallCompletion >= 80.0f)
    {
        switch (CurrentPhase)
        {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::Prototype;
            break;
        case EDir_ProductionPhase::Prototype:
            CurrentPhase = EDir_ProductionPhase::Alpha;
            break;
        case EDir_ProductionPhase::Alpha:
            CurrentPhase = EDir_ProductionPhase::Beta;
            break;
        case EDir_ProductionPhase::Beta:
            CurrentPhase = EDir_ProductionPhase::Release;
            break;
        default:
            break;
        }
        
        LogProductionEvent(FString::Printf(TEXT("Advanced to phase: %s"), 
            *UEnum::GetValueAsString(CurrentPhase)));
    }
}

bool ADir_ProductionCoordinator::ValidateBuildQuality()
{
    // Validate minimum requirements for current phase
    switch (CurrentPhase)
    {
    case EDir_ProductionPhase::Prototype:
        return CurrentMetrics.TotalActors >= 50 && 
               CurrentMetrics.DinosaurCount >= 5 && 
               CurrentMetrics.CharacterSystems >= 1;
    case EDir_ProductionPhase::Alpha:
        return CurrentMetrics.TotalActors >= 200 && 
               CurrentMetrics.DinosaurCount >= 15 && 
               CurrentMetrics.TerrainElements >= 5;
    case EDir_ProductionPhase::Beta:
        return CurrentMetrics.TotalActors >= 500 && 
               CurrentMetrics.DinosaurCount >= 30 && 
               CurrentMetrics.LightingSystems >= 5;
    default:
        return true;
    }
}

float ADir_ProductionCoordinator::CalculateOverallCompletion()
{
    float TotalCompletion = 0.0f;
    int32 ValidTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status != EDir_AgentStatus::Failed)
        {
            TotalCompletion += Task.CompletionPercentage;
            ValidTasks++;
        }
    }
    
    return ValidTasks > 0 ? TotalCompletion / ValidTasks : 0.0f;
}

TArray<FString> ADir_ProductionCoordinator::GetBlockedAgents()
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || Task.Status == EDir_AgentStatus::Failed)
        {
            BlockedAgents.Add(FString::Printf(TEXT("Agent %d: %s"), Task.AgentID, *Task.AgentName));
        }
    }
    
    return BlockedAgents;
}

void ADir_ProductionCoordinator::UpdateProductionMetrics()
{
    AnalyzeProductionState();
    GenerateAgentPriorities();
    
    // Check if we can advance production phase
    AdvanceProductionPhase();
    
    // Validate current build quality
    bool bBuildValid = ValidateBuildQuality();
    if (!bBuildValid)
    {
        LogProductionEvent(TEXT("WARNING: Build quality below phase requirements"));
    }
}

void ADir_ProductionCoordinator::LogProductionEvent(const FString& EventDescription)
{
    FString TimestampedEvent = FString::Printf(TEXT("[%s] %s"), 
        *FDateTime::Now().ToString(), *EventDescription);
    
    ProductionLog.Add(TimestampedEvent);
    
    // Keep only last 100 events
    if (ProductionLog.Num() > 100)
    {
        ProductionLog.RemoveAt(0);
    }
    
    // Also log to UE console
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: %s"), *EventDescription);
}