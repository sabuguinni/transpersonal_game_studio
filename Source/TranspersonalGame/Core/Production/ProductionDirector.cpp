#include "ProductionDirector.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Landscape.h"
#include "GameFramework/Character.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"

AProductionDirector::AProductionDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentCycleNumber = 6;
    PlayablePrototypeScore = 0.0f;
    TotalActorsInLevel = 0;
    TerrainActorCount = 0;
    CharacterActorCount = 0;
    DinosaurActorCount = 0;
}

void AProductionDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    SetupDefaultMilestones();
    AnalyzeProductionState();
}

void AProductionDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update production metrics every 5 seconds
    static float TimeSinceLastUpdate = 0.0f;
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= 5.0f)
    {
        AnalyzeProductionState();
        ValidateMilestones();
        TimeSinceLastUpdate = 0.0f;
    }
}

void AProductionDirector::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Initializing production pipeline for Cycle %d"), CurrentCycleNumber);
    
    // Clear existing tasks
    ActiveTasks.Empty();
    
    // Setup critical agent tasks for playable prototype
    AssignTaskToAgent(TEXT("Agent_02_Engine"), TEXT("Architecture review and core system validation"), 10);
    AssignTaskToAgent(TEXT("Agent_03_Core"), TEXT("Physics and collision system implementation"), 9);
    AssignTaskToAgent(TEXT("Agent_05_World"), TEXT("Terrain enhancement and biome creation"), 8);
    AssignTaskToAgent(TEXT("Agent_09_Character"), TEXT("Player character refinement and MetaHuman integration"), 9);
    AssignTaskToAgent(TEXT("Agent_10_Animation"), TEXT("Movement and combat animation systems"), 7);
    AssignTaskToAgent(TEXT("Agent_12_Combat"), TEXT("Dinosaur AI and combat mechanics"), 8);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Assigned %d critical tasks to agents"), ActiveTasks.Num());
}

void AProductionDirector::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bCompleted = false;
    NewTask.CompletionPercentage = 0.0f;
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionDirector: Assigned task to %s: %s (Priority: %d)"), 
           *AgentName, *TaskDescription, Priority);
}

void AProductionDirector::UpdateTaskProgress(const FString& AgentName, float CompletionPercentage)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName && !Task.bCompleted)
        {
            Task.CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
            
            if (Task.CompletionPercentage >= 100.0f)
            {
                Task.bCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Task completed by %s"), *AgentName);
            }
            break;
        }
    }
}

void AProductionDirector::CompleteAgentTask(const FString& AgentName)
{
    UpdateTaskProgress(AgentName, 100.0f);
}

void AProductionDirector::AnalyzeProductionState()
{
    CountLevelActors();
    PlayablePrototypeScore = CalculatePlayableScore();
    LogProductionStatus();
}

void AProductionDirector::CountLevelActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    TotalActorsInLevel = 0;
    TerrainActorCount = 0;
    CharacterActorCount = 0;
    DinosaurActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TotalActorsInLevel++;
        
        // Count terrain actors
        if (Actor->IsA<ALandscape>())
        {
            TerrainActorCount++;
        }
        
        // Count character actors
        if (Actor->IsA<ACharacter>())
        {
            CharacterActorCount++;
        }
        
        // Count dinosaur actors (by name pattern)
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
            ActorName.Contains(TEXT("brach")) || ActorName.Contains(TEXT("dinosaur")))
        {
            DinosaurActorCount++;
        }
    }
}

float AProductionDirector::CalculatePlayableScore()
{
    float Score = 0.0f;
    
    // Character presence (critical for playability)
    Score += CharacterActorCount * 30.0f;
    
    // Dinosaur variety (core gameplay element)
    Score += DinosaurActorCount * 20.0f;
    
    // Terrain (essential for movement)
    Score += TerrainActorCount * 25.0f;
    
    // Basic lighting and atmosphere
    int32 LightCount = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->FindComponentByClass<UDirectionalLightComponent>() || 
                     Actor->FindComponentByClass<UPointLightComponent>()))
        {
            LightCount++;
        }
    }
    Score += LightCount * 5.0f;
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

void AProductionDirector::SetupDefaultMilestones()
{
    ProductionMilestones.Empty();
    
    // Milestone 1: Walk Around
    FDir_ProductionMilestone WalkAroundMilestone;
    WalkAroundMilestone.MilestoneName = TEXT("Walk Around");
    WalkAroundMilestone.RequiredAgents.Add(TEXT("Agent_09_Character"));
    WalkAroundMilestone.RequiredAgents.Add(TEXT("Agent_05_World"));
    WalkAroundMilestone.MinActorCount = 5;
    WalkAroundMilestone.bAchieved = false;
    ProductionMilestones.Add(WalkAroundMilestone);
    
    // Milestone 2: Dinosaur Encounter
    FDir_ProductionMilestone DinosaurMilestone;
    DinosaurMilestone.MilestoneName = TEXT("Dinosaur Encounter");
    DinosaurMilestone.RequiredAgents.Add(TEXT("Agent_12_Combat"));
    DinosaurMilestone.RequiredAgents.Add(TEXT("Agent_10_Animation"));
    DinosaurMilestone.MinActorCount = 3;
    DinosaurMilestone.bAchieved = false;
    ProductionMilestones.Add(DinosaurMilestone);
}

void AProductionDirector::ValidateMilestones()
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.bAchieved)
        {
            continue;
        }
        
        bool bCanAchieve = true;
        
        if (Milestone.MilestoneName == TEXT("Walk Around"))
        {
            bCanAchieve = (CharacterActorCount >= 1 && TerrainActorCount >= 1);
        }
        else if (Milestone.MilestoneName == TEXT("Dinosaur Encounter"))
        {
            bCanAchieve = (DinosaurActorCount >= 3 && CharacterActorCount >= 1);
        }
        
        if (bCanAchieve)
        {
            Milestone.bAchieved = true;
            UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Milestone achieved - %s"), *Milestone.MilestoneName);
        }
    }
}

TArray<FDir_AgentTask> AProductionDirector::GetPendingTasks()
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (!Task.bCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    return PendingTasks;
}

bool AProductionDirector::IsMilestoneAchieved(const FString& MilestoneName)
{
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.bAchieved;
        }
    }
    return false;
}

void AProductionDirector::GenerateProductionReport()
{
    AnalyzeProductionState();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - CYCLE %d ==="), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Playable Prototype Score: %.1f/100"), PlayablePrototypeScore);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Terrain: %d, Characters: %d, Dinosaurs: %d"), 
           TerrainActorCount, CharacterActorCount, DinosaurActorCount);
    
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), ActiveTasks.Num());
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        UE_LOG(LogTemp, Log, TEXT("- %s: %s (%.1f%% complete)"), 
               *Task.AgentName, *Task.TaskDescription, Task.CompletionPercentage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Milestones:"));
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        UE_LOG(LogTemp, Log, TEXT("- %s: %s"), 
               *Milestone.MilestoneName, Milestone.bAchieved ? TEXT("ACHIEVED") : TEXT("PENDING"));
    }
}

void AProductionDirector::LogProductionStatus()
{
    UE_LOG(LogTemp, Log, TEXT("ProductionDirector: Score %.1f/100 | Actors: %d | Characters: %d | Dinosaurs: %d | Terrain: %d"), 
           PlayablePrototypeScore, TotalActorsInLevel, CharacterActorCount, DinosaurActorCount, TerrainActorCount);
}