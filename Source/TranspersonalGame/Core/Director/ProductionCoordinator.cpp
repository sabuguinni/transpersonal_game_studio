#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"

UProductionCoordinator::UProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    TotalCppFiles = 0;
    TotalHeaderFiles = 0;
    VisibleActorsInMap = 0;
    bPlayerCanMove = false;
    bDinosaursVisible = false;
    
    CurrentCycleID = "PROD_CYCLE_AUTO_20260503_006";
}

void UProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing for Cycle %s"), *CurrentCycleID);
    
    InitializeMilestone1();
    GenerateAgentTaskList();
    UpdateProductionMetrics();
}

void UProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateProductionMetrics();
    CheckMilestone1Completion();
}

void UProductionCoordinator::InitializeMilestone1()
{
    Milestone1Progress.MilestoneName = "Walk Around - Minimum Viable Prototype";
    Milestone1Progress.Phase = EDir_MilestonePhase::InProgress;
    Milestone1Progress.CompletionPercentage = 0.0f;
    Milestone1Progress.BlockingIssues = "";
    
    // Clear existing tasks
    Milestone1Progress.Tasks.Empty();
    
    // Define Milestone 1 critical tasks
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentName = "Agent #09 - Character Artist";
    CharacterTask.TaskDescription = "Ensure TranspersonalCharacter has working movement (WASD + jump)";
    CharacterTask.Priority = 10.0f;
    CharacterTask.ExpectedOutput = "Player can walk, run, jump in MinPlayableMap";
    CharacterTask.Status = EDir_AgentStatus::Working;
    Milestone1Progress.Tasks.Add(CharacterTask);
    
    FDir_AgentTask TerrainTask;
    TerrainTask.AgentName = "Agent #05 - Procedural World Generator";
    TerrainTask.TaskDescription = "Create varied landscape with hills and valleys";
    TerrainTask.Priority = 9.0f;
    TerrainTask.ExpectedOutput = "Landscape with height variation, not flat plane";
    TerrainTask.Status = EDir_AgentStatus::Working;
    Milestone1Progress.Tasks.Add(TerrainTask);
    
    FDir_AgentTask DinosaurTask;
    DinosaurTask.AgentName = "Agent #12 - Combat & Enemy AI";
    DinosaurTask.TaskDescription = "Place 3-5 visible dinosaur meshes in world";
    DinosaurTask.Priority = 8.0f;
    DinosaurTask.ExpectedOutput = "Static dinosaur meshes visible and walkable around";
    DinosaurTask.Status = EDir_AgentStatus::Working;
    Milestone1Progress.Tasks.Add(DinosaurTask);
    
    FDir_AgentTask LightingTask;
    LightingTask.AgentName = "Agent #08 - Lighting & Atmosphere";
    LightingTask.TaskDescription = "Ensure proper lighting with sun, sky, fog";
    LightingTask.Priority = 7.0f;
    LightingTask.ExpectedOutput = "Directional light + sky atmosphere + fog working";
    LightingTask.Status = EDir_AgentStatus::Working;
    Milestone1Progress.Tasks.Add(LightingTask);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Milestone 1 initialized with %d tasks"), Milestone1Progress.Tasks.Num());
}

void UProductionCoordinator::UpdateAgentTask(const FString& AgentName, EDir_AgentStatus NewStatus, const FString& Output)
{
    for (FDir_AgentTask& Task : Milestone1Progress.Tasks)
    {
        if (Task.AgentName.Contains(AgentName))
        {
            Task.Status = NewStatus;
            if (!Output.IsEmpty())
            {
                Task.ExpectedOutput = Output;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Updated %s status to %d"), 
                   *AgentName, (int32)NewStatus);
            break;
        }
    }
    
    // Update completion percentage
    CheckMilestone1Completion();
}

