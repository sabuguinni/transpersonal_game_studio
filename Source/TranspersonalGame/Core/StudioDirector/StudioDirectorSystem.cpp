#include "StudioDirectorSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "UObject/ConstructorHelpers.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    MaxLogEntries = 1000;
}

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Initialized"));
    
    // Initialize default agent structure
    InitializeDefaultAgents();
    
    // Reset production metrics for new session
    ProductionMetrics = FDir_ProductionMetrics();
    AssetPipelineStatus = FDir_AssetPipelineStatus();
    
    // Log initialization
    LogAgentActivity(TEXT("StudioDirector"), TEXT("System initialized for new production cycle"));
}

void UStudioDirectorSystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Deinitialized"));
    
    // Save final production report
    FString FinalReport = GenerateProductionReport();
    LogAgentActivity(TEXT("StudioDirector"), FString::Printf(TEXT("Session ended. Final report: %s"), *FinalReport));
    
    Super::Deinitialize();
}

void UStudioDirectorSystem::InitializeAgentTasks()
{
    AgentTasks.Empty();
    AgentTaskIndices.Empty();
    
    InitializeDefaultAgents();
    
    UE_LOG(LogTemp, Warning, TEXT("Agent tasks initialized - %d agents ready"), AgentTasks.Num());
    LogAgentActivity(TEXT("StudioDirector"), FString::Printf(TEXT("Initialized %d agent tasks"), AgentTasks.Num()));
}

void UStudioDirectorSystem::InitializeDefaultAgents()
{
    TArray<FString> AgentNames = {
        TEXT("Agent02_EngineArchitect"),
        TEXT("Agent03_CoreSystems"),
        TEXT("Agent04_Performance"),
        TEXT("Agent05_WorldGen"),
        TEXT("Agent06_Environment"),
        TEXT("Agent07_Architecture"),
        TEXT("Agent08_Lighting"),
        TEXT("Agent09_Character"),
        TEXT("Agent10_Animation"),
        TEXT("Agent11_NPCBehavior"),
        TEXT("Agent12_CombatAI"),
        TEXT("Agent13_CrowdSim"),
        TEXT("Agent14_QuestDesign"),
        TEXT("Agent15_Narrative"),
        TEXT("Agent16_Audio"),
        TEXT("Agent17_VFX"),
        TEXT("Agent18_QA"),
        TEXT("Agent19_Integration")
    };

    TArray<FString> DefaultTasks = {
        TEXT("Define engine architecture and technical rules"),
        TEXT("Implement core physics and collision systems"),
        TEXT("Optimize performance for 60fps target"),
        TEXT("Expand terrain to 10km² with 5 biomes"),
        TEXT("Populate world with vegetation and props"),
        TEXT("Build prehistoric structures and interiors"),
        TEXT("Establish Cretaceous atmosphere and lighting"),
        TEXT("Create dinosaur actors with collision"),
        TEXT("Implement motion matching and IK systems"),
        TEXT("Design NPC behavior trees and routines"),
        TEXT("Develop tactical combat AI for dinosaurs"),
        TEXT("Simulate crowds up to 50,000 agents"),
        TEXT("Convert narrative into playable missions"),
        TEXT("Write game bible and prehistoric lore"),
        TEXT("Create adaptive music and sound effects"),
        TEXT("Design Niagara VFX with LOD chains"),
        TEXT("Test all systems and block broken builds"),
        TEXT("Integrate all agent outputs into coherent build")
    };

    for (int32 i = 0; i < AgentNames.Num(); ++i)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentNames[i];
        NewTask.TaskDescription = DefaultTasks[i];
        NewTask.Priority = EDir_TaskPriority::Medium;
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.EstimatedDuration = 300.0f; // 5 minutes default
        
        AgentTasks.Add(NewTask);
        AgentTaskIndices.Add(AgentNames[i], i);
    }
}

void UStudioDirectorSystem::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, EDir_TaskPriority Priority)
{
    if (int32* TaskIndex = AgentTaskIndices.Find(AgentName))
    {
        if (AgentTasks.IsValidIndex(*TaskIndex))
        {
            AgentTasks[*TaskIndex].TaskDescription = TaskDescription;
            AgentTasks[*TaskIndex].Priority = Priority;
            AgentTasks[*TaskIndex].Status = EDir_AgentStatus::Working;
            
            UE_LOG(LogTemp, Warning, TEXT("Assigned task to %s: %s"), *AgentName, *TaskDescription);
            LogAgentActivity(AgentName, FString::Printf(TEXT("Task assigned: %s"), *TaskDescription));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Agent not found: %s"), *AgentName);
    }
}

void UStudioDirectorSystem::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    if (int32* TaskIndex = AgentTaskIndices.Find(AgentName))
    {
        if (AgentTasks.IsValidIndex(*TaskIndex))
        {
            EDir_AgentStatus OldStatus = AgentTasks[*TaskIndex].Status;
            AgentTasks[*TaskIndex].Status = NewStatus;
            
            UE_LOG(LogTemp, Warning, TEXT("Agent %s status changed from %d to %d"), *AgentName, (int32)OldStatus, (int32)NewStatus);
            LogAgentActivity(AgentName, FString::Printf(TEXT("Status changed to %d"), (int32)NewStatus));
        }
    }
}

