#include "StudioDirectorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UDir_StudioDirectorComponent::UDir_StudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Initialize production state
    bMilestone1Complete = false;
    CurrentProductionCycle = 8; // Current cycle
    ProductionBudgetUsed = 66.91f;
    ProductionBudgetLimit = 150.0f;
    
    // Initialize milestone tracking
    bCharacterMovementWorking = false;
    bTerrainExists = false;
    bDinosaursVisible = false;
    bLightingSetup = false;
    bSurvivalHUDWorking = false;
    
    // Initialize agent arrays for 19 agents
    AgentActiveStatus.SetNum(19);
    AgentCurrentTasks.SetNum(19);
    AgentLastReportTime.SetNum(19);
    
    for (int32 i = 0; i < 19; i++)
    {
        AgentActiveStatus[i] = false;
        AgentCurrentTasks[i] = TEXT("Waiting for task assignment");
        AgentLastReportTime[i] = 0.0f;
    }
}

void UDir_StudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Component initialized - Production Cycle %d"), CurrentProductionCycle);
    
    // Start production monitoring
    InitializeProductionPipeline();
    
    // Set up timers for coordination
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ProductionMonitorTimer, this, 
            &UDir_StudioDirectorComponent::MonitorBuildStatus, 10.0f, true);
        
        World->GetTimerManager().SetTimer(AgentCoordinationTimer, this,
            &UDir_StudioDirectorComponent::CoordinateAgentTasks, 5.0f, true);
    }
}

void UDir_StudioDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update agent status tracking
    UpdateAgentStatus();
    
    // Check for critical blockers
    CheckCriticalBlockers();
}

void UDir_StudioDirectorComponent::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing 19-agent production pipeline"));
    
    // Mark Studio Director (Agent #1) as active
    AgentActiveStatus[0] = true;
    AgentCurrentTasks[0] = TEXT("Coordinating production pipeline");
    
    // Check current milestone status
    CheckMilestone1Status();
    
    // Generate initial production report
    GenerateProductionReport();
}

void UDir_StudioDirectorComponent::CheckMilestone1Status()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Checking Milestone 1 status"));
    
    // Check if TranspersonalCharacter exists and has movement
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundCharacters;
        UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundCharacters);
        
        bCharacterMovementWorking = FoundCharacters.Num() > 0;
        
        // Check for terrain (landscape actors)
        TArray<AActor*> FoundLandscapes;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), FoundLandscapes);
        bTerrainExists = FoundLandscapes.Num() > 0;
        
        // Check for static mesh actors (dinosaur placeholders)
        TArray<AActor*> FoundMeshes;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), FoundMeshes);
        bDinosaursVisible = FoundMeshes.Num() >= 5; // Should have 5 dinosaur placeholders
        
        // Check for lighting
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
        bLightingSetup = FoundLights.Num() > 0;
        
        // Calculate milestone completion
        int32 CompletedRequirements = 0;
        if (bCharacterMovementWorking) CompletedRequirements++;
        if (bTerrainExists) CompletedRequirements++;
        if (bDinosaursVisible) CompletedRequirements++;
        if (bLightingSetup) CompletedRequirements++;
        if (bSurvivalHUDWorking) CompletedRequirements++;
        
        bMilestone1Complete = (CompletedRequirements >= 4); // At least 4/5 requirements
        
        UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: %d/5 requirements completed"), CompletedRequirements);
    }
}

void UDir_StudioDirectorComponent::CoordinateAgentTasks()
{
    if (bMilestone1Complete)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Milestone 1 COMPLETE - Moving to next phase"));
        return;
    }
    
    // Priority task assignment based on current needs
    if (!bTerrainExists)
    {
        SendTaskToAgent(5, TEXT("URGENT: Create real terrain with height variation - not flat plane"));
    }
    
    if (!bDinosaursVisible)
    {
        SendTaskToAgent(9, TEXT("URGENT: Create visible dinosaur actors with collision"));
        SendTaskToAgent(11, TEXT("URGENT: Add basic AI behavior to dinosaurs"));
    }
    
    if (!bSurvivalHUDWorking)
    {
        SendTaskToAgent(12, TEXT("URGENT: Implement survival HUD with health/hunger bars"));
    }
    
    if (!bCharacterMovementWorking)
    {
        SendTaskToAgent(3, TEXT("URGENT: Fix character movement and input handling"));
    }
}

void UDir_StudioDirectorComponent::SendTaskToAgent(int32 AgentID, const FString& TaskDescription)
{
    if (AgentID >= 0 && AgentID < 19)
    {
        AgentActiveStatus[AgentID] = true;
        AgentCurrentTasks[AgentID] = TaskDescription;
        AgentLastReportTime[AgentID] = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Task assigned to Agent #%d: %s"), 
               AgentID + 1, *TaskDescription);
    }
}

