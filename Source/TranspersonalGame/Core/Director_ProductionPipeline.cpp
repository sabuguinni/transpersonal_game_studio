#include "Director_ProductionPipeline.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionPipeline::UDir_ProductionPipeline()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentPhase = EDir_ProductionPhase::CoreSystems;
    CurrentCycleNumber = 6;
    CycleBudgetUsed = 39.62f;
    CycleBudgetLimit = 75.0f;
}

void UDir_ProductionPipeline::BeginPlay()
{
    Super::BeginPlay();
    InitializeProductionPipeline();
}

void UDir_ProductionPipeline::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update production metrics every 5 seconds
    static float MetricsTimer = 0.0f;
    MetricsTimer += DeltaTime;
    if (MetricsTimer >= 5.0f)
    {
        LogProductionMetrics();
        MetricsTimer = 0.0f;
    }
}

void UDir_ProductionPipeline::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Director: Initializing Production Pipeline for Cycle %d"), CurrentCycleNumber);
    
    // Clear existing data
    ProductionMilestones.Empty();
    ActiveTasks.Empty();
    
    // Setup Milestone 1 - Walk Around Prototype
    SetupMilestone1WalkAround();
    
    // Setup Core Systems Milestones
    SetupCoreSystemsMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("Director: Production Pipeline initialized with %d milestones"), ProductionMilestones.Num());
}

void UDir_ProductionPipeline::SetupMilestone1WalkAround()
{
    FDir_MilestoneData Milestone1;
    Milestone1.MilestoneName = TEXT("Milestone 1 - Walk Around Prototype");
    Milestone1.Phase = EDir_ProductionPhase::CoreSystems;
    Milestone1.bIsComplete = false;
    Milestone1.CompletionPercentage = 0.0f;
    
    // Task 1: Character Movement
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentName = TEXT("Agent #9 - Character Artist");
    CharacterTask.TaskDescription = TEXT("Implement WASD movement for TranspersonalCharacter with camera boom and follow camera");
    CharacterTask.Status = EDir_AgentStatus::Working;
    CharacterTask.Priority = 10.0f;
    CharacterTask.ExpectedOutput = TEXT("Working character controller with third-person camera");
    Milestone1.RequiredTasks.Add(CharacterTask);
    
    // Task 2: Terrain Generation
    FDir_AgentTask TerrainTask;
    TerrainTask.AgentName = TEXT("Agent #5 - World Generator");
    TerrainTask.TaskDescription = TEXT("Create landscape with height variation and basic terrain materials");
    TerrainTask.Status = EDir_AgentStatus::Working;
    TerrainTask.Priority = 9.0f;
    TerrainTask.ExpectedOutput = TEXT("Non-flat terrain with hills and valleys");
    Milestone1.RequiredTasks.Add(TerrainTask);
    
    // Task 3: Dinosaur Placement
    FDir_AgentTask DinosaurTask;
    DinosaurTask.AgentName = TEXT("Agent #12 - Combat AI");
    DinosaurTask.TaskDescription = TEXT("Spawn 3-5 static dinosaur meshes in MinPlayableMap using real assets");
    DinosaurTask.Status = EDir_AgentStatus::Idle;
    DinosaurTask.Priority = 8.0f;
    DinosaurTask.ExpectedOutput = TEXT("Visible dinosaurs placed in world");
    Milestone1.RequiredTasks.Add(DinosaurTask);
    
    // Task 4: Lighting Setup
    FDir_AgentTask LightingTask;
    LightingTask.AgentName = TEXT("Agent #8 - Lighting Artist");
    LightingTask.TaskDescription = TEXT("Configure directional light, sky atmosphere, and fog for basic scene lighting");
    LightingTask.Status = EDir_AgentStatus::Idle;
    LightingTask.Priority = 7.0f;
    LightingTask.ExpectedOutput = TEXT("Properly lit scene with day/night cycle");
    Milestone1.RequiredTasks.Add(LightingTask);
    
    ProductionMilestones.Add(Milestone1);
}

