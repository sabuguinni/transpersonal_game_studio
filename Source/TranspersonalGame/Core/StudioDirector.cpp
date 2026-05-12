#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

AStudioDirector::AStudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create task board mesh
    TaskBoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TaskBoardMesh"));
    TaskBoardMesh->SetupAttachment(RootComponent);

    // Create status display
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetRelativeLocation(FVector(0, 0, 100));
    StatusDisplay->SetRelativeRotation(FRotator(0, 180, 0));
    StatusDisplay->SetText(FText::FromString(TEXT("STUDIO DIRECTOR - INITIALIZING")));
    StatusDisplay->SetTextRenderColor(FColor::Green);
    StatusDisplay->SetWorldSize(50.0f);

    // Initialize production settings
    CurrentPhase = EDir_ProductionPhase::Planning;
    CycleTimeLimit = 1800.0f; // 30 minutes
    CurrentCycleNumber = 0;
    bProductionActive = false;
}

void AStudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentPipeline();
    UpdateStatusDisplay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized - Production pipeline ready"));
}

void AStudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bProductionActive)
    {
        CheckAgentTimeouts();
        ProcessAgentDependencies();
        UpdateStatusDisplay();
    }
}

void AStudioDirector::InitializeAgentPipeline()
{
    AgentTasks.Empty();
    
    // Initialize all 18 agents
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),
        TEXT("Engine Architect"),
        TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"),
        TEXT("Procedural World Generator"),
        TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"),
        TEXT("Lighting & Atmosphere Agent"),
        TEXT("Character Artist Agent"),
        TEXT("Animation Agent"),
        TEXT("NPC Behavior Agent"),
        TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"),
        TEXT("Quest & Mission Designer"),
        TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"),
        TEXT("VFX Agent"),
        TEXT("QA & Testing Agent"),
        TEXT("Integration & Build Agent")
    };

    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentNumber = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.CurrentTask = TEXT("Awaiting Assignment");
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.ProgressPercentage = 0.0f;
        NewTask.Dependencies = TEXT("");
        NewTask.LastUpdate = FDateTime::Now();
        
        AgentTasks.Add(NewTask);
    }

    // Create initial milestones
    CreateMilestone(TEXT("MILESTONE_1_WALKABLE"), 
                   TEXT("Player can walk around with WASD movement"), 
                   {2, 3, 5, 9, 10});
                   
    CreateMilestone(TEXT("COMPILATION_COMPLETE"), 
                   TEXT("All C++ modules compile without errors"), 
                   {2, 3, 4});
                   
    CreateMilestone(TEXT("BASIC_WORLD"), 
                   TEXT("Landscape with terrain and basic lighting"), 
                   {5, 6, 8});

    UE_LOG(LogTemp, Warning, TEXT("Agent pipeline initialized with %d agents"), AgentTasks.Num());
}

void AStudioDirector::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    if (AgentNumber < 1 || AgentNumber > AgentTasks.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid agent number: %d"), AgentNumber);
        return;
    }

    FDir_AgentTask& Task = AgentTasks[AgentNumber - 1];
    Task.Status = NewStatus;
    Task.CurrentTask = TaskDescription;
    Task.LastUpdate = FDateTime::Now();

    // Update progress based on status
    switch (NewStatus)
    {
        case EDir_AgentStatus::Working:
            Task.ProgressPercentage = 50.0f;
            break;
        case EDir_AgentStatus::Complete:
            Task.ProgressPercentage = 100.0f;
            break;
        case EDir_AgentStatus::Error:
        case EDir_AgentStatus::Blocked:
            Task.ProgressPercentage = 0.0f;
            break;
        default:
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s) status updated: %s"), 
           AgentNumber, *Task.AgentName, *TaskDescription);
}

void AStudioDirector::StartProductionCycle()
{
    CurrentCycleNumber++;
    bProductionActive = true;
    
    // Reset all agent statuses to idle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status != EDir_AgentStatus::Complete)
        {
            Task.Status = EDir_AgentStatus::Idle;
            Task.ProgressPercentage = 0.0f;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Production Cycle #%d started"), CurrentCycleNumber);
}

void AStudioDirector::CompleteProductionCycle()
{
    bProductionActive = false;
    
    // Count completed tasks
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            CompletedTasks++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Production Cycle #%d completed - %d/%d tasks finished"), 
           CurrentCycleNumber, CompletedTasks, AgentTasks.Num());
}

bool AStudioDirector::CheckMilestoneCompletion(const FString& MilestoneName)
{
    for (FDir_ProductionMilestone& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName && !Milestone.bIsComplete)
        {
            // Check if all required agents are complete
            bool bAllComplete = true;
            for (int32 AgentNum : Milestone.RequiredAgents)
            {
                if (AgentNum <= AgentTasks.Num())
                {
                    if (AgentTasks[AgentNum - 1].Status != EDir_AgentStatus::Complete)
                    {
                        bAllComplete = false;
                        break;
                    }
                }
            }

            if (bAllComplete)
            {
                Milestone.bIsComplete = true;
                Milestone.CompletionDate = FDateTime::Now();
                UE_LOG(LogTemp, Warning, TEXT("Milestone completed: %s"), *MilestoneName);
                return true;
            }
        }
    }
    return false;
}

void AStudioDirector::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Production phase changed to: %d"), (int32)NewPhase);
}

void AStudioDirector::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& Dependencies)
{
    if (AgentNumber < 1 || AgentNumber > AgentTasks.Num())
    {
        return;
    }

    FDir_AgentTask& Task = AgentTasks[AgentNumber - 1];
    Task.CurrentTask = TaskDescription;
    Task.Dependencies = Dependencies;
    Task.Status = EDir_AgentStatus::Working;
    Task.LastUpdate = FDateTime::Now();
}

