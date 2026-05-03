#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize milestone tracking
    bTerrainGenerated = false;
    bCharacterMovementWorking = false;
    bDinosaursSpawned = false;
    bLightingSetup = false;
    bSurvivalHUDActive = false;

    CurrentCycle = 0;
    LastVerificationTime = 0.0f;
    bInitialized = false;

    // Initialize task arrays
    PendingTasks.Empty();
    CompletedTasks.Empty();
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();

    // Initialize production coordination
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing Milestone 1 tracking"));
    
    // Perform initial verification
    CheckMilestone1Progress();
    bInitialized = true;
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Verify milestone progress every 10 seconds
    LastVerificationTime += DeltaTime;
    if (LastVerificationTime >= 10.0f)
    {
        CheckMilestone1Progress();
        LastVerificationTime = 0.0f;
    }
}

bool ADir_ProductionCoordinator::CheckMilestone1Progress()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Verify terrain (landscape actors)
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    bTerrainGenerated = LandscapeActors.Num() > 0;

    // Verify character movement
    VerifyCharacterMovement();

    // Verify dinosaur actors
    VerifyDinosaurActors();

    // Verify lighting setup
    VerifyLightingSetup();

    // Log current status
    LogProductionStatus();

    return GetMilestone1Completion() >= 0.8f; // 80% completion threshold
}

void ADir_ProductionCoordinator::VerifyCharacterMovement()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        bCharacterMovementWorking = false;
        return;
    }

    // Find TranspersonalCharacter or any character
    TArray<AActor*> CharacterActors;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), CharacterActors);
    
    bCharacterMovementWorking = false;
    for (AActor* Actor : CharacterActors)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->GetMovementComponent())
        {
            bCharacterMovementWorking = true;
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Character movement verified - %s"), *Character->GetName());
            break;
        }
    }

    if (!bCharacterMovementWorking)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: No functional character found"));
    }
}

void ADir_ProductionCoordinator::VerifyDinosaurActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        bDinosaursSpawned = false;
        return;
    }

    // Count actors with "dinosaur" or specific dinosaur names in their class or name
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetName().ToLower();
        FString ClassName = Actor->GetClass()->GetName().ToLower();
        
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
            ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("dinosaur")) ||
            ClassName.Contains(TEXT("dinosaur")))
        {
            DinosaurCount++;
        }
    }

    bDinosaursSpawned = DinosaurCount >= 3; // Minimum 3 dinosaurs for milestone
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Found %d dinosaur actors"), DinosaurCount);
}

void ADir_ProductionCoordinator::VerifyLightingSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        bLightingSetup = false;
        return;
    }

    // Check for essential lighting components
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);

    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);

    // Basic lighting setup requires at least one directional light
    bLightingSetup = DirectionalLights.Num() >= 1;
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Lighting - DirectionalLights: %d, SkyLights: %d"), 
           DirectionalLights.Num(), SkyLights.Num());
}

void ADir_ProductionCoordinator::VerifyTerrainQuality()
{
    // This will be expanded by Agent #5 (Procedural World Generator)
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Terrain quality check - delegated to Agent #5"));
}

float ADir_ProductionCoordinator::GetMilestone1Completion()
{
    int32 CompletedItems = 0;
    int32 TotalItems = 5;

    if (bTerrainGenerated) CompletedItems++;
    if (bCharacterMovementWorking) CompletedItems++;
    if (bDinosaursSpawned) CompletedItems++;
    if (bLightingSetup) CompletedItems++;
    if (bSurvivalHUDActive) CompletedItems++;

    return static_cast<float>(CompletedItems) / static_cast<float>(TotalItems);
}

void ADir_ProductionCoordinator::LogProductionStatus()
{
    float Completion = GetMilestone1Completion() * 100.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("=== MILESTONE 1 PROGRESS: %.1f%% ==="), Completion);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Generated: %s"), bTerrainGenerated ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Character Movement: %s"), bCharacterMovementWorking ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs Spawned: %s"), bDinosaursSpawned ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Lighting Setup: %s"), bLightingSetup ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Survival HUD: %s"), bSurvivalHUDActive ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Pending Tasks: %d"), PendingTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), CompletedTasks.Num());
}

void ADir_ProductionCoordinator::AssignAgentTask(const FString& AgentName, const FString& TaskDescription)
{
    FString FullTask = FString::Printf(TEXT("[%s] %s"), *AgentName, *TaskDescription);
    PendingTasks.AddUnique(FullTask);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Assigned task to %s: %s"), *AgentName, *TaskDescription);
}

void ADir_ProductionCoordinator::CompleteAgentTask(const FString& TaskDescription)
{
    // Move from pending to completed
    for (int32 i = PendingTasks.Num() - 1; i >= 0; i--)
    {
        if (PendingTasks[i].Contains(TaskDescription))
        {
            CompletedTasks.Add(PendingTasks[i]);
            PendingTasks.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Task completed: %s"), *TaskDescription);
            break;
        }
    }
}

void ADir_ProductionCoordinator::InitializeCycle(int32 CycleNumber)
{
    CurrentCycle = CycleNumber;
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing Cycle %d"), CycleNumber);
    
    // Clear old pending tasks and set new priorities
    PendingTasks.Empty();
    
    // Define cycle-specific priorities based on Milestone 1 needs
    if (CurrentCycle >= 5)
    {
        if (!bTerrainGenerated)
        {
            AssignAgentTask(TEXT("Agent #5"), TEXT("Generate realistic terrain with hills and valleys"));
        }
        
        if (!bCharacterMovementWorking)
        {
            AssignAgentTask(TEXT("Agent #9"), TEXT("Implement functional character movement and input"));
        }
        
        if (!bDinosaursSpawned)
        {
            AssignAgentTask(TEXT("Agent #10"), TEXT("Spawn 5 dinosaur actors with basic collision"));
        }
        
        if (!bLightingSetup)
        {
            AssignAgentTask(TEXT("Agent #8"), TEXT("Setup proper day/night lighting cycle"));
        }
        
        if (!bSurvivalHUDActive)
        {
            AssignAgentTask(TEXT("Agent #12"), TEXT("Create survival HUD with health/hunger/thirst bars"));
        }
    }
}

TArray<FString> ADir_ProductionCoordinator::GetNextAgentPriorities()
{
    TArray<FString> Priorities;
    
    // Return current pending tasks as priorities
    for (const FString& Task : PendingTasks)
    {
        Priorities.Add(Task);
    }
    
    // If no pending tasks, generate default priorities
    if (Priorities.Num() == 0)
    {
        Priorities.Add(TEXT("Focus on Milestone 1: Playable prototype"));
        Priorities.Add(TEXT("Ensure character can walk around terrain"));
        Priorities.Add(TEXT("Place visible dinosaurs in the world"));
        Priorities.Add(TEXT("Verify lighting and atmosphere"));
    }
    
    return Priorities;
}