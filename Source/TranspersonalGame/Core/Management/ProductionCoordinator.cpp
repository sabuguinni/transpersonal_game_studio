#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"

UProductionCoordinator::UProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    CurrentPhase = EDir_ProductionPhase::Prototype;
    PhaseProgress = 0.0f;
}

void UProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the production system
    InitializeDefaultAgents();
    UpdateProductionMetrics();
    
    // Set up periodic updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MetricsUpdateTimer, this, &UProductionCoordinator::UpdateProductionMetrics, 5.0f, true);
        World->GetTimerManager().SetTimer(DependencyCheckTimer, this, &UProductionCoordinator::CheckDependencies, 2.0f, true);
    }
    
    LogProductionEvent(TEXT("Production Coordinator initialized"));
}

void UProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update phase progress based on agent completion
    UpdatePhaseProgress();
}

void UProductionCoordinator::InitializeDefaultAgents()
{
    // Clear existing tasks
    AgentTasks.Empty();
    AgentStatusMap.Empty();
    
    // Define the 19 agent pipeline
    TArray<TPair<FString, FString>> AgentDefinitions = {
        {TEXT("Engine_Architect"), TEXT("Define core engine architecture and technical standards")},
        {TEXT("Core_Systems"), TEXT("Implement physics, collision, and core gameplay systems")},
        {TEXT("Performance_Optimizer"), TEXT("Ensure 60fps PC / 30fps console performance")},
        {TEXT("World_Generator"), TEXT("Generate procedural terrain and biomes")},
        {TEXT("Environment_Artist"), TEXT("Populate world with vegetation and props")},
        {TEXT("Architecture_Agent"), TEXT("Build prehistoric structures and shelters")},
        {TEXT("Lighting_Agent"), TEXT("Implement day/night cycle and atmospheric lighting")},
        {TEXT("Character_Artist"), TEXT("Create player character and NPCs")},
        {TEXT("Animation_Agent"), TEXT("Add Motion Matching and IK animations")},
        {TEXT("NPC_Behavior"), TEXT("Implement NPC AI and daily routines")},
        {TEXT("Combat_AI"), TEXT("Create dinosaur combat and territorial behavior")},
        {TEXT("Crowd_Simulation"), TEXT("Implement Mass AI for large-scale simulations")},
        {TEXT("Quest_Designer"), TEXT("Convert narrative into playable missions")},
        {TEXT("Narrative_Agent"), TEXT("Write game bible and prehistoric lore")},
        {TEXT("Audio_Agent"), TEXT("Create adaptive music and sound effects")},
        {TEXT("VFX_Agent"), TEXT("Implement Niagara particle effects")},
        {TEXT("QA_Testing"), TEXT("Test all systems and validate gameplay")},
        {TEXT("Integration_Agent"), TEXT("Integrate all systems into cohesive build")}
    };
    
    // Create agent tasks with dependencies
    for (int32 i = 0; i < AgentDefinitions.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentDefinitions[i].Key;
        NewTask.TaskDescription = AgentDefinitions[i].Value;
        NewTask.Status = (i == 0) ? EDir_AgentStatus::Active : EDir_AgentStatus::Idle;
        NewTask.CompletionPercentage = 0.0f;
        NewTask.LastUpdate = FDateTime::Now();
        
        // Add dependencies (each agent depends on the previous one)
        if (i > 0)
        {
            NewTask.Dependencies.Add(AgentDefinitions[i-1].Key);
        }
        
        AgentTasks.Add(NewTask);
        AgentStatusMap.Add(NewTask.AgentName, NewTask.Status);
    }
    
    LogProductionEvent(FString::Printf(TEXT("Initialized %d agents in production pipeline"), AgentTasks.Num()));
}

void UProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, float CompletionPercentage)
{
    // Find and update the agent task
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            Task.CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
            Task.LastUpdate = FDateTime::Now();
            break;
        }
    }
    
    // Update status map
    AgentStatusMap.Add(AgentName, NewStatus);
    
    LogProductionEvent(FString::Printf(TEXT("Agent %s status updated to %d with %.1f%% completion"), 
        *AgentName, (int32)NewStatus, CompletionPercentage));
    
    // Check if we can advance the production phase
    if (CanAdvancePhase())
    {
        AdvanceToNextPhase();
    }
}

void UProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, const TArray<FString>& Dependencies)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Idle;
    NewTask.CompletionPercentage = 0.0f;
    NewTask.LastUpdate = FDateTime::Now();
    NewTask.Dependencies = Dependencies;
    
    AgentTasks.Add(NewTask);
    AgentStatusMap.Add(AgentName, EDir_AgentStatus::Idle);
    
    LogProductionEvent(FString::Printf(TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription));
}

EDir_AgentStatus UProductionCoordinator::GetAgentStatus(const FString& AgentName) const
{
    if (const EDir_AgentStatus* Status = AgentStatusMap.Find(AgentName))
    {
        return *Status;
    }
    return EDir_AgentStatus::Error;
}

TArray<FString> UProductionCoordinator::GetBlockedAgents() const
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(Task.AgentName);
        }
    }
    
    return BlockedAgents;
}

void UProductionCoordinator::AdvanceToNextPhase()
{
    EDir_ProductionPhase NextPhase = CurrentPhase;
    
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            NextPhase = EDir_ProductionPhase::Prototype;
            break;
        case EDir_ProductionPhase::Prototype:
            NextPhase = EDir_ProductionPhase::Alpha;
            break;
        case EDir_ProductionPhase::Alpha:
            NextPhase = EDir_ProductionPhase::Beta;
            break;
        case EDir_ProductionPhase::Beta:
            NextPhase = EDir_ProductionPhase::Release;
            break;
        case EDir_ProductionPhase::Release:
            // Already at final phase
            return;
    }
    
    CurrentPhase = NextPhase;
    PhaseProgress = 0.0f;
    
    LogProductionEvent(FString::Printf(TEXT("Advanced to production phase: %s"), *GetCurrentPhaseDescription()));
}

