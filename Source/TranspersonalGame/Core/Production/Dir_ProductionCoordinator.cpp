#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    CurrentPhase = EDir_ProductionPhase::Milestone1_WalkAround;
    ProductionStartTime = FDateTime::Now();
    CurrentCycleNumber = 20;
}

void UDir_ProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator Initialized"));
    
    InitializeProductionPipeline();
}

void UDir_ProductionCoordinator::Deinitialize()
{
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator Shutdown"));
}

void UDir_ProductionCoordinator::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Production Pipeline - Cycle %d"), CurrentCycleNumber);
    
    // Clear existing data
    AgentTasks.Empty();
    Milestones.Empty();
    
    // Setup all 19 agents
    SetupDefaultAgents();
    
    // Setup Milestone 1 requirements
    SetupMilestone1Requirements();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Pipeline Initialized: %d Agents, %d Milestones"), 
           AgentTasks.Num(), Milestones.Num());
}

void UDir_ProductionCoordinator::SetupDefaultAgents()
{
    TArray<FString> AgentNames = {
        TEXT("Agent_02_Engine_Architect"),
        TEXT("Agent_03_Core_Systems"),
        TEXT("Agent_04_Performance_Optimizer"),
        TEXT("Agent_05_World_Generator"),
        TEXT("Agent_06_Environment_Artist"),
        TEXT("Agent_07_Architecture_Interior"),
        TEXT("Agent_08_Lighting_Atmosphere"),
        TEXT("Agent_09_Character_Artist"),
        TEXT("Agent_10_Animation"),
        TEXT("Agent_11_NPC_Behavior"),
        TEXT("Agent_12_Combat_Enemy_AI"),
        TEXT("Agent_13_Crowd_Traffic"),
        TEXT("Agent_14_Quest_Mission"),
        TEXT("Agent_15_Narrative_Dialogue"),
        TEXT("Agent_16_Audio"),
        TEXT("Agent_17_VFX"),
        TEXT("Agent_18_QA_Testing"),
        TEXT("Agent_19_Integration_Build")
    };
    
    TArray<FString> DefaultTasks = {
        TEXT("Review and validate core engine architecture"),
        TEXT("Implement physics, collision, and ragdoll systems"),
        TEXT("Optimize for 60fps PC / 30fps console performance"),
        TEXT("Generate terrain with height variation and biomes"),
        TEXT("Populate world with vegetation, rocks, and props"),
        TEXT("Build prehistoric structures and interiors"),
        TEXT("Setup day/night cycle and atmospheric lighting"),
        TEXT("Create playable character with MetaHuman"),
        TEXT("Implement character and dinosaur animations"),
        TEXT("Program NPC behavior trees and daily routines"),
        TEXT("Design dinosaur AI and combat systems"),
        TEXT("Setup crowd simulation for up to 50k agents"),
        TEXT("Convert narrative into playable missions"),
        TEXT("Write game bible and prehistoric lore"),
        TEXT("Create adaptive music and sound effects"),
        TEXT("Implement particle effects with Niagara"),
        TEXT("Test all systems and validate gameplay"),
        TEXT("Integrate all agent outputs into cohesive build")
    };
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentNames[i];
        NewTask.TaskDescription = DefaultTasks[i];
        NewTask.Status = EDir_AgentStatus::Working;
        NewTask.Priority = (i < 6) ? 10.0f : 5.0f; // Higher priority for core systems
        NewTask.Deadline = FDateTime::Now() + FTimespan::FromDays(2);
        
        AgentTasks.Add(NewTask);
    }
}

void UDir_ProductionCoordinator::SetupMilestone1Requirements()
{
    TArray<FString> Milestone1Reqs = {
        TEXT("ThirdPersonCharacter with WASD movement"),
        TEXT("Camera boom and follow camera"),
        TEXT("Landscape with height variation"),
        TEXT("Player can walk, run, jump"),
        TEXT("3-5 static dinosaur meshes placed"),
        TEXT("Directional light + sky + fog"),
        TEXT("Basic survival HUD (health/hunger/thirst/stamina)")
    };
    
    FDir_MilestoneProgress Milestone1;
    Milestone1.MilestoneName = TEXT("Milestone_1_Walk_Around");
    Milestone1.Requirements = Milestone1Reqs;
    Milestone1.CompletionStatus.SetNum(Milestone1Reqs.Num());
    Milestone1.TargetDate = FDateTime::Now() + FTimespan::FromDays(1);
    
    // Initialize all as incomplete
    for (int32 i = 0; i < Milestone1.CompletionStatus.Num(); i++)
    {
        Milestone1.CompletionStatus[i] = false;
    }
    
    Milestones.Add(Milestone1);
}