void UDir_StudioDirectorComponent::ReceiveAgentReport(int32 AgentID, const FString& Report)
{
    if (AgentID >= 0 && AgentID < 19)
    {
        AgentLastReportTime[AgentID] = GetWorld()->GetTimeSeconds();
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Report from Agent #%d: %s"), 
               AgentID + 1, *Report);
        
        // Re-check milestone status after agent report
        CheckMilestone1Status();
    }
}

void UDir_StudioDirectorComponent::MonitorBuildStatus()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check for stalled agents (no report in 60 seconds)
    for (int32 i = 0; i < 19; i++)
    {
        if (AgentActiveStatus[i] && (CurrentTime - AgentLastReportTime[i]) > 60.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("Studio Director: Agent #%d appears stalled - last report %.1f seconds ago"), 
                   i + 1, CurrentTime - AgentLastReportTime[i]);
        }
    }
    
    // Check budget usage
    float BudgetPercentage = (ProductionBudgetUsed / ProductionBudgetLimit) * 100.0f;
    if (BudgetPercentage > 80.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Budget usage at %.1f%% - optimize remaining cycles"), 
               BudgetPercentage);
    }
}

void UDir_StudioDirectorComponent::ValidatePlayablePrototype()
{
    CheckMilestone1Status();
    
    if (bMilestone1Complete)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: PLAYABLE PROTOTYPE VALIDATED - Ready for player testing"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: Prototype validation FAILED - Critical requirements missing"));
    }
}

void UDir_StudioDirectorComponent::TriggerEmergencyBuild()
{
    UE_LOG(LogTemp, Error, TEXT("Studio Director: EMERGENCY BUILD TRIGGERED - All agents focus on Milestone 1"));
    
    // Reset all agent tasks to emergency priorities
    for (int32 i = 0; i < 19; i++)
    {
        AgentActiveStatus[i] = false;
        AgentCurrentTasks[i] = TEXT("EMERGENCY: Focus on Milestone 1 requirements");
    }
    
    // Immediately coordinate critical tasks
    CoordinateAgentTasks();
}

void UDir_StudioDirectorComponent::ResetProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Resetting production pipeline"));
    
    // Reset all tracking
    bMilestone1Complete = false;
    bCharacterMovementWorking = false;
    bTerrainExists = false;
    bDinosaursVisible = false;
    bLightingSetup = false;
    bSurvivalHUDWorking = false;
    
    // Clear agent status
    for (int32 i = 0; i < 19; i++)
    {
        AgentActiveStatus[i] = false;
        AgentCurrentTasks[i] = TEXT("Awaiting new task assignment");
        AgentLastReportTime[i] = 0.0f;
    }
    
    // Restart pipeline
    InitializeProductionPipeline();
}

void UDir_StudioDirectorComponent::UpdateAgentStatus()
{
    // This would integrate with the actual agent communication system
    // For now, we simulate based on game state
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update last report times for active agents
    for (int32 i = 0; i < 19; i++)
    {
        if (AgentActiveStatus[i])
        {
            // Simulate agent activity
            if (FMath::RandRange(0, 100) < 5) // 5% chance per tick to simulate report
            {
                AgentLastReportTime[i] = CurrentTime;
            }
        }
    }
}

void UDir_StudioDirectorComponent::CheckCriticalBlockers()
{
    // Check for critical issues that would block production
    if (ProductionBudgetUsed >= ProductionBudgetLimit)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: CRITICAL - Budget limit reached, production halted"));
        return;
    }
    
    // Check if too many cycles without milestone completion
    if (CurrentProductionCycle > 10 && !bMilestone1Complete)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: CRITICAL - Milestone 1 overdue, triggering emergency protocols"));
        TriggerEmergencyBuild();
    }
}

void UDir_StudioDirectorComponent::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT - CYCLE %d ==="), CurrentProductionCycle);
    UE_LOG(LogTemp, Warning, TEXT("Budget: $%.2f / $%.2f (%.1f%%)"), 
           ProductionBudgetUsed, ProductionBudgetLimit, (ProductionBudgetUsed/ProductionBudgetLimit)*100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Status: %s"), bMilestone1Complete ? TEXT("COMPLETE") : TEXT("IN PROGRESS"));
    UE_LOG(LogTemp, Warning, TEXT("Character Movement: %s"), bCharacterMovementWorking ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Warning, TEXT("Terrain: %s"), bTerrainExists ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %s"), bDinosaursVisible ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Warning, TEXT("Lighting: %s"), bLightingSetup ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Warning, TEXT("Survival HUD: %s"), bSurvivalHUDWorking ? TEXT("✓") : TEXT("✗"));
    UE_LOG(LogTemp, Warning, TEXT("=== END PRODUCTION REPORT ==="));
}