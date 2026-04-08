#include "TranspersonalGameMode.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PhysicsCore/PhysicsSystemManager.h"
#include "PhysicsCore/RagdollSystem.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Set default classes
    DefaultPawnClass = nullptr; // Will be set in Blueprint
    PlayerControllerClass = APlayerController::StaticClass();
    
    // Default game settings
    bSurvivalMode = true;
    TimeProgressionSpeed = 1.0f;
    MaxCreatureCount = 100;
    RespawnDelay = 5.0f;
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Starting game initialization"));
    
    // Get Studio Director subsystem
    StudioDirector = GetWorld()->GetSubsystem<UStudioDirectorSubsystem>();
    
    if (!StudioDirector)
    {
        UE_LOG(LogTemp, Error, TEXT("TranspersonalGameMode: Failed to get Studio Director subsystem"));
        return;
    }
    
    // Initialize all game systems
    InitializeGameSystems();
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Game initialization complete"));
}

void ATranspersonalGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear all timers
    if (UWorld* World = GetWorld())
    {
        for (auto& RespawnTimer : RespawnTimers)
        {
            World->GetTimerManager().ClearTimer(RespawnTimer.Value);
        }
        RespawnTimers.Empty();
    }
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Game ending"));
    
    Super::EndPlay(EndPlayReason);
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Initializing game for map: %s"), *MapName);
    
    // Parse game options
    if (Options.Contains(TEXT("Survival=false")))
    {
        bSurvivalMode = false;
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Survival mode disabled via options"));
    }
    
    // Setup game rules based on mode
    SetupGameRules();
}

void ATranspersonalGameMode::InitializeGameSystems()
{
    if (!StudioDirector)
    {
        UE_LOG(LogTemp, Error, TEXT("TranspersonalGameMode: No Studio Director available"));
        return;
    }
    
    // Initialize systems in order
    InitializePhysicsSystems();
    InitializeAISystems();
    InitializeWorldSystems();
    
    // Notify Studio Director that initialization is complete
    StudioDirector->OnGameSystemsInitialized();
}

UStudioDirectorSubsystem* ATranspersonalGameMode::GetStudioDirector() const
{
    return StudioDirector;
}

void ATranspersonalGameMode::HandlePlayerDeath(APawn* DeadPlayer, const FVector& DeathLocation)
{
    if (!DeadPlayer)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player death at location: %s"), *DeathLocation.ToString());
    
    // Activate ragdoll if available
    if (URagdollSystemComponent* RagdollComponent = DeadPlayer->FindComponentByClass<URagdollSystemComponent>())
    {
        RagdollComponent->ActivateRagdoll(DeathLocation, 1000.0f);
    }
    
    // Handle respawn in survival mode
    if (bSurvivalMode)
    {
        if (AController* PlayerController = DeadPlayer->GetController())
        {
            // Schedule respawn after delay
            FTimerHandle& RespawnTimer = RespawnTimers.FindOrAdd(PlayerController);
            GetWorld()->GetTimerManager().SetTimer(
                RespawnTimer,
                FTimerDelegate::CreateUObject(this, &ATranspersonalGameMode::ExecuteRespawn, PlayerController),
                RespawnDelay,
                false
            );
        }
    }
    
    // Notify Studio Director
    if (StudioDirector)
    {
        StudioDirector->OnPlayerDeath(DeadPlayer, DeathLocation);
    }
}

void ATranspersonalGameMode::HandleCreatureDeath(APawn* DeadCreature, const FVector& DeathLocation)
{
    if (!DeadCreature)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Creature death: %s at location: %s"), 
           *DeadCreature->GetName(), *DeathLocation.ToString());
    
    // Activate ragdoll if available
    if (URagdollSystemComponent* RagdollComponent = DeadCreature->FindComponentByClass<URagdollSystemComponent>())
    {
        RagdollComponent->ActivateRagdoll(DeathLocation, 500.0f);
    }
    
    // Notify Studio Director
    if (StudioDirector)
    {
        StudioDirector->OnCreatureDeath(DeadCreature, DeathLocation);
    }
}

void ATranspersonalGameMode::RespawnPlayer(AController* PlayerController)
{
    if (!PlayerController)
    {
        return;
    }
    
    // Find a suitable player start
    AActor* PlayerStart = FindPlayerStart(PlayerController);
    if (!PlayerStart)
    {
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalGameMode: No player start found for respawn"));
        return;
    }
    
    // Spawn new pawn
    if (DefaultPawnClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
            DefaultPawnClass,
            PlayerStart->GetActorLocation(),
            PlayerStart->GetActorRotation(),
            SpawnParams
        );
        
        if (NewPawn)
        {
            PlayerController->Possess(NewPawn);
            UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player respawned successfully"));
        }
    }
}

void ATranspersonalGameMode::InitializePhysicsSystems()
{
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Initializing physics systems"));
    
    // Ensure Physics System Manager is available
    if (UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(GetWorld()))
    {
        PhysicsManager->InitializePhysicsSystems();
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Physics systems initialized"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalGameMode: Physics System Manager not available"));
    }
}

void ATranspersonalGameMode::InitializeAISystems()
{
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Initializing AI systems"));
    
    // AI system initialization will be handled by other agents
    // For now, just log that we're ready for AI systems
    if (StudioDirector)
    {
        StudioDirector->RequestAISystemInitialization();
    }
}

void ATranspersonalGameMode::InitializeWorldSystems()
{
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Initializing world systems"));
    
    // World system initialization will be handled by other agents
    // For now, just log that we're ready for world systems
    if (StudioDirector)
    {
        StudioDirector->RequestWorldSystemInitialization();
    }
}

void ATranspersonalGameMode::SetupGameRules()
{
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Setting up game rules - Survival Mode: %s"), 
           bSurvivalMode ? TEXT("Enabled") : TEXT("Disabled"));
    
    // Configure game rules based on survival mode
    if (bSurvivalMode)
    {
        // Enable permadeath, resource scarcity, etc.
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Survival rules active"));
    }
    else
    {
        // More forgiving rules for exploration/creative mode
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Creative rules active"));
    }
}

void ATranspersonalGameMode::ExecuteRespawn(AController* PlayerController)
{
    if (PlayerController)
    {
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Executing respawn for player"));
        RespawnPlayer(PlayerController);
        
        // Clean up timer
        RespawnTimers.Remove(PlayerController);
    }
}