void UDir_ProductionCoordinator::UpdateProductionStatus()
{
    ValidateProductionState();
    LogProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Status Updated - Cycle %d"), CurrentCycleNumber);
}

void UDir_ProductionCoordinator::ValidateProductionState()
{
    // Check if we have a valid world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found for production validation"));
        return;
    }
    
    // Count actors in the level
    int32 CharacterCount = 0;
    int32 DinosaurCount = 0;
    int32 TerrainCount = 0;
    int32 LightCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName();
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("Character")) || ActorName.Contains(TEXT("Character")))
        {
            CharacterCount++;
        }
        else if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")))
        {
            DinosaurCount++;
        }
        else if (ClassName.Contains(TEXT("Landscape")) || ActorName.Contains(TEXT("Terrain")))
        {
            TerrainCount++;
        }
        else if (ClassName.Contains(TEXT("Light")))
        {
            LightCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production Validation: Characters=%d, Dinosaurs=%d, Terrain=%d, Lights=%d"), 
           CharacterCount, DinosaurCount, TerrainCount, LightCount);
    
    // Update Milestone 1 progress based on validation
    if (Milestones.Num() > 0)
    {
        FDir_MilestoneProgress& Milestone1 = Milestones[0];
        if (Milestone1.CompletionStatus.Num() >= 7)
        {
            Milestone1.CompletionStatus[0] = CharacterCount > 0; // Character exists
            Milestone1.CompletionStatus[1] = CharacterCount > 0; // Camera (assumed with character)
            Milestone1.CompletionStatus[2] = TerrainCount > 0;   // Terrain exists
            Milestone1.CompletionStatus[3] = CharacterCount > 0; // Movement (assumed with character)
            Milestone1.CompletionStatus[4] = DinosaurCount >= 3; // 3+ dinosaurs
            Milestone1.CompletionStatus[5] = LightCount > 0;     // Lighting
            Milestone1.CompletionStatus[6] = false;              // HUD (needs implementation)
            
            // Calculate overall progress
            int32 CompletedCount = 0;
            for (bool Status : Milestone1.CompletionStatus)
            {
                if (Status) CompletedCount++;
            }
            Milestone1.OverallProgress = (float)CompletedCount / (float)Milestone1.CompletionStatus.Num() * 100.0f;
        }
    }
}

void UDir_ProductionCoordinator::LogProductionMetrics()
{
    int32 ActiveAgents = GetActiveAgentCount();
    int32 BlockedAgents = GetBlockedAgentCount();
    float OverallProgress = GetOverallProductionProgress();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION METRICS CYCLE %d ==="), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), ActiveAgents);
    UE_LOG(LogTemp, Warning, TEXT("Blocked Agents: %d"), BlockedAgents);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), OverallProgress);
    
    if (Milestones.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: %.1f%%"), Milestones[0].OverallProgress);
    }
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    FString ReportContent = TEXT("=== TRANSPERSONAL GAME STUDIO PRODUCTION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Cycle: %d\n"), CurrentCycleNumber);
    ReportContent += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(CurrentPhase));
    ReportContent += FString::Printf(TEXT("Report Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    // Agent Status Summary
    ReportContent += TEXT("AGENT STATUS:\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusStr = UEnum::GetValueAsString(Task.Status);
        ReportContent += FString::Printf(TEXT("- %s: %s (%s)\n"), 
                                        *Task.AgentName, 
                                        *StatusStr, 
                                        *Task.TaskDescription);
    }
    
    // Milestone Progress
    ReportContent += TEXT("\nMILESTONE PROGRESS:\n");
    for (const FDir_MilestoneProgress& Milestone : Milestones)
    {
        ReportContent += FString::Printf(TEXT("- %s: %.1f%% complete\n"), 
                                        *Milestone.MilestoneName, 
                                        Milestone.OverallProgress);
        
        for (int32 i = 0; i < Milestone.Requirements.Num() && i < Milestone.CompletionStatus.Num(); i++)
        {
            FString StatusIcon = Milestone.CompletionStatus[i] ? TEXT("✓") : TEXT("✗");
            ReportContent += FString::Printf(TEXT("  %s %s\n"), *StatusIcon, *Milestone.Requirements[i]);
        }
    }
    
    // Save report to file
    FString FilePath = FPaths::ProjectLogDir() / TEXT("ProductionReport_Cycle") + FString::FromInt(CurrentCycleNumber) + TEXT(".txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Warning, TEXT("Production Report Generated: %s"), *FilePath);
}

void UDir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.TaskDescription = TaskDescription;
            Task.Priority = Priority;
            Task.Status = EDir_AgentStatus::Working;
            Task.BlockingReason = TEXT("");
            
            UE_LOG(LogTemp, Warning, TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription);
            return;
        }
    }
    
    // Create new task if agent not found
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.Deadline = FDateTime::Now() + FTimespan::FromDays(1);
    
    AgentTasks.Add(NewTask);
    UE_LOG(LogTemp, Warning, TEXT("New agent task created: %s"), *AgentName);
}

