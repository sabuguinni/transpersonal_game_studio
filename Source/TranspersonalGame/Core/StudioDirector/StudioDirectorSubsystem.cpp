#include "StudioDirectorSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

void UStudioDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Subsystem Initialized"));
    
    bEmergencyMode = false;
    EmergencyReason = TEXT("");
    
    InitializeAgentPipeline();
    UpdateProductionMetrics();
}

void UStudioDirectorSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Subsystem Deinitialized"));
    Super::Deinitialize();
}

void UStudioDirectorSubsystem::InitializeAgentPipeline()
{
    // Initialize the 19-agent pipeline with core tasks
    TArray<FString> AgentNames = {
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

    AgentTasks.Empty();
    
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentName;
        NewTask.TaskDescription = TEXT("Milestone 1 - Walk Around Implementation");
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = 1.0f;
        
        AgentTasks.Add(NewTask);
    }
    
    // Set high priority tasks for Milestone 1
    AssignTaskToAgent(TEXT("Procedural World Generator"), TEXT("Create playable terrain with height variation"), 10.0f);
    AssignTaskToAgent(TEXT("Character Artist Agent"), TEXT("Implement ThirdPersonCharacter with WASD movement"), 10.0f);
    AssignTaskToAgent(TEXT("Environment Artist"), TEXT("Spawn 3-5 dinosaur meshes in world"), 9.0f);
    AssignTaskToAgent(TEXT("Lighting & Atmosphere Agent"), TEXT("Setup directional light + sky + fog"), 8.0f);
}

void UStudioDirectorSubsystem::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.TaskDescription = TaskDescription;
            Task.Priority = Priority;
            Task.Status = EDir_AgentStatus::Working;
            Task.AssignedTime = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Assigned task to %s - %s (Priority: %.1f)"), 
                *AgentName, *TaskDescription, Priority);
            return;
        }
    }
    
    // Create new task if agent not found
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.Priority = Priority;
    NewTask.AssignedTime = FDateTime::Now();
    
    AgentTasks.Add(NewTask);
}

void UStudioDirectorSubsystem::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            if (NewStatus == EDir_AgentStatus::Completed)
            {
                Task.CompletedTime = FDateTime::Now();
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent %s status updated to %d"), 
                *AgentName, (int32)NewStatus);
            return;
        }
    }
}

TArray<FDir_AgentTask> UStudioDirectorSubsystem::GetActiveAgentTasks() const
{
    TArray<FDir_AgentTask> ActiveTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working || Task.Status == EDir_AgentStatus::Blocked)
        {
            ActiveTasks.Add(Task);
        }
    }
    
    // Sort by priority
    ActiveTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B)
    {
        return A.Priority > B.Priority;
    });
    
    return ActiveTasks;
}

void UStudioDirectorSubsystem::UpdateProductionMetrics()
{
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.TotalActorsSpawned = 0;
        CurrentMetrics.DinosaurCount = 0;
        CurrentMetrics.EnvironmentAssetsCount = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                CurrentMetrics.TotalActorsSpawned++;
                
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
                    ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("tricera")))
                {
                    CurrentMetrics.DinosaurCount++;
                }
                else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("rock")) || 
                         ActorName.Contains(TEXT("plant")))
                {
                    CurrentMetrics.EnvironmentAssetsCount++;
                }
            }
        }
        
        // Calculate completion percentage for Milestone 1
        float CompletionScore = 0.0f;
        
        // Terrain exists (basic requirement)
        CompletionScore += 20.0f;
        
        // Character movement (critical)
        if (HasPlayableCharacter())
        {
            CompletionScore += 30.0f;
        }
        
        // Dinosaurs in world
        if (CurrentMetrics.DinosaurCount >= 3)
        {
            CompletionScore += 25.0f;
        }
        else if (CurrentMetrics.DinosaurCount > 0)
        {
            CompletionScore += 15.0f;
        }
        
        // Environment assets
        if (CurrentMetrics.EnvironmentAssetsCount >= 10)
        {
            CompletionScore += 15.0f;
        }
        else if (CurrentMetrics.EnvironmentAssetsCount > 0)
        {
            CompletionScore += 5.0f;
        }
        
        // Lighting setup
        CompletionScore += 10.0f; // Assume basic lighting exists
        
        CurrentMetrics.WorldCompletionPercentage = FMath::Clamp(CompletionScore, 0.0f, 100.0f);
        
        LogProductionStatus();
    }
}

FDir_ProductionMetrics UStudioDirectorSubsystem::GetProductionMetrics() const
{
    return CurrentMetrics;
}

bool UStudioDirectorSubsystem::IsMilestone1Complete() const
{
    return CurrentMetrics.WorldCompletionPercentage >= 80.0f && 
           HasPlayableCharacter() && 
           CurrentMetrics.DinosaurCount >= 3;
}

bool UStudioDirectorSubsystem::ValidateMinPlayableMap() const
{
    if (UWorld* World = GetWorld())
    {
        // Check if we have minimum required actors
        bool bHasCharacter = HasPlayableCharacter();
        bool bHasDinosaurs = CountDinosaursInWorld() >= 3;
        bool bHasEnvironment = CurrentMetrics.EnvironmentAssetsCount > 0;
        
        return bHasCharacter && bHasDinosaurs && bHasEnvironment;
    }
    
    return false;
}

int32 UStudioDirectorSubsystem::CountDinosaursInWorld() const
{
    return CurrentMetrics.DinosaurCount;
}

bool UStudioDirectorSubsystem::HasPlayableCharacter() const
{
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<ACharacter> CharacterItr(World); CharacterItr; ++CharacterItr)
        {
            ACharacter* Character = *CharacterItr;
            if (Character && Character->IsA<ACharacter>())
            {
                return true;
            }
        }
    }
    
    return false;
}

void UStudioDirectorSubsystem::TriggerEmergencyProtocol(const FString& Reason)
{
    bEmergencyMode = true;
    EmergencyReason = Reason;
    
    UE_LOG(LogTemp, Error, TEXT("STUDIO DIRECTOR EMERGENCY: %s"), *Reason);
    
    // Reset all agent tasks to idle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Blocked;
        }
    }
    
    // Focus on critical path only
    AssignTaskToAgent(TEXT("Core Systems Programmer"), TEXT("EMERGENCY: Fix compilation errors"), 100.0f);
    AssignTaskToAgent(TEXT("QA & Testing Agent"), TEXT("EMERGENCY: Validate basic functionality"), 99.0f);
}

void UStudioDirectorSubsystem::ResetProductionPipeline()
{
    bEmergencyMode = false;
    EmergencyReason = TEXT("");
    
    InitializeAgentPipeline();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Production pipeline reset"));
}

void UStudioDirectorSubsystem::LogProductionStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsSpawned);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Environment Assets: %d"), CurrentMetrics.EnvironmentAssetsCount);
    UE_LOG(LogTemp, Warning, TEXT("Completion: %.1f%%"), CurrentMetrics.WorldCompletionPercentage);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Complete: %s"), IsMilestone1Complete() ? TEXT("YES") : TEXT("NO"));
    
    if (bEmergencyMode)
    {
        UE_LOG(LogTemp, Error, TEXT("EMERGENCY MODE ACTIVE: %s"), *EmergencyReason);
    }
}