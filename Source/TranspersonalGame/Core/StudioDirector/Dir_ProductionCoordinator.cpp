#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Components/TextRenderComponent.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create status display
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetWorldSize(150.0f);
    StatusDisplay->SetTextRenderColor(FLinearColor::Green);
    StatusDisplay->SetText(FText::FromString(TEXT("Production Coordinator - Initializing...")));

    // Initialize default values
    CurrentCycle = 34;
    TotalActorsInLevel = 0;
    CharacterActorCount = 0;
    DinosaurActorCount = 0;
    TerrainActorCount = 0;

    // Setup Milestone 1 by default
    SetupMilestone1Tasks();
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize production tracking
    InitializeCycle(CurrentCycle);
    ScanLevelForActors();
    UpdateStatusDisplay();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized for Cycle %d"), CurrentCycle);
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update status display every few seconds
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    if (UpdateTimer >= 5.0f)
    {
        ScanLevelForActors();
        UpdateStatusDisplay();
        UpdateTimer = 0.0f;
    }
}

void ADir_ProductionCoordinator::InitializeCycle(int32 CycleNumber)
{
    CurrentCycle = CycleNumber;
    CurrentCycleTasks.Empty();
    
    // Setup Milestone 1 tasks
    SetupMilestone1Tasks();
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized Production Cycle %d with %d tasks"), 
           CurrentCycle, CurrentCycleTasks.Num());
}

void ADir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& Task, EDir_AgentPriority Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = Task;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;
    NewTask.AssignedTime = FDateTime::Now();
    
    CurrentCycleTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Assigned task to %s: %s"), *AgentName, *Task);
}

void ADir_ProductionCoordinator::CompleteAgentTask(const FString& AgentName)
{
    for (FDir_AgentTask& Task : CurrentCycleTasks)
    {
        if (Task.AgentName == AgentName && !Task.bIsCompleted)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Completed task for agent: %s"), *AgentName);
            break;
        }
    }
    
    // Update milestone progress
    CurrentMilestone.CompletionPercentage = CalculateMilestoneProgress();
}

void ADir_ProductionCoordinator::UpdateMilestoneStatus(EDir_MilestoneStatus NewStatus)
{
    CurrentMilestone.Status = NewStatus;
    
    FString StatusString;
    switch (NewStatus)
    {
        case EDir_MilestoneStatus::NotStarted: StatusString = TEXT("Not Started"); break;
        case EDir_MilestoneStatus::InProgress: StatusString = TEXT("In Progress"); break;
        case EDir_MilestoneStatus::Testing: StatusString = TEXT("Testing"); break;
        case EDir_MilestoneStatus::Complete: StatusString = TEXT("Complete"); break;
        case EDir_MilestoneStatus::Blocked: StatusString = TEXT("Blocked"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone status updated to: %s"), *StatusString);
}

float ADir_ProductionCoordinator::CalculateMilestoneProgress()
{
    if (CurrentCycleTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : CurrentCycleTasks)
    {
        if (Task.bIsCompleted)
        {
            CompletedTasks++;
        }
    }
    
    return (float)CompletedTasks / (float)CurrentCycleTasks.Num() * 100.0f;
}

void ADir_ProductionCoordinator::ScanLevelForActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    TotalActorsInLevel = 0;
    CharacterActorCount = 0;
    DinosaurActorCount = 0;
    TerrainActorCount = 0;
    
    // Count all actors in the level
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TotalActorsInLevel++;
        
        FString ActorName = Actor->GetName().ToLower();
        FString ClassName = Actor->GetClass()->GetName().ToLower();
        
        // Count characters
        if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")) || 
            ClassName.Contains(TEXT("character")))
        {
            CharacterActorCount++;
        }
        
        // Count dinosaurs
        if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("trex")) || 
            ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachio")))
        {
            DinosaurActorCount++;
        }
        
        // Count terrain
        if (ClassName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")) ||
            ClassName.Contains(TEXT("terrain")))
        {
            TerrainActorCount++;
        }
    }
}

FString ADir_ProductionCoordinator::GenerateStatusReport()
{
    FString Report = FString::Printf(
        TEXT("PRODUCTION STATUS - CYCLE %d\n")
        TEXT("Milestone: %s (%.1f%% complete)\n")
        TEXT("Total Actors: %d\n")
        TEXT("Characters: %d\n")
        TEXT("Dinosaurs: %d\n")
        TEXT("Terrain: %d\n")
        TEXT("Active Tasks: %d\n")
        TEXT("Completed Tasks: %d"),
        CurrentCycle,
        *CurrentMilestone.MilestoneName,
        CurrentMilestone.CompletionPercentage,
        TotalActorsInLevel,
        CharacterActorCount,
        DinosaurActorCount,
        TerrainActorCount,
        CurrentCycleTasks.Num(),
        CurrentCycleTasks.FilterByPredicate([](const FDir_AgentTask& Task) { return Task.bIsCompleted; }).Num()
    );
    
    return Report;
}

