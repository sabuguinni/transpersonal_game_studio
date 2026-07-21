#include "Dir_ProductionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UDir_ProductionManager::UDir_ProductionManager()
{
    CurrentPhase = EDir_ProductionPhase::Prototype;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260602_005");
    CycleNumber = 5;
    bMilestone1Blocked = false;
}

void UDir_ProductionManager::InitializeProduction()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing Production Manager for Cycle %d"), CycleNumber);
    
    SetupMilestone1();
    SetupAgentDependencies();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Manager initialized with %d milestones and %d agent tasks"), 
           Milestones.Num(), AgentTasks.Num());
}

void UDir_ProductionManager::SetupMilestone1()
{
    // Create Milestone 1: "Walk Around" Prototype
    FDir_MilestoneStatus Milestone1;
    Milestone1.MilestoneName = TEXT("Walk Around Prototype");
    Milestone1.bIsComplete = false;
    Milestone1.CompletionPercentage = 0.0f;
    
    // Define required features for Milestone 1
    Milestone1.RequiredFeatures.Add(TEXT("ThirdPersonCharacter with WASD movement"));
    Milestone1.RequiredFeatures.Add(TEXT("Camera boom + follow camera"));
    Milestone1.RequiredFeatures.Add(TEXT("Landscape with basic terrain"));
    Milestone1.RequiredFeatures.Add(TEXT("Player can walk, run, jump"));
    Milestone1.RequiredFeatures.Add(TEXT("3-5 static dinosaur meshes in world"));
    Milestone1.RequiredFeatures.Add(TEXT("Directional light + sky + fog"));
    
    Milestones.Add(Milestone1);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 setup complete with %d required features"), 
           Milestone1.RequiredFeatures.Num());
}

void UDir_ProductionManager::SetupAgentDependencies()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Agent #2 - Engine Architect (Critical Path)
    FDir_AgentTask Task2;
    Task2.AgentNumber = 2;
    Task2.AgentName = TEXT("Engine Architect");
    Task2.TaskDescription = TEXT("Define character movement architecture and UE5 class structure");
    Task2.Status = EDir_AgentStatus::Working;
    Task2.Priority = 10.0f;
    Task2.ExpectedDeliverable = TEXT("Character movement system architecture");
    AgentTasks.Add(Task2);
    
    // Agent #3 - Core Systems (Depends on #2)
    FDir_AgentTask Task3;
    Task3.AgentNumber = 3;
    Task3.AgentName = TEXT("Core Systems Programmer");
    Task3.TaskDescription = TEXT("Implement ThirdPersonCharacter with movement components");
    Task3.Status = EDir_AgentStatus::Idle;
    Task3.Priority = 9.0f;
    Task3.DependsOnAgents.Add(2);
    Task3.ExpectedDeliverable = TEXT("Working ThirdPersonCharacter class");
    AgentTasks.Add(Task3);
    
    // Agent #5 - Procedural World Generator (Critical Path)
    FDir_AgentTask Task5;
    Task5.AgentNumber = 5;
    Task5.AgentName = TEXT("Procedural World Generator");
    Task5.TaskDescription = TEXT("Create landscape with height variation and basic terrain");
    Task5.Status = EDir_AgentStatus::Working;
    Task5.Priority = 9.0f;
    Task5.ExpectedDeliverable = TEXT("Playable landscape with hills and valleys");
    AgentTasks.Add(Task5);
    
    // Agent #8 - Lighting & Atmosphere (Depends on #5)
    FDir_AgentTask Task8;
    Task8.AgentNumber = 8;
    Task8.AgentName = TEXT("Lighting & Atmosphere");
    Task8.TaskDescription = TEXT("Setup directional light, sky atmosphere, and fog");
    Task8.Status = EDir_AgentStatus::Idle;
    Task8.Priority = 7.0f;
    Task8.DependsOnAgents.Add(5);
    Task8.ExpectedDeliverable = TEXT("Complete lighting setup");
    AgentTasks.Add(Task8);
    
    // Agent #9 - Character Artist (Depends on #3)
    FDir_AgentTask Task9;
    Task9.AgentNumber = 9;
    Task9.AgentName = TEXT("Character Artist");
    Task9.TaskDescription = TEXT("Place 3-5 dinosaur placeholder meshes in world");
    Task9.Status = EDir_AgentStatus::Idle;
    Task9.Priority = 8.0f;
    Task9.DependsOnAgents.Add(3);
    Task9.ExpectedDeliverable = TEXT("Visible dinosaur actors in MinPlayableMap");
    AgentTasks.Add(Task9);
    
    // Agent #10 - Animation (Depends on #9)
    FDir_AgentTask Task10;
    Task10.AgentNumber = 10;
    Task10.AgentName = TEXT("Animation Agent");
    Task10.TaskDescription = TEXT("Add basic movement animations to character");
    Task10.Status = EDir_AgentStatus::Idle;
    Task10.Priority = 6.0f;
    Task10.DependsOnAgents.Add(9);
    Task10.ExpectedDeliverable = TEXT("Character with walk/run/jump animations");
    AgentTasks.Add(Task10);
    
    // Define critical path
    CriticalPathAgents.Empty();
    CriticalPathAgents.Add(2); // Engine Architect
    CriticalPathAgents.Add(3); // Core Systems
    CriticalPathAgents.Add(5); // World Generator
    CriticalPathAgents.Add(9); // Character Artist
    
    UE_LOG(LogTemp, Warning, TEXT("Agent dependencies setup complete. Critical path has %d agents"), 
           CriticalPathAgents.Num());
}

