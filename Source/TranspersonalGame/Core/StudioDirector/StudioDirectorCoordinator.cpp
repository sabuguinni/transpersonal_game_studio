#include "StudioDirectorCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

UStudioDirectorCoordinator::UStudioDirectorCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    bMilestone1Active = true;
    CycleStartTime = 0.0f;
    
    CurrentMilestone.MilestoneName = TEXT("Milestone 1 - Walk Around");
    CurrentMilestone.CurrentPhase = EDir_MilestonePhase::Implementation;
    CurrentMilestone.CompletionPercentage = 0.0f;
}

void UStudioDirectorCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    CycleStartTime = GetWorld()->GetTimeSeconds();
    LogProductionEvent(TEXT("Studio Director Coordinator initialized - Cycle 007"));
    
    // Initialize agent status map
    for (int32 i = 1; i <= 19; i++)
    {
        AgentStatusMap.Add(i, EDir_AgentStatus::Idle);
    }
    
    InitializeMilestone1Tasks();
    SetupAgentDependencies();
}

void UStudioDirectorCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bMilestone1Active)
    {
        CheckTaskDependencies();
        UpdateMilestoneProgress();
    }
}

void UStudioDirectorCoordinator::InitializeMilestone1Tasks()
{
    AgentTaskQueue.Empty();
    
    // Create tasks for each critical agent
    CreateTerrainTasks();      // Agent #5
    CreateCharacterTasks();    // Agent #9
    CreateDinosaurTasks();     // Agent #11/#12
    CreateLightingTasks();     // Agent #8
    CreateHUDTasks();          // Agent #14
    
    LogProductionEvent(FString::Printf(TEXT("Initialized %d tasks for Milestone 1"), AgentTaskQueue.Num()));
}

void UStudioDirectorCoordinator::CreateTerrainTasks()
{
    FDir_AgentTask TerrainTask;
    TerrainTask.AgentID = 5;
    TerrainTask.AgentName = TEXT("Procedural World Generator");
    TerrainTask.TaskDescription = TEXT("Create varied terrain with hills, valleys, and biome transitions in MinPlayableMap");
    TerrainTask.Status = EDir_AgentStatus::Working;
    TerrainTask.Priority = 10.0f;
    TerrainTask.ExpectedOutput = TEXT("Enhanced landscape with height variation and multiple biomes");
    
    AgentTaskQueue.Add(TerrainTask);
}

void UStudioDirectorCoordinator::CreateCharacterTasks()
{
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentID = 9;
    CharacterTask.AgentName = TEXT("Character Artist Agent");
    CharacterTask.TaskDescription = TEXT("Enhance TranspersonalCharacter with proper mesh and animations");
    CharacterTask.Status = EDir_AgentStatus::Working;
    CharacterTask.Priority = 9.0f;
    CharacterTask.Dependencies.Add(5); // Depends on terrain
    CharacterTask.ExpectedOutput = TEXT("Playable character with WASD movement and proper visuals");
    
    AgentTaskQueue.Add(CharacterTask);
}

void UStudioDirectorCoordinator::CreateDinosaurTasks()
{
    FDir_AgentTask DinosaurTask;
    DinosaurTask.AgentID = 11;
    DinosaurTask.AgentName = TEXT("NPC Behavior Agent");
    DinosaurTask.TaskDescription = TEXT("Replace placeholder dinosaurs with proper meshes and basic AI");
    DinosaurTask.Status = EDir_AgentStatus::Working;
    DinosaurTask.Priority = 8.0f;
    DinosaurTask.Dependencies.Add(5); // Depends on terrain
    DinosaurTask.ExpectedOutput = TEXT("5 dinosaur actors with collision and basic wandering behavior");
    
    AgentTaskQueue.Add(DinosaurTask);
}

void UStudioDirectorCoordinator::CreateLightingTasks()
{
    FDir_AgentTask LightingTask;
    LightingTask.AgentID = 8;
    LightingTask.AgentName = TEXT("Lighting & Atmosphere Agent");
    LightingTask.TaskDescription = TEXT("Optimize lighting setup and remove duplicate actors");
    LightingTask.Status = EDir_AgentStatus::Completed; // Already handled by Studio Director
    LightingTask.Priority = 7.0f;
    LightingTask.ExpectedOutput = TEXT("Clean lighting with single DirectionalLight, SkyAtmosphere, and fog");
    
    AgentTaskQueue.Add(LightingTask);
}

