#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize milestone tracking
    bCharacterMovementComplete = false;
    bTerrainExpanded = false;
    bDinosaurSpawnsComplete = false;
    bLightingFixed = false;
    bHUDImplemented = false;

    TotalMilestoneTasks = 5;
    CompletedMilestoneTasks = 0;
    LastUpdateTime = 0.0f;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create visual mesh component
    CoordinatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoordinatorMesh"));
    CoordinatorMesh->SetupAttachment(RootComponent);

    // Load cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CoordinatorMesh->SetStaticMesh(CubeMeshAsset.Object);
        CoordinatorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.1f)); // Flat platform
    }

    // Create status display
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    StatusDisplay->SetText(FText::FromString(TEXT("Production Coordinator\nMilestone 1: 0%")));
    StatusDisplay->SetTextRenderColor(FColor::Green);
    StatusDisplay->SetWorldSize(100.0f);

    // Initialize task arrays
    PendingTasks.Empty();
    CompletedTasks.Empty();
    BlockedTasks.Empty();

    // Add initial Milestone 1 tasks
    PendingTasks.Add(TEXT("Character Movement System"));
    PendingTasks.Add(TEXT("Terrain Expansion to 200km²"));
    PendingTasks.Add(TEXT("Dinosaur Spawn Distribution"));
    PendingTasks.Add(TEXT("Lighting System Cleanup"));
    PendingTasks.Add(TEXT("Survival HUD Implementation"));
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized for Milestone 1"));
    LogProductionStatus();
    UpdateStatusDisplay();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    
    // Update status every 5 seconds
    if (LastUpdateTime >= 5.0f)
    {
        CheckMilestoneCompletion();
        UpdateStatusDisplay();
        LastUpdateTime = 0.0f;
    }
}

void AProductionCoordinator::UpdateTaskStatus(const FString& TaskName, bool bCompleted)
{
    if (bCompleted)
    {
        // Move from pending to completed
        if (PendingTasks.Contains(TaskName))
        {
            PendingTasks.Remove(TaskName);
            CompletedTasks.AddUnique(TaskName);
            
            UE_LOG(LogTemp, Warning, TEXT("Task completed: %s"), *TaskName);
            
            // Update milestone flags
            if (TaskName.Contains(TEXT("Character Movement")))
            {
                bCharacterMovementComplete = true;
            }
            else if (TaskName.Contains(TEXT("Terrain Expansion")))
            {
                bTerrainExpanded = true;
            }
            else if (TaskName.Contains(TEXT("Dinosaur Spawn")))
            {
                bDinosaurSpawnsComplete = true;
            }
            else if (TaskName.Contains(TEXT("Lighting")))
            {
                bLightingFixed = true;
            }
            else if (TaskName.Contains(TEXT("HUD")))
            {
                bHUDImplemented = true;
            }
        }
    }
    else
    {
        // Move from completed back to pending (rollback)
        if (CompletedTasks.Contains(TaskName))
        {
            CompletedTasks.Remove(TaskName);
            PendingTasks.AddUnique(TaskName);
        }
    }
    
    CheckMilestoneCompletion();
    UpdateStatusDisplay();
}

void AProductionCoordinator::AddPendingTask(const FString& TaskName, int32 AgentID)
{
    FString FormattedTask = FString::Printf(TEXT("[Agent %d] %s"), AgentID, *TaskName);
    PendingTasks.AddUnique(FormattedTask);
    
    UE_LOG(LogTemp, Log, TEXT("Added task for Agent %d: %s"), AgentID, *TaskName);
    UpdateStatusDisplay();
}

bool AProductionCoordinator::IsTaskComplete(const FString& TaskName)
{
    return CompletedTasks.Contains(TaskName);
}

