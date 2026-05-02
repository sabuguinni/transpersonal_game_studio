#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create coordination mesh component
    CoordinationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoordinationMesh"));
    RootComponent = CoordinationMesh;

    // Set default mesh to cube for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        CoordinationMesh->SetStaticMesh(CubeMeshAsset.Object);
        CoordinationMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }

    // Initialize agent list
    ActiveAgents = {
        "Agent02_EngineArchitect",
        "Agent03_CoreSystems", 
        "Agent05_WorldGen",
        "Agent06_EnvironmentArt",
        "Agent09_CharacterArt",
        "Agent10_Animation",
        "Agent11_NPCBehavior",
        "Agent12_CombatAI"
    };

    // Initialize pending tasks for Milestone 1
    PendingTasks = {
        "Agent02: Core architecture review and stability",
        "Agent03: Physics collision and ragdoll systems",
        "Agent05: Terrain enhancement with height variation",
        "Agent06: Environment population with props",
        "Agent09: Character model and movement refinement",
        "Agent10: Animation system and movement blending",
        "Agent11: Basic NPC behavior patterns",
        "Agent12: Dinosaur AI and territorial behavior"
    };
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized for Cycle %s"), *CurrentCycleID);
    
    // Initialize production cycle
    InitializeProductionCycle();
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    ProductionTimer += DeltaTime;
    
    // Update production metrics every 5 seconds
    if (FMath::Fmod(ProductionTimer, 5.0f) < DeltaTime)
    {
        UpdateProductionMetrics();
    }
}

void ADir_ProductionCoordinator::InitializeProductionCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Production Cycle %s"), *CurrentCycleID);
    
    // Clean up any duplicate actors first
    CleanupDuplicateActors();
    
    // Dispatch tasks to agents
    DispatchAgentTasks();
    
    // Validate current milestone progress
    ValidateMilestone1Progress();
}

void ADir_ProductionCoordinator::DispatchAgentTasks()
{
    UE_LOG(LogTemp, Warning, TEXT("Dispatching tasks to %d active agents"), ActiveAgents.Num());
    
    // Assign specific tasks for Milestone 1
    AssignTaskToAgent("Agent02_EngineArchitect", "Review and stabilize core architecture");
    AssignTaskToAgent("Agent03_CoreSystems", "Implement physics collision and ragdoll");
    AssignTaskToAgent("Agent05_WorldGen", "Enhance terrain with realistic height variation");
    AssignTaskToAgent("Agent06_EnvironmentArt", "Populate world with vegetation and props");
    AssignTaskToAgent("Agent09_CharacterArt", "Refine character model and movement");
    AssignTaskToAgent("Agent10_Animation", "Implement movement animations and blending");
    AssignTaskToAgent("Agent11_NPCBehavior", "Create basic NPC behavior patterns");
    AssignTaskToAgent("Agent12_CombatAI", "Implement dinosaur AI and territorial behavior");
}

void ADir_ProductionCoordinator::ValidateMilestone1Progress()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating Milestone 1 progress"));
    
    // Check for character movement system
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), Characters);
    bCharacterMovementReady = Characters.Num() > 0;
    
    // Check for terrain
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Landscapes);
    bTerrainComplete = Landscapes.Num() > 10; // Basic check for populated world
    
    // Check for dinosaurs
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains("dinosaur") || ActorName.Contains("trex") || ActorName.Contains("raptor"))
        {
            DinosaurCount++;
        }
    }
    bDinosaursPlaced = DinosaurCount >= 3;
    
    // Check for lighting
    TArray<AActor*> Lights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Lights);
    int32 LightCount = 0;
    for (AActor* Actor : Lights)
    {
        if (Actor->GetClass()->GetName().Contains("Light"))
        {
            LightCount++;
        }
    }
    bLightingSetup = LightCount > 0;
    
    // Camera system is assumed ready if character exists
    bCameraSystemReady = bCharacterMovementReady;
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: Character=%d, Terrain=%d, Dinosaurs=%d, Lighting=%d, Camera=%d"), 
           bCharacterMovementReady, bTerrainComplete, bDinosaursPlaced, bLightingSetup, bCameraSystemReady);
}

bool ADir_ProductionCoordinator::CheckPlayablePrototypeReady()
{
    return bCharacterMovementReady && bTerrainComplete && bDinosaursPlaced && bLightingSetup && bCameraSystemReady;
}

