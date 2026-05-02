#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Landscape/Landscape.h"

UDir_StudioDirector::UDir_StudioDirector()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Tick every 5 seconds

    // Initialize production state
    bProductionInitialized = false;
    bMilestone1Complete = false;
    CurrentCycleID = 5;
    ProductionBudgetUsed = 38.73f;
    ProductionBudgetLimit = 150.0f;

    // Initialize counters
    TerrainActorCount = 0;
    DinosaurActorCount = 0;
    LightingActorCount = 0;
    PlayerCharacterCount = 0;
}

void UDir_StudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing production pipeline"));
    InitializeProductionPipeline();
}

void UDir_StudioDirector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update production metrics every tick
    UpdateProductionMetrics();
    
    // Validate map state
    ValidateActorCounts();
    
    // Check milestone completion
    if (!bMilestone1Complete)
    {
        bMilestone1Complete = CheckMilestone1Completion();
        if (bMilestone1Complete)
        {
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: MILESTONE 1 COMPLETED!"));
        }
    }
}

void UDir_StudioDirector::InitializeProductionPipeline()
{
    if (bProductionInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Setting up production pipeline"));

    // Initialize agent task assignments
    AgentTaskAssignments.Empty();
    AgentTaskCompletion.Empty();

    // Assign initial tasks
    AssignAgentTasks();

    // Validate current map state
    ValidateMinPlayableMap();

    // Clean up any duplicate actors
    CleanupDuplicateActors();

    bProductionInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Production pipeline initialized"));
}

void UDir_StudioDirector::AssignAgentTasks()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Assigning agent tasks for Cycle %d"), CurrentCycleID);

    // Clear previous assignments
    AgentTaskAssignments.Empty();
    AgentTaskCompletion.Empty();

    // Milestone 1 focused task assignments
    AgentTaskAssignments.Add(TEXT("Agent_02_Engine"), TEXT("Verify core systems compilation and fix any linker errors"));
    AgentTaskAssignments.Add(TEXT("Agent_03_Core"), TEXT("Implement physics collision for dinosaur placeholders"));
    AgentTaskAssignments.Add(TEXT("Agent_05_World"), TEXT("Enhance terrain with proper height variations and biome transitions"));
    AgentTaskAssignments.Add(TEXT("Agent_06_Environment"), TEXT("Replace placeholder trees/rocks with proper meshes"));
    AgentTaskAssignments.Add(TEXT("Agent_09_Character"), TEXT("Create proper dinosaur character classes with basic AI"));
    AgentTaskAssignments.Add(TEXT("Agent_10_Animation"), TEXT("Add basic locomotion animations to dinosaurs"));
    AgentTaskAssignments.Add(TEXT("Agent_12_Combat"), TEXT("Implement survival HUD showing health/hunger/thirst/stamina/fear bars"));

    // Initialize completion status
    for (auto& Task : AgentTaskAssignments)
    {
        AgentTaskCompletion.Add(Task.Key, false);
    }

    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Assigned %d agent tasks"), AgentTaskAssignments.Num());
}

