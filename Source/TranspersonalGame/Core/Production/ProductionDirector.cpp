#include "ProductionDirector.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UProductionDirector::UProductionDirector()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    OverallProgress = 0.0f;
    TotalActorsInLevel = 0;
    DinosaurCount = 0;
    CharacterCount = 0;
}

void UProductionDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    SetupDefaultMilestones();
}

void UProductionDirector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateProductionMetrics();
    AnalyzeProductionState();
}

void UProductionDirector::InitializeProductionPipeline()
{
    // Initialize all 19 agent statuses
    AgentStatuses.Empty();
    
    TArray<FString> AgentNames = {
        TEXT("Studio_Director"),
        TEXT("Engine_Architect"),
        TEXT("Core_Systems_Programmer"),
        TEXT("Performance_Optimizer"),
        TEXT("Procedural_World_Generator"),
        TEXT("Environment_Artist"),
        TEXT("Architecture_Interior"),
        TEXT("Lighting_Atmosphere"),
        TEXT("Character_Artist"),
        TEXT("Animation_Agent"),
        TEXT("NPC_Behavior"),
        TEXT("Combat_Enemy_AI"),
        TEXT("Crowd_Traffic_Simulation"),
        TEXT("Quest_Mission_Designer"),
        TEXT("Narrative_Dialogue"),
        TEXT("Audio_Agent"),
        TEXT("VFX_Agent"),
        TEXT("QA_Testing"),
        TEXT("Integration_Build")
    };
    
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentStatus NewStatus;
        NewStatus.AgentName = AgentName;
        NewStatus.CurrentTask = TEXT("Initializing");
        NewStatus.CompletionPercentage = 0.0f;
        NewStatus.bIsActive = true;
        NewStatus.LastDeliverable = TEXT("None");
        
        AgentStatuses.Add(NewStatus);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Initialized %d agents"), AgentStatuses.Num());
}

void UProductionDirector::SetupDefaultMilestones()
{
    ProductionMilestones.Empty();
    
    // Milestone 1: Walk Around Prototype
    FDir_ProductionMilestone Milestone1;
    Milestone1.MilestoneName = TEXT("Walk_Around_Prototype");
    Milestone1.Description = TEXT("Minimum viable playable prototype with character movement");
    Milestone1.RequiredDeliverables.Add(TEXT("ThirdPersonCharacter_Implementation"));
    Milestone1.RequiredDeliverables.Add(TEXT("Basic_Terrain_Generation"));
    Milestone1.RequiredDeliverables.Add(TEXT("Dinosaur_Placeholder_Actors"));
    Milestone1.RequiredDeliverables.Add(TEXT("Lighting_System"));
    Milestone1.bIsCompleted = false;
    Milestone1.Priority = 10.0f;
    ProductionMilestones.Add(Milestone1);
    
    // Milestone 2: Core Systems
    FDir_ProductionMilestone Milestone2;
    Milestone2.MilestoneName = TEXT("Core_Systems_Integration");
    Milestone2.Description = TEXT("Physics, collision, and core game systems");
    Milestone2.RequiredDeliverables.Add(TEXT("Physics_System_Manager"));
    Milestone2.RequiredDeliverables.Add(TEXT("Collision_Detection"));
    Milestone2.RequiredDeliverables.Add(TEXT("Survival_Stats_System"));
    Milestone2.bIsCompleted = false;
    Milestone2.Priority = 9.0f;
    ProductionMilestones.Add(Milestone2);
    
    // Milestone 3: World Population
    FDir_ProductionMilestone Milestone3;
    Milestone3.MilestoneName = TEXT("World_Population");
    Milestone3.Description = TEXT("Populated world with dinosaurs and environment");
    Milestone3.RequiredDeliverables.Add(TEXT("Dinosaur_AI_Behavior"));
    Milestone3.RequiredDeliverables.Add(TEXT("Environment_Props"));
    Milestone3.RequiredDeliverables.Add(TEXT("Audio_Integration"));
    Milestone3.bIsCompleted = false;
    Milestone3.Priority = 8.0f;
    ProductionMilestones.Add(Milestone3);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Setup %d milestones"), ProductionMilestones.Num());
}

void UProductionDirector::UpdateAgentStatus(const FString& AgentName, const FString& Task, float Completion)
{
    for (FDir_AgentStatus& Status : AgentStatuses)
    {
        if (Status.AgentName == AgentName)
        {
            Status.CurrentTask = Task;
            Status.CompletionPercentage = FMath::Clamp(Completion, 0.0f, 100.0f);
            Status.bIsActive = true;
            
            UE_LOG(LogTemp, Log, TEXT("ProductionDirector: Updated %s - %s (%.1f%%)"), 
                *AgentName, *Task, Completion);
            break;
        }
    }
}

void UProductionDirector::CompleteMilestone(const FString& MilestoneName)
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Completed milestone %s"), *MilestoneName);
            break;
        }
    }
}