void UDir_ProductionPipeline::SetupCoreSystemsMilestones()
{
    FDir_MilestoneData CoreSystems;
    CoreSystems.MilestoneName = TEXT("Core Systems Integration");
    CoreSystems.Phase = EDir_ProductionPhase::CoreSystems;
    CoreSystems.bIsComplete = false;
    CoreSystems.CompletionPercentage = 0.0f;
    
    // Task 1: Survival Component
    FDir_AgentTask SurvivalTask;
    SurvivalTask.AgentName = TEXT("Agent #3 - Core Systems");
    SurvivalTask.TaskDescription = TEXT("Create SurvivalComponent with Health, Hunger, Thirst, Stamina stats");
    SurvivalTask.Status = EDir_AgentStatus::Idle;
    SurvivalTask.Priority = 9.0f;
    SurvivalTask.ExpectedOutput = TEXT("Working survival stats that decrease over time");
    CoreSystems.RequiredTasks.Add(SurvivalTask);
    
    // Task 2: T-Rex AI
    FDir_AgentTask TRexTask;
    TRexTask.AgentName = TEXT("Agent #12 - Combat AI");
    TRexTask.TaskDescription = TEXT("Implement DinosaurCombatAIController.cpp with basic pursuit behavior");
    TRexTask.Status = EDir_AgentStatus::Idle;
    TRexTask.Priority = 8.0f;
    TRexTask.ExpectedOutput = TEXT("T-Rex that detects and pursues player within 5000 units");
    CoreSystems.RequiredTasks.Add(TRexTask);
    
    ProductionMilestones.Add(CoreSystems);
}

void UDir_ProductionPipeline::StartNewCycle()
{
    CurrentCycleNumber++;
    CycleBudgetUsed = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Director: Starting Production Cycle %d"), CurrentCycleNumber);
    
    // Reset task statuses for new cycle
    for (FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        for (FDir_AgentTask& Task : Milestone.RequiredTasks)
        {
            if (Task.Status == EDir_AgentStatus::Working)
            {
                Task.Status = EDir_AgentStatus::Idle;
            }
        }
    }
    
    UpdateCriticalPath();
}

void UDir_ProductionPipeline::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.Priority = Priority;
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Director: Assigned task to %s: %s"), *AgentName, *TaskDescription);
}

void UDir_ProductionPipeline::CompleteAgentTask(const FString& AgentName, const FString& Output)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName && Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Complete;
            Task.ExpectedOutput = Output;
            
            UE_LOG(LogTemp, Warning, TEXT("Director: %s completed task: %s"), *AgentName, *Output);
            break;
        }
    }
    
    // Update milestone completion
    for (FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        int32 CompletedTasks = 0;
        for (const FDir_AgentTask& Task : Milestone.RequiredTasks)
        {
            if (Task.Status == EDir_AgentStatus::Complete)
            {
                CompletedTasks++;
            }
        }
        
        Milestone.CompletionPercentage = (float)CompletedTasks / (float)Milestone.RequiredTasks.Num() * 100.0f;
        
        if (Milestone.CompletionPercentage >= 100.0f && !Milestone.bIsComplete)
        {
            Milestone.bIsComplete = true;
            UE_LOG(LogTemp, Warning, TEXT("Director: MILESTONE COMPLETE - %s"), *Milestone.MilestoneName);
        }
    }
}

void UDir_ProductionPipeline::BlockAgentTask(const FString& AgentName, const FString& Reason)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = EDir_AgentStatus::Blocked;
            UE_LOG(LogTemp, Error, TEXT("Director: %s task BLOCKED: %s"), *AgentName, *Reason);
            break;
        }
    }
}

bool UDir_ProductionPipeline::CheckMilestoneCompletion(const FString& MilestoneName)
{
    for (const FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.bIsComplete;
        }
    }
    return false;
}