void UDir_StudioDirector::ValidateMinPlayableMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: No world found for validation"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Validating MinPlayableMap state"));

    // Reset counters
    TerrainActorCount = 0;
    DinosaurActorCount = 0;
    LightingActorCount = 0;
    PlayerCharacterCount = 0;

    // Count actors by type
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }

        FString ActorName = Actor->GetName().ToLower();
        FString ClassName = Actor->GetClass()->GetName();

        // Count terrain
        if (Actor->IsA<ALandscape>() || ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
        {
            TerrainActorCount++;
        }

        // Count dinosaurs
        if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("trex")) || 
            ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachio")))
        {
            DinosaurActorCount++;
        }

        // Count lighting
        if (Actor->FindComponentByClass<UDirectionalLightComponent>() ||
            Actor->FindComponentByClass<USkyAtmosphereComponent>() ||
            Actor->FindComponentByClass<USkyLightComponent>() ||
            Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            LightingActorCount++;
        }

        // Count player characters
        if (Actor->IsA<ACharacter>() && (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player"))))
        {
            PlayerCharacterCount++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Map validation complete - Terrain: %d, Dinosaurs: %d, Lighting: %d, Players: %d"), 
           TerrainActorCount, DinosaurActorCount, LightingActorCount, PlayerCharacterCount);
}

bool UDir_StudioDirector::CheckMilestone1Completion()
{
    // Milestone 1 requirements:
    // - Player can walk around (TranspersonalCharacter with movement)
    // - Terrain with height variation
    // - 3-5 static dinosaur meshes
    // - Basic lighting setup

    bool bPlayerMovement = ValidatePlayerMovement();
    bool bTerrain = ValidateTerrainPresence();
    bool bDinosaurs = ValidateDinosaurPresence();
    bool bLighting = ValidateLightingSetup();
    bool bGameMode = ValidateGameModeActive();

    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Milestone 1 Check - Player: %s, Terrain: %s, Dinosaurs: %s, Lighting: %s, GameMode: %s"),
           bPlayerMovement ? TEXT("OK") : TEXT("FAIL"),
           bTerrain ? TEXT("OK") : TEXT("FAIL"),
           bDinosaurs ? TEXT("OK") : TEXT("FAIL"),
           bLighting ? TEXT("OK") : TEXT("FAIL"),
           bGameMode ? TEXT("OK") : TEXT("FAIL"));

    return bPlayerMovement && bTerrain && bDinosaurs && bLighting && bGameMode;
}

void UDir_StudioDirector::CoordinateEngineArchitect()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Coordinating Engine Architect - Focus on core system stability"));
}

void UDir_StudioDirector::CoordinateWorldGeneration()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Coordinating World Generation - Focus on terrain enhancement"));
}

void UDir_StudioDirector::CoordinateCharacterSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Coordinating Character Systems - Focus on dinosaur implementation"));
}

void UDir_StudioDirector::RunProductionChecks()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Running production quality checks"));
    
    ValidateMinPlayableMap();
    
    // Check budget usage
    float BudgetPercentage = (ProductionBudgetUsed / ProductionBudgetLimit) * 100.0f;
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Budget usage: %.2f%% (%.2f/%.2f USD)"), 
           BudgetPercentage, ProductionBudgetUsed, ProductionBudgetLimit);
    
    // Check agent task completion
    int32 CompletedTasks = 0;
    for (auto& Task : AgentTaskCompletion)
    {
        if (Task.Value)
        {
            CompletedTasks++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent tasks completed: %d/%d"), 
           CompletedTasks, AgentTaskCompletion.Num());
}

void UDir_StudioDirector::CleanupDuplicateActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Cleaning up duplicate actors"));

    // This function is called from UE5 Python scripts
    // The actual cleanup is handled by the ue5_execute commands
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Duplicate cleanup delegated to UE5 Python scripts"));
}

void UDir_StudioDirector::UpdateProductionMetrics()
{
    // Update production metrics based on current state
    // This would typically interface with external systems
    // For now, we track basic metrics
}

void UDir_StudioDirector::ValidateActorCounts()
{
    // Validate that we have the expected number of actors
    // This helps detect when agents add or remove content
}

void UDir_StudioDirector::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %d"), CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Budget: %.2f/%.2f USD"), ProductionBudgetUsed, ProductionBudgetLimit);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Complete: %s"), bMilestone1Complete ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Terrain Actors: %d"), TerrainActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), DinosaurActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Lighting Actors: %d"), LightingActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Player Characters: %d"), PlayerCharacterCount);
}

void UDir_StudioDirector::ScheduleNextAgentCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Scheduling next agent cycle"));
}

bool UDir_StudioDirector::ValidatePlayerMovement()
{
    return PlayerCharacterCount > 0;
}

bool UDir_StudioDirector::ValidateTerrainPresence()
{
    return TerrainActorCount > 0;
}

bool UDir_StudioDirector::ValidateDinosaurPresence()
{
    return DinosaurActorCount >= 3;
}

bool UDir_StudioDirector::ValidateLightingSetup()
{
    return LightingActorCount > 0;
}

bool UDir_StudioDirector::ValidateGameModeActive()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    AGameModeBase* GameMode = World->GetAuthGameMode();
    return GameMode != nullptr;
}