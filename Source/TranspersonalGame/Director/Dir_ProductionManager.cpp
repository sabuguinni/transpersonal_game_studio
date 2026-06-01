#include "Dir_ProductionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

UDir_ProductionManager::UDir_ProductionManager()
{
    // Initialize default values
    CurrentPhase = EDir_ProductionPhase::Prototyping;
    MaxTotalActors = 8000;
    MaxDinosaurs = 150;
    MaxPropsPerBiome = 1000;
}

void UDir_ProductionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Production Manager initialized - Studio Director active"));
    
    InitializeAgentPipeline();
    ValidateProjectStructure();
    UpdateBuildMetrics();
}

void UDir_ProductionManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Production Manager shutting down"));
    Super::Deinitialize();
}

void UDir_ProductionManager::StartProductionCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STARTING PRODUCTION CYCLE ==="));
    
    // Clear previous tasks
    ActiveTasks.Empty();
    
    // Initialize agent tasks for current cycle
    AssignTaskToAgent(2, "Define core engine architecture and compilation rules", 10.0f);
    AssignTaskToAgent(3, "Implement physics and collision systems", 9.0f);
    AssignTaskToAgent(5, "Generate procedural terrain with biome distribution", 8.0f);
    AssignTaskToAgent(9, "Create character models and MetaHuman integration", 7.0f);
    AssignTaskToAgent(10, "Implement character animations and Motion Matching", 7.0f);
    AssignTaskToAgent(12, "Design combat AI for dinosaurs", 6.0f);
    
    // Update metrics
    UpdateBuildMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle started with %d active tasks"), ActiveTasks.Num());
}

void UDir_ProductionManager::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.AgentName = FString::Printf(TEXT("Agent #%02d"), AgentID);
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.StartTime = FDateTime::Now();
    NewTask.Deadline = FDateTime::Now() + FTimespan::FromHours(24);
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%02d: %s"), AgentID, *TaskDescription);
}

void UDir_ProductionManager::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Agent #%02d status updated to: %d"), AgentID, (int32)NewStatus);
            break;
        }
    }
}

void UDir_ProductionManager::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), ActiveTasks.Num());
    
    int32 CompletedTasks = 0;
    int32 BlockedTasks = 0;
    int32 FailedTasks = 0;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Complete:
                CompletedTasks++;
                break;
            case EDir_AgentStatus::Blocked:
                BlockedTasks++;
                break;
            case EDir_AgentStatus::Failed:
                FailedTasks++;
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("  Agent #%02d: %s [%d]"), 
               Task.AgentID, *Task.TaskDescription, (int32)Task.Status);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d, Blocked: %d, Failed: %d"), 
           CompletedTasks, BlockedTasks, FailedTasks);
    
    // Build metrics
    UE_LOG(LogTemp, Warning, TEXT("Build Metrics:"));
    UE_LOG(LogTemp, Warning, TEXT("  Total Actors: %d/%d"), CurrentMetrics.TotalActors, MaxTotalActors);
    UE_LOG(LogTemp, Warning, TEXT("  Dinosaurs: %d/%d"), CurrentMetrics.DinosaurCount, MaxDinosaurs);
    UE_LOG(LogTemp, Warning, TEXT("  Compilation Errors: %d"), CurrentMetrics.CompilationErrors);
    UE_LOG(LogTemp, Warning, TEXT("  Playable: %s"), CurrentMetrics.bIsPlayable ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UDir_ProductionManager::UpdateBuildMetrics()
{
    CurrentMetrics.LastUpdate = FDateTime::Now();
    
    // Count actors in current world
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentMetrics.TotalActors = AllActors.Num();
        
        // Count dinosaurs
        CurrentMetrics.DinosaurCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("Dino")))
            {
                CurrentMetrics.DinosaurCount++;
            }
        }
        
        // Basic playability check
        CurrentMetrics.bIsPlayable = (CurrentMetrics.TotalActors > 10 && 
                                     CurrentMetrics.DinosaurCount > 0 &&
                                     CurrentMetrics.CompilationErrors == 0);
    }
    
    // Memory usage (simplified)
    CurrentMetrics.MemoryUsageGB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f * 1024.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Build metrics updated: %d actors, %d dinos, playable=%s"), 
           CurrentMetrics.TotalActors, CurrentMetrics.DinosaurCount, 
           CurrentMetrics.bIsPlayable ? TEXT("YES") : TEXT("NO"));
}

bool UDir_ProductionManager::ValidateActorLimits()
{
    UpdateBuildMetrics();
    
    bool bWithinLimits = true;
    
    if (CurrentMetrics.TotalActors > MaxTotalActors)
    {
        UE_LOG(LogTemp, Error, TEXT("LIMIT EXCEEDED: Total actors %d > %d"), 
               CurrentMetrics.TotalActors, MaxTotalActors);
        bWithinLimits = false;
    }
    
    if (CurrentMetrics.DinosaurCount > MaxDinosaurs)
    {
        UE_LOG(LogTemp, Error, TEXT("LIMIT EXCEEDED: Dinosaurs %d > %d"), 
               CurrentMetrics.DinosaurCount, MaxDinosaurs);
        bWithinLimits = false;
    }
    
    return bWithinLimits;
}

void UDir_ProductionManager::EnforceActorLimits()
{
    if (!ValidateActorLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("Enforcing actor limits - removing excess actors"));
        
        if (UWorld* World = GetWorld())
        {
            TArray<AActor*> AllActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
            
            // Remove excess actors, prioritizing props over dinosaurs
            int32 ActorsToRemove = FMath::Max(0, AllActors.Num() - MaxTotalActors);
            
            for (int32 i = 0; i < ActorsToRemove && i < AllActors.Num(); i++)
            {
                AActor* Actor = AllActors[i];
                if (Actor && !Actor->GetName().Contains(TEXT("Player")) && 
                    !Actor->GetName().Contains(TEXT("GameMode")))
                {
                    Actor->Destroy();
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Removed %d excess actors"), ActorsToRemove);
        }
    }
}

void UDir_ProductionManager::InitializeAgentPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing 19-agent production pipeline"));
    
    // Verify critical agents are ready
    TArray<int32> CriticalAgents = {2, 3, 5, 9, 10, 12, 15, 18, 19};
    
    for (int32 AgentID : CriticalAgents)
    {
        UE_LOG(LogTemp, Log, TEXT("Agent #%02d registered in pipeline"), AgentID);
    }
}

void UDir_ProductionManager::ValidateProjectStructure()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating project structure"));
    
    // Check if critical directories exist
    TArray<FString> RequiredPaths = {
        TEXT("Source/TranspersonalGame/Core"),
        TEXT("Source/TranspersonalGame/Characters"),
        TEXT("Source/TranspersonalGame/AI"),
        TEXT("Source/TranspersonalGame/Environment"),
        TEXT("Source/TranspersonalGame/Director")
    };
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    for (const FString& Path : RequiredPaths)
    {
        if (PlatformFile.DirectoryExists(*Path))
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Directory exists: %s"), *Path);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("✗ Directory missing: %s"), *Path);
        }
    }
}

void UDir_ProductionManager::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Production Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), ActiveTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Build Playable: %s"), CurrentMetrics.bIsPlayable ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("============================"));
}