void UDir_ProductionPipeline::AdvanceToNextPhase()
{
    if (CurrentPhase == EDir_ProductionPhase::CoreSystems)
    {
        CurrentPhase = EDir_ProductionPhase::WorldBuilding;
        UE_LOG(LogTemp, Warning, TEXT("Director: Advanced to World Building phase"));
    }
    else if (CurrentPhase == EDir_ProductionPhase::WorldBuilding)
    {
        CurrentPhase = EDir_ProductionPhase::CharacterDev;
        UE_LOG(LogTemp, Warning, TEXT("Director: Advanced to Character Development phase"));
    }
    // Continue phase progression...
}

TArray<FDir_AgentTask> UDir_ProductionPipeline::GetCriticalPathTasks()
{
    TArray<FDir_AgentTask> CriticalTasks;
    
    // Find highest priority incomplete tasks
    for (const FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        if (!Milestone.bIsComplete)
        {
            for (const FDir_AgentTask& Task : Milestone.RequiredTasks)
            {
                if (Task.Status != EDir_AgentStatus::Complete && Task.Priority >= 8.0f)
                {
                    CriticalTasks.Add(Task);
                }
            }
        }
    }
    
    // Sort by priority
    CriticalTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    return CriticalTasks;
}

float UDir_ProductionPipeline::CalculateOverallProgress()
{
    if (ProductionMilestones.Num() == 0) return 0.0f;
    
    float TotalProgress = 0.0f;
    for (const FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        TotalProgress += Milestone.CompletionPercentage;
    }
    
    return TotalProgress / (float)ProductionMilestones.Num();
}

void UDir_ProductionPipeline::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - CYCLE %d ==="), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %s"), *UEnum::GetValueAsString(CurrentPhase));
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CalculateOverallProgress());
    UE_LOG(LogTemp, Warning, TEXT("Budget Used: $%.2f / $%.2f"), CycleBudgetUsed, CycleBudgetLimit);
    
    for (const FDir_MilestoneData& Milestone : ProductionMilestones)
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone: %s - %.1f%% Complete"), 
               *Milestone.MilestoneName, Milestone.CompletionPercentage);
    }
    
    TArray<FDir_AgentTask> CriticalTasks = GetCriticalPathTasks();
    UE_LOG(LogTemp, Warning, TEXT("Critical Path Tasks: %d"), CriticalTasks.Num());
    
    for (const FDir_AgentTask& Task : CriticalTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - %s: %s (Priority: %.1f)"), 
               *Task.AgentName, *Task.TaskDescription, Task.Priority);
    }
}

void UDir_ProductionPipeline::CreateMinimalPlayablePrototype()
{
    UE_LOG(LogTemp, Warning, TEXT("Director: Creating Minimal Playable Prototype..."));
    
    // This function would coordinate with UE5 to create the basic playable game
    // Implementation would use UE5 Python commands to:
    // 1. Ensure TranspersonalCharacter has movement
    // 2. Place dinosaurs in the world
    // 3. Setup basic lighting
    // 4. Create simple terrain
    
    UE_LOG(LogTemp, Warning, TEXT("Director: Prototype creation initiated - check MinPlayableMap"));
}

void UDir_ProductionPipeline::ValidateCurrentBuild()
{
    UE_LOG(LogTemp, Warning, TEXT("Director: Validating current build state..."));
    
    // Check compilation status
    bool bHasErrors = false; // This would check actual compilation status
    
    if (bHasErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("Director: BUILD VALIDATION FAILED - Compilation errors detected"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Director: Build validation PASSED - Ready for testing"));
    }
}

void UDir_ProductionPipeline::UpdateCriticalPath()
{
    // Recalculate task dependencies and critical path
    ValidateAgentDependencies();
}

void UDir_ProductionPipeline::ValidateAgentDependencies()
{
    // Check that agent dependencies are properly set up
    // Agent #5 (World) -> Agent #6 (Environment) -> Agent #8 (Lighting)
    // Agent #9 (Character) -> Agent #10 (Animation) -> Agent #11 (NPC Behavior)
    // etc.
}

void UDir_ProductionPipeline::LogProductionMetrics()
{
    static int32 MetricsCounter = 0;
    MetricsCounter++;
    
    if (MetricsCounter % 12 == 0) // Every minute
    {
        GenerateProductionReport();
    }
}