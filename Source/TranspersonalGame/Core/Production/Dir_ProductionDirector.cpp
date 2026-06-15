#include "Dir_ProductionDirector.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"

UDir_ProductionDirector::UDir_ProductionDirector()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    CurrentPhase = EDir_ProductionPhase::Prototype;
    LastMetricsUpdate = 0.0f;
    bProductionActive = true;
    
    // Initialize default metrics
    CurrentMetrics = FDir_ProductionMetrics();
}

void UDir_ProductionDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentTasks();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Director initialized - Phase: %d"), (int32)CurrentPhase);
}

void UDir_ProductionDirector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bProductionActive) return;
    
    LastMetricsUpdate += DeltaTime;
    
    // Update metrics every 5 seconds
    if (LastMetricsUpdate >= 5.0f)
    {
        UpdateProductionMetrics();
        MonitorPerformanceMetrics();
        EnforceProductionRules();
        LastMetricsUpdate = 0.0f;
    }
    
    // Coordinate agent chain every 10 seconds
    static float ChainCoordinationTimer = 0.0f;
    ChainCoordinationTimer += DeltaTime;
    if (ChainCoordinationTimer >= 10.0f)
    {
        CoordinateAgentChain();
        ChainCoordinationTimer = 0.0f;
    }
}

void UDir_ProductionDirector::InitializeAgentTasks()
{
    AgentTasks.Empty();
    
    // Initialize all 19 agents with their core tasks
    TArray<FString> AgentNames = {
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
    
    TArray<FString> TaskDescriptions = {
        TEXT("Define core architecture and technical standards"),
        TEXT("Implement physics, collision, and destruction systems"),
        TEXT("Optimize for 60fps PC / 30fps console performance"),
        TEXT("Generate terrain, biomes, and world structure"),
        TEXT("Populate world with vegetation and environmental assets"),
        TEXT("Build prehistoric structures and interiors"),
        TEXT("Implement lighting, weather, and atmosphere"),
        TEXT("Create player and NPC character models"),
        TEXT("Implement character animations and motion systems"),
        TEXT("Design NPC behavior trees and AI routines"),
        TEXT("Implement combat AI for dinosaurs and enemies"),
        TEXT("Create crowd simulation for large groups"),
        TEXT("Design quests and mission objectives"),
        TEXT("Write narrative content and dialogue"),
        TEXT("Implement adaptive audio and sound effects"),
        TEXT("Create visual effects using Niagara"),
        TEXT("Test and validate all systems"),
        TEXT("Integrate all components into final build")
    };
    
    for (int32 i = 0; i < AgentNames.Num() && i < TaskDescriptions.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentNames[i];
        NewTask.TaskDescription = TaskDescriptions[i];
        NewTask.Status = EDir_AgentStatus::Working;
        NewTask.Priority = (i < 5) ? 10.0f : 5.0f; // Higher priority for core systems
        NewTask.CompletionPercentage = FMath::RandRange(15.0f, 45.0f); // Simulated progress
        NewTask.StartTime = FDateTime::Now();
        NewTask.EstimatedCompletion = FDateTime::Now() + FTimespan::FromHours(FMath::RandRange(2, 8));
        
        AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agent tasks"), AgentTasks.Num());
}

void UDir_ProductionDirector::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    // Find existing agent or create new task
    FDir_AgentTask* ExistingTask = AgentTasks.FindByPredicate([&AgentName](const FDir_AgentTask& Task)
    {
        return Task.AgentName == AgentName;
    });
    
    if (ExistingTask)
    {
        ExistingTask->TaskDescription = TaskDescription;
        ExistingTask->Priority = Priority;
        ExistingTask->Status = EDir_AgentStatus::Working;
        ExistingTask->StartTime = FDateTime::Now();
    }
    else
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentName;
        NewTask.TaskDescription = TaskDescription;
        NewTask.Priority = Priority;
        NewTask.Status = EDir_AgentStatus::Working;
        NewTask.StartTime = FDateTime::Now();
        NewTask.EstimatedCompletion = FDateTime::Now() + FTimespan::FromHours(4);
        
        AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Assigned task to %s: %s"), *AgentName, *TaskDescription);
}

void UDir_ProductionDirector::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, float CompletionPercentage)
{
    FDir_AgentTask* Task = AgentTasks.FindByPredicate([&AgentName](const FDir_AgentTask& T)
    {
        return T.AgentName == AgentName;
    });
    
    if (Task)
    {
        Task->Status = NewStatus;
        Task->CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Updated %s status to %d, completion: %.1f%%"), 
               *AgentName, (int32)NewStatus, CompletionPercentage);
    }
}

