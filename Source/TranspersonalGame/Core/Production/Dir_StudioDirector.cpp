#include "Dir_StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"

ADir_StudioDirector::ADir_StudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize production settings
    CurrentPhase = EDir_ProductionPhase::Prototype;
    CycleUpdateInterval = 60.0f; // 1 minute cycles
    LastUpdateTime = 0.0f;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260615_006");

    SetupAgentChain();
}

void ADir_StudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentTasks();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized - Current Phase: %s"), 
           *UEnum::GetValueAsString(CurrentPhase));
}

void ADir_StudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= CycleUpdateInterval)
    {
        UpdateProductionMetrics();
        CoordinateAgentChain();
        ValidateAgentOutputs();
        EnforceProductionRules();
        
        LastUpdateTime = 0.0f;
    }
}

void ADir_StudioDirector::SetupAgentChain()
{
    AgentChain.Empty();
    AgentChain.Add(TEXT("Studio Director"));           // #01
    AgentChain.Add(TEXT("Engine Architect"));          // #02
    AgentChain.Add(TEXT("Core Systems Programmer"));   // #03
    AgentChain.Add(TEXT("Performance Optimizer"));     // #04
    AgentChain.Add(TEXT("Procedural World Generator")); // #05
    AgentChain.Add(TEXT("Environment Artist"));        // #06
    AgentChain.Add(TEXT("Architecture & Interior"));   // #07
    AgentChain.Add(TEXT("Lighting & Atmosphere"));     // #08
    AgentChain.Add(TEXT("Character Artist"));          // #09
    AgentChain.Add(TEXT("Animation Agent"));           // #10
    AgentChain.Add(TEXT("NPC Behavior Agent"));        // #11
    AgentChain.Add(TEXT("Combat & Enemy AI"));         // #12
    AgentChain.Add(TEXT("Crowd & Traffic Simulation"));// #13
    AgentChain.Add(TEXT("Quest & Mission Designer"));  // #14
    AgentChain.Add(TEXT("Narrative & Dialogue"));      // #15
    AgentChain.Add(TEXT("Audio Agent"));               // #16
    AgentChain.Add(TEXT("VFX Agent"));                 // #17
    AgentChain.Add(TEXT("QA & Testing"));              // #18
    AgentChain.Add(TEXT("Integration & Build"));       // #19
}

void ADir_StudioDirector::InitializeAgentTasks()
{
    AgentTasks.Empty();
    
    // Milestone 1 - "Walk Around" tasks
    AssignTaskToAgent(TEXT("Core Systems Programmer"), 
                     TEXT("Implement ThirdPersonCharacter with WASD movement"), 10);
    AssignTaskToAgent(TEXT("Procedural World Generator"), 
                     TEXT("Create landscape with height variation"), 9);
    AssignTaskToAgent(TEXT("Environment Artist"), 
                     TEXT("Place 3-5 static dinosaur meshes in world"), 8);
    AssignTaskToAgent(TEXT("Lighting & Atmosphere"), 
                     TEXT("Setup directional light + sky + fog"), 7);
    AssignTaskToAgent(TEXT("Character Artist"), 
                     TEXT("Configure camera boom + follow camera"), 6);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agent tasks for Milestone 1"), AgentTasks.Num());
}

void ADir_StudioDirector::UpdateProductionMetrics()
{
    // Count actors in current level
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        ProductionMetrics.TotalActorsInLevel = AllActors.Num();
        
        // Count dinosaurs (actors with "dino" in name)
        ProductionMetrics.DinosaurCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().ToLower().Contains(TEXT("dino")))
            {
                ProductionMetrics.DinosaurCount++;
            }
        }
    }
    
    // Calculate completion metrics
    ProductionMetrics.CompletedTasks = 0;
    ProductionMetrics.BlockedTasks = 0;
    float TotalProgress = 0.0f;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.CompletionPercentage >= 100.0f)
        {
            ProductionMetrics.CompletedTasks++;
        }
        if (Task.bIsBlocked)
        {
            ProductionMetrics.BlockedTasks++;
        }
        TotalProgress += Task.CompletionPercentage;
    }
    
    ProductionMetrics.OverallProgress = AgentTasks.Num() > 0 ? 
                                       TotalProgress / AgentTasks.Num() : 0.0f;
}

void ADir_StudioDirector::AssignTaskToAgent(const FString& AgentName, 
                                           const FString& TaskDescription, 
                                           int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.CompletionPercentage = 0.0f;
    NewTask.bIsBlocked = false;
    
    AgentTasks.Add(NewTask);
    AgentPriorities.Add(AgentName, Priority);
    
    UE_LOG(LogTemp, Log, TEXT("Assigned task to %s: %s (Priority: %d)"), 
           *AgentName, *TaskDescription, Priority);
}

void ADir_StudioDirector::MarkTaskCompleted(const FString& AgentName, float CompletionPercentage)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
            if (Task.bIsBlocked && CompletionPercentage > 0.0f)
            {
                Task.bIsBlocked = false;
                Task.BlockingReason = TEXT("");
            }
            break;
        }
    }
}

void ADir_StudioDirector::BlockTask(const FString& AgentName, const FString& Reason)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.bIsBlocked = true;
            Task.BlockingReason = Reason;
            UE_LOG(LogTemp, Warning, TEXT("Task blocked for %s: %s"), *AgentName, *Reason);
            break;
        }
    }
}

