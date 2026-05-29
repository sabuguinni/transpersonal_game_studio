#include "Director_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    CurrentPhase = EDir_ProductionPhase::Phase1_BasicMovement;
    bCharacterMovementComplete = false;
    bSurvivalSystemsComplete = false;
    bDinosaurAIComplete = false;
    bTerrainSystemComplete = false;
    LastProgressUpdate = 0.0f;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    InitializeProductionPipeline();
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastProgressUpdate += DeltaTime;
    if (LastProgressUpdate >= 5.0f)
    {
        UpdateMilestoneProgress();
        CheckDependencies();
        ValidateAgentSequencing();
        LastProgressUpdate = 0.0f;
    }
}

void UDir_ProductionCoordinator::InitializeProductionPipeline()
{
    // Clear existing data
    ProductionMilestones.Empty();
    AgentAssignments.Empty();
    CriticalPathErrors.Empty();

    // Initialize Phase 1: Basic Movement
    CreateMilestone(TEXT("Character WASD Movement"), EDir_ProductionPhase::Phase1_BasicMovement, 
        {TEXT("TranspersonalCharacter.cpp"), TEXT("Input Component"), TEXT("Camera System")});
    
    CreateMilestone(TEXT("Third Person Camera"), EDir_ProductionPhase::Phase1_BasicMovement,
        {TEXT("SpringArm Component"), TEXT("Camera Component"), TEXT("Mouse Look")});

    CreateMilestone(TEXT("Basic Terrain Collision"), EDir_ProductionPhase::Phase1_BasicMovement,
        {TEXT("Landscape Actor"), TEXT("Collision Mesh"), TEXT("Physics Integration")});

    // Initialize Phase 2: Survival Systems
    CreateMilestone(TEXT("Survival Stats"), EDir_ProductionPhase::Phase2_SurvivalSystems,
        {TEXT("Health Component"), TEXT("Hunger System"), TEXT("Thirst System"), TEXT("Stamina System")});

    CreateMilestone(TEXT("Survival HUD"), EDir_ProductionPhase::Phase2_SurvivalSystems,
        {TEXT("Health Bar"), TEXT("Hunger Bar"), TEXT("Thirst Bar"), TEXT("Stamina Bar")});

    // Initialize Phase 3: Dinosaur AI
    CreateMilestone(TEXT("T-Rex Basic AI"), EDir_ProductionPhase::Phase3_DinosaurAI,
        {TEXT("DinosaurCombatAIController.cpp"), TEXT("Behavior Tree"), TEXT("Detection System")});

    CreateMilestone(TEXT("Dinosaur Spawning"), EDir_ProductionPhase::Phase3_DinosaurAI,
        {TEXT("Spawn Points"), TEXT("Dinosaur Meshes"), TEXT("AI Controllers")});

    // Initialize Agent Assignments
    for (int32 i = 1; i <= 19; ++i)
    {
        FDir_AgentTaskAssignment Assignment;
        Assignment.AgentID = i;
        Assignment.AgentName = FString::Printf(TEXT("Agent #%d"), i);
        
        // Assign phases based on agent specialization
        if (i <= 4)
        {
            Assignment.AssignedPhase = EDir_ProductionPhase::Phase1_BasicMovement;
            Assignment.CurrentTask = TEXT("Core Systems Implementation");
        }
        else if (i <= 8)
        {
            Assignment.AssignedPhase = EDir_ProductionPhase::Phase2_SurvivalSystems;
            Assignment.CurrentTask = TEXT("World and Environment Systems");
        }
        else if (i <= 13)
        {
            Assignment.AssignedPhase = EDir_ProductionPhase::Phase3_DinosaurAI;
            Assignment.CurrentTask = TEXT("Character and AI Systems");
        }
        else
        {
            Assignment.AssignedPhase = EDir_ProductionPhase::Phase4_WorldPopulation;
            Assignment.CurrentTask = TEXT("Content and Polish");
        }
        
        AgentAssignments.Add(Assignment);
    }

    UE_LOG(LogTemp, Warning, TEXT("Production Pipeline Initialized - %d milestones, %d agents"), 
        ProductionMilestones.Num(), AgentAssignments.Num());
}

void UDir_ProductionCoordinator::AdvanceToNextPhase()
{
    if (!ValidatePhaseCompletion(CurrentPhase))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot advance phase - current phase not complete"));
        return;
    }

    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::Phase1_BasicMovement:
            CurrentPhase = EDir_ProductionPhase::Phase2_SurvivalSystems;
            break;
        case EDir_ProductionPhase::Phase2_SurvivalSystems:
            CurrentPhase = EDir_ProductionPhase::Phase3_DinosaurAI;
            break;
        case EDir_ProductionPhase::Phase3_DinosaurAI:
            CurrentPhase = EDir_ProductionPhase::Phase4_WorldPopulation;
            break;
        case EDir_ProductionPhase::Phase4_WorldPopulation:
            CurrentPhase = EDir_ProductionPhase::Phase5_Polish;
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Already at final phase"));
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Advanced to phase: %d"), (int32)CurrentPhase);
}