void ADir_ProductionCoordinator::RefreshProductionStatus()
{
    ScanLevelForActors();
    CurrentMilestone.CompletionPercentage = CalculateMilestoneProgress();
    UpdateStatusDisplay();
    
    UE_LOG(LogTemp, Warning, TEXT("Production status refreshed"));
}

void ADir_ProductionCoordinator::ExportCycleReport()
{
    FString Report = GenerateStatusReport();
    
    // Add detailed task breakdown
    Report += TEXT("\n\nDETAILED TASK BREAKDOWN:\n");
    for (const FDir_AgentTask& Task : CurrentCycleTasks)
    {
        FString Status = Task.bIsCompleted ? TEXT("COMPLETE") : TEXT("PENDING");
        Report += FString::Printf(TEXT("- %s: %s [%s]\n"), 
                                  *Task.AgentName, *Task.TaskDescription, *Status);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cycle Report:\n%s"), *Report);
}

void ADir_ProductionCoordinator::UpdateStatusDisplay()
{
    if (!StatusDisplay)
    {
        return;
    }
    
    FString DisplayText = FString::Printf(
        TEXT("CYCLE %d - %s\n%.1f%% Complete\nActors: %d | Chars: %d | Dinos: %d"),
        CurrentCycle,
        *CurrentMilestone.MilestoneName,
        CurrentMilestone.CompletionPercentage,
        TotalActorsInLevel,
        CharacterActorCount,
        DinosaurActorCount
    );
    
    StatusDisplay->SetText(FText::FromString(DisplayText));
    
    // Change color based on progress
    if (CurrentMilestone.CompletionPercentage >= 80.0f)
    {
        StatusDisplay->SetTextRenderColor(FLinearColor::Green);
    }
    else if (CurrentMilestone.CompletionPercentage >= 50.0f)
    {
        StatusDisplay->SetTextRenderColor(FLinearColor::Yellow);
    }
    else
    {
        StatusDisplay->SetTextRenderColor(FLinearColor::Red);
    }
}

void ADir_ProductionCoordinator::SetupMilestone1Tasks()
{
    // Initialize Milestone 1: Playable Prototype
    CurrentMilestone.MilestoneName = TEXT("Milestone 1: Playable Prototype");
    CurrentMilestone.Status = EDir_MilestoneStatus::InProgress;
    CurrentMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(2);
    CurrentMilestone.CompletionPercentage = 0.0f;
    
    // Clear existing tasks
    CurrentCycleTasks.Empty();
    
    // Add critical path tasks
    AssignTaskToAgent(TEXT("Agent_05_World_Generator"), 
                      TEXT("Create varied terrain with hills, valleys, water features"), 
                      EDir_AgentPriority::Critical);
    
    AssignTaskToAgent(TEXT("Agent_11_NPC_Behavior"), 
                      TEXT("Implement basic dinosaur AI with movement and territorial behavior"), 
                      EDir_AgentPriority::Critical);
    
    // Add high priority tasks
    AssignTaskToAgent(TEXT("Agent_02_Engine_Architect"), 
                      TEXT("Fix compilation issues and ensure module stability"), 
                      EDir_AgentPriority::High);
    
    AssignTaskToAgent(TEXT("Agent_09_Character_Artist"), 
                      TEXT("Enhance TranspersonalCharacter with proper mesh and materials"), 
                      EDir_AgentPriority::High);
    
    AssignTaskToAgent(TEXT("Agent_10_Animation"), 
                      TEXT("Implement character movement animations"), 
                      EDir_AgentPriority::High);
    
    // Add medium priority tasks
    AssignTaskToAgent(TEXT("Agent_12_Combat_AI"), 
                      TEXT("Create survival HUD showing player stats"), 
                      EDir_AgentPriority::Medium);
    
    AssignTaskToAgent(TEXT("Agent_06_Environment"), 
                      TEXT("Populate world with enhanced vegetation and props"), 
                      EDir_AgentPriority::Medium);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 tasks initialized: %d total tasks"), CurrentCycleTasks.Num());
}