void UProductionDirector::UpdateProductionMetrics()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        TotalActorsInLevel = AllActors.Num();
        
        // Count dinosaurs
        DinosaurCount = 0;
        CharacterCount = 0;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                FString ActorName = Actor->GetName().ToLower();
                
                if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("trex")) || 
                    ActorName.Contains(TEXT("veloci")) || ActorName.Contains(TEXT("brachi")))
                {
                    DinosaurCount++;
                }
                
                if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
                {
                    CharacterCount++;
                }
            }
        }
    }
    
    OverallProgress = CalculateOverallProgress();
}

float UProductionDirector::CalculateOverallProgress()
{
    if (AgentStatuses.Num() == 0)
        return 0.0f;
    
    float TotalProgress = 0.0f;
    int32 ActiveAgents = 0;
    
    for (const FDir_AgentStatus& Status : AgentStatuses)
    {
        if (Status.bIsActive)
        {
            TotalProgress += Status.CompletionPercentage;
            ActiveAgents++;
        }
    }
    
    return ActiveAgents > 0 ? TotalProgress / ActiveAgents : 0.0f;
}

void UProductionDirector::AnalyzeProductionState()
{
    // Check milestone completion
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (!Milestone.bIsCompleted)
        {
            int32 CompletedDeliverables = 0;
            
            // Simple heuristic based on actor counts and agent progress
            if (Milestone.MilestoneName == TEXT("Walk_Around_Prototype"))
            {
                if (CharacterCount > 0 && DinosaurCount > 0 && TotalActorsInLevel > 10)
                {
                    CompletedDeliverables = Milestone.RequiredDeliverables.Num();
                }
            }
            
            if (CompletedDeliverables >= Milestone.RequiredDeliverables.Num())
            {
                CompleteMilestone(Milestone.MilestoneName);
            }
        }
    }
    
    LogProductionState();
}

void UProductionDirector::LogProductionState()
{
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Log every 30 seconds
    if (CurrentTime - LastLogTime > 30.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATE ==="));
        UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), OverallProgress);
        UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorsInLevel);
        UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), DinosaurCount);
        UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), CharacterCount);
        
        int32 CompletedMilestones = 0;
        for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
        {
            if (Milestone.bIsCompleted)
                CompletedMilestones++;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Milestones: %d/%d completed"), CompletedMilestones, ProductionMilestones.Num());
        
        LastLogTime = CurrentTime;
    }
}

FString UProductionDirector::GetProductionReport()
{
    FString Report = TEXT("=== PRODUCTION REPORT ===\n");
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), OverallProgress);
    Report += FString::Printf(TEXT("Total Actors: %d\n"), TotalActorsInLevel);
    Report += FString::Printf(TEXT("Dinosaurs: %d\n"), DinosaurCount);
    Report += FString::Printf(TEXT("Characters: %d\n"), CharacterCount);
    
    Report += TEXT("\nAgent Status:\n");
    for (const FDir_AgentStatus& Status : AgentStatuses)
    {
        if (Status.bIsActive)
        {
            Report += FString::Printf(TEXT("- %s: %s (%.1f%%)\n"), 
                *Status.AgentName, *Status.CurrentTask, Status.CompletionPercentage);
        }
    }
    
    Report += TEXT("\nMilestones:\n");
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        FString Status = Milestone.bIsCompleted ? TEXT("COMPLETED") : TEXT("IN PROGRESS");
        Report += FString::Printf(TEXT("- %s: %s\n"), *Milestone.MilestoneName, *Status);
    }
    
    return Report;
}

void UProductionDirector::CoordinateAgentTasks()
{
    // Priority-based task coordination
    for (FDir_AgentStatus& Status : AgentStatuses)
    {
        if (Status.bIsActive && Status.CompletionPercentage < 100.0f)
        {
            // Assign tasks based on current milestone priorities
            if (Status.AgentName.Contains(TEXT("World_Generator")) && DinosaurCount < 5)
            {
                UpdateAgentStatus(Status.AgentName, TEXT("Generate terrain with dinosaur spawn points"), Status.CompletionPercentage + 10.0f);
            }
            else if (Status.AgentName.Contains(TEXT("Character_Artist")) && CharacterCount == 0)
            {
                UpdateAgentStatus(Status.AgentName, TEXT("Create playable character"), Status.CompletionPercentage + 15.0f);
            }
            else if (Status.AgentName.Contains(TEXT("Environment_Artist")) && TotalActorsInLevel < 20)
            {
                UpdateAgentStatus(Status.AgentName, TEXT("Populate world with props"), Status.CompletionPercentage + 8.0f);
            }
        }
    }
}

void UProductionDirector::RunProductionAnalysis()
{
    UE_LOG(LogTemp, Warning, TEXT("Running Production Analysis..."));
    
    UpdateProductionMetrics();
    AnalyzeProductionState();
    CoordinateAgentTasks();
    
    FString Report = GetProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}