TArray<FDir_AgentTask> AStudioDirector::GetBlockedAgents()
{
    TArray<FDir_AgentTask> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(Task);
        }
    }
    
    return BlockedAgents;
}

void AStudioDirector::ResolveAgentDependency(int32 AgentNumber)
{
    if (AgentNumber < 1 || AgentNumber > AgentTasks.Num())
    {
        return;
    }

    FDir_AgentTask& Task = AgentTasks[AgentNumber - 1];
    if (Task.Status == EDir_AgentStatus::Blocked)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.Dependencies = TEXT("");
    }
}

void AStudioDirector::CreateMilestone(const FString& Name, const FString& Description, const TArray<int32>& RequiredAgents)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.MilestoneName = Name;
    NewMilestone.Description = Description;
    NewMilestone.RequiredAgents = RequiredAgents;
    NewMilestone.bIsComplete = false;
    NewMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(1);
    
    Milestones.Add(NewMilestone);
}

void AStudioDirector::CompleteMilestone(const FString& MilestoneName)
{
    for (FDir_ProductionMilestone& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.bIsComplete = true;
            Milestone.CompletionDate = FDateTime::Now();
            break;
        }
    }
}

float AStudioDirector::GetOverallProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }

    float TotalProgress = 0.0f;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TotalProgress += Task.ProgressPercentage;
    }

    return TotalProgress / AgentTasks.Num();
}

FString AStudioDirector::GetProductionStatusReport()
{
    FString Report = FString::Printf(TEXT("=== STUDIO DIRECTOR STATUS REPORT ===\n"));
    Report += FString::Printf(TEXT("Cycle: #%d | Phase: %d | Active: %s\n"), 
                             CurrentCycleNumber, (int32)CurrentPhase, bProductionActive ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\nAGENT STATUS:\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusStr;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusStr = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusStr = TEXT("WORK"); break;
            case EDir_AgentStatus::Complete: StatusStr = TEXT("DONE"); break;
            case EDir_AgentStatus::Blocked: StatusStr = TEXT("BLOCK"); break;
            case EDir_AgentStatus::Error: StatusStr = TEXT("ERROR"); break;
        }
        
        Report += FString::Printf(TEXT("#%02d %s [%s] %.0f%% - %s\n"), 
                                 Task.AgentNumber, *StatusStr, *Task.AgentName, 
                                 Task.ProgressPercentage, *Task.CurrentTask);
    }

    Report += TEXT("\nMILESTONES:\n");
    for (const FDir_ProductionMilestone& Milestone : Milestones)
    {
        Report += FString::Printf(TEXT("%s: %s - %s\n"), 
                                 Milestone.bIsComplete ? TEXT("[DONE]") : TEXT("[PEND]"),
                                 *Milestone.MilestoneName, *Milestone.Description);
    }

    return Report;
}

void AStudioDirector::EmergencyStopProduction()
{
    bProductionActive = false;
    
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Error;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY STOP - Production halted"));
}

void AStudioDirector::EditorRefreshTaskBoard()
{
    UpdateStatusDisplay();
    UE_LOG(LogTemp, Warning, TEXT("Task board refreshed in editor"));
}

void AStudioDirector::EditorGenerateStatusReport()
{
    FString Report = GetProductionStatusReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void AStudioDirector::UpdateStatusDisplay()
{
    if (StatusDisplay)
    {
        FString DisplayText = FormatStatusText();
        StatusDisplay->SetText(FText::FromString(DisplayText));
    }
}

void AStudioDirector::CheckAgentTimeouts()
{
    FDateTime CurrentTime = FDateTime::Now();
    FTimespan TimeoutLimit = FTimespan::FromMinutes(30);

    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            FTimespan TimeSinceUpdate = CurrentTime - Task.LastUpdate;
            if (TimeSinceUpdate > TimeoutLimit)
            {
                Task.Status = EDir_AgentStatus::Error;
                UE_LOG(LogTemp, Error, TEXT("Agent #%d timed out"), Task.AgentNumber);
            }
        }
    }
}

void AStudioDirector::ProcessAgentDependencies()
{
    // Check if blocked agents can be unblocked
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked && !Task.Dependencies.IsEmpty())
        {
            // Simple dependency resolution - check if dependency agent is complete
            // This could be expanded for more complex dependency parsing
            bool bCanUnblock = true; // Simplified for now
            
            if (bCanUnblock)
            {
                Task.Status = EDir_AgentStatus::Idle;
                Task.Dependencies = TEXT("");
            }
        }
    }
}

FString AStudioDirector::FormatStatusText()
{
    FString StatusText = FString::Printf(TEXT("STUDIO DIRECTOR\nCycle #%d | %s\n"), 
                                        CurrentCycleNumber, 
                                        bProductionActive ? TEXT("ACTIVE") : TEXT("IDLE"));
    
    StatusText += FString::Printf(TEXT("Progress: %.1f%%\n"), GetOverallProgress());
    
    // Show first 5 agents status
    for (int32 i = 0; i < FMath::Min(5, AgentTasks.Num()); i++)
    {
        const FDir_AgentTask& Task = AgentTasks[i];
        FString StatusStr;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusStr = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusStr = TEXT("WORK"); break;
            case EDir_AgentStatus::Complete: StatusStr = TEXT("DONE"); break;
            case EDir_AgentStatus::Blocked: StatusStr = TEXT("BLOCK"); break;
            case EDir_AgentStatus::Error: StatusStr = TEXT("ERROR"); break;
        }
        
        StatusText += FString::Printf(TEXT("#%d %s\n"), Task.AgentNumber, *StatusStr);
    }
    
    return StatusText;
}