void UProductionCoordinator::CheckMilestone1Completion()
{
    if (Milestone1Progress.Tasks.Num() == 0)
    {
        return;
    }
    
    int32 CompletedTasks = 0;
    int32 FailedTasks = 0;
    
    for (const FDir_AgentTask& Task : Milestone1Progress.Tasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedTasks++;
        }
        else if (Task.Status == EDir_AgentStatus::Failed)
        {
            FailedTasks++;
        }
    }
    
    float NewPercentage = (float)CompletedTasks / (float)Milestone1Progress.Tasks.Num() * 100.0f;
    Milestone1Progress.CompletionPercentage = NewPercentage;
    
    // Check if milestone is complete
    if (CompletedTasks == Milestone1Progress.Tasks.Num())
    {
        Milestone1Progress.Phase = EDir_MilestonePhase::Completed;
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: MILESTONE 1 COMPLETED! 🎉"));
    }
    else if (FailedTasks > 0)
    {
        Milestone1Progress.Phase = EDir_MilestonePhase::Failed;
        Milestone1Progress.BlockingIssues = FString::Printf(TEXT("%d tasks failed"), FailedTasks);
    }
    
    // Verify actual game state
    bool bActuallyPlayable = VerifyMinPlayableMapState();
    if (bActuallyPlayable && NewPercentage >= 75.0f)
    {
        Milestone1Progress.Phase = EDir_MilestonePhase::Completed;
    }
}

FString UProductionCoordinator::GetNextPriorityAgent()
{
    FString NextAgent = "";
    float HighestPriority = 0.0f;
    
    for (const FDir_AgentTask& Task : Milestone1Progress.Tasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle || Task.Status == EDir_AgentStatus::Working)
        {
            if (Task.Priority > HighestPriority)
            {
                HighestPriority = Task.Priority;
                NextAgent = Task.AgentName;
            }
        }
    }
    
    return NextAgent;
}

void UProductionCoordinator::LogProductionStatus()
{
    FString Report = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator Report:\n%s"), *Report);
    
    // Also log to file for external monitoring
    FString LogPath = FPaths::ProjectLogDir() / TEXT("production_status.log");
    FFileHelper::SaveStringToFile(Report, *LogPath);
}

bool UProductionCoordinator::VerifyMinPlayableMapState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    bool bHasPlayerStart = false;
    bool bHasCharacter = false;
    bool bHasLighting = false;
    bool bHasTerrain = false;
    int32 DinosaurCount = 0;
    
    // Check all actors in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains("PlayerStart"))
        {
            bHasPlayerStart = true;
        }
        else if (ClassName.Contains("TranspersonalCharacter") || ClassName.Contains("Character"))
        {
            bHasCharacter = true;
        }
        else if (ClassName.Contains("DirectionalLight"))
        {
            bHasLighting = true;
        }
        else if (ClassName.Contains("Landscape") || ClassName.Contains("Terrain"))
        {
            bHasTerrain = true;
        }
        else if (ClassName.Contains("Dinosaur") || ClassName.Contains("TRex") || ClassName.Contains("Raptor"))
        {
            DinosaurCount++;
        }
    }
    
    // Update metrics
    bPlayerCanMove = bHasPlayerStart && bHasCharacter;
    bDinosaursVisible = DinosaurCount >= 3;
    
    bool bIsPlayable = bHasPlayerStart && bHasCharacter && bHasLighting && DinosaurCount >= 1;
    
    UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap State: PlayerStart=%s, Character=%s, Lighting=%s, Terrain=%s, Dinosaurs=%d, Playable=%s"),
           bHasPlayerStart ? TEXT("YES") : TEXT("NO"),
           bHasCharacter ? TEXT("YES") : TEXT("NO"),
           bHasLighting ? TEXT("YES") : TEXT("NO"),
           bHasTerrain ? TEXT("YES") : TEXT("NO"),
           DinosaurCount,
           bIsPlayable ? TEXT("YES") : TEXT("NO"));
    
    return bIsPlayable;
}