bool UDir_ProductionCoordinator::ValidatePhaseCompletion(EDir_ProductionPhase Phase)
{
    switch (Phase)
    {
        case EDir_ProductionPhase::Phase1_BasicMovement:
            return bCharacterMovementComplete && bTerrainSystemComplete;
        case EDir_ProductionPhase::Phase2_SurvivalSystems:
            return bSurvivalSystemsComplete;
        case EDir_ProductionPhase::Phase3_DinosaurAI:
            return bDinosaurAIComplete;
        default:
            return true;
    }
}

void UDir_ProductionCoordinator::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, EDir_ProductionPhase Phase)
{
    FDir_AgentTaskAssignment* Assignment = AgentAssignments.FindByPredicate([AgentID](const FDir_AgentTaskAssignment& A) {
        return A.AgentID == AgentID;
    });

    if (Assignment)
    {
        Assignment->CurrentTask = TaskDescription;
        Assignment->AssignedPhase = Phase;
        Assignment->TaskProgress = 0.0f;
        Assignment->bTaskBlocked = false;
        
        UE_LOG(LogTemp, Log, TEXT("Assigned task to Agent #%d: %s"), AgentID, *TaskDescription);
    }
}

void UDir_ProductionCoordinator::ReportAgentProgress(int32 AgentID, float ProgressPercentage)
{
    FDir_AgentTaskAssignment* Assignment = AgentAssignments.FindByPredicate([AgentID](const FDir_AgentTaskAssignment& A) {
        return A.AgentID == AgentID;
    });

    if (Assignment)
    {
        Assignment->TaskProgress = FMath::Clamp(ProgressPercentage, 0.0f, 100.0f);
        UE_LOG(LogTemp, Log, TEXT("Agent #%d progress: %.1f%%"), AgentID, ProgressPercentage);
    }
}

void UDir_ProductionCoordinator::BlockAgent(int32 AgentID, const FString& BlockingReason)
{
    FDir_AgentTaskAssignment* Assignment = AgentAssignments.FindByPredicate([AgentID](const FDir_AgentTaskAssignment& A) {
        return A.AgentID == AgentID;
    });

    if (Assignment)
    {
        Assignment->bTaskBlocked = true;
        CriticalPathErrors.AddUnique(FString::Printf(TEXT("Agent #%d blocked: %s"), AgentID, *BlockingReason));
        UE_LOG(LogTemp, Error, TEXT("Agent #%d blocked: %s"), AgentID, *BlockingReason);
    }
}

void UDir_ProductionCoordinator::UnblockAgent(int32 AgentID)
{
    FDir_AgentTaskAssignment* Assignment = AgentAssignments.FindByPredicate([AgentID](const FDir_AgentTaskAssignment& A) {
        return A.AgentID == AgentID;
    });

    if (Assignment)
    {
        Assignment->bTaskBlocked = false;
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d unblocked"), AgentID);
    }
}

void UDir_ProductionCoordinator::CreateMilestone(const FString& Name, EDir_ProductionPhase Phase, const TArray<FString>& Requirements)
{
    FDir_ProductionMilestone Milestone;
    Milestone.MilestoneName = Name;
    Milestone.Phase = Phase;
    Milestone.RequiredComponents = Requirements;
    Milestone.bCompleted = false;
    Milestone.CompletionPercentage = 0.0f;
    
    ProductionMilestones.Add(Milestone);
}

void UDir_ProductionCoordinator::CompleteMilestone(const FString& MilestoneName)
{
    FDir_ProductionMilestone* Milestone = ProductionMilestones.FindByPredicate([&MilestoneName](const FDir_ProductionMilestone& M) {
        return M.MilestoneName == MilestoneName;
    });

    if (Milestone)
    {
        Milestone->bCompleted = true;
        Milestone->CompletionPercentage = 100.0f;
        UE_LOG(LogTemp, Warning, TEXT("Milestone completed: %s"), *MilestoneName);
    }
}

float UDir_ProductionCoordinator::GetOverallProgress() const
{
    if (ProductionMilestones.Num() == 0) return 0.0f;

    float TotalProgress = 0.0f;
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        TotalProgress += Milestone.CompletionPercentage;
    }

    return TotalProgress / ProductionMilestones.Num();
}

void UDir_ProductionCoordinator::ValidateCharacterMovement()
{
    // Check if TranspersonalCharacter has movement implementation
    bCharacterMovementComplete = true; // Placeholder - would check actual implementation
    UE_LOG(LogTemp, Warning, TEXT("Character movement validation: %s"), bCharacterMovementComplete ? TEXT("PASS") : TEXT("FAIL"));
}

void UDir_ProductionCoordinator::ValidateSurvivalSystems()
{
    // Check if survival components are implemented
    bSurvivalSystemsComplete = false; // Placeholder - would check actual implementation
    UE_LOG(LogTemp, Warning, TEXT("Survival systems validation: %s"), bSurvivalSystemsComplete ? TEXT("PASS") : TEXT("FAIL"));
}