void ADir_StudioDirector::UnblockTask(const FString& AgentName)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.bIsBlocked = false;
            Task.BlockingReason = TEXT("");
            UE_LOG(LogTemp, Log, TEXT("Task unblocked for %s"), *AgentName);
            break;
        }
    }
}

void ADir_StudioDirector::AdvanceProductionPhase()
{
    if (CanAdvancePhase())
    {
        switch (CurrentPhase)
        {
            case EDir_ProductionPhase::PreProduction:
                CurrentPhase = EDir_ProductionPhase::Prototype;
                break;
            case EDir_ProductionPhase::Prototype:
                CurrentPhase = EDir_ProductionPhase::Production;
                break;
            case EDir_ProductionPhase::Production:
                CurrentPhase = EDir_ProductionPhase::Polish;
                break;
            case EDir_ProductionPhase::Polish:
                CurrentPhase = EDir_ProductionPhase::Release;
                break;
            default:
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Advanced to production phase: %s"), 
               *UEnum::GetValueAsString(CurrentPhase));
    }
}

bool ADir_StudioDirector::CanAdvancePhase() const
{
    // Milestone 1 requirements for advancing from Prototype
    if (CurrentPhase == EDir_ProductionPhase::Prototype)
    {
        return IsMilestone1Complete();
    }
    
    // General requirement: 80% of tasks completed
    return ProductionMetrics.OverallProgress >= 80.0f;
}

bool ADir_StudioDirector::IsMilestone1Complete() const
{
    // Check Milestone 1 - "Walk Around" requirements
    bool bHasCharacterMovement = false;
    bool bHasLandscape = false;
    bool bHasDinosaurs = false;
    bool bHasLighting = false;
    
    // Check if critical tasks are completed
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.CompletionPercentage >= 100.0f)
        {
            if (Task.TaskDescription.Contains(TEXT("ThirdPersonCharacter")))
                bHasCharacterMovement = true;
            if (Task.TaskDescription.Contains(TEXT("landscape")))
                bHasLandscape = true;
            if (Task.TaskDescription.Contains(TEXT("dinosaur")))
                bHasDinosaurs = true;
            if (Task.TaskDescription.Contains(TEXT("light")))
                bHasLighting = true;
        }
    }
    
    // Also check actor counts
    bool bHasMinimumContent = (ProductionMetrics.TotalActorsInLevel >= 10 && 
                              ProductionMetrics.DinosaurCount >= 3);
    
    return bHasCharacterMovement && bHasLandscape && bHasDinosaurs && 
           bHasLighting && bHasMinimumContent;
}

void ADir_StudioDirector::ValidatePlayablePrototype()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PLAYABLE PROTOTYPE VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), ProductionMetrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Count: %d"), ProductionMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d/%d"), 
           ProductionMetrics.CompletedTasks, AgentTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), ProductionMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Complete: %s"), 
           IsMilestone1Complete() ? TEXT("YES") : TEXT("NO"));
}

void ADir_StudioDirector::CoordinateAgentChain()
{
    // Log current agent coordination status
    UE_LOG(LogTemp, Log, TEXT("Coordinating agent chain - Phase: %s"), 
           *UEnum::GetValueAsString(CurrentPhase));
    
    // Check for blocked agents and resolve dependencies
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bIsBlocked)
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent %s blocked: %s"), 
                   *Task.AgentName, *Task.BlockingReason);
        }
    }
}

FString ADir_StudioDirector::GetNextAgentInChain(const FString& CurrentAgent) const
{
    int32 CurrentIndex = AgentChain.Find(CurrentAgent);
    if (CurrentIndex != INDEX_NONE && CurrentIndex < AgentChain.Num() - 1)
    {
        return AgentChain[CurrentIndex + 1];
    }
    return AgentChain.Num() > 0 ? AgentChain[0] : TEXT("");
}

void ADir_StudioDirector::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle ID: %s"), *CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *UEnum::GetValueAsString(CurrentPhase));
    UE_LOG(LogTemp, Warning, TEXT("Actors in Level: %d"), ProductionMetrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), ProductionMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Progress: %.1f%%"), ProductionMetrics.OverallProgress);
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        UE_LOG(LogTemp, Log, TEXT("Agent %s: %.1f%% - %s"), 
               *Task.AgentName, Task.CompletionPercentage, *Task.TaskDescription);
    }
}

void ADir_StudioDirector::GenerateProductionReport()
{
    ValidatePlayablePrototype();
    LogProductionStatus();
}

void ADir_StudioDirector::ValidateAgentOutputs()
{
    // Check if agents are producing visible results
    UpdateProductionMetrics();
    
    if (ProductionMetrics.TotalActorsInLevel < 5)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Too few actors in level (%d). Agents not producing visible content!"), 
               ProductionMetrics.TotalActorsInLevel);
    }
}

void ADir_StudioDirector::EnforceProductionRules()
{
    // Enforce CAP limits
    if (ProductionMetrics.TotalActorsInLevel > 8000)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Actor count exceeded limit (%d/8000)"), 
               ProductionMetrics.TotalActorsInLevel);
    }
    
    if (ProductionMetrics.DinosaurCount > 150)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Dinosaur count exceeded limit (%d/150)"), 
               ProductionMetrics.DinosaurCount);
    }
    
    // Check for gameplay-first compliance
    if (CurrentPhase == EDir_ProductionPhase::Prototype && !IsMilestone1Complete())
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone 1 incomplete - enforcing gameplay-first directive"));
    }
}