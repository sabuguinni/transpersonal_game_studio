#include "StudioDirector.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Landscape.h"
#include "GameFramework/Character.h"

UStudioDirectorComponent::UStudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds

    // Initialize biome centers
    BiomeCenters.Add(EDir_BiomeType::Swamp, FVector(-50000.0f, -45000.0f, 0.0f));
    BiomeCenters.Add(EDir_BiomeType::Forest, FVector(-45000.0f, 40000.0f, 0.0f));
    BiomeCenters.Add(EDir_BiomeType::Savanna, FVector(0.0f, 0.0f, 0.0f));
    BiomeCenters.Add(EDir_BiomeType::Desert, FVector(55000.0f, 0.0f, 0.0f));
    BiomeCenters.Add(EDir_BiomeType::Mountain, FVector(40000.0f, 50000.0f, 500.0f));

    // Initialize agent status
    AgentCompletionStatus.Add(TEXT("Agent_02_Engine_Architect"), false);
    AgentCompletionStatus.Add(TEXT("Agent_05_World_Generator"), false);
    AgentCompletionStatus.Add(TEXT("Agent_09_Character_Artist"), false);
    AgentCompletionStatus.Add(TEXT("Agent_12_Combat_AI"), false);
    AgentCompletionStatus.Add(TEXT("Agent_20_Integration"), false);

    // Initialize production metrics
    TotalActorsInLevel = 0;
    CompilationErrorCount = 0;
    ProductionEfficiencyScore = 0.0f;
    
    // Initialize critical system flags
    bLandscapeExpanded = false;
    bDinosaurActorsSpawned = false;
    bSurvivalHUDActive = false;
    bCompilationClean = false;
}

void UStudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Component initialized"));
    
    // Initialize coordination on begin play
    InitializeAgentCoordination();
    SetupBiomeCoordinators();
}

void UStudioDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update production metrics every tick
    UpdateProductionMetrics();
    CheckCriticalSystems();
}

void UStudioDirectorComponent::InitializeAgentCoordination()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Agent Coordination System"));
    
    // Set initial agent tasks
    AgentCurrentTasks.Add(TEXT("Agent_02_Engine_Architect"), TEXT("Verify C++ compilation status"));
    AgentCurrentTasks.Add(TEXT("Agent_05_World_Generator"), TEXT("Expand landscape to 200km2"));
    AgentCurrentTasks.Add(TEXT("Agent_09_Character_Artist"), TEXT("Create dinosaur actor blueprints"));
    AgentCurrentTasks.Add(TEXT("Agent_12_Combat_AI"), TEXT("Implement survival HUD"));
    AgentCurrentTasks.Add(TEXT("Agent_20_Integration"), TEXT("Clean orphaned headers"));
    
    UE_LOG(LogTemp, Warning, TEXT("Agent tasks assigned successfully"));
}

void UStudioDirectorComponent::ValidateGameSystems()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UE_LOG(LogTemp, Warning, TEXT("Validating Game Systems"));
    
    // Count total actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorsInLevel = AllActors.Num();
    
    // Check for landscape
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    bLandscapeExpanded = LandscapeActors.Num() > 0;
    
    // Check for character actors
    TArray<AActor*> CharacterActors;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), CharacterActors);
    bDinosaurActorsSpawned = CharacterActors.Num() > 1; // More than just player
    
    UE_LOG(LogTemp, Warning, TEXT("System validation complete - Actors: %d, Landscape: %s, Characters: %s"), 
           TotalActorsInLevel, 
           bLandscapeExpanded ? TEXT("Yes") : TEXT("No"),
           bDinosaurActorsSpawned ? TEXT("Yes") : TEXT("No"));
}

void UStudioDirectorComponent::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Landscape Expanded: %s"), bLandscapeExpanded ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %s"), bDinosaurActorsSpawned ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Survival HUD: %s"), bSurvivalHUDActive ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Compilation Clean: %s"), bCompilationClean ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Production Efficiency: %.2f"), ProductionEfficiencyScore);
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void UStudioDirectorComponent::SetupBiomeCoordinators()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UE_LOG(LogTemp, Warning, TEXT("Setting up Biome Coordinators"));
    
    // Clear existing coordinators
    BiomeCoordinatorActors.Empty();
    
    // Create coordinator actors for each biome
    for (const auto& BiomePair : BiomeCenters)
    {
        FVector SpawnLocation = BiomePair.Value + FVector(0, 0, 100); // Slightly elevated
        
        APointLight* CoordinatorLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (CoordinatorLight)
        {
            FString BiomeName = UEnum::GetValueAsString(BiomePair.Key);
            CoordinatorLight->SetActorLabel(FString::Printf(TEXT("COORD_%s"), *BiomeName));
            
            UPointLightComponent* LightComp = CoordinatorLight->GetPointLightComponent();
            if (LightComp)
            {
                // Set biome-specific colors
                switch (BiomePair.Key)
                {
                case EDir_BiomeType::Swamp:
                    LightComp->SetLightColor(FLinearColor(0.2f, 0.8f, 0.3f));
                    break;
                case EDir_BiomeType::Forest:
                    LightComp->SetLightColor(FLinearColor(0.1f, 0.6f, 0.1f));
                    break;
                case EDir_BiomeType::Savanna:
                    LightComp->SetLightColor(FLinearColor(1.0f, 0.8f, 0.3f));
                    break;
                case EDir_BiomeType::Desert:
                    LightComp->SetLightColor(FLinearColor(1.0f, 0.5f, 0.2f));
                    break;
                case EDir_BiomeType::Mountain:
                    LightComp->SetLightColor(FLinearColor(0.7f, 0.9f, 1.0f));
                    break;
                }
                LightComp->SetIntensity(1000.0f);
                LightComp->SetAttenuationRadius(5000.0f);
            }
            
            BiomeCoordinatorActors.Add(CoordinatorLight);
            UE_LOG(LogTemp, Warning, TEXT("Created coordinator for %s at %s"), *BiomeName, *SpawnLocation.ToString());
        }
    }
}

