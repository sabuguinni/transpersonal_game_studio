#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Landscape/Landscape.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    bAutoCoordination = true;
    CoordinationInterval = 30.0f; // Coordinate every 30 seconds
    LastCoordinationTime = 0.0f;
}

void UStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized"));
    
    // Initialize production metrics
    UpdateProductionMetrics();
    
    // Issue initial coordination commands
    if (bAutoCoordination)
    {
        IssueTerrainExpansionCommand();
        IssueDinosaurCreationCommand();
        IssueSurvivalHUDCommand();
    }
}

void UStudioDirectorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastCoordinationTime += DeltaTime;
    
    // Perform automatic coordination
    if (bAutoCoordination && LastCoordinationTime >= CoordinationInterval)
    {
        PerformAutomaticCoordination();
        LastCoordinationTime = 0.0f;
    }
    
    // Update metrics periodically
    static float MetricsUpdateTimer = 0.0f;
    MetricsUpdateTimer += DeltaTime;
    if (MetricsUpdateTimer >= 5.0f) // Update every 5 seconds
    {
        UpdateProductionMetrics();
        MetricsUpdateTimer = 0.0f;
    }
}

void UStudioDirectorSystem::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.AssignedTime = FDateTime::Now();
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Assigned task '%s' to agent '%s'"), 
           *TaskDescription, *AgentName);
}

void UStudioDirectorSystem::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent '%s' status updated to %d"), 
                   *AgentName, (int32)NewStatus);
            break;
        }
    }
}

TArray<FDir_AgentTask> UStudioDirectorSystem::GetActiveAgentTasks() const
{
    return ActiveTasks;
}

FDir_ProductionMetrics UStudioDirectorSystem::GetCurrentProductionMetrics() const
{
    return CurrentMetrics;
}

void UStudioDirectorSystem::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count total actors
    CurrentMetrics.TotalActorsInLevel = 0;
    CurrentMetrics.DinosaurActors = 0;
    CurrentMetrics.TerrainActors = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            CurrentMetrics.TotalActorsInLevel++;
            
            // Count dinosaur actors (containing "Dinosaur" or "Rex" or "Raptor")
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Dinosaur")) || 
                ActorName.Contains(TEXT("Rex")) || 
                ActorName.Contains(TEXT("Raptor")) ||
                ActorName.Contains(TEXT("Brachio")))
            {
                CurrentMetrics.DinosaurActors++;
            }
            
            // Count terrain actors
            if (Actor->IsA<ALandscape>() || ActorName.Contains(TEXT("Landscape")))
            {
                CurrentMetrics.TerrainActors++;
            }
        }
    }
    
    // Calculate terrain coverage (simplified)
    CurrentMetrics.TerrainCoverage = CurrentMetrics.TerrainActors > 0 ? 
        FMath::Min(100.0f, CurrentMetrics.TerrainActors * 20.0f) : 0.0f;
    
    // Check compilation health (simplified check)
    CurrentMetrics.bCompilationHealthy = CurrentMetrics.TotalActorsInLevel > 0;
    
    // Estimate frame rate (simplified)
    CurrentMetrics.FrameRate = GetWorld()->GetDeltaSeconds() > 0 ? 
        1.0f / GetWorld()->GetDeltaSeconds() : 60.0f;
    CurrentMetrics.FrameRate = FMath::Clamp(CurrentMetrics.FrameRate, 1.0f, 120.0f);
}

void UStudioDirectorSystem::IssueTerrainExpansionCommand()
{
    AssignTaskToAgent(TEXT("Agent #5 - Procedural World Generator"), 
                     TEXT("Expand terrain to 10km2 with 5 distinct biomes: Swamp SW, Forest NW, Savanna Center, Desert E, Mountain NE"), 
                     10.0f);
}

void UStudioDirectorSystem::IssueDinosaurCreationCommand()
{
    AssignTaskToAgent(TEXT("Agent #9 - Character Artist"), 
                     TEXT("Create dinosaur actors with collision and basic AI - TRex, Raptors, Brachiosaurus"), 
                     9.0f);
    
    AssignTaskToAgent(TEXT("Agent #12 - Combat AI"), 
                     TEXT("Implement dinosaur behavior trees and territorial AI"), 
                     8.0f);
}

void UStudioDirectorSystem::IssueSurvivalHUDCommand()
{
    AssignTaskToAgent(TEXT("Agent #12 - Combat System"), 
                     TEXT("Implement survival HUD with health, hunger, thirst, stamina bars"), 
                     7.0f);
}

void UStudioDirectorSystem::IssuePerformanceOptimizationCommand()
{
    AssignTaskToAgent(TEXT("Agent #4 - Performance Optimizer"), 
                     TEXT("Ensure 60fps PC / 30fps console performance with LOD optimization"), 
                     6.0f);
}

bool UStudioDirectorSystem::ValidateMinPlayableMap() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Check for minimum required elements
    bool bHasLandscape = false;
    bool bHasCharacter = false;
    bool bHasLighting = false;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            if (Actor->IsA<ALandscape>())
            {
                bHasLandscape = true;
            }
            else if (Actor->IsA<ACharacter>())
            {
                bHasCharacter = true;
            }
            else if (Actor->GetName().Contains(TEXT("Light")))
            {
                bHasLighting = true;
            }
        }
    }
    
    return bHasLandscape && bHasCharacter && bHasLighting;
}

TArray<FString> UStudioDirectorSystem::GetCriticalIssues() const
{
    TArray<FString> Issues;
    
    if (CurrentMetrics.TotalActorsInLevel < 10)
    {
        Issues.Add(TEXT("Too few actors in level - need more content"));
    }
    
    if (CurrentMetrics.DinosaurActors == 0)
    {
        Issues.Add(TEXT("No dinosaur actors found - core gameplay missing"));
    }
    
    if (CurrentMetrics.TerrainActors == 0)
    {
        Issues.Add(TEXT("No terrain found - world is empty"));
    }
    
    if (CurrentMetrics.FrameRate < 30.0f)
    {
        Issues.Add(TEXT("Performance issues - frame rate below 30fps"));
    }
    
    if (!CurrentMetrics.bCompilationHealthy)
    {
        Issues.Add(TEXT("Compilation errors detected"));
    }
    
    return Issues;
}

void UStudioDirectorSystem::PerformAutomaticCoordination()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Performing automatic coordination"));
    
    // Update metrics
    UpdateProductionMetrics();
    
    // Analyze current state and issue commands as needed
    if (CurrentMetrics.TerrainCoverage < 50.0f)
    {
        IssueTerrainExpansionCommand();
    }
    
    if (CurrentMetrics.DinosaurActors < 5)
    {
        IssueDinosaurCreationCommand();
    }
    
    if (CurrentMetrics.FrameRate < 45.0f)
    {
        IssuePerformanceOptimizationCommand();
    }
    
    // Clean up completed tasks
    ActiveTasks.RemoveAll([](const FDir_AgentTask& Task) {
        return Task.Status == EDir_AgentStatus::Complete;
    });
}

void UStudioDirectorSystem::AnalyzeAgentPerformance()
{
    // Analyze which agents are taking too long or failing
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        FTimespan TaskAge = FDateTime::Now() - Task.AssignedTime;
        if (TaskAge.GetTotalMinutes() > 10.0 && Task.Status == EDir_AgentStatus::Working)
        {
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent '%s' task taking too long"), 
                   *Task.AgentName);
        }
    }
}

void UStudioDirectorSystem::OptimizeTaskDistribution()
{
    // Sort tasks by priority
    ActiveTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
}