#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/Character/TranspersonalCharacter.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize production state
    CurrentCycle = 11;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260611_011");
    bProductionActive = true;
    ProductionStartTime = 0.0f;
    
    // Initialize metrics
    CurrentMetrics = FDir_ProductionMetrics();
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    ProductionStartTime = GetWorld()->GetTimeSeconds();
    InitializeProduction();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update metrics every 5 seconds
    static float MetricsUpdateTimer = 0.0f;
    MetricsUpdateTimer += DeltaTime;
    
    if (MetricsUpdateTimer >= 5.0f)
    {
        UpdateProductionMetrics();
        MetricsUpdateTimer = 0.0f;
    }
}

void AProductionCoordinator::InitializeProduction()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing production for %s"), *CurrentCycleID);
    
    // Setup Milestone 1 requirements
    SetupMilestone1Requirements();
    
    // Initialize agent tasks
    AssignAgentTasks();
    
    // Perform initial metrics update
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Production initialization complete"));
}

void AProductionCoordinator::SetupMilestone1Requirements()
{
    // Clear existing milestones
    ProductionMilestones.Empty();
    
    // Create Milestone 1: "Walk Around"
    FDir_ProductionMilestone Milestone1;
    Milestone1.MilestoneName = TEXT("Milestone 1 - Walk Around");
    Milestone1.bIsComplete = false;
    Milestone1.CompletionPercentage = 0.0f;
    
    // Add requirements
    Milestone1.Requirements.Add(TEXT("ThirdPersonCharacter with WASD movement"));
    Milestone1.Requirements.Add(TEXT("Camera boom + follow camera"));
    Milestone1.Requirements.Add(TEXT("Landscape with basic terrain"));
    Milestone1.Requirements.Add(TEXT("Player can walk, run, jump"));
    Milestone1.Requirements.Add(TEXT("3-5 static dinosaur meshes in world"));
    Milestone1.Requirements.Add(TEXT("Directional light + sky atmosphere"));
    
    ProductionMilestones.Add(Milestone1);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Milestone 1 requirements set up"));
}

void AProductionCoordinator::AssignAgentTasks()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Define tasks for each agent based on current production needs
    TArray<TPair<int32, FString>> TaskAssignments = {
        {1, TEXT("Studio Director - Coordinate Milestone 1 completion")},
        {2, TEXT("Engine Architect - Validate core architecture")},
        {3, TEXT("Core Systems - Implement physics and collision")},
        {4, TEXT("Performance Optimizer - Ensure 60fps target")},
        {5, TEXT("World Generator - Create varied terrain landscape")},
        {6, TEXT("Environment Artist - Place vegetation and props")},
        {7, TEXT("Architecture Agent - Build primitive structures")},
        {8, TEXT("Lighting Agent - Setup day/night cycle")},
        {9, TEXT("Character Artist - Refine TranspersonalCharacter")},
        {10, TEXT("Animation Agent - Implement character animations")},
        {11, TEXT("NPC Behavior - Create basic dinosaur AI")},
        {12, TEXT("Combat AI - Implement survival mechanics")},
        {13, TEXT("Crowd Simulation - Optimize dinosaur herds")},
        {14, TEXT("Quest Designer - Create survival objectives")},
        {15, TEXT("Narrative Agent - Write survival lore")},
        {16, TEXT("Audio Agent - Add ambient prehistoric sounds")},
        {17, TEXT("VFX Agent - Create environmental effects")},
        {18, TEXT("QA Agent - Test Milestone 1 requirements")},
        {19, TEXT("Integration Agent - Build final playable prototype")}
    };
    
    for (const auto& Assignment : TaskAssignments)
    {
        FDir_AgentTask Task;
        Task.AgentID = Assignment.Key;
        Task.AgentName = FString::Printf(TEXT("Agent #%02d"), Assignment.Key);
        Task.CurrentTask = Assignment.Value;
        Task.TaskProgress = 0.0f;
        Task.bTaskComplete = false;
        Task.CycleCount = CurrentCycle;
        
        AgentTasks.Add(Task);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent tasks assigned for %d agents"), AgentTasks.Num());
}

void AProductionCoordinator::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count all level actors
    CountLevelActors();
    
    // Validate game systems
    ValidateGameSystems();
    
    // Calculate overall completion
    CalculateCompletionPercentage();
    
    // Update Milestone 1 validation
    ValidateMilestone1();
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Metrics updated - %d total actors, %.1f%% complete"), 
           CurrentMetrics.TotalActorsInLevel, CurrentMetrics.OverallCompletion);
}

void AProductionCoordinator::CountLevelActors()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    CurrentMetrics.TotalActorsInLevel = AllActors.Num();
    CurrentMetrics.CharacterCount = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.EnvironmentActorCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        // Count characters
        if (Actor->IsA<ATranspersonalCharacter>())
        {
            CurrentMetrics.CharacterCount++;
        }
        
        // Count dinosaur placeholders (static mesh actors with dinosaur names)
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("trex")) || 
                ActorName.Contains(TEXT("raptor")) || 
                ActorName.Contains(TEXT("brachio")) ||
                ActorName.Contains(TEXT("dinosaur")))
            {
                CurrentMetrics.DinosaurCount++;
            }
            else
            {
                CurrentMetrics.EnvironmentActorCount++;
            }
        }
    }
}