void UDir_ProductionCoordinator::ValidateDinosaurAI()
{
    // Check if dinosaur AI is implemented
    bDinosaurAIComplete = false; // Placeholder - would check actual implementation
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur AI validation: %s"), bDinosaurAIComplete ? TEXT("PASS") : TEXT("FAIL"));
}

void UDir_ProductionCoordinator::ValidateTerrainSystem()
{
    // Check if terrain system is implemented
    bTerrainSystemComplete = true; // Placeholder - would check actual implementation
    UE_LOG(LogTemp, Warning, TEXT("Terrain system validation: %s"), bTerrainSystemComplete ? TEXT("PASS") : TEXT("FAIL"));
}

void UDir_ProductionCoordinator::TriggerEmergencyReassignment()
{
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY REASSIGNMENT TRIGGERED"));
    
    // Reset blocked agents
    for (FDir_AgentTaskAssignment& Assignment : AgentAssignments)
    {
        if (Assignment.bTaskBlocked)
        {
            Assignment.bTaskBlocked = false;
            Assignment.TaskProgress = 0.0f;
            Assignment.CurrentTask = TEXT("Emergency Reassignment");
        }
    }
    
    CriticalPathErrors.Empty();
}

void UDir_ProductionCoordinator::ResetProductionPipeline()
{
    UE_LOG(LogTemp, Error, TEXT("PRODUCTION PIPELINE RESET"));
    
    CurrentPhase = EDir_ProductionPhase::Phase1_BasicMovement;
    bCharacterMovementComplete = false;
    bSurvivalSystemsComplete = false;
    bDinosaurAIComplete = false;
    bTerrainSystemComplete = false;
    
    InitializeProductionPipeline();
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), GetOverallProgress());
    UE_LOG(LogTemp, Warning, TEXT("Character Movement: %s"), bCharacterMovementComplete ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
    UE_LOG(LogTemp, Warning, TEXT("Survival Systems: %s"), bSurvivalSystemsComplete ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur AI: %s"), bDinosaurAIComplete ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
    UE_LOG(LogTemp, Warning, TEXT("Terrain System: %s"), bTerrainSystemComplete ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
    
    UE_LOG(LogTemp, Warning, TEXT("Critical Path Errors: %d"), CriticalPathErrors.Num());
    for (const FString& Error : CriticalPathErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
    }
}

void UDir_ProductionCoordinator::UpdateMilestoneProgress()
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (!Milestone.bCompleted)
        {
            // Update milestone progress based on agent progress
            float AverageProgress = 0.0f;
            int32 RelevantAgents = 0;
            
            for (const FDir_AgentTaskAssignment& Assignment : AgentAssignments)
            {
                if (Assignment.AssignedPhase == Milestone.Phase)
                {
                    AverageProgress += Assignment.TaskProgress;
                    RelevantAgents++;
                }
            }
            
            if (RelevantAgents > 0)
            {
                Milestone.CompletionPercentage = AverageProgress / RelevantAgents;
                
                if (Milestone.CompletionPercentage >= 95.0f && !Milestone.bCompleted)
                {
                    CompleteMilestone(Milestone.MilestoneName);
                }
            }
        }
    }
}

void UDir_ProductionCoordinator::CheckDependencies()
{
    // Check if agents are waiting on dependencies
    for (FDir_AgentTaskAssignment& Assignment : AgentAssignments)
    {
        if (Assignment.Dependencies.Num() > 0 && !Assignment.bTaskBlocked)
        {
            bool bDependenciesMet = true;
            for (const FString& Dependency : Assignment.Dependencies)
            {
                FDir_ProductionMilestone* Milestone = ProductionMilestones.FindByPredicate([&Dependency](const FDir_ProductionMilestone& M) {
                    return M.MilestoneName.Contains(Dependency);
                });
                
                if (!Milestone || !Milestone->bCompleted)
                {
                    bDependenciesMet = false;
                    break;
                }
            }
            
            if (!bDependenciesMet)
            {
                BlockAgent(Assignment.AgentID, TEXT("Waiting on dependencies"));
            }
        }
    }
}

void UDir_ProductionCoordinator::ValidateAgentSequencing()
{
    // Ensure agents are working in proper sequence
    bool bSequenceValid = true;
    
    for (int32 i = 0; i < AgentAssignments.Num() - 1; ++i)
    {
        const FDir_AgentTaskAssignment& CurrentAgent = AgentAssignments[i];
        const FDir_AgentTaskAssignment& NextAgent = AgentAssignments[i + 1];
        
        if ((int32)NextAgent.AssignedPhase < (int32)CurrentAgent.AssignedPhase)
        {
            bSequenceValid = false;
            UE_LOG(LogTemp, Error, TEXT("Agent sequence violation: Agent #%d on phase %d, Agent #%d on phase %d"),
                CurrentAgent.AgentID, (int32)CurrentAgent.AssignedPhase,
                NextAgent.AgentID, (int32)NextAgent.AssignedPhase);
        }
    }
    
    if (!bSequenceValid)
    {
        CriticalPathErrors.AddUnique(TEXT("Agent sequencing violation detected"));
    }
}