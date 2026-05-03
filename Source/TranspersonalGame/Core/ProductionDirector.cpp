#include "ProductionDirector.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

ADir_ProductionDirector::ADir_ProductionDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create director mesh component
    DirectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DirectorMesh"));
    DirectorMesh->SetupAttachment(RootComponent);

    // Set default cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        DirectorMesh->SetStaticMesh(CubeMeshAsset.Object);
    }

    // Create status display
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    StatusDisplay->SetWorldSize(50.0f);
    StatusDisplay->SetTextRenderColor(FColor::Green);
    StatusDisplay->SetText(FText::FromString(TEXT("PRODUCTION DIRECTOR\nMilestone 1: 60%")));

    // Initialize production data
    OverallProgress = 60.0f;
    bMilestone1Complete = false;
    CurrentCycle = 20;
    CurrentPhase = TEXT("Milestone 1 - Walk Around");

    // Set default scale
    SetActorScale3D(FVector(2.0f, 2.0f, 2.0f));
}

void ADir_ProductionDirector::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize tasks if empty
    if (Milestone1Tasks.Num() == 0)
    {
        SetupDefaultTasks();
    }

    UpdateStatusDisplay();
    LogProductionStatus();
}

void ADir_ProductionDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update status display every 5 seconds
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    if (UpdateTimer >= 5.0f)
    {
        UpdateTimer = 0.0f;
        UpdateStatusDisplay();
    }
}

void ADir_ProductionDirector::SetupDefaultTasks()
{
    Milestone1Tasks.Empty();

    // Task 1: Character Movement
    FDir_MilestoneTask Task1;
    Task1.TaskName = TEXT("Character Movement - WASD Controls");
    Task1.Status = EDir_MilestoneStatus::Completed;
    Task1.Priority = 1;
    Task1.AssignedAgent = TEXT("Agent #9 - Character Artist");
    Task1.CompletionPercentage = 100.0f;
    Milestone1Tasks.Add(Task1);

    // Task 2: Camera System
    FDir_MilestoneTask Task2;
    Task2.TaskName = TEXT("Camera System - Third Person Follow");
    Task2.Status = EDir_MilestoneStatus::Completed;
    Task2.Priority = 1;
    Task2.AssignedAgent = TEXT("Agent #10 - Animation");
    Task2.CompletionPercentage = 100.0f;
    Milestone1Tasks.Add(Task2);

    // Task 3: Terrain
    FDir_MilestoneTask Task3;
    Task3.TaskName = TEXT("Terrain - Hills and Valleys");
    Task3.Status = EDir_MilestoneStatus::Completed;
    Task3.Priority = 1;
    Task3.AssignedAgent = TEXT("Agent #5 - World Generator");
    Task3.CompletionPercentage = 100.0f;
    Milestone1Tasks.Add(Task3);

    // Task 4: Dinosaur Actors
    FDir_MilestoneTask Task4;
    Task4.TaskName = TEXT("Dinosaur Actors - 5 Visible Creatures");
    Task4.Status = EDir_MilestoneStatus::InProgress;
    Task4.Priority = 1;
    Task4.AssignedAgent = TEXT("Agent #12 - Combat AI");
    Task4.CompletionPercentage = 70.0f;
    Milestone1Tasks.Add(Task4);

    // Task 5: Lighting
    FDir_MilestoneTask Task5;
    Task5.TaskName = TEXT("Lighting - Sun and Atmosphere");
    Task5.Status = EDir_MilestoneStatus::InProgress;
    Task5.Priority = 2;
    Task5.AssignedAgent = TEXT("Agent #8 - Lighting");
    Task5.CompletionPercentage = 80.0f;
    Milestone1Tasks.Add(Task5);

    // Task 6: Survival HUD
    FDir_MilestoneTask Task6;
    Task6.TaskName = TEXT("Survival HUD - Health/Hunger Bars");
    Task6.Status = EDir_MilestoneStatus::NotStarted;
    Task6.Priority = 2;
    Task6.AssignedAgent = TEXT("Agent #14 - Quest Designer");
    Task6.CompletionPercentage = 0.0f;
    Milestone1Tasks.Add(Task6);

    RecalculateProgress();
}

