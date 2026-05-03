#include "Dir_MilestoneTracker.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADir_MilestoneTracker::ADir_MilestoneTracker()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    bShowProgressInViewport = true;
    UpdateInterval = 5.0f;
    LastUpdateTime = 0.0f;
    
    // Set up colors
    CompletedTaskColor = FLinearColor::Green;
    InProgressTaskColor = FLinearColor::Yellow;
    BlockedTaskColor = FLinearColor::Red;
    
    SetupComponents();
}

void ADir_MilestoneTracker::SetupComponents()
{
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create tracker mesh component
    TrackerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrackerMesh"));
    TrackerMesh->SetupAttachment(RootComponent);
    
    // Try to load a simple cube mesh as default
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        TrackerMesh->SetStaticMesh(CubeMeshAsset.Object);
        TrackerMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }
    
    // Create progress display component
    ProgressDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ProgressDisplay"));
    ProgressDisplay->SetupAttachment(RootComponent);
    ProgressDisplay->SetWorldSize(150.0f);
    ProgressDisplay->SetTextRenderColor(FColor::White);
    ProgressDisplay->SetHorizontalAlignment(EHTA_Center);
    ProgressDisplay->SetVerticalAlignment(EVTA_TextCenter);
    ProgressDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
}

void ADir_MilestoneTracker::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize Milestone 1 data
    InitializeMilestone1();
    
    // Start progress updates
    UpdateProgressDisplay();
    
    UE_LOG(LogTemp, Warning, TEXT("Dir_MilestoneTracker: Milestone 1 tracking initialized"));
}

void ADir_MilestoneTracker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update progress display at intervals
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime > UpdateInterval)
    {
        UpdateProgressDisplay();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void ADir_MilestoneTracker::InitializeMilestone1()
{
    CurrentMilestone.MilestoneName = TEXT("Milestone 1: Walk Around Prototype");
    CurrentMilestone.Description = TEXT("Create minimum viable playable prototype with character movement and dinosaurs");
    CurrentMilestone.OverallStatus = EDir_MilestoneStatus::InProgress;
    CurrentMilestone.Tasks.Empty();
    
    // Define critical tasks for Milestone 1
    TArray<FDir_AgentTask> CriticalTasks;
    
    // Agent #5 - World Generator
    FDir_AgentTask WorldTask;
    WorldTask.AgentName = TEXT("Agent #5 - World Generator");
    WorldTask.TaskDescription = TEXT("Generate real terrain with hills/valleys, place biomes");
    WorldTask.Status = EDir_MilestoneStatus::NotStarted;
    WorldTask.Priority = 10.0f;
    WorldTask.Dependencies = TEXT("Engine Architecture");
    CriticalTasks.Add(WorldTask);
    
    // Agent #9 - Character Artist
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentName = TEXT("Agent #9 - Character Artist");
    CharacterTask.TaskDescription = TEXT("Create visible character mesh with proper collision");
    CharacterTask.Status = EDir_MilestoneStatus::NotStarted;
    CharacterTask.Priority = 9.0f;
    CharacterTask.Dependencies = TEXT("Core Systems");
    CriticalTasks.Add(CharacterTask);
    
    // Agent #11 - NPC Behavior
    FDir_AgentTask DinosaurTask;
    DinosaurTask.AgentName = TEXT("Agent #11 - NPC Behavior");
    DinosaurTask.TaskDescription = TEXT("Spawn 5 dinosaurs with basic AI and collision");
    DinosaurTask.Status = EDir_MilestoneStatus::NotStarted;
    DinosaurTask.Priority = 8.0f;
    DinosaurTask.Dependencies = TEXT("Character System");
    CriticalTasks.Add(DinosaurTask);
    
    // Agent #12 - Combat AI (HUD)
    FDir_AgentTask HUDTask;
    HUDTask.AgentName = TEXT("Agent #12 - Combat AI");
    HUDTask.TaskDescription = TEXT("Implement survival HUD with health/hunger/thirst bars");
    HUDTask.Status = EDir_MilestoneStatus::NotStarted;
    HUDTask.Priority = 7.0f;
    HUDTask.Dependencies = TEXT("Character Stats");
    CriticalTasks.Add(HUDTask);
    
    // Agent #8 - Lighting
    FDir_AgentTask LightingTask;
    LightingTask.AgentName = TEXT("Agent #8 - Lighting");
    LightingTask.TaskDescription = TEXT("Setup day/night cycle with proper lighting transitions");
    LightingTask.Status = EDir_MilestoneStatus::NotStarted;
    LightingTask.Priority = 6.0f;
    LightingTask.Dependencies = TEXT("Environment");
    CriticalTasks.Add(LightingTask);
    
    CurrentMilestone.Tasks = CriticalTasks;
    CurrentMilestone.CompletionPercentage = CalculateCompletionPercentage();
}

void ADir_MilestoneTracker::UpdateTaskStatus(const FString& AgentName, EDir_MilestoneStatus NewStatus)
{
    for (FDir_AgentTask& Task : CurrentMilestone.Tasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Dir_MilestoneTracker: Updated %s status to %d"), *AgentName, (int32)NewStatus);
            break;
        }
    }
    
    // Recalculate completion percentage
    CurrentMilestone.CompletionPercentage = CalculateCompletionPercentage();
    
    // Check if milestone is complete
    if (IsMilestoneComplete())
    {
        CurrentMilestone.OverallStatus = EDir_MilestoneStatus::Completed;
        UE_LOG(LogTemp, Warning, TEXT("Dir_MilestoneTracker: MILESTONE 1 COMPLETED!"));
    }
}

