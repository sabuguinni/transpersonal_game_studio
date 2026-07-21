#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ActorComponent.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    CurrentMilestone = EDir_ProductionMilestone::WalkAround;
    InitializeAgentTasks();
    InitializeCriticalSystems();
}

void UDir_ProductionCoordinator::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing Production Pipeline"));
    
    // Reset all metrics
    ProductionMetrics = FDir_ProductionMetrics();
    ProductionMetrics.CurrentMilestone = EDir_ProductionMilestone::WalkAround;
    
    // Initialize agent tasks for Milestone 1
    InitializeAgentTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Pipeline initialized with %d agents"), ProductionMetrics.TotalAgents);
}

bool UDir_ProductionCoordinator::ValidateMilestone(EDir_ProductionMilestone Milestone)
{
    switch (Milestone)
    {
        case EDir_ProductionMilestone::WalkAround:
            return CheckMilestone1Requirements();
        case EDir_ProductionMilestone::BasicSurvival:
            return CheckMilestone2Requirements();
        case EDir_ProductionMilestone::DinosaurEncounters:
            return CheckMilestone3Requirements();
        default:
            return false;
    }
}

void UDir_ProductionCoordinator::AdvanceToNextMilestone()
{
    if (!ValidateMilestone(CurrentMilestone))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot advance milestone - current milestone not complete"));
        return;
    }
    
    switch (CurrentMilestone)
    {
        case EDir_ProductionMilestone::WalkAround:
            CurrentMilestone = EDir_ProductionMilestone::BasicSurvival;
            UE_LOG(LogTemp, Warning, TEXT("Advanced to Milestone 2: Basic Survival"));
            break;
        case EDir_ProductionMilestone::BasicSurvival:
            CurrentMilestone = EDir_ProductionMilestone::DinosaurEncounters;
            UE_LOG(LogTemp, Warning, TEXT("Advanced to Milestone 3: Dinosaur Encounters"));
            break;
        case EDir_ProductionMilestone::DinosaurEncounters:
            CurrentMilestone = EDir_ProductionMilestone::FullGameplay;
            UE_LOG(LogTemp, Warning, TEXT("Advanced to Milestone 4: Full Gameplay"));
            break;
        default:
            break;
    }
    
    ProductionMetrics.CurrentMilestone = CurrentMilestone;
}

float UDir_ProductionCoordinator::CalculateMilestoneProgress()
{
    int32 CompletedTasks = 0;
    int32 TotalTasks = AgentTasks.Num();
    
    for (const auto& TaskPair : AgentTasks)
    {
        if (TaskPair.Value.Status == EDir_AgentStatus::Complete)
        {
            CompletedTasks++;
        }
    }
    
    float Progress = TotalTasks > 0 ? (float)CompletedTasks / (float)TotalTasks : 0.0f;
    ProductionMetrics.MilestoneProgress = Progress;
    
    return Progress;
}

void UDir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task)
{
    AgentTasks.Add(AgentName, Task);
    AgentStatusMap.Add(AgentName, Task.Status);
    
    UE_LOG(LogTemp, Warning, TEXT("Assigned task to %s: %s"), *AgentName, *Task.TaskDescription);
}