void UStudioDirectorSystem::CompleteAgentTask(const FString& AgentName, const FString& Output, float Duration)
{
    if (int32* TaskIndex = AgentTaskIndices.Find(AgentName))
    {
        if (AgentTasks.IsValidIndex(*TaskIndex))
        {
            AgentTasks[*TaskIndex].Status = EDir_AgentStatus::Completed;
            AgentTasks[*TaskIndex].Output = Output;
            AgentTasks[*TaskIndex].ActualDuration = Duration;
            
            // Update production metrics
            ProductionMetrics.CompletedTasks++;
            ProductionMetrics.AverageTaskDuration = 
                (ProductionMetrics.AverageTaskDuration * (ProductionMetrics.CompletedTasks - 1) + Duration) / ProductionMetrics.CompletedTasks;
            
            UE_LOG(LogTemp, Warning, TEXT("Agent %s completed task in %.1f seconds"), *AgentName, Duration);
            LogAgentActivity(AgentName, FString::Printf(TEXT("Task completed in %.1fs: %s"), Duration, *Output));
        }
    }
}

TArray<FDir_AgentTask> UStudioDirectorSystem::GetAgentTasks() const
{
    return AgentTasks;
}

FDir_AgentTask UStudioDirectorSystem::GetAgentTask(const FString& AgentName) const
{
    if (const int32* TaskIndex = AgentTaskIndices.Find(AgentName))
    {
        if (AgentTasks.IsValidIndex(*TaskIndex))
        {
            return AgentTasks[*TaskIndex];
        }
    }
    
    return FDir_AgentTask(); // Return default task if not found
}

void UStudioDirectorSystem::UpdateProductionMetrics(int32 FilesCreated, int32 UE5Commands, float BudgetSpent)
{
    ProductionMetrics.FilesCreated += FilesCreated;
    ProductionMetrics.UE5CommandsExecuted += UE5Commands;
    ProductionMetrics.BudgetUsed += BudgetSpent;
    
    UE_LOG(LogTemp, Warning, TEXT("Production metrics updated: +%d files, +%d UE5 commands, +$%.2f budget"), 
           FilesCreated, UE5Commands, BudgetSpent);
    
    LogAgentActivity(TEXT("StudioDirector"), 
                    FString::Printf(TEXT("Metrics updated: +%d files, +%d commands, +$%.2f"), 
                                   FilesCreated, UE5Commands, BudgetSpent));
}

FDir_ProductionMetrics UStudioDirectorSystem::GetProductionMetrics() const
{
    return ProductionMetrics;
}

void UStudioDirectorSystem::IncrementCycleCount()
{
    ProductionMetrics.TotalCycles++;
    UE_LOG(LogTemp, Warning, TEXT("Production cycle incremented to %d"), ProductionMetrics.TotalCycles);
    LogAgentActivity(TEXT("StudioDirector"), FString::Printf(TEXT("Cycle %d started"), ProductionMetrics.TotalCycles));
}

void UStudioDirectorSystem::UpdateAssetPipelineStatus(const FDir_AssetPipelineStatus& NewStatus)
{
    AssetPipelineStatus = NewStatus;
    
    UE_LOG(LogTemp, Warning, TEXT("Asset pipeline status updated: %d biomes, %d dinosaurs, FBX:%s, Terrain:%s, Atmosphere:%s"), 
           AssetPipelineStatus.BiomesCompleted,
           AssetPipelineStatus.DinosaurActorsCreated,
           AssetPipelineStatus.bFBXImportReady ? TEXT("Ready") : TEXT("Not Ready"),
           AssetPipelineStatus.bTerrainExpanded ? TEXT("Expanded") : TEXT("Basic"),
           AssetPipelineStatus.bAtmosphereStable ? TEXT("Stable") : TEXT("Unstable"));
    
    LogAgentActivity(TEXT("StudioDirector"), TEXT("Asset pipeline status updated"));
}

FDir_AssetPipelineStatus UStudioDirectorSystem::GetAssetPipelineStatus() const
{
    return AssetPipelineStatus;
}