void UDir_ProductionDirector::UpdateProductionMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Count actors in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    CurrentMetrics.TotalActors = AllActors.Num();
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.CharacterCount = 0;
    CurrentMetrics.TerrainActors = 0;
    
    // Categorize actors
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        if (ActorName.Contains(TEXT("dino")) || ActorLabel.Contains(TEXT("dino")) ||
            ActorName.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("trex")) ||
            ActorName.Contains(TEXT("raptor")) || ActorLabel.Contains(TEXT("raptor")))
        {
            CurrentMetrics.DinosaurCount++;
        }
        else if (ActorName.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("character")))
        {
            CurrentMetrics.CharacterCount++;
        }
        else if (ActorName.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("landscape")) ||
                 ActorName.Contains(TEXT("terrain")) || ActorLabel.Contains(TEXT("terrain")))
        {
            CurrentMetrics.TerrainActors++;
        }
    }
    
    // Calculate overall progress
    CalculateOverallProgress();
    
    // Get performance metrics
    CurrentMetrics.FrameRate = 1.0f / World->GetDeltaSeconds();
    CurrentMetrics.MemoryUsage = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f * 1024.0f); // GB
    
    UE_LOG(LogTemp, Log, TEXT("Metrics Updated - Actors: %d, Dinos: %d, Characters: %d, FPS: %.1f"), 
           CurrentMetrics.TotalActors, CurrentMetrics.DinosaurCount, 
           CurrentMetrics.CharacterCount, CurrentMetrics.FrameRate);
}

void UDir_ProductionDirector::CalculateOverallProgress()
{
    if (AgentTasks.Num() == 0)
    {
        CurrentMetrics.OverallProgress = 0.0f;
        return;
    }
    
    float TotalProgress = 0.0f;
    float TotalWeight = 0.0f;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        float Weight = Task.Priority;
        TotalProgress += Task.CompletionPercentage * Weight;
        TotalWeight += Weight;
    }
    
    CurrentMetrics.OverallProgress = (TotalWeight > 0.0f) ? (TotalProgress / TotalWeight) : 0.0f;
}

void UDir_ProductionDirector::MonitorPerformanceMetrics()
{
    // Check for performance issues
    if (CurrentMetrics.FrameRate < 30.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: FPS below 30 (%.1f)"), CurrentMetrics.FrameRate);
        
        // Automatically assign optimization task
        AssignTaskToAgent(TEXT("Performance Optimizer"), 
                         TEXT("Critical: FPS below 30, immediate optimization required"), 
                         15.0f);
    }
    
    if (CurrentMetrics.TotalActors > 8000)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor Count Warning: %d actors (limit: 8000)"), CurrentMetrics.TotalActors);
    }
    
    if (CurrentMetrics.MemoryUsage > 8.0f) // 8GB limit
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory Warning: %.2f GB used"), CurrentMetrics.MemoryUsage);
    }
}

void UDir_ProductionDirector::EnforceProductionRules()
{
    // Rule 1: No agent can advance if core systems aren't ready
    bool bCoreSystemsReady = false;
    FDir_AgentTask* CoreTask = AgentTasks.FindByPredicate([](const FDir_AgentTask& Task)
    {
        return Task.AgentName == TEXT("Core Systems Programmer");
    });
    
    if (CoreTask && CoreTask->CompletionPercentage >= 50.0f)
    {
        bCoreSystemsReady = true;
    }
    
    // Rule 2: Block creative agents if narrative isn't approved
    bool bNarrativeReady = false;
    FDir_AgentTask* NarrativeTask = AgentTasks.FindByPredicate([](const FDir_AgentTask& Task)
    {
        return Task.AgentName == TEXT("Narrative & Dialogue Agent");
    });
    
    if (NarrativeTask && NarrativeTask->CompletionPercentage >= 30.0f)
    {
        bNarrativeReady = true;
    }
    
    // Apply blocks if necessary
    TArray<FString> CreativeAgents = {
        TEXT("Environment Artist"),
        TEXT("Character Artist Agent"),
        TEXT("Quest & Mission Designer")
    };
    
    for (const FString& AgentName : CreativeAgents)
    {
        if (!bNarrativeReady)
        {
            UpdateAgentStatus(AgentName, EDir_AgentStatus::Blocked, 0.0f);
        }
    }
}