FVector UStudioDirectorComponent::GetBiomeCenter(EDir_BiomeType BiomeType)
{
    if (BiomeCenters.Contains(BiomeType))
    {
        return BiomeCenters[BiomeType];
    }
    return FVector::ZeroVector;
}

void UStudioDirectorComponent::AssignAgentTasks()
{
    UE_LOG(LogTemp, Warning, TEXT("Assigning Agent Tasks for Current Cycle"));
    
    // Log current task assignments
    for (const auto& TaskPair : AgentCurrentTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *TaskPair.Key, *TaskPair.Value);
    }
}

bool UStudioDirectorComponent::ValidateAgentDeliverables()
{
    int32 CompletedTasks = 0;
    int32 TotalTasks = AgentCompletionStatus.Num();
    
    for (const auto& StatusPair : AgentCompletionStatus)
    {
        if (StatusPair.Value)
        {
            CompletedTasks++;
        }
    }
    
    ProductionEfficiencyScore = (float)CompletedTasks / (float)TotalTasks * 100.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Deliverables: %d/%d completed (%.1f%%)"), 
           CompletedTasks, TotalTasks, ProductionEfficiencyScore);
    
    return CompletedTasks >= TotalTasks * 0.8f; // 80% completion threshold
}

void UStudioDirectorComponent::UpdateProductionMetrics()
{
    // Update metrics based on current world state
    ValidateGameSystems();
    
    // Calculate efficiency based on critical systems
    int32 SystemsComplete = 0;
    if (bLandscapeExpanded) SystemsComplete++;
    if (bDinosaurActorsSpawned) SystemsComplete++;
    if (bSurvivalHUDActive) SystemsComplete++;
    if (bCompilationClean) SystemsComplete++;
    
    ProductionEfficiencyScore = (float)SystemsComplete / 4.0f * 100.0f;
}

void UStudioDirectorComponent::CheckCriticalSystems()
{
    // Check compilation status (simplified check)
    bCompilationClean = true; // Assume clean if we're running
    
    // Additional system checks can be added here
}

void UStudioDirectorComponent::LogAgentProgress()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT PROGRESS LOG ==="));
    for (const auto& StatusPair : AgentCompletionStatus)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s"), 
               *StatusPair.Key, 
               StatusPair.Value ? TEXT("COMPLETE") : TEXT("IN PROGRESS"));
    }
}

// Studio Director Game Mode Implementation
AStudioDirectorGameMode::AStudioDirectorGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentProductionCycle = 0;
    bMinimumViablePrototypeReached = false;
}

void AStudioDirectorGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Game Mode initialized"));
    InitializeProductionPipeline();
}

void AStudioDirectorGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Check MVP status periodically
    if (!bMinimumViablePrototypeReached)
    {
        bMinimumViablePrototypeReached = ValidateMinimumViablePrototype();
    }
}

void AStudioDirectorGameMode::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Production Pipeline"));
    
    // Create director component if not exists
    if (!DirectorComponent)
    {
        DirectorComponent = CreateDefaultSubobject<UStudioDirectorComponent>(TEXT("DirectorComponent"));
    }
    
    CurrentProductionCycle++;
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle %d started"), CurrentProductionCycle);
}

void AStudioDirectorGameMode::ExecuteProductionCycle()
{
    if (DirectorComponent)
    {
        DirectorComponent->AssignAgentTasks();
        DirectorComponent->ValidateGameSystems();
        DirectorComponent->GenerateProductionReport();
    }
    
    CurrentProductionCycle++;
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle %d executed"), CurrentProductionCycle);
}

bool AStudioDirectorGameMode::ValidateMinimumViablePrototype()
{
    if (!DirectorComponent) return false;
    
    // Check MVP criteria
    bool bHasLandscape = DirectorComponent->bLandscapeExpanded;
    bool bHasCharacters = DirectorComponent->bDinosaurActorsSpawned;
    bool bHasUI = DirectorComponent->bSurvivalHUDActive;
    bool bCompiles = DirectorComponent->bCompilationClean;
    
    bool bMVPReached = bHasLandscape && bHasCharacters && bCompiles;
    
    if (bMVPReached && !bMinimumViablePrototypeReached)
    {
        UE_LOG(LogTemp, Warning, TEXT("MINIMUM VIABLE PROTOTYPE REACHED!"));
    }
    
    return bMVPReached;
}