float ADir_MilestoneTracker::CalculateCompletionPercentage()
{
    if (CurrentMilestone.Tasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : CurrentMilestone.Tasks)
    {
        if (Task.Status == EDir_MilestoneStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    return (float)CompletedTasks / (float)CurrentMilestone.Tasks.Num() * 100.0f;
}

void ADir_MilestoneTracker::UpdateProgressDisplay()
{
    if (!bShowProgressInViewport || !ProgressDisplay)
    {
        return;
    }
    
    UpdateDisplayText();
    LogMilestoneProgress();
}

void ADir_MilestoneTracker::UpdateDisplayText()
{
    FString DisplayText = FString::Printf(TEXT("%s\nProgress: %.1f%%\n\n"), 
        *CurrentMilestone.MilestoneName, 
        CurrentMilestone.CompletionPercentage);
    
    // Add task status
    for (const FDir_AgentTask& Task : CurrentMilestone.Tasks)
    {
        FString StatusSymbol;
        switch (Task.Status)
        {
            case EDir_MilestoneStatus::Completed:
                StatusSymbol = TEXT("✓");
                break;
            case EDir_MilestoneStatus::InProgress:
                StatusSymbol = TEXT("◐");
                break;
            case EDir_MilestoneStatus::Blocked:
                StatusSymbol = TEXT("✗");
                break;
            default:
                StatusSymbol = TEXT("○");
                break;
        }
        
        DisplayText += FString::Printf(TEXT("%s %s\n"), *StatusSymbol, *Task.AgentName);
    }
    
    ProgressDisplay->SetText(FText::FromString(DisplayText));
    
    // Update color based on overall progress
    if (CurrentMilestone.CompletionPercentage >= 100.0f)
    {
        ProgressDisplay->SetTextRenderColor(CompletedTaskColor.ToFColor(false));
    }
    else if (CurrentMilestone.CompletionPercentage > 0.0f)
    {
        ProgressDisplay->SetTextRenderColor(InProgressTaskColor.ToFColor(false));
    }
    else
    {
        ProgressDisplay->SetTextRenderColor(FColor::White);
    }
}

void ADir_MilestoneTracker::LogMilestoneProgress()
{
    FString ProgressReport = GenerateProgressReport();
    UE_LOG(LogTemp, Warning, TEXT("Dir_MilestoneTracker Progress Report:\n%s"), *ProgressReport);
}

FString ADir_MilestoneTracker::GenerateProgressReport()
{
    FString Report = FString::Printf(TEXT("=== %s ===\n"), *CurrentMilestone.MilestoneName);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), CurrentMilestone.CompletionPercentage);
    Report += FString::Printf(TEXT("Status: %s\n\n"), 
        CurrentMilestone.OverallStatus == EDir_MilestoneStatus::Completed ? TEXT("COMPLETED") : TEXT("IN PROGRESS"));
    
    Report += TEXT("Task Breakdown:\n");
    for (const FDir_AgentTask& Task : CurrentMilestone.Tasks)
    {
        FString StatusText;
        switch (Task.Status)
        {
            case EDir_MilestoneStatus::Completed:
                StatusText = TEXT("COMPLETED");
                break;
            case EDir_MilestoneStatus::InProgress:
                StatusText = TEXT("IN PROGRESS");
                break;
            case EDir_MilestoneStatus::Blocked:
                StatusText = TEXT("BLOCKED");
                break;
            default:
                StatusText = TEXT("NOT STARTED");
                break;
        }
        
        Report += FString::Printf(TEXT("- %s: %s\n"), *Task.AgentName, *StatusText);
    }
    
    return Report;
}

bool ADir_MilestoneTracker::IsMilestoneComplete()
{
    for (const FDir_AgentTask& Task : CurrentMilestone.Tasks)
    {
        if (Task.Status != EDir_MilestoneStatus::Completed)
        {
            return false;
        }
    }
    return CurrentMilestone.Tasks.Num() > 0;
}

TArray<FString> ADir_MilestoneTracker::GetBlockedTasks()
{
    TArray<FString> BlockedTasks;
    for (const FDir_AgentTask& Task : CurrentMilestone.Tasks)
    {
        if (Task.Status == EDir_MilestoneStatus::Blocked)
        {
            BlockedTasks.Add(Task.AgentName);
        }
    }
    return BlockedTasks;
}

TArray<FString> ADir_MilestoneTracker::GetNextPriorityTasks()
{
    TArray<FString> PriorityTasks;
    
    // Sort tasks by priority and find not started ones
    TArray<FDir_AgentTask> SortedTasks = CurrentMilestone.Tasks;
    SortedTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    for (const FDir_AgentTask& Task : SortedTasks)
    {
        if (Task.Status == EDir_MilestoneStatus::NotStarted)
        {
            PriorityTasks.Add(Task.AgentName);
        }
    }
    
    return PriorityTasks;
}