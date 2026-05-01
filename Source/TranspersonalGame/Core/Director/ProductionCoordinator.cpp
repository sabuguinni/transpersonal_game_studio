#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create coordinator mesh component
    CoordinatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoordinatorMesh"));
    CoordinatorMesh->SetupAttachment(RootComponent);

    // Try to load a default cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CoordinatorMesh->SetStaticMesh(CubeMeshAsset.Object);
        CoordinatorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }

    // Create status display
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    StatusDisplay->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    StatusDisplay->SetText(FText::FromString(TEXT("Production Coordinator\nInitializing...")));
    StatusDisplay->SetTextRenderColor(FColor::Green);
    StatusDisplay->SetXScale(2.0f);
    StatusDisplay->SetYScale(2.0f);

    // Initialize default values
    CurrentCycleID = TEXT("AUTO_009");
    TotalCycles = 0;
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup default milestones and tasks
    SetupDefaultMilestones();
    SetupDefaultTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: System initialized for cycle %s"), *CurrentCycleID);
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update status display every few seconds
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    if (UpdateTimer >= 3.0f)
    {
        UpdateStatusDisplay();
        UpdateTimer = 0.0f;
    }
}

void ADir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Updated %s status to %d"), *AgentName, (int32)NewStatus);
            break;
        }
    }
    UpdateStatusDisplay();
}

void ADir_ProductionCoordinator::AddAgentTask(const FString& AgentName, const FString& TaskDescription, int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Waiting;
    NewTask.CycleID = CurrentCycleID;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Added task for %s: %s"), *AgentName, *TaskDescription);
}

void ADir_ProductionCoordinator::CompleteMilestone(const FString& MilestoneName)
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.bCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Milestone completed - %s"), *MilestoneName);
            break;
        }
    }
    UpdateStatusDisplay();
}

bool ADir_ProductionCoordinator::AreAllAgentsReady()
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            return false;
        }
    }
    return true;
}

FString ADir_ProductionCoordinator::GetProductionSummary()
{
    FString Summary = FString::Printf(TEXT("Production Summary - Cycle %s\n"), *CurrentCycleID);
    
    int32 WaitingCount = 0;
    int32 ActiveCount = 0;
    int32 CompletedCount = 0;
    int32 BlockedCount = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Waiting: WaitingCount++; break;
            case EDir_AgentStatus::Active: ActiveCount++; break;
            case EDir_AgentStatus::Completed: CompletedCount++; break;
            case EDir_AgentStatus::Blocked: BlockedCount++; break;
        }
    }
    
    Summary += FString::Printf(TEXT("Tasks: %d Waiting, %d Active, %d Completed, %d Blocked\n"), 
                              WaitingCount, ActiveCount, CompletedCount, BlockedCount);
    
    int32 CompletedMilestones = 0;
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.bCompleted)
        {
            CompletedMilestones++;
        }
    }
    
    Summary += FString::Printf(TEXT("Milestones: %d/%d Completed"), CompletedMilestones, ProductionMilestones.Num());
    
    return Summary;
}

void ADir_ProductionCoordinator::InitializeProductionCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    TotalCycles++;
    
    // Reset all task statuses for new cycle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status != EDir_AgentStatus::Completed)
        {
            Task.Status = EDir_AgentStatus::Waiting;
        }
        Task.CycleID = CycleID;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized cycle %s (Total: %d)"), *CycleID, TotalCycles);
    UpdateStatusDisplay();
}

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetPendingTasks()
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Waiting || Task.Status == EDir_AgentStatus::Active)
        {
            PendingTasks.Add(Task);
        }
    }
    
    // Sort by priority
    PendingTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    return PendingTasks;
}

void ADir_ProductionCoordinator::SetupDefaultTasks()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Add critical path tasks for Milestone 1
    AddAgentTask(TEXT("Engine Architect"), TEXT("Define core architecture and module structure"), 10);
    AddAgentTask(TEXT("Core Systems"), TEXT("Implement physics and collision systems"), 9);
    AddAgentTask(TEXT("World Generator"), TEXT("Create real terrain with height variation"), 8);
    AddAgentTask(TEXT("Environment Artist"), TEXT("Add vegetation and props to terrain"), 7);
    AddAgentTask(TEXT("Character Artist"), TEXT("Improve TranspersonalCharacter visuals"), 6);
    AddAgentTask(TEXT("Animation Agent"), TEXT("Add movement animations to character"), 5);
    AddAgentTask(TEXT("Combat AI"), TEXT("Implement basic dinosaur AI behavior"), 4);
    AddAgentTask(TEXT("Audio Agent"), TEXT("Add ambient sounds and footsteps"), 3);
    AddAgentTask(TEXT("VFX Agent"), TEXT("Add particle effects for atmosphere"), 2);
    AddAgentTask(TEXT("QA Testing"), TEXT("Test playable prototype functionality"), 1);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Default tasks setup complete"));
}

void ADir_ProductionCoordinator::ResetAllTasks()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Waiting;
    }
    
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        Milestone.bCompleted = false;
    }
    
    UpdateStatusDisplay();
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: All tasks and milestones reset"));
}

void ADir_ProductionCoordinator::UpdateStatusDisplay()
{
    if (StatusDisplay)
    {
        FString DisplayText = FString::Printf(TEXT("Production Coordinator\nCycle: %s\n%s"), 
                                            *CurrentCycleID, 
                                            *GetProductionSummary());
        StatusDisplay->SetText(FText::FromString(DisplayText));
    }
}

void ADir_ProductionCoordinator::SetupDefaultMilestones()
{
    // Clear existing milestones
    ProductionMilestones.Empty();
    
    // Milestone 1: Walk Around
    FDir_ProductionMilestone Milestone1;
    Milestone1.MilestoneName = TEXT("Walk Around");
    Milestone1.Description = TEXT("Player can walk, run, jump in the world");
    Milestone1.bCompleted = false;
    Milestone1.RequiredAgents.Add(TEXT("Engine Architect"));
    Milestone1.RequiredAgents.Add(TEXT("Core Systems"));
    Milestone1.RequiredAgents.Add(TEXT("Character Artist"));
    Milestone1.RequiredAgents.Add(TEXT("Animation Agent"));
    Milestone1.WorldLocation = FVector(-1000, 0, 200);
    ProductionMilestones.Add(Milestone1);
    
    // Milestone 2: Dinosaurs Visible
    FDir_ProductionMilestone Milestone2;
    Milestone2.MilestoneName = TEXT("Dinosaurs Visible");
    Milestone2.Description = TEXT("Dinosaurs are visible and moving in the world");
    Milestone2.bCompleted = false;
    Milestone2.RequiredAgents.Add(TEXT("World Generator"));
    Milestone2.RequiredAgents.Add(TEXT("Environment Artist"));
    Milestone2.RequiredAgents.Add(TEXT("Combat AI"));
    Milestone2.WorldLocation = FVector(-2000, 0, 200);
    ProductionMilestones.Add(Milestone2);
    
    // Milestone 3: Survival Mechanics
    FDir_ProductionMilestone Milestone3;
    Milestone3.MilestoneName = TEXT("Basic Survival");
    Milestone3.Description = TEXT("Health, hunger, thirst mechanics work");
    Milestone3.bCompleted = false;
    Milestone3.RequiredAgents.Add(TEXT("Audio Agent"));
    Milestone3.RequiredAgents.Add(TEXT("VFX Agent"));
    Milestone3.RequiredAgents.Add(TEXT("QA Testing"));
    Milestone3.WorldLocation = FVector(-3000, 0, 200);
    ProductionMilestones.Add(Milestone3);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Default milestones setup complete"));
}