void UDir_ProductionDirector::CoordinateAgentChain()
{
    // Implement agent dependency chain coordination
    TArray<FString> ChainOrder = {
        TEXT("Engine Architect"),
        TEXT("Core Systems Programmer"),
        TEXT("Procedural World Generator"),
        TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"),
        TEXT("Lighting & Atmosphere Agent"),
        TEXT("Character Artist Agent"),
        TEXT("Animation Agent"),
        TEXT("NPC Behavior Agent"),
        TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"),
        TEXT("Narrative & Dialogue Agent"),
        TEXT("Quest & Mission Designer"),
        TEXT("Audio Agent"),
        TEXT("VFX Agent"),
        TEXT("QA & Testing Agent"),
        TEXT("Integration & Build Agent")
    };
    
    // Check dependencies and unblock agents when prerequisites are met
    for (int32 i = 1; i < ChainOrder.Num(); i++)
    {
        FDir_AgentTask* CurrentTask = AgentTasks.FindByPredicate([&ChainOrder, i](const FDir_AgentTask& Task)
        {
            return Task.AgentName == ChainOrder[i];
        });
        
        FDir_AgentTask* PreviousTask = AgentTasks.FindByPredicate([&ChainOrder, i](const FDir_AgentTask& Task)
        {
            return Task.AgentName == ChainOrder[i-1];
        });
        
        if (CurrentTask && PreviousTask)
        {
            // Unblock if previous agent is 70% complete
            if (PreviousTask->CompletionPercentage >= 70.0f && CurrentTask->Status == EDir_AgentStatus::Blocked)
            {
                CurrentTask->Status = EDir_AgentStatus::Working;
                UE_LOG(LogTemp, Log, TEXT("Unblocked %s (prerequisite %s at %.1f%%)"), 
                       *CurrentTask->AgentName, *PreviousTask->AgentName, PreviousTask->CompletionPercentage);
            }
        }
    }
}

void UDir_ProductionDirector::ValidateProductionState()
{
    int32 WorkingAgents = 0;
    int32 BlockedAgents = 0;
    int32 CompletedAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Working:
                WorkingAgents++;
                break;
            case EDir_AgentStatus::Blocked:
                BlockedAgents++;
                break;
            case EDir_AgentStatus::Complete:
                CompletedAgents++;
                break;
            default:
                break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production State - Working: %d, Blocked: %d, Complete: %d"), 
           WorkingAgents, BlockedAgents, CompletedAgents);
}

void UDir_ProductionDirector::AdvanceProductionPhase()
{
    float Progress = GetOverallProgress();
    
    EDir_ProductionPhase NewPhase = CurrentPhase;
    
    if (Progress >= 90.0f && CurrentPhase == EDir_ProductionPhase::Beta)
    {
        NewPhase = EDir_ProductionPhase::Gold;
    }
    else if (Progress >= 75.0f && CurrentPhase == EDir_ProductionPhase::Alpha)
    {
        NewPhase = EDir_ProductionPhase::Beta;
    }
    else if (Progress >= 50.0f && CurrentPhase == EDir_ProductionPhase::Prototype)
    {
        NewPhase = EDir_ProductionPhase::Alpha;
    }
    else if (Progress >= 25.0f && CurrentPhase == EDir_ProductionPhase::PreProduction)
    {
        NewPhase = EDir_ProductionPhase::Prototype;
    }
    
    if (NewPhase != CurrentPhase)
    {
        CurrentPhase = NewPhase;
        UE_LOG(LogTemp, Warning, TEXT("Production Phase Advanced to: %d"), (int32)CurrentPhase);
    }
}

FDir_ProductionMetrics UDir_ProductionDirector::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

TArray<FDir_AgentTask> UDir_ProductionDirector::GetAgentTasks() const
{
    return AgentTasks;
}

float UDir_ProductionDirector::GetOverallProgress() const
{
    return CurrentMetrics.OverallProgress;
}

TArray<FString> UDir_ProductionDirector::GetBlockedAgents() const
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

void UDir_ProductionDirector::ResolveBlockedAgent(const FString& AgentName)
{
    FDir_AgentTask* Task = AgentTasks.FindByPredicate([&AgentName](const FDir_AgentTask& T)
    {
        return T.AgentName == AgentName;
    });
    
    if (Task && Task->Status == EDir_AgentStatus::Blocked)
    {
        Task->Status = EDir_AgentStatus::Working;
        UE_LOG(LogTemp, Warning, TEXT("Manually resolved blocked agent: %s"), *AgentName);
    }
}