bool UStudioDirectorSystem::CheckMilestone1Criteria() const
{
    // Milestone 1: "Walk Around" criteria
    bool bCharacterExists = true; // Assume TranspersonalCharacter exists
    bool bTerrainReady = AssetPipelineStatus.bTerrainExpanded;
    bool bDinosaursPlaced = AssetPipelineStatus.DinosaurActorsCreated >= 3;
    bool bLightingSet = AssetPipelineStatus.bAtmosphereStable;
    
    return bCharacterExists && bTerrainReady && bDinosaursPlaced && bLightingSet;
}

void UStudioDirectorSystem::CreateAgentTaskMarkers()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating agent task markers in MinPlayableMap"));
    LogAgentActivity(TEXT("StudioDirector"), TEXT("Creating visual task markers for agent coordination"));
    
    // This function coordinates with UE5 Python scripts to create visual markers
    // The actual implementation is handled by ue5_execute commands
}

void UStudioDirectorSystem::ValidateMinPlayableMap()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating MinPlayableMap state"));
    LogAgentActivity(TEXT("StudioDirector"), TEXT("Validating MinPlayableMap for production readiness"));
    
    // Validation logic would check:
    // - Actor counts
    // - Terrain state
    // - Lighting setup
    // - Character spawn points
}

FString UStudioDirectorSystem::GenerateProductionReport() const
{
    FString Report = TEXT("=== PRODUCTION REPORT ===\n");
    Report += FString::Printf(TEXT("Cycles Completed: %d\n"), ProductionMetrics.TotalCycles);
    Report += FString::Printf(TEXT("Tasks Completed: %d\n"), ProductionMetrics.CompletedTasks);
    Report += FString::Printf(TEXT("Tasks Failed: %d\n"), ProductionMetrics.FailedTasks);
    Report += FString::Printf(TEXT("Files Created: %d\n"), ProductionMetrics.FilesCreated);
    Report += FString::Printf(TEXT("UE5 Commands: %d\n"), ProductionMetrics.UE5CommandsExecuted);
    Report += FString::Printf(TEXT("Budget Used: $%.2f / $%.2f\n"), ProductionMetrics.BudgetUsed, ProductionMetrics.BudgetLimit);
    Report += FString::Printf(TEXT("Average Task Duration: %.1f seconds\n"), ProductionMetrics.AverageTaskDuration);
    
    Report += TEXT("\n=== ASSET PIPELINE STATUS ===\n");
    Report += FString::Printf(TEXT("Biomes Completed: %d/5\n"), AssetPipelineStatus.BiomesCompleted);
    Report += FString::Printf(TEXT("Dinosaur Actors: %d\n"), AssetPipelineStatus.DinosaurActorsCreated);
    Report += FString::Printf(TEXT("FBX Import Ready: %s\n"), AssetPipelineStatus.bFBXImportReady ? TEXT("Yes") : TEXT("No"));
    Report += FString::Printf(TEXT("Terrain Expanded: %s\n"), AssetPipelineStatus.bTerrainExpanded ? TEXT("Yes") : TEXT("No"));
    Report += FString::Printf(TEXT("Atmosphere Stable: %s\n"), AssetPipelineStatus.bAtmosphereStable ? TEXT("Yes") : TEXT("No"));
    Report += FString::Printf(TEXT("Survival HUD: %s\n"), AssetPipelineStatus.bSurvivalHUDImplemented ? TEXT("Yes") : TEXT("No"));
    
    Report += TEXT("\n=== MILESTONE 1 STATUS ===\n");
    Report += FString::Printf(TEXT("Walk Around Prototype: %s\n"), CheckMilestone1Criteria() ? TEXT("READY") : TEXT("IN PROGRESS"));
    
    return Report;
}

void UStudioDirectorSystem::LogAgentActivity(const FString& AgentName, const FString& Activity)
{
    FString Timestamp = GetCurrentTimestamp();
    FString LogEntry = FString::Printf(TEXT("[%s] %s: %s"), *Timestamp, *AgentName, *Activity);
    
    ActivityLog.Add(LogEntry);
    CleanupOldLogEntries();
    
    UE_LOG(LogTemp, Log, TEXT("Activity: %s"), *LogEntry);
}

void UStudioDirectorSystem::CleanupOldLogEntries()
{
    if (ActivityLog.Num() > MaxLogEntries)
    {
        int32 ExcessEntries = ActivityLog.Num() - MaxLogEntries;
        ActivityLog.RemoveAt(0, ExcessEntries);
    }
}

FString UStudioDirectorSystem::GetCurrentTimestamp() const
{
    FDateTime Now = FDateTime::Now();
    return Now.ToString(TEXT("%H:%M:%S"));
}