void UDir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    if (AgentStatusMap.Contains(AgentName))
    {
        AgentStatusMap[AgentName] = NewStatus;
        
        if (AgentTasks.Contains(AgentName))
        {
            AgentTasks[AgentName].Status = NewStatus;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Updated %s status to %d"), *AgentName, (int32)NewStatus);
    }
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetTasksByPriority()
{
    TArray<FDir_AgentTask> SortedTasks;
    
    for (const auto& TaskPair : AgentTasks)
    {
        SortedTasks.Add(TaskPair.Value);
    }
    
    SortedTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    return SortedTasks;
}

TArray<FString> UDir_ProductionCoordinator::GetBlockedAgents()
{
    TArray<FString> BlockedAgents;
    
    for (const auto& StatusPair : AgentStatusMap)
    {
        if (StatusPair.Value == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(StatusPair.Key);
        }
    }
    
    return BlockedAgents;
}

FDir_ProductionMetrics UDir_ProductionCoordinator::GetProductionMetrics()
{
    // Update metrics
    ProductionMetrics.ActiveAgents = 0;
    ProductionMetrics.CompletedTasks = 0;
    ProductionMetrics.BlockedTasks = 0;
    
    for (const auto& StatusPair : AgentStatusMap)
    {
        switch (StatusPair.Value)
        {
            case EDir_AgentStatus::Working:
                ProductionMetrics.ActiveAgents++;
                break;
            case EDir_AgentStatus::Complete:
                ProductionMetrics.CompletedTasks++;
                break;
            case EDir_AgentStatus::Blocked:
                ProductionMetrics.BlockedTasks++;
                break;
            default:
                break;
        }
    }
    
    ProductionMetrics.MilestoneProgress = CalculateMilestoneProgress();
    
    return ProductionMetrics;
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    FDir_ProductionMetrics Metrics = GetProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Milestone: %d"), (int32)Metrics.CurrentMilestone);
    UE_LOG(LogTemp, Warning, TEXT("Milestone Progress: %.1f%%"), Metrics.MilestoneProgress * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d/%d"), Metrics.ActiveAgents, Metrics.TotalAgents);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), Metrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked Tasks: %d"), Metrics.BlockedTasks);
    
    TArray<FString> BlockedAgents = GetBlockedAgents();
    if (BlockedAgents.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Blocked Agents: %s"), *FString::Join(BlockedAgents, TEXT(", ")));
    }
}

bool UDir_ProductionCoordinator::ValidateGameplayElements()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }
    
    // Check for critical gameplay elements
    bool bHasPlayerCharacter = false;
    bool bHasTerrain = false;
    bool bHasDinosaurs = false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ClassName = Actor->GetClass()->GetName();
        FString ActorName = Actor->GetName();
        
        if (ClassName.Contains(TEXT("Character")) || ClassName.Contains(TEXT("Pawn")))
        {
            bHasPlayerCharacter = true;
        }
        
        if (ClassName.Contains(TEXT("Landscape")) || ActorName.Contains(TEXT("Terrain")))
        {
            bHasTerrain = true;
        }
        
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")))
        {
            bHasDinosaurs = true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Gameplay Validation - Character: %s, Terrain: %s, Dinosaurs: %s"), 
           bHasPlayerCharacter ? TEXT("YES") : TEXT("NO"),
           bHasTerrain ? TEXT("YES") : TEXT("NO"),
           bHasDinosaurs ? TEXT("YES") : TEXT("NO"));
    
    return bHasPlayerCharacter && bHasTerrain && bHasDinosaurs;
}

TArray<FString> UDir_ProductionCoordinator::GetMissingCriticalSystems()
{
    TArray<FString> MissingSystems;
    
    if (!ValidateGameplayElements())
    {
        UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
        if (World)
        {
            bool bHasPlayerCharacter = false;
            bool bHasTerrain = false;
            bool bHasDinosaurs = false;
            
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (!Actor) continue;
                
                FString ClassName = Actor->GetClass()->GetName();
                FString ActorName = Actor->GetName();
                
                if (ClassName.Contains(TEXT("Character"))) bHasPlayerCharacter = true;
                if (ClassName.Contains(TEXT("Landscape"))) bHasTerrain = true;
                if (ActorName.Contains(TEXT("Dinosaur"))) bHasDinosaurs = true;
            }
            
            if (!bHasPlayerCharacter) MissingSystems.Add(TEXT("Player Character Controller"));
            if (!bHasTerrain) MissingSystems.Add(TEXT("Terrain System"));
            if (!bHasDinosaurs) MissingSystems.Add(TEXT("Dinosaur Actors"));
        }
    }
    
    return MissingSystems;
}

