#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create coordinator mesh component
    CoordinatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoordinatorMesh"));
    CoordinatorMesh->SetupAttachment(RootComponent);
    
    // Try to load a basic cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CoordinatorMesh->SetStaticMesh(CubeMeshAsset.Object);
        CoordinatorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }
    
    // Create status display component
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    StatusDisplay->SetText(FText::FromString(TEXT("Production Coordinator\nInitializing...")));
    StatusDisplay->SetTextRenderColor(FColor::Green);
    StatusDisplay->SetWorldSize(100.0f);
    StatusDisplay->SetHorizontalAlignment(EHTA_Center);
    StatusDisplay->SetVerticalAlignment(EVRTA_TextCenter);
    
    // Initialize properties
    bAutoUpdateStatus = true;
    UpdateInterval = 5.0f;
    LastUpdateTime = 0.0f;
    bMilestone1Initialized = false;
    
    // Initialize current milestone
    CurrentMilestone.MilestoneName = TEXT("Milestone 1 - Walk Around");
    CurrentMilestone.Description = TEXT("Player can walk in world with visible dinosaurs");
    CurrentMilestone.Status = EDir_MilestoneStatus::InProgress;
    CurrentMilestone.CompletionPercentage = 0.0f;
    CurrentMilestone.StartTime = FDateTime::Now();
    CurrentMilestone.TargetTime = FDateTime::Now() + FTimespan::FromDays(2);
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bMilestone1Initialized)
    {
        InitializeMilestone1();
    }
    
    UpdateStatusDisplay();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoUpdateStatus)
    {
        LastUpdateTime += DeltaTime;
        if (LastUpdateTime >= UpdateInterval)
        {
            RefreshProductionStatus();
            LastUpdateTime = 0.0f;
        }
    }
}

void AProductionCoordinator::InitializeMilestone1()
{
    if (bMilestone1Initialized)
    {
        return;
    }
    
    // Clear existing tasks
    ActiveTasks.Empty();
    
    // Define Milestone 1 critical tasks
    TArray<FDir_AgentTask> Milestone1Tasks = {
        {2, TEXT("Engine Architect"), TEXT("Validate core architecture and fix compilation errors"), EDir_MilestoneStatus::InProgress, 10.0f, TEXT("None"), TEXT("Clean compilation, architecture validation")},
        {3, TEXT("Core Systems"), TEXT("Implement physics and collision systems"), EDir_MilestoneStatus::NotStarted, 9.0f, TEXT("Agent #2"), TEXT("Working physics components, collision detection")},
        {5, TEXT("World Generator"), TEXT("Expand landscape to 200km² with proper biomes"), EDir_MilestoneStatus::InProgress, 8.0f, TEXT("Agent #2"), TEXT("Large-scale terrain, 5 distinct biomes")},
        {6, TEXT("Environment Artist"), TEXT("Populate biomes with vegetation and props"), EDir_MilestoneStatus::NotStarted, 7.0f, TEXT("Agent #5"), TEXT("Trees, rocks, environmental details")},
        {9, TEXT("Character Artist"), TEXT("Create functional player character with movement"), EDir_MilestoneStatus::InProgress, 9.5f, TEXT("Agent #3"), TEXT("Playable character, WASD movement, camera")},
        {10, TEXT("Animation Agent"), TEXT("Implement character animations and IK"), EDir_MilestoneStatus::NotStarted, 6.0f, TEXT("Agent #9"), TEXT("Walk/run animations, foot IK")},
        {11, TEXT("NPC Behavior"), TEXT("Create basic dinosaur actors with AI"), EDir_MilestoneStatus::NotStarted, 8.0f, TEXT("Agent #6"), TEXT("5 dinosaur types, basic behavior")},
        {12, TEXT("Combat & UI"), TEXT("Implement survival HUD (health, hunger, thirst)"), EDir_MilestoneStatus::NotStarted, 7.5f, TEXT("Agent #9"), TEXT("Functional HUD, survival stats")},
        {15, TEXT("Narrative"), TEXT("Write core game bible and setting"), EDir_MilestoneStatus::InProgress, 5.0f, TEXT("None"), TEXT("Game bible, prehistoric setting lore")},
        {18, TEXT("QA Testing"), TEXT("Validate playable prototype"), EDir_MilestoneStatus::NotStarted, 8.5f, TEXT("All agents"), TEXT("Playability report, bug list")}
    };
    
    // Add tasks to active list
    for (const FDir_AgentTask& Task : Milestone1Tasks)
    {
        ActiveTasks.Add(Task);
    }
    
    // Add milestone to history
    AllMilestones.Add(CurrentMilestone);
    
    bMilestone1Initialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Milestone 1 initialized with %d tasks"), ActiveTasks.Num());
}

void AProductionCoordinator::UpdateTaskStatus(int32 AgentID, EDir_MilestoneStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Agent #%d status updated to %d"), AgentID, (int32)NewStatus);
            break;
        }
    }
    
    // Recalculate milestone progress
    CurrentMilestone.CompletionPercentage = CalculateMilestoneProgress();
    CheckMilestoneCompletion();
    UpdateStatusDisplay();
}

void AProductionCoordinator::AddAgentTask(const FDir_AgentTask& NewTask)
{
    ActiveTasks.Add(NewTask);
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Added task for Agent #%d: %s"), NewTask.AgentID, *NewTask.TaskDescription);
}