void UDir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, const FString& BlockingReason)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            Task.BlockingReason = BlockingReason;
            
            FString StatusStr = UEnum::GetValueAsString(NewStatus);
            UE_LOG(LogTemp, Warning, TEXT("Agent %s status updated to %s"), *AgentName, *StatusStr);
            return;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Agent not found for status update: %s"), *AgentName);
}

FDir_AgentTask UDir_ProductionCoordinator::GetAgentTask(const FString& AgentName) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            return Task;
        }
    }
    
    return FDir_AgentTask(); // Return default task if not found
}

void UDir_ProductionCoordinator::CreateMilestone(const FString& Name, const TArray<FString>& Requirements, const FDateTime& TargetDate)
{
    FDir_MilestoneProgress NewMilestone;
    NewMilestone.MilestoneName = Name;
    NewMilestone.Requirements = Requirements;
    NewMilestone.CompletionStatus.SetNum(Requirements.Num());
    NewMilestone.TargetDate = TargetDate;
    NewMilestone.OverallProgress = 0.0f;
    
    // Initialize all requirements as incomplete
    for (int32 i = 0; i < NewMilestone.CompletionStatus.Num(); i++)
    {
        NewMilestone.CompletionStatus[i] = false;
    }
    
    Milestones.Add(NewMilestone);
    UE_LOG(LogTemp, Warning, TEXT("Milestone created: %s with %d requirements"), *Name, Requirements.Num());
}

void UDir_ProductionCoordinator::UpdateMilestoneProgress(const FString& MilestoneName, int32 RequirementIndex, bool IsComplete)
{
    for (FDir_MilestoneProgress& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            if (RequirementIndex >= 0 && RequirementIndex < Milestone.CompletionStatus.Num())
            {
                Milestone.CompletionStatus[RequirementIndex] = IsComplete;
                
                // Recalculate overall progress
                int32 CompletedCount = 0;
                for (bool Status : Milestone.CompletionStatus)
                {
                    if (Status) CompletedCount++;
                }
                Milestone.OverallProgress = (float)CompletedCount / (float)Milestone.CompletionStatus.Num() * 100.0f;
                
                UE_LOG(LogTemp, Warning, TEXT("Milestone %s updated: %.1f%% complete"), 
                       *MilestoneName, Milestone.OverallProgress);
            }
            return;
        }
    }
}

float UDir_ProductionCoordinator::GetMilestoneProgress(const FString& MilestoneName) const
{
    for (const FDir_MilestoneProgress& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.OverallProgress;
        }
    }
    return 0.0f;
}

int32 UDir_ProductionCoordinator::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Count++;
        }
    }
    return Count;
}

int32 UDir_ProductionCoordinator::GetBlockedAgentCount() const
{
    int32 Count = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            Count++;
        }
    }
    return Count;
}

float UDir_ProductionCoordinator::GetOverallProductionProgress() const
{
    if (Milestones.Num() == 0) return 0.0f;
    
    float TotalProgress = 0.0f;
    for (const FDir_MilestoneProgress& Milestone : Milestones)
    {
        TotalProgress += Milestone.OverallProgress;
    }
    
    return TotalProgress / (float)Milestones.Num();
}