void UProductionCoordinator::GenerateAgentTaskList()
{
    ActiveTasks.Empty();
    
    // Generate specific tasks for each agent based on current needs
    FDir_AgentTask EngineArchitectTask;
    EngineArchitectTask.AgentName = "Agent #02 - Engine Architect";
    EngineArchitectTask.TaskDescription = "Review and fix compilation errors, ensure all .h files have .cpp implementations";
    EngineArchitectTask.Priority = 9.5f;
    EngineArchitectTask.Status = EDir_AgentStatus::Idle;
    ActiveTasks.Add(EngineArchitectTask);
    
    FDir_AgentTask CoreSystemsTask;
    CoreSystemsTask.AgentName = "Agent #03 - Core Systems Programmer";
    CoreSystemsTask.TaskDescription = "Implement physics and collision for character movement and dinosaur interactions";
    CoreSystemsTask.Priority = 9.0f;
    CoreSystemsTask.Status = EDir_AgentStatus::Idle;
    ActiveTasks.Add(CoreSystemsTask);
    
    FDir_AgentTask WorldGenTask;
    WorldGenTask.AgentName = "Agent #05 - Procedural World Generator";
    WorldGenTask.TaskDescription = "Create varied landscape with hills, valleys, and natural features";
    WorldGenTask.Priority = 8.5f;
    WorldGenTask.Status = EDir_AgentStatus::Idle;
    ActiveTasks.Add(WorldGenTask);
    
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentName = "Agent #09 - Character Artist";
    CharacterTask.TaskDescription = "Ensure TranspersonalCharacter responds to WASD input and can jump";
    CharacterTask.Priority = 8.0f;
    CharacterTask.Status = EDir_AgentStatus::Idle;
    ActiveTasks.Add(CharacterTask);
    
    FDir_AgentTask AITask;
    AITask.AgentName = "Agent #12 - Combat & Enemy AI";
    AITask.TaskDescription = "Place static dinosaur meshes in world for player to see and walk around";
    AITask.Priority = 7.5f;
    AITask.Status = EDir_AgentStatus::Idle;
    ActiveTasks.Add(AITask);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Generated %d agent tasks"), ActiveTasks.Num());
}

void UProductionCoordinator::DebugPrintAllTasks()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION COORDINATOR DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: %.1f%% (%s)"), 
           Milestone1Progress.CompletionPercentage, 
           *UEnum::GetValueAsString(Milestone1Progress.Phase));
    
    for (int32 i = 0; i < Milestone1Progress.Tasks.Num(); i++)
    {
        const FDir_AgentTask& Task = Milestone1Progress.Tasks[i];
        UE_LOG(LogTemp, Warning, TEXT("Task %d: %s - %s (Priority: %.1f)"), 
               i + 1, 
               *Task.AgentName, 
               *UEnum::GetValueAsString(Task.Status),
               Task.Priority);
    }
}

void UProductionCoordinator::DebugCheckMapActors()
{
    VerifyMinPlayableMapState();
    LogProductionStatus();
}

void UProductionCoordinator::UpdateProductionMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    VisibleActorsInMap = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent() && Actor->GetRootComponent()->IsVisible())
        {
            VisibleActorsInMap++;
        }
    }
}

FString UProductionCoordinator::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION STATUS REPORT - %s ===\n"), *CurrentCycleID);
    Report += FString::Printf(TEXT("Milestone 1 Progress: %.1f%% (%s)\n"), 
                             Milestone1Progress.CompletionPercentage,
                             *UEnum::GetValueAsString(Milestone1Progress.Phase));
    
    if (!Milestone1Progress.BlockingIssues.IsEmpty())
    {
        Report += FString::Printf(TEXT("BLOCKING ISSUES: %s\n"), *Milestone1Progress.BlockingIssues);
    }
    
    Report += FString::Printf(TEXT("Visible Actors in Map: %d\n"), VisibleActorsInMap);
    Report += FString::Printf(TEXT("Player Can Move: %s\n"), bPlayerCanMove ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("Dinosaurs Visible: %s\n"), bDinosaursVisible ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\nAgent Tasks:\n");
    for (const FDir_AgentTask& Task : Milestone1Progress.Tasks)
    {
        Report += FString::Printf(TEXT("- %s: %s (Priority: %.1f)\n"), 
                                 *Task.AgentName, 
                                 *UEnum::GetValueAsString(Task.Status),
                                 Task.Priority);
    }
    
    FString NextAgent = GetNextPriorityAgent();
    if (!NextAgent.IsEmpty())
    {
        Report += FString::Printf(TEXT("\nNEXT PRIORITY AGENT: %s\n"), *NextAgent);
    }
    
    return Report;
}

void UProductionCoordinator::ValidateAgentDependencies()
{
    // Check if agents have completed their dependencies
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        bool bCanStart = true;
        for (const FString& Dependency : Task.Dependencies)
        {
            if (!CompletedAgents.Contains(Dependency))
            {
                bCanStart = false;
                break;
            }
        }
        
        if (!bCanStart && Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Blocked;
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Blocked %s due to unmet dependencies"), *Task.AgentName);
        }
    }
}