float AProductionCoordinator::CalculateMilestoneProgress()
{
    if (ActiveTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalWeight = 0.0f;
    float CompletedWeight = 0.0f;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        TotalWeight += Task.Priority;
        
        if (Task.Status == EDir_MilestoneStatus::Complete)
        {
            CompletedWeight += Task.Priority;
        }
        else if (Task.Status == EDir_MilestoneStatus::Testing)
        {
            CompletedWeight += Task.Priority * 0.8f;
        }
        else if (Task.Status == EDir_MilestoneStatus::InProgress)
        {
            CompletedWeight += Task.Priority * 0.3f;
        }
    }
    
    return (TotalWeight > 0.0f) ? (CompletedWeight / TotalWeight) * 100.0f : 0.0f;
}

FString AProductionCoordinator::GetProductionStatusReport()
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION STATUS REPORT ===\n"));
    Report += FString::Printf(TEXT("Milestone: %s\n"), *CurrentMilestone.MilestoneName);
    Report += FString::Printf(TEXT("Progress: %.1f%%\n"), CurrentMilestone.CompletionPercentage);
    Report += FString::Printf(TEXT("Status: %s\n\n"), 
        CurrentMilestone.Status == EDir_MilestoneStatus::Complete ? TEXT("COMPLETE") :
        CurrentMilestone.Status == EDir_MilestoneStatus::InProgress ? TEXT("IN PROGRESS") :
        CurrentMilestone.Status == EDir_MilestoneStatus::Testing ? TEXT("TESTING") :
        CurrentMilestone.Status == EDir_MilestoneStatus::Blocked ? TEXT("BLOCKED") : TEXT("NOT STARTED"));
    
    Report += TEXT("AGENT TASKS:\n");
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        FString StatusStr = 
            Task.Status == EDir_MilestoneStatus::Complete ? TEXT("✓ DONE") :
            Task.Status == EDir_MilestoneStatus::InProgress ? TEXT("⚠ WORKING") :
            Task.Status == EDir_MilestoneStatus::Testing ? TEXT("🔍 TESTING") :
            Task.Status == EDir_MilestoneStatus::Blocked ? TEXT("❌ BLOCKED") : TEXT("⏳ WAITING");
            
        Report += FString::Printf(TEXT("Agent #%02d: %s [%s]\n"), Task.AgentID, *Task.TaskDescription, *StatusStr);
    }
    
    return Report;
}

void AProductionCoordinator::ValidateAgentDeliverables()
{
    // This would integrate with the file system to check actual deliverables
    // For now, we'll simulate validation based on expected outputs
    
    int32 ValidatedTasks = 0;
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_MilestoneStatus::InProgress)
        {
            // Simulate validation check
            if (FMath::RandRange(0.0f, 1.0f) > 0.7f) // 30% chance of completion per check
            {
                Task.Status = EDir_MilestoneStatus::Testing;
                ValidatedTasks++;
            }
        }
        else if (Task.Status == EDir_MilestoneStatus::Testing)
        {
            // Testing tasks have higher chance of completion
            if (FMath::RandRange(0.0f, 1.0f) > 0.5f)
            {
                Task.Status = EDir_MilestoneStatus::Complete;
                ValidatedTasks++;
            }
        }
    }
    
    if (ValidatedTasks > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Validated %d tasks"), ValidatedTasks);
    }
}

void AProductionCoordinator::RefreshProductionStatus()
{
    ValidateAgentDeliverables();
    CurrentMilestone.CompletionPercentage = CalculateMilestoneProgress();
    CheckMilestoneCompletion();
    UpdateStatusDisplay();
}

void AProductionCoordinator::GenerateTaskAssignments()
{
    FString TaskReport = GetProductionStatusReport();
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator Task Assignments:\n%s"), *TaskReport);
    
    // This could be expanded to generate specific task files or blueprints
    // for each agent to follow
}

void AProductionCoordinator::UpdateStatusDisplay()
{
    if (StatusDisplay)
    {
        FString DisplayText = FormatStatusText();
        StatusDisplay->SetText(FText::FromString(DisplayText));
        
        // Change color based on milestone status
        FColor DisplayColor = FColor::Yellow;
        if (CurrentMilestone.Status == EDir_MilestoneStatus::Complete)
        {
            DisplayColor = FColor::Green;
        }
        else if (CurrentMilestone.Status == EDir_MilestoneStatus::Blocked)
        {
            DisplayColor = FColor::Red;
        }
        else if (CurrentMilestone.CompletionPercentage > 75.0f)
        {
            DisplayColor = FColor::Cyan;
        }
        
        StatusDisplay->SetTextRenderColor(DisplayColor);
    }
}

void AProductionCoordinator::CheckMilestoneCompletion()
{
    if (CurrentMilestone.CompletionPercentage >= 100.0f && CurrentMilestone.Status != EDir_MilestoneStatus::Complete)
    {
        CurrentMilestone.Status = EDir_MilestoneStatus::Complete;
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: MILESTONE 1 COMPLETED!"));
        
        // Trigger celebration or next milestone initialization
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                TEXT("MILESTONE 1 COMPLETE: Player can walk around with visible dinosaurs!"));
        }
    }
}

FString AProductionCoordinator::FormatStatusText()
{
    FString StatusText = FString::Printf(TEXT("MILESTONE 1\n%.1f%% Complete\n"), CurrentMilestone.CompletionPercentage);
    
    int32 CompletedTasks = 0;
    int32 InProgressTasks = 0;
    int32 BlockedTasks = 0;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_MilestoneStatus::Complete)
        {
            CompletedTasks++;
        }
        else if (Task.Status == EDir_MilestoneStatus::InProgress || Task.Status == EDir_MilestoneStatus::Testing)
        {
            InProgressTasks++;
        }
        else if (Task.Status == EDir_MilestoneStatus::Blocked)
        {
            BlockedTasks++;
        }
    }
    
    StatusText += FString::Printf(TEXT("✓ %d  ⚠ %d  ❌ %d"), CompletedTasks, InProgressTasks, BlockedTasks);
    
    return StatusText;
}