void UDir_ProductionCoordinator::InitializeAgentTasks()
{
    // Clear existing tasks
    AgentTasks.Empty();
    AgentStatusMap.Empty();
    
    // Milestone 1: Walk Around - Critical Path Tasks
    FDir_AgentTask Task;
    
    // Agent #02 - Engine Architect
    Task = FDir_AgentTask();
    Task.AgentName = TEXT("Agent_02_Engine_Architect");
    Task.TaskDescription = TEXT("Validate core architecture and establish technical foundation");
    Task.Priority = 10;
    Task.EstimatedHours = 2.0f;
    Task.Status = EDir_AgentStatus::Working;
    AssignTaskToAgent(Task.AgentName, Task);
    
    // Agent #03 - Core Systems
    Task = FDir_AgentTask();
    Task.AgentName = TEXT("Agent_03_Core_Systems");
    Task.TaskDescription = TEXT("Implement physics, collision, and movement systems");
    Task.Priority = 9;
    Task.EstimatedHours = 4.0f;
    Task.Dependencies.Add(TEXT("Agent_02_Engine_Architect"));
    AssignTaskToAgent(Task.AgentName, Task);
    
    // Agent #05 - World Generator
    Task = FDir_AgentTask();
    Task.AgentName = TEXT("Agent_05_World_Generator");
    Task.TaskDescription = TEXT("Create playable terrain with height variation"));
    Task.Priority = 8;
    Task.EstimatedHours = 3.0f;
    AssignTaskToAgent(Task.AgentName, Task);
    
    // Agent #09 - Character Artist
    Task = FDir_AgentTask();
    Task.AgentName = TEXT("Agent_09_Character_Artist");
    Task.TaskDescription = TEXT("Implement player character with WASD movement"));
    Task.Priority = 9;
    Task.EstimatedHours = 3.0f;
    Task.Dependencies.Add(TEXT("Agent_03_Core_Systems"));
    AssignTaskToAgent(Task.AgentName, Task);
    
    // Agent #10 - Animation
    Task = FDir_AgentTask();
    Task.AgentName = TEXT("Agent_10_Animation");
    Task.TaskDescription = TEXT("Add basic character and dinosaur animations"));
    Task.Priority = 7;
    Task.EstimatedHours = 4.0f;
    Task.Dependencies.Add(TEXT("Agent_09_Character_Artist"));
    AssignTaskToAgent(Task.AgentName, Task);
    
    // Agent #12 - Combat AI
    Task = FDir_AgentTask();
    Task.AgentName = TEXT("Agent_12_Combat_AI");
    Task.TaskDescription = TEXT("Place static dinosaur actors in world"));
    Task.Priority = 6;
    Task.EstimatedHours = 2.0f;
    Task.Dependencies.Add(TEXT("Agent_05_World_Generator"));
    AssignTaskToAgent(Task.AgentName, Task);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agent tasks for Milestone 1"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::InitializeCriticalSystems()
{
    CriticalSystemsChecklist.Empty();
    CriticalSystemsChecklist.Add(TEXT("Player Character Controller"));
    CriticalSystemsChecklist.Add(TEXT("Camera System"));
    CriticalSystemsChecklist.Add(TEXT("Movement Input"));
    CriticalSystemsChecklist.Add(TEXT("Terrain Generation"));
    CriticalSystemsChecklist.Add(TEXT("Collision Detection"));
    CriticalSystemsChecklist.Add(TEXT("Dinosaur Placement"));
    CriticalSystemsChecklist.Add(TEXT("Lighting System"));
    CriticalSystemsChecklist.Add(TEXT("Game Mode"));
}

bool UDir_ProductionCoordinator::CheckMilestone1Requirements()
{
    // Milestone 1: Walk Around
    // Requirements: Character movement, terrain, basic dinosaurs, lighting
    
    TArray<FString> MissingSystems = GetMissingCriticalSystems();
    bool bMilestone1Complete = MissingSystems.Num() == 0;
    
    if (!bMilestone1Complete)
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone 1 incomplete. Missing: %s"), 
               *FString::Join(MissingSystems, TEXT(", ")));
    }
    
    return bMilestone1Complete;
}

bool UDir_ProductionCoordinator::CheckMilestone2Requirements()
{
    // Milestone 2: Basic Survival
    // Requirements: Health/hunger/thirst systems, basic crafting, inventory
    
    if (!CheckMilestone1Requirements())
    {
        return false;
    }
    
    // Additional checks for survival systems would go here
    return true;
}

bool UDir_ProductionCoordinator::CheckMilestone3Requirements()
{
    // Milestone 3: Dinosaur Encounters
    // Requirements: AI behavior, combat system, dinosaur interactions
    
    if (!CheckMilestone2Requirements())
    {
        return false;
    }
    
    // Additional checks for combat systems would go here
    return true;
}