void ADir_ProductionDirector::UpdateTaskStatus(const FString& TaskName, EDir_MilestoneStatus NewStatus)
{
    for (FDir_MilestoneTask& Task : Milestone1Tasks)
    {
        if (Task.TaskName.Contains(TaskName))
        {
            Task.Status = NewStatus;
            
            // Update completion percentage based on status
            switch (NewStatus)
            {
                case EDir_MilestoneStatus::NotStarted:
                    Task.CompletionPercentage = 0.0f;
                    break;
                case EDir_MilestoneStatus::InProgress:
                    Task.CompletionPercentage = FMath::Max(Task.CompletionPercentage, 25.0f);
                    break;
                case EDir_MilestoneStatus::Completed:
                    Task.CompletionPercentage = 100.0f;
                    break;
                case EDir_MilestoneStatus::Blocked:
                    // Keep current percentage
                    break;
            }
            
            RecalculateProgress();
            UpdateStatusDisplay();
            break;
        }
    }
}

void ADir_ProductionDirector::RecalculateProgress()
{
    if (Milestone1Tasks.Num() == 0)
    {
        OverallProgress = 0.0f;
        return;
    }

    float TotalProgress = 0.0f;
    for (const FDir_MilestoneTask& Task : Milestone1Tasks)
    {
        TotalProgress += Task.CompletionPercentage;
    }

    OverallProgress = TotalProgress / Milestone1Tasks.Num();
    bMilestone1Complete = (OverallProgress >= 95.0f);
}

void ADir_ProductionDirector::UpdateStatusDisplay()
{
    FString DisplayText = FString::Printf(
        TEXT("PRODUCTION DIRECTOR\nCycle: %d\nMilestone 1: %.0f%%\nPhase: %s"),
        CurrentCycle,
        OverallProgress,
        *CurrentPhase
    );

    if (StatusDisplay)
    {
        StatusDisplay->SetText(FText::FromString(DisplayText));
        
        // Change color based on progress
        if (bMilestone1Complete)
        {
            StatusDisplay->SetTextRenderColor(FColor::Green);
        }
        else if (OverallProgress >= 50.0f)
        {
            StatusDisplay->SetTextRenderColor(FColor::Yellow);
        }
        else
        {
            StatusDisplay->SetTextRenderColor(FColor::Red);
        }
    }
}

void ADir_ProductionDirector::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION DIRECTOR STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %d"), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Complete: %s"), bMilestone1Complete ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Warning, TEXT("Task Breakdown:"));
    for (const FDir_MilestoneTask& Task : Milestone1Tasks)
    {
        FString StatusString;
        switch (Task.Status)
        {
            case EDir_MilestoneStatus::NotStarted: StatusString = TEXT("NOT_STARTED"); break;
            case EDir_MilestoneStatus::InProgress: StatusString = TEXT("IN_PROGRESS"); break;
            case EDir_MilestoneStatus::Completed: StatusString = TEXT("COMPLETED"); break;
            case EDir_MilestoneStatus::Blocked: StatusString = TEXT("BLOCKED"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s (%.0f%%) - %s"), 
            *Task.TaskName, *StatusString, Task.CompletionPercentage, *Task.AssignedAgent);
    }
}

void ADir_ProductionDirector::InitializeMilestone1Tasks()
{
    SetupDefaultTasks();
    UpdateStatusDisplay();
    LogProductionStatus();
}

void ADir_ProductionDirector::CheckMilestone1Completion()
{
    RecalculateProgress();
    
    if (bMilestone1Complete)
    {
        UE_LOG(LogTemp, Warning, TEXT("🎉 MILESTONE 1 COMPLETED! 🎉"));
        UE_LOG(LogTemp, Warning, TEXT("Player can walk around with visible dinosaurs!"));
        CurrentPhase = TEXT("Milestone 2 - Basic Survival");
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: %.1f%% - Continue production"), OverallProgress);
    }
    
    UpdateStatusDisplay();
}