void UDir_ProductionManager::UpdateProductionMetrics()
{
    // Get current world
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot update metrics - no valid world"));
        return;
    }
    
    // Count actors in the world
    CurrentMetrics.TotalActorsInMap = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.CharacterCount = 0;
    CurrentMetrics.EnvironmentActors = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            CurrentMetrics.TotalActorsInMap++;
            
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
            {
                CurrentMetrics.CharacterCount++;
            }
            else if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
                     ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")))
            {
                CurrentMetrics.DinosaurCount++;
            }
            else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("rock")) || 
                     ActorName.Contains(TEXT("landscape")))
            {
                CurrentMetrics.EnvironmentActors++;
            }
        }
    }
    
    // Update task counts
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.PendingTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            CurrentMetrics.CompletedTasks++;
        }
        else
        {
            CurrentMetrics.PendingTasks++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Metrics updated - Actors: %d, Dinosaurs: %d, Characters: %d"), 
           CurrentMetrics.TotalActorsInMap, CurrentMetrics.DinosaurCount, CurrentMetrics.CharacterCount);
}

void UDir_ProductionManager::AssignAgentTask(int32 AgentNumber, const FString& TaskDescription, float Priority)
{
    // Find existing task for this agent
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.TaskDescription = TaskDescription;
            Task.Priority = Priority;
            Task.Status = EDir_AgentStatus::Working;
            UE_LOG(LogTemp, Warning, TEXT("Updated task for Agent #%d: %s"), AgentNumber, *TaskDescription);
            return;
        }
    }
    
    // Create new task if not found
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Assigned new task to Agent #%d: %s"), AgentNumber, *TaskDescription);
}

void UDir_ProductionManager::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d status updated to: %d"), AgentNumber, (int32)NewStatus);
            
            // Check if this affects critical path
            if (CriticalPathAgents.Contains(AgentNumber) && NewStatus == EDir_AgentStatus::Failed)
            {
                bMilestone1Blocked = true;
                UE_LOG(LogTemp, Error, TEXT("CRITICAL: Agent #%d failed - Milestone 1 blocked!"), AgentNumber);
            }
            break;
        }
    }
}

bool UDir_ProductionManager::CheckMilestoneCompletion(const FString& MilestoneName)
{
    for (FDir_MilestoneStatus& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            // For Milestone 1, check specific requirements
            if (MilestoneName == TEXT("Walk Around Prototype"))
            {
                return ValidateWalkAroundPrototype();
            }
            
            // Generic completion check
            return Milestone.CompletedFeatures.Num() >= Milestone.RequiredFeatures.Num();
        }
    }
    
    return false;
}

bool UDir_ProductionManager::ValidateWalkAroundPrototype()
{
    UpdateProductionMetrics();
    
    // Check requirements for Milestone 1
    bool bHasCharacter = CurrentMetrics.CharacterCount > 0;
    bool bHasLandscape = CurrentMetrics.EnvironmentActors > 0;
    bool bHasDinosaurs = CurrentMetrics.DinosaurCount >= 3;
    bool bHasActors = CurrentMetrics.TotalActorsInMap > 10;
    
    bool bMilestone1Complete = bHasCharacter && bHasLandscape && bHasDinosaurs && bHasActors;
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Validation - Character: %s, Landscape: %s, Dinosaurs: %s, Complete: %s"),
           bHasCharacter ? TEXT("YES") : TEXT("NO"),
           bHasLandscape ? TEXT("YES") : TEXT("NO"),
           bHasDinosaurs ? TEXT("YES") : TEXT("NO"),
           bMilestone1Complete ? TEXT("YES") : TEXT("NO"));
    
    return bMilestone1Complete;
}

TArray<FString> UDir_ProductionManager::GetMilestone1Requirements()
{
    TArray<FString> Requirements;
    Requirements.Add(TEXT("ThirdPersonCharacter with WASD movement"));
    Requirements.Add(TEXT("Camera boom + follow camera"));
    Requirements.Add(TEXT("Landscape with basic terrain"));
    Requirements.Add(TEXT("Player can walk, run, jump"));
    Requirements.Add(TEXT("3-5 static dinosaur meshes in world"));
    Requirements.Add(TEXT("Directional light + sky + fog"));
    
    return Requirements;
}

void UDir_ProductionManager::CheckCriticalPath()
{
    bMilestone1Blocked = false;
    
    for (int32 AgentNumber : CriticalPathAgents)
    {
        for (const FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.AgentNumber == AgentNumber && 
                (Task.Status == EDir_AgentStatus::Failed || Task.Status == EDir_AgentStatus::Blocked))
            {
                bMilestone1Blocked = true;
                UE_LOG(LogTemp, Error, TEXT("Critical path blocked by Agent #%d"), AgentNumber);
                break;
            }
        }
    }
}

TArray<int32> UDir_ProductionManager::GetBlockedAgents()
{
    TArray<int32> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || Task.Status == EDir_AgentStatus::Failed)
        {
            BlockedAgents.Add(Task.AgentNumber);
        }
    }
    
    return BlockedAgents;
}

float UDir_ProductionManager::GetOverallProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    float CompletedWeight = 0.0f;
    float TotalWeight = 0.0f;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TotalWeight += Task.Priority;
        
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            CompletedWeight += Task.Priority;
        }
        else if (Task.Status == EDir_AgentStatus::Working)
        {
            CompletedWeight += Task.Priority * 0.5f; // 50% credit for in-progress
        }
    }
    
    return (TotalWeight > 0.0f) ? (CompletedWeight / TotalWeight) * 100.0f : 0.0f;
}

void UDir_ProductionManager::AdvanceToNextPhase()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::Prototype;
            break;
        case EDir_ProductionPhase::Prototype:
            CurrentPhase = EDir_ProductionPhase::Production;
            break;
        case EDir_ProductionPhase::Production:
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
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced to production phase: %d"), (int32)CurrentPhase);
}