void ADir_ProductionCoordinator::CleanupDuplicateActors()
{
    UE_LOG(LogTemp, Warning, TEXT("Cleaning up duplicate actors"));
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    // Track lighting actors for cleanup
    TArray<AActor*> DirectionalLights;
    TArray<AActor*> SkyLights;
    TArray<AActor*> SkyAtmospheres;
    TArray<AActor*> ExponentialFogs;
    
    for (AActor* Actor : AllActors)
    {
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName == "DirectionalLight")
        {
            DirectionalLights.Add(Actor);
        }
        else if (ClassName == "SkyLight")
        {
            SkyLights.Add(Actor);
        }
        else if (ClassName == "SkyAtmosphere")
        {
            SkyAtmospheres.Add(Actor);
        }
        else if (ClassName == "ExponentialHeightFog")
        {
            ExponentialFogs.Add(Actor);
        }
    }
    
    // Keep only one of each lighting type
    int32 CleanupCount = 0;
    
    // Remove duplicate DirectionalLights (keep first)
    for (int32 i = 1; i < DirectionalLights.Num(); i++)
    {
        DirectionalLights[i]->Destroy();
        CleanupCount++;
    }
    
    // Remove duplicate SkyLights (keep first)
    for (int32 i = 1; i < SkyLights.Num(); i++)
    {
        SkyLights[i]->Destroy();
        CleanupCount++;
    }
    
    // Remove duplicate SkyAtmospheres (keep first)
    for (int32 i = 1; i < SkyAtmospheres.Num(); i++)
    {
        SkyAtmospheres[i]->Destroy();
        CleanupCount++;
    }
    
    // Remove duplicate ExponentialFogs (keep first)
    for (int32 i = 1; i < ExponentialFogs.Num(); i++)
    {
        ExponentialFogs[i]->Destroy();
        CleanupCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cleaned up %d duplicate actors"), CleanupCount);
}

FString ADir_ProductionCoordinator::GetProductionStatusReport()
{
    FString Report = FString::Printf(TEXT("Production Status Report - Cycle %s\n"), *CurrentCycleID);
    Report += FString::Printf(TEXT("Budget: $%.2f / $%.2f\n"), BudgetUsedToday, BudgetLimitDaily);
    Report += FString::Printf(TEXT("Active Agents: %d\n"), ActiveAgents.Num());
    Report += FString::Printf(TEXT("Pending Tasks: %d\n"), PendingTasks.Num());
    Report += FString::Printf(TEXT("Completed Tasks: %d\n"), CompletedTasks.Num());
    
    Report += TEXT("\nMilestone 1 Progress:\n");
    Report += FString::Printf(TEXT("- Character Movement: %s\n"), bCharacterMovementReady ? TEXT("READY") : TEXT("PENDING"));
    Report += FString::Printf(TEXT("- Terrain: %s\n"), bTerrainComplete ? TEXT("READY") : TEXT("PENDING"));
    Report += FString::Printf(TEXT("- Dinosaurs: %s\n"), bDinosaursPlaced ? TEXT("READY") : TEXT("PENDING"));
    Report += FString::Printf(TEXT("- Lighting: %s\n"), bLightingSetup ? TEXT("READY") : TEXT("PENDING"));
    Report += FString::Printf(TEXT("- Camera: %s\n"), bCameraSystemReady ? TEXT("READY") : TEXT("PENDING"));
    
    bool bPrototypeReady = CheckPlayablePrototypeReady();
    Report += FString::Printf(TEXT("\nPlayable Prototype: %s\n"), bPrototypeReady ? TEXT("READY") : TEXT("IN PROGRESS"));
    
    return Report;
}

void ADir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription)
{
    FString TaskEntry = FString::Printf(TEXT("%s: %s"), *AgentName, *TaskDescription);
    if (!PendingTasks.Contains(TaskEntry))
    {
        PendingTasks.Add(TaskEntry);
        UE_LOG(LogTemp, Log, TEXT("Assigned task to %s: %s"), *AgentName, *TaskDescription);
    }
}

void ADir_ProductionCoordinator::MarkTaskCompleted(const FString& AgentName, const FString& TaskDescription)
{
    FString TaskEntry = FString::Printf(TEXT("%s: %s"), *AgentName, *TaskDescription);
    
    if (PendingTasks.Contains(TaskEntry))
    {
        PendingTasks.Remove(TaskEntry);
        CompletedTasks.Add(TaskEntry);
        UE_LOG(LogTemp, Log, TEXT("Task completed by %s: %s"), *AgentName, *TaskDescription);
    }
}

TArray<FString> ADir_ProductionCoordinator::GetPendingTasksForAgent(const FString& AgentName)
{
    TArray<FString> AgentTasks;
    
    for (const FString& Task : PendingTasks)
    {
        if (Task.StartsWith(AgentName))
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

void ADir_ProductionCoordinator::UpdateProductionMetrics()
{
    // Update production timer and metrics
    ValidateMilestone1Progress();
    
    // Log periodic status
    if (FMath::Fmod(ProductionTimer, 30.0f) < 1.0f) // Every 30 seconds
    {
        FString StatusReport = GetProductionStatusReport();
        UE_LOG(LogTemp, Warning, TEXT("Production Update:\n%s"), *StatusReport);
    }
}

void ADir_ProductionCoordinator::CheckAgentDeliverables()
{
    // Check if agents have produced their required deliverables
    // This would integrate with the actual agent system in a full implementation
    UE_LOG(LogTemp, Log, TEXT("Checking agent deliverables for cycle %s"), *CurrentCycleID);
}

void ADir_ProductionCoordinator::EnsureMinimalPlayableElements()
{
    // Ensure the minimal elements for a playable prototype exist
    // This is a safety check to maintain the playable state
    
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
    
    if (PlayerStarts.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found - playable prototype may not work"));
    }
    
    // Additional checks could be added here for other essential elements
}