bool UProductionCoordinator::CanAdvancePhase() const
{
    // Check if all agents in current phase are complete
    int32 CompletedAgents = 0;
    int32 TotalAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status != EDir_AgentStatus::Idle)
        {
            TotalAgents++;
            if (Task.Status == EDir_AgentStatus::Complete)
            {
                CompletedAgents++;
            }
        }
    }
    
    // Require 80% completion to advance phase
    return TotalAgents > 0 && (float(CompletedAgents) / float(TotalAgents)) >= 0.8f;
}

FString UProductionCoordinator::GetCurrentPhaseDescription() const
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            return TEXT("Pre-Production: Planning and Architecture");
        case EDir_ProductionPhase::Prototype:
            return TEXT("Prototype: Core Systems and Playable Build");
        case EDir_ProductionPhase::Alpha:
            return TEXT("Alpha: Feature Complete with Polish");
        case EDir_ProductionPhase::Beta:
            return TEXT("Beta: Testing and Bug Fixes");
        case EDir_ProductionPhase::Release:
            return TEXT("Release: Final Build and Distribution");
        default:
            return TEXT("Unknown Phase");
    }
}

void UProductionCoordinator::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    CurrentMetrics.TotalActors = AllActors.Num();
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.CharacterCount = 0;
    CurrentMetrics.TerrainElements = 0;
    
    // Categorize actors
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("Dino")))
        {
            CurrentMetrics.DinosaurCount++;
        }
        else if (Actor && Actor->GetActorLabel().Contains(TEXT("Character")))
        {
            CurrentMetrics.CharacterCount++;
        }
        else if (Actor && Actor->GetActorLabel().Contains(TEXT("Landscape")))
        {
            CurrentMetrics.TerrainElements++;
        }
    }
    
    // Calculate overall progress
    CurrentMetrics.OverallProgress = GetOverallProgress();
    CurrentMetrics.LastMetricsUpdate = FDateTime::Now();
    
    // Add to history
    MetricsHistory.Add(CurrentMetrics);
    
    // Keep only last 100 metrics entries
    if (MetricsHistory.Num() > 100)
    {
        MetricsHistory.RemoveAt(0, MetricsHistory.Num() - 100);
    }
}

float UProductionCoordinator::GetOverallProgress() const
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TotalProgress += Task.CompletionPercentage;
    }
    
    return TotalProgress / float(AgentTasks.Num());
}

void UProductionCoordinator::GenerateProgressReport()
{
    FString Report = TEXT("=== PRODUCTION PROGRESS REPORT ===\n");
    Report += FString::Printf(TEXT("Current Phase: %s (%.1f%% complete)\n"), *GetCurrentPhaseDescription(), PhaseProgress);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), GetOverallProgress());
    Report += FString::Printf(TEXT("Total Actors: %d\n"), CurrentMetrics.TotalActors);
    Report += FString::Printf(TEXT("Dinosaurs: %d\n"), CurrentMetrics.DinosaurCount);
    Report += FString::Printf(TEXT("Characters: %d\n"), CurrentMetrics.CharacterCount);
    Report += TEXT("\nAgent Status:\n");
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusText;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusText = TEXT("IDLE"); break;
            case EDir_AgentStatus::Active: StatusText = TEXT("ACTIVE"); break;
            case EDir_AgentStatus::Complete: StatusText = TEXT("COMPLETE"); break;
            case EDir_AgentStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Error: StatusText = TEXT("ERROR"); break;
        }
        
        Report += FString::Printf(TEXT("  %s: %s (%.1f%%)\n"), *Task.AgentName, *StatusText, Task.CompletionPercentage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, Report);
    }
}

void UProductionCoordinator::InitializeAgentTasks()
{
    InitializeDefaultAgents();
    UpdateProductionMetrics();
    GenerateProgressReport();
}

void UProductionCoordinator::ResetProductionState()
{
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    PhaseProgress = 0.0f;
    AgentTasks.Empty();
    AgentStatusMap.Empty();
    MetricsHistory.Empty();
    
    InitializeDefaultAgents();
    LogProductionEvent(TEXT("Production state reset to initial configuration"));
}

void UProductionCoordinator::PrintProductionStatus()
{
    GenerateProgressReport();
}

void UProductionCoordinator::CheckDependencies()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle)
        {
            bool CanStart = true;
            
            // Check if all dependencies are complete
            for (const FString& Dependency : Task.Dependencies)
            {
                EDir_AgentStatus DepStatus = GetAgentStatus(Dependency);
                if (DepStatus != EDir_AgentStatus::Complete)
                {
                    CanStart = false;
                    break;
                }
            }
            
            // Activate the agent if dependencies are met
            if (CanStart)
            {
                Task.Status = EDir_AgentStatus::Active;
                AgentStatusMap.Add(Task.AgentName, EDir_AgentStatus::Active);
                LogProductionEvent(FString::Printf(TEXT("Agent %s activated - dependencies satisfied"), *Task.AgentName));
            }
        }
    }
}

void UProductionCoordinator::UpdatePhaseProgress()
{
    float Progress = GetOverallProgress();
    PhaseProgress = Progress;
    
    // Phase-specific progress calculations could be added here
}

void UProductionCoordinator::LogProductionEvent(const FString& Event)
{
    FString LogMessage = FString::Printf(TEXT("[ProductionCoordinator] %s"), *Event);
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, LogMessage);
    }
}