void AProductionCoordinator::ValidateGameSystems()
{
    CurrentMetrics.SystemsImplemented = 0;
    
    // Check for character system
    if (CurrentMetrics.CharacterCount > 0)
    {
        CurrentMetrics.SystemsImplemented++;
    }
    
    // Check for dinosaur system
    if (CurrentMetrics.DinosaurCount >= 3)
    {
        CurrentMetrics.SystemsImplemented++;
    }
    
    // Check for environment system
    if (CurrentMetrics.EnvironmentActorCount > 10)
    {
        CurrentMetrics.SystemsImplemented++;
    }
    
    // Check for game mode
    if (GetWorld()->GetAuthGameMode())
    {
        CurrentMetrics.SystemsImplemented++;
    }
    
    // Check for lighting
    TArray<AActor*> Lights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), Lights);
    if (Lights.Num() > 0)
    {
        CurrentMetrics.SystemsImplemented++;
    }
}

void AProductionCoordinator::CalculateCompletionPercentage()
{
    CurrentMetrics.OverallCompletion = (float(CurrentMetrics.SystemsImplemented) / float(CurrentMetrics.TotalSystems)) * 100.0f;
}

void AProductionCoordinator::ValidateMilestone1()
{
    if (ProductionMilestones.Num() == 0) return;
    
    FDir_ProductionMilestone& Milestone1 = ProductionMilestones[0];
    Milestone1.CompletedRequirements.Empty();
    
    // Validate each requirement
    if (ValidateCharacterMovement())
    {
        Milestone1.CompletedRequirements.Add(TEXT("ThirdPersonCharacter with WASD movement"));
    }
    
    if (ValidateTerrainSystem())
    {
        Milestone1.CompletedRequirements.Add(TEXT("Landscape with basic terrain"));
    }
    
    if (ValidateDinosaurPlaceholders())
    {
        Milestone1.CompletedRequirements.Add(TEXT("3-5 static dinosaur meshes in world"));
    }
    
    if (ValidateLightingSystem())
    {
        Milestone1.CompletedRequirements.Add(TEXT("Directional light + sky atmosphere"));
    }
    
    if (ValidateGameMode())
    {
        Milestone1.CompletedRequirements.Add(TEXT("Player can walk, run, jump"));
    }
    
    // Calculate completion percentage
    Milestone1.CompletionPercentage = (float(Milestone1.CompletedRequirements.Num()) / float(Milestone1.Requirements.Num())) * 100.0f;
    Milestone1.bIsComplete = (Milestone1.CompletionPercentage >= 100.0f);
}

bool AProductionCoordinator::ValidateCharacterMovement() const
{
    return CurrentMetrics.CharacterCount > 0;
}

bool AProductionCoordinator::ValidateTerrainSystem() const
{
    return CurrentMetrics.TotalActorsInLevel > 20; // Basic terrain should have multiple actors
}

bool AProductionCoordinator::ValidateDinosaurPlaceholders() const
{
    return CurrentMetrics.DinosaurCount >= 3;
}

bool AProductionCoordinator::ValidateLightingSystem() const
{
    TArray<AActor*> Lights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), Lights);
    return Lights.Num() > 0;
}

bool AProductionCoordinator::ValidateGameMode() const
{
    return GetWorld() && GetWorld()->GetAuthGameMode() != nullptr;
}

void AProductionCoordinator::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("PRODUCTION REPORT - %s\n"), *CurrentCycleID);
    Report += FString::Printf(TEXT("Cycle: %d\n"), CurrentCycle);
    Report += FString::Printf(TEXT("Total Actors: %d\n"), CurrentMetrics.TotalActorsInLevel);
    Report += FString::Printf(TEXT("Characters: %d\n"), CurrentMetrics.CharacterCount);
    Report += FString::Printf(TEXT("Dinosaurs: %d\n"), CurrentMetrics.DinosaurCount);
    Report += FString::Printf(TEXT("Environment: %d\n"), CurrentMetrics.EnvironmentActorCount);
    Report += FString::Printf(TEXT("Systems: %d/%d\n"), CurrentMetrics.SystemsImplemented, CurrentMetrics.TotalSystems);
    Report += FString::Printf(TEXT("Completion: %.1f%%\n"), CurrentMetrics.OverallCompletion);
    
    if (ProductionMilestones.Num() > 0)
    {
        const FDir_ProductionMilestone& M1 = ProductionMilestones[0];
        Report += FString::Printf(TEXT("Milestone 1: %.1f%% (%s)\n"), 
                                 M1.CompletionPercentage, 
                                 M1.bIsComplete ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

float AProductionCoordinator::GetOverallProgress() const
{
    return CurrentMetrics.OverallCompletion;
}

bool AProductionCoordinator::IsMilestone1Complete() const
{
    return ProductionMilestones.Num() > 0 && ProductionMilestones[0].bIsComplete;
}

FString AProductionCoordinator::GetCurrentProductionStatus() const
{
    if (IsMilestone1Complete())
    {
        return TEXT("Milestone 1 Complete - Ready for Milestone 2");
    }
    else if (CurrentMetrics.OverallCompletion > 50.0f)
    {
        return TEXT("Milestone 1 In Progress - Good Progress");
    }
    else
    {
        return TEXT("Milestone 1 In Progress - Needs Attention");
    }
}

void AProductionCoordinator::UpdateAgentTask(int32 AgentID, const FString& TaskDescription, float Progress)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.CurrentTask = TaskDescription;
            Task.TaskProgress = FMath::Clamp(Progress, 0.0f, 100.0f);
            Task.bTaskComplete = (Progress >= 100.0f);
            Task.CycleCount = CurrentCycle;
            break;
        }
    }
}

FDir_AgentTask AProductionCoordinator::GetAgentTask(int32 AgentID) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            return Task;
        }
    }
    
    return FDir_AgentTask(); // Return default task if not found
}

TArray<FDir_AgentTask> AProductionCoordinator::GetIncompleteAgentTasks() const
{
    TArray<FDir_AgentTask> IncompleteTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bTaskComplete)
        {
            IncompleteTasks.Add(Task);
        }
    }
    
    return IncompleteTasks;
}