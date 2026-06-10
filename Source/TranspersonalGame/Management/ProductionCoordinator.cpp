#include "ProductionCoordinator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create debug visualization mesh
    DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMesh"));
    DebugMesh->SetupAttachment(RootComponent);

    // Try to load a basic cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        DebugMesh->SetStaticMesh(CubeMeshAsset.Object);
        DebugMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }

    // Create status text component
    StatusText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusText"));
    StatusText->SetupAttachment(RootComponent);
    StatusText->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    StatusText->SetText(FText::FromString(TEXT("Production Coordinator\nInitializing...")));
    StatusText->SetTextRenderColor(FColor::Yellow);
    StatusText->SetWorldSize(50.0f);

    // Initialize production values
    CurrentCycle = 20;
    OverallProgress = 0.0f;
    CurrentPhase = TEXT("Milestone 1 - Walk Around");
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultMilestones();
    UpdateStatusText();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized for Cycle %d"), CurrentCycle);
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update overall progress every frame
    OverallProgress = CalculateOverallProgress();
    
    // Update status display periodically
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    if (UpdateTimer >= 2.0f)
    {
        UpdateStatusText();
        UpdateTimer = 0.0f;
    }
}

void AProductionCoordinator::UpdateMilestone(const FString& MilestoneName, bool bComplete, float Percentage)
{
    for (FDir_MilestoneStatus& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.bIsComplete = bComplete;
            Milestone.CompletionPercentage = FMath::Clamp(Percentage, 0.0f, 100.0f);
            Milestone.LastUpdate = FDateTime::Now().ToString();
            
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Updated milestone '%s' - Complete: %s, Progress: %.1f%%"), 
                   *MilestoneName, bComplete ? TEXT("Yes") : TEXT("No"), Percentage);
            return;
        }
    }
    
    // If milestone doesn't exist, create it
    AddMilestone(MilestoneName, TEXT("Unassigned"));
    UpdateMilestone(MilestoneName, bComplete, Percentage);
}

void AProductionCoordinator::AddMilestone(const FString& MilestoneName, const FString& AssignedAgent)
{
    FDir_MilestoneStatus NewMilestone;
    NewMilestone.MilestoneName = MilestoneName;
    NewMilestone.AssignedAgent = AssignedAgent;
    NewMilestone.LastUpdate = FDateTime::Now().ToString();
    
    Milestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Added milestone '%s' assigned to '%s'"), 
           *MilestoneName, *AssignedAgent);
}

bool AProductionCoordinator::IsMilestoneComplete(const FString& MilestoneName)
{
    for (const FDir_MilestoneStatus& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.bIsComplete;
        }
    }
    return false;
}

void AProductionCoordinator::AssignTask(const FString& AgentName, const FString& TaskDescription, int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bIsActive = true;
    NewTask.Status = TEXT("Assigned");
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Assigned task to %s: %s (Priority: %d)"), 
           *AgentName, *TaskDescription, Priority);
}

void AProductionCoordinator::CompleteTask(const FString& AgentName, const FString& TaskDescription)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName && Task.TaskDescription == TaskDescription)
        {
            Task.bIsActive = false;
            Task.Status = TEXT("Completed");
            
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Task completed by %s: %s"), 
                   *AgentName, *TaskDescription);
            return;
        }
    }
}

TArray<FDir_AgentTask> AProductionCoordinator::GetActiveTasksForAgent(const FString& AgentName)
{
    TArray<FDir_AgentTask> ActiveTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName && Task.bIsActive)
        {
            ActiveTasks.Add(Task);
        }
    }
    
    return ActiveTasks;
}

void AProductionCoordinator::AdvanceCycle()
{
    CurrentCycle++;
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Advanced to Cycle %d"), CurrentCycle);
}

float AProductionCoordinator::CalculateOverallProgress()
{
    if (Milestones.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_MilestoneStatus& Milestone : Milestones)
    {
        TotalProgress += Milestone.CompletionPercentage;
    }
    
    return TotalProgress / Milestones.Num();
}

void AProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - CYCLE %d ==="), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), OverallProgress);
    
    UE_LOG(LogTemp, Warning, TEXT("MILESTONES:"));
    for (const FDir_MilestoneStatus& Milestone : Milestones)
    {
        FString Status = Milestone.bIsComplete ? TEXT("COMPLETE") : TEXT("IN PROGRESS");
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s (%.1f%%) - %s"), 
               *Milestone.MilestoneName, *Status, Milestone.CompletionPercentage, *Milestone.AssignedAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ACTIVE TASKS:"));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bIsActive)
        {
            UE_LOG(LogTemp, Warning, TEXT("  %s: %s (Priority: %d)"), 
                   *Task.AgentName, *Task.TaskDescription, Task.Priority);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void AProductionCoordinator::InitializeMilestone1Tasks()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Add Milestone 1 tasks
    AssignTask(TEXT("Agent02_EngineArchitect"), TEXT("Validate UE5.5 compatibility and core architecture"), 1);
    AssignTask(TEXT("Agent03_CoreSystems"), TEXT("Implement character-dinosaur collision physics"), 1);
    AssignTask(TEXT("Agent05_WorldGenerator"), TEXT("Create terrain with height variation and biomes"), 2);
    AssignTask(TEXT("Agent09_CharacterArtist"), TEXT("Replace capsule with proper character mesh"), 2);
    AssignTask(TEXT("Agent10_Animation"), TEXT("Implement basic movement animations"), 3);
    AssignTask(TEXT("Agent12_CombatAI"), TEXT("Add basic dinosaur AI and collision detection"), 3);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized Milestone 1 tasks"));
}

void AProductionCoordinator::RefreshStatusDisplay()
{
    UpdateStatusText();
    GenerateProductionReport();
}

void AProductionCoordinator::UpdateStatusText()
{
    FString StatusString = FString::Printf(
        TEXT("PRODUCTION COORDINATOR\nCycle: %d\nPhase: %s\nProgress: %.1f%%\nMilestones: %d\nActive Tasks: %d"),
        CurrentCycle,
        *CurrentPhase,
        OverallProgress,
        Milestones.Num(),
        AgentTasks.Num()
    );
    
    StatusText->SetText(FText::FromString(StatusString));
    
    // Change color based on progress
    if (OverallProgress >= 80.0f)
    {
        StatusText->SetTextRenderColor(FColor::Green);
    }
    else if (OverallProgress >= 50.0f)
    {
        StatusText->SetTextRenderColor(FColor::Yellow);
    }
    else
    {
        StatusText->SetTextRenderColor(FColor::Red);
    }
}

void AProductionCoordinator::InitializeDefaultMilestones()
{
    // Clear existing milestones
    Milestones.Empty();
    
    // Add Milestone 1 requirements
    AddMilestone(TEXT("ThirdPersonCharacter"), TEXT("Agent09_CharacterArtist"));
    AddMilestone(TEXT("CameraBoomAndFollowCamera"), TEXT("Agent03_CoreSystems"));
    AddMilestone(TEXT("TerrainWithHeightVariation"), TEXT("Agent05_WorldGenerator"));
    AddMilestone(TEXT("PlayerMovementWASD"), TEXT("Agent03_CoreSystems"));
    AddMilestone(TEXT("DinosaurMeshesInWorld"), TEXT("Agent12_CombatAI"));
    AddMilestone(TEXT("DirectionalLightAndSky"), TEXT("Agent08_Lighting"));
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized default Milestone 1 requirements"));
}