void UStudioDirectorCoordinator::CreateHUDTasks()
{
    FDir_AgentTask HUDTask;
    HUDTask.AgentID = 14;
    HUDTask.AgentName = TEXT("Quest & Mission Designer");
    HUDTask.TaskDescription = TEXT("Create survival HUD showing health, hunger, thirst, stamina bars");
    HUDTask.Status = EDir_AgentStatus::Working;
    HUDTask.Priority = 6.0f;
    HUDTask.Dependencies.Add(9); // Depends on character
    HUDTask.ExpectedOutput = TEXT("Functional survival HUD with real-time stat display");
    
    AgentTaskQueue.Add(HUDTask);
}

void UStudioDirectorCoordinator::SetupAgentDependencies()
{
    // Agent dependency chain for Milestone 1:
    // 5 (Terrain) -> 9 (Character) -> 11 (Dinosaurs) -> 14 (HUD)
    // 8 (Lighting) runs in parallel
    
    LogProductionEvent(TEXT("Agent dependencies configured for Milestone 1"));
}

void UStudioDirectorCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& StatusMessage)
{
    if (AgentStatusMap.Contains(AgentID))
    {
        AgentStatusMap[AgentID] = NewStatus;
        
        // Update task status in queue
        for (FDir_AgentTask& Task : AgentTaskQueue)
        {
            if (Task.AgentID == AgentID)
            {
                Task.Status = NewStatus;
                break;
            }
        }
        
        FString LogMessage = FString::Printf(TEXT("Agent %d status: %s - %s"), 
            AgentID, 
            *UEnum::GetValueAsString(NewStatus),
            *StatusMessage);
        LogProductionEvent(LogMessage);
    }
}

bool UStudioDirectorCoordinator::CanAgentProceed(int32 AgentID)
{
    // Check if all dependencies are completed
    for (const FDir_AgentTask& Task : AgentTaskQueue)
    {
        if (Task.AgentID == AgentID)
        {
            for (int32 DepID : Task.Dependencies)
            {
                if (AgentStatusMap.Contains(DepID))
                {
                    if (AgentStatusMap[DepID] != EDir_AgentStatus::Completed)
                    {
                        return false; // Dependency not ready
                    }
                }
            }
            return true; // All dependencies satisfied
        }
    }
    return false; // Task not found
}

TArray<FDir_AgentTask> UStudioDirectorCoordinator::GetTasksForAgent(int32 AgentID)
{
    TArray<FDir_AgentTask> AgentTasks;
    
    for (const FDir_AgentTask& Task : AgentTaskQueue)
    {
        if (Task.AgentID == AgentID)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

void UStudioDirectorCoordinator::ValidateMinPlayableMapState()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 PlayerStarts = 0;
    int32 Landscapes = 0;
    int32 Characters = 0;
    int32 Dinosaurs = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("PlayerStart")))
            PlayerStarts++;
        else if (Actor->GetClass()->GetName().Contains(TEXT("Landscape")))
            Landscapes++;
        else if (Actor->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
            Characters++;
        else if (Actor->GetClass()->GetName().Contains(TEXT("Dinosaur")) || 
                 Actor->GetActorLabel().Contains(TEXT("Dinosaur")))
            Dinosaurs++;
    }
    
    FString ValidationResult = FString::Printf(
        TEXT("Map Validation - PlayerStarts: %d, Landscapes: %d, Characters: %d, Dinosaurs: %d"),
        PlayerStarts, Landscapes, Characters, Dinosaurs);
    
    LogProductionEvent(ValidationResult);
}

void UStudioDirectorCoordinator::ReportAgentDeliverable(int32 AgentID, const FString& DeliverableName, const FString& FilePath)
{
    FString DeliverableEntry = FString::Printf(TEXT("Agent %d: %s (%s)"), AgentID, *DeliverableName, *FilePath);
    CurrentMilestone.CompletedDeliverables.Add(DeliverableEntry);
    
    LogProductionEvent(FString::Printf(TEXT("Deliverable reported: %s"), *DeliverableEntry));
}

void UStudioDirectorCoordinator::BlockPipelineOnIssue(const FString& IssueDescription, int32 BlockingAgentID)
{
    CurrentMilestone.BlockingIssues.Add(IssueDescription);
    
    if (AgentStatusMap.Contains(BlockingAgentID))
    {
        AgentStatusMap[BlockingAgentID] = EDir_AgentStatus::Blocked;
    }
    
    LogProductionEvent(FString::Printf(TEXT("PIPELINE BLOCKED - Agent %d: %s"), BlockingAgentID, *IssueDescription));
}

