#include "StudioDirectorCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UStudioDirectorCoordinator::UStudioDirectorCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    TotalActorsSpawned = 0;
    DinosaurActorsCount = 0;
    bPlayerMovementWorking = false;
    bMilestone1Complete = false;
}

void UStudioDirectorCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Coordinator initialized"));
    InitializeAgentChain();
    
    // Start validation timer
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &UStudioDirectorCoordinator::ValidateProductionState,
        5.0f, // Every 5 seconds
        true
    );
}

void UStudioDirectorCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAgentChain();
    ProcessTaskQueue();
}

void UStudioDirectorCoordinator::InitializeAgentChain()
{
    AgentStatusList.Empty();
    
    // Initialize all 19 agents
    for (int32 i = 1; i <= 19; i++)
    {
        FDir_AgentStatus NewAgent;
        NewAgent.AgentNumber = i;
        NewAgent.AgentName = FString::Printf(TEXT("Agent_%02d"), i);
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.ProgressPercent = 0.0f;
        NewAgent.LastUpdateTime = FDateTime::Now();
        
        AgentStatusList.Add(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent chain initialized with %d agents"), AgentStatusList.Num());
}

void UStudioDirectorCoordinator::DispatchTaskToAgent(int32 AgentNumber, const FString& TaskDescription)
{
    if (AgentNumber < 1 || AgentNumber > 19)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid agent number: %d"), AgentNumber);
        return;
    }
    
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = EDir_TaskPriority::High;
    NewTask.CreatedTime = FDateTime::Now();
    NewTask.Status = EDir_TaskStatus::Pending;
    
    PendingTasks.Add(NewTask);
    
    // Update agent status
    if (AgentStatusList.IsValidIndex(AgentNumber - 1))
    {
        AgentStatusList[AgentNumber - 1].Status = EDir_AgentStatus::Working;
        AgentStatusList[AgentNumber - 1].CurrentTask = TaskDescription;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Task dispatched to Agent %d: %s"), AgentNumber, *TaskDescription);
}

void UStudioDirectorCoordinator::ReportAgentProgress(int32 AgentNumber, float ProgressPercent, const FString& StatusMessage)
{
    if (AgentStatusList.IsValidIndex(AgentNumber - 1))
    {
        AgentStatusList[AgentNumber - 1].ProgressPercent = ProgressPercent;
        AgentStatusList[AgentNumber - 1].StatusMessage = StatusMessage;
        AgentStatusList[AgentNumber - 1].LastUpdateTime = FDateTime::Now();
        
        if (ProgressPercent >= 100.0f)
        {
            AgentStatusList[AgentNumber - 1].Status = EDir_AgentStatus::Complete;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Agent %d progress: %.1f%% - %s"), AgentNumber, ProgressPercent, *StatusMessage);
}

bool UStudioDirectorCoordinator::ValidateMinimumViablePrototype()
{
    CheckMilestone1WalkAround();
    
    // Check all MVP requirements
    bool bHasPlayerCharacter = false;
    bool bHasLandscape = false;
    bool bHasDinosaurs = false;
    bool bHasLighting = false;
    
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Count actors and validate requirements
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    TotalActorsSpawned = AllActors.Num();
    DinosaurActorsCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetName().Contains(TEXT("Character")) || Actor->GetName().Contains(TEXT("Player")))
        {
            bHasPlayerCharacter = true;
        }
        else if (Actor->GetName().Contains(TEXT("Landscape")) || Actor->GetName().Contains(TEXT("Terrain")))
        {
            bHasLandscape = true;
        }
        else if (Actor->GetName().Contains(TEXT("Rex")) || Actor->GetName().Contains(TEXT("Raptor")) || 
                 Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            bHasDinosaurs = true;
            DinosaurActorsCount++;
        }
        else if (Actor->GetName().Contains(TEXT("Light")) || Actor->GetName().Contains(TEXT("Sun")))
        {
            bHasLighting = true;
        }
    }
    
    bMilestone1Complete = bHasPlayerCharacter && bHasLandscape && bHasDinosaurs && bHasLighting;
    
    UE_LOG(LogTemp, Warning, TEXT("MVP Validation: Player=%s, Landscape=%s, Dinosaurs=%s, Lighting=%s"),
        bHasPlayerCharacter ? TEXT("YES") : TEXT("NO"),
        bHasLandscape ? TEXT("YES") : TEXT("NO"),
        bHasDinosaurs ? TEXT("YES") : TEXT("NO"),
        bHasLighting ? TEXT("YES") : TEXT("NO"));
    
    return bMilestone1Complete;
}

void UStudioDirectorCoordinator::CheckMilestone1WalkAround()
{
    // Priority tasks for Milestone 1
    if (!bPlayerMovementWorking)
    {
        DispatchTaskToAgent(9, TEXT("Implement WASD movement for TranspersonalCharacter"));
        DispatchTaskToAgent(10, TEXT("Add camera boom and follow camera"));
    }
    
    if (DinosaurActorsCount < 5)
    {
        DispatchTaskToAgent(12, TEXT("Spawn 5 dinosaur actors with collision"));
    }
    
    if (TotalActorsSpawned < 100)
    {
        DispatchTaskToAgent(5, TEXT("Generate landscape with terrain variation"));
        DispatchTaskToAgent(6, TEXT("Populate world with environmental props"));
    }
}

void UStudioDirectorCoordinator::SpawnDinosaurActors()
{
    // This will be called by UE5 Python scripts
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur spawn request logged - execute via Python"));
}

void UStudioDirectorCoordinator::ValidatePlayerMovement()
{
    // Check if player character exists and has movement component
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (PlayerPawn)
    {
        bPlayerMovementWorking = true;
        UE_LOG(LogTemp, Warning, TEXT("Player movement validated successfully"));
    }
    else
    {
        bPlayerMovementWorking = false;
        UE_LOG(LogTemp, Error, TEXT("Player movement validation failed - no player pawn"));
    }
}

void UStudioDirectorCoordinator::UpdateAgentChain()
{
    // Check for stalled agents
    FDateTime CurrentTime = FDateTime::Now();
    
    for (FDir_AgentStatus& Agent : AgentStatusList)
    {
        FTimespan TimeSinceUpdate = CurrentTime - Agent.LastUpdateTime;
        
        if (TimeSinceUpdate.GetTotalMinutes() > 10.0 && Agent.Status == EDir_AgentStatus::Working)
        {
            Agent.Status = EDir_AgentStatus::Stalled;
            UE_LOG(LogTemp, Warning, TEXT("Agent %d marked as stalled"), Agent.AgentNumber);
        }
    }
}

void UStudioDirectorCoordinator::ProcessTaskQueue()
{
    // Move completed tasks
    for (int32 i = PendingTasks.Num() - 1; i >= 0; i--)
    {
        if (PendingTasks[i].Status == EDir_TaskStatus::Complete)
        {
            CompletedTasks.Add(PendingTasks[i]);
            PendingTasks.RemoveAt(i);
        }
    }
}

void UStudioDirectorCoordinator::ValidateProductionState()
{
    LogProductionMetrics();
    ValidateMinimumViablePrototype();
    
    // Auto-dispatch critical tasks if needed
    if (!bMilestone1Complete)
    {
        CheckMilestone1WalkAround();
    }
}

bool UStudioDirectorCoordinator::CheckAgentDependencies(int32 AgentNumber)
{
    // Agent dependency chain validation
    switch (AgentNumber)
    {
        case 3: // Core Systems depends on Engine Architect
            return AgentStatusList.IsValidIndex(1) && AgentStatusList[1].Status == EDir_AgentStatus::Complete;
        case 5: // World Generator depends on Core Systems
            return AgentStatusList.IsValidIndex(2) && AgentStatusList[2].Status == EDir_AgentStatus::Complete;
        case 6: // Environment depends on World Generator
            return AgentStatusList.IsValidIndex(4) && AgentStatusList[4].Status == EDir_AgentStatus::Complete;
        default:
            return true;
    }
}

void UStudioDirectorCoordinator::LogProductionMetrics()
{
    int32 WorkingAgents = 0;
    int32 CompleteAgents = 0;
    int32 StalledAgents = 0;
    
    for (const FDir_AgentStatus& Agent : AgentStatusList)
    {
        switch (Agent.Status)
        {
            case EDir_AgentStatus::Working: WorkingAgents++; break;
            case EDir_AgentStatus::Complete: CompleteAgents++; break;
            case EDir_AgentStatus::Stalled: StalledAgents++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production Metrics: %d Working, %d Complete, %d Stalled, %d Total Actors, %d Dinosaurs"),
        WorkingAgents, CompleteAgents, StalledAgents, TotalActorsSpawned, DinosaurActorsCount);
}