float AProductionCoordinator::GetMilestone1Progress()
{
    CompletedMilestoneTasks = 0;
    
    if (bCharacterMovementComplete) CompletedMilestoneTasks++;
    if (bTerrainExpanded) CompletedMilestoneTasks++;
    if (bDinosaurSpawnsComplete) CompletedMilestoneTasks++;
    if (bLightingFixed) CompletedMilestoneTasks++;
    if (bHUDImplemented) CompletedMilestoneTasks++;
    
    return (float)CompletedMilestoneTasks / (float)TotalMilestoneTasks;
}

FString AProductionCoordinator::GetCurrentPriorityTask()
{
    // Return highest priority pending task
    if (PendingTasks.Num() > 0)
    {
        // Prioritize character movement first
        for (const FString& Task : PendingTasks)
        {
            if (Task.Contains(TEXT("Character Movement")))
            {
                return Task;
            }
        }
        
        // Then terrain expansion
        for (const FString& Task : PendingTasks)
        {
            if (Task.Contains(TEXT("Terrain")))
            {
                return Task;
            }
        }
        
        // Return first pending task
        return PendingTasks[0];
    }
    
    return TEXT("No pending tasks");
}

void AProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: %.1f%%"), GetMilestone1Progress() * 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks (%d):"), CompletedTasks.Num());
    for (const FString& Task : CompletedTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("  ✓ %s"), *Task);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Pending Tasks (%d):"), PendingTasks.Num());
    for (const FString& Task : PendingTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("  ○ %s"), *Task);
    }
    
    if (BlockedTasks.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Blocked Tasks (%d):"), BlockedTasks.Num());
        for (const FString& Task : BlockedTasks)
        {
            UE_LOG(LogTemp, Error, TEXT("  ✗ %s"), *Task);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Current Priority: %s"), *GetCurrentPriorityTask());
}

TArray<FString> AProductionCoordinator::GetTasksForAgent(int32 AgentID)
{
    TArray<FString> AgentTasks;
    
    // Filter tasks assigned to specific agent
    for (const FString& Task : PendingTasks)
    {
        if (Task.Contains(FString::Printf(TEXT("[Agent %d]"), AgentID)))
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

void AProductionCoordinator::ReportAgentProgress(int32 AgentID, const FString& TaskName, float Progress)
{
    UE_LOG(LogTemp, Log, TEXT("Agent %d progress on '%s': %.1f%%"), AgentID, *TaskName, Progress * 100.0f);
    
    // If progress is 100%, mark task as complete
    if (Progress >= 1.0f)
    {
        UpdateTaskStatus(TaskName, true);
    }
}

void AProductionCoordinator::UpdateStatusDisplay()
{
    float Progress = GetMilestone1Progress();
    int32 PendingCount = PendingTasks.Num();
    int32 CompletedCount = CompletedTasks.Num();
    
    FString StatusText = FString::Printf(
        TEXT("Production Coordinator\nMilestone 1: %.0f%%\nCompleted: %d | Pending: %d\nPriority: %s"),
        Progress * 100.0f,
        CompletedCount,
        PendingCount,
        *GetCurrentPriorityTask()
    );
    
    if (StatusDisplay)
    {
        StatusDisplay->SetText(FText::FromString(StatusText));
        
        // Change color based on progress
        if (Progress >= 1.0f)
        {
            StatusDisplay->SetTextRenderColor(FColor::Green);
        }
        else if (Progress >= 0.5f)
        {
            StatusDisplay->SetTextRenderColor(FColor::Yellow);
        }
        else
        {
            StatusDisplay->SetTextRenderColor(FColor::Red);
        }
    }
}

void AProductionCoordinator::CheckMilestoneCompletion()
{
    float Progress = GetMilestone1Progress();
    
    if (Progress >= 1.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("🎉 MILESTONE 1 COMPLETED! 🎉"));
        UE_LOG(LogTemp, Warning, TEXT("All core systems are ready for playable prototype"));
        
        // Trigger celebration or next milestone setup
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                TEXT("MILESTONE 1 COMPLETE - Playable Prototype Ready!"));
        }
    }
}