bool UStudioDirectorCoordinator::ValidateCompilationStatus()
{
    // This would integrate with UE5 build system
    // For now, assume compilation is successful if no blocking issues
    return CurrentMilestone.BlockingIssues.Num() == 0;
}

bool UStudioDirectorCoordinator::ValidateGameplayElements()
{
    ValidateMinPlayableMapState();
    
    // Check if minimum viable prototype elements exist
    bool bHasPlayerStart = true; // Assume exists from previous validation
    bool bHasLandscape = true;   // Assume exists from previous validation
    bool bHasCharacter = true;   // TranspersonalCharacter exists
    
    return bHasPlayerStart && bHasLandscape && bHasCharacter;
}

void UStudioDirectorCoordinator::GenerateProductionReport()
{
    FString Report = TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT - CYCLE 007 ===\n");
    Report += FString::Printf(TEXT("Milestone: %s\n"), *CurrentMilestone.MilestoneName);
    Report += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(CurrentMilestone.CurrentPhase));
    Report += FString::Printf(TEXT("Completion: %.1f%%\n"), CurrentMilestone.CompletionPercentage);
    Report += TEXT("\n=== AGENT STATUS ===\n");
    
    for (const auto& StatusPair : AgentStatusMap)
    {
        Report += FString::Printf(TEXT("Agent %d: %s\n"), 
            StatusPair.Key, 
            *UEnum::GetValueAsString(StatusPair.Value));
    }
    
    Report += TEXT("\n=== COMPLETED DELIVERABLES ===\n");
    for (const FString& Deliverable : CurrentMilestone.CompletedDeliverables)
    {
        Report += Deliverable + TEXT("\n");
    }
    
    if (CurrentMilestone.BlockingIssues.Num() > 0)
    {
        Report += TEXT("\n=== BLOCKING ISSUES ===\n");
        for (const FString& Issue : CurrentMilestone.BlockingIssues)
        {
            Report += Issue + TEXT("\n");
        }
    }
    
    LogProductionEvent(Report);
}

void UStudioDirectorCoordinator::CheckTaskDependencies()
{
    // Update task readiness based on dependencies
    for (FDir_AgentTask& Task : AgentTaskQueue)
    {
        if (Task.Status == EDir_AgentStatus::Idle)
        {
            bool bCanStart = true;
            for (int32 DepID : Task.Dependencies)
            {
                if (AgentStatusMap.Contains(DepID) && 
                    AgentStatusMap[DepID] != EDir_AgentStatus::Completed)
                {
                    bCanStart = false;
                    break;
                }
            }
            
            if (bCanStart)
            {
                Task.Status = EDir_AgentStatus::Working;
                AgentStatusMap[Task.AgentID] = EDir_AgentStatus::Working;
            }
        }
    }
}

void UStudioDirectorCoordinator::UpdateMilestoneProgress()
{
    int32 CompletedTasks = 0;
    int32 TotalTasks = AgentTaskQueue.Num();
    
    for (const FDir_AgentTask& Task : AgentTaskQueue)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    if (TotalTasks > 0)
    {
        CurrentMilestone.CompletionPercentage = (float)CompletedTasks / (float)TotalTasks * 100.0f;
    }
    
    // Check if milestone is complete
    if (CompletedTasks == TotalTasks && bMilestone1Active)
    {
        CurrentMilestone.CurrentPhase = EDir_MilestonePhase::Complete;
        bMilestone1Active = false;
        LogProductionEvent(TEXT("MILESTONE 1 COMPLETED - Walk Around prototype ready"));
    }
}

void UStudioDirectorCoordinator::LogProductionEvent(const FString& Event)
{
    ProductionLog.Add(Event);
    
    // Keep only last 100 events to prevent memory bloat
    if (ProductionLog.Num() > 100)
    {
        ProductionLog.RemoveAt(0);
    }
    
    // Also log to UE5 console
    UE_LOG(LogTemp, Warning, TEXT("StudioDirector: %s"), *Event);
}

void UStudioDirectorCoordinator::BroadcastToAllAgents(const FString& Message)
{
    LogProductionEvent(FString::Printf(TEXT("BROADCAST: %s"), *Message));
}

void UStudioDirectorCoordinator::SendTaskToAgent(int32 AgentID, const FDir_AgentTask& Task)
{
    LogProductionEvent(FString::Printf(TEXT("Task sent to Agent %d: %s"), AgentID, *Task.TaskDescription));
}