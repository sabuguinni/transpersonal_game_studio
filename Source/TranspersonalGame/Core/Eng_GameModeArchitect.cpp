#include "Eng_GameModeArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AEng_GameModeArchitect::AEng_GameModeArchitect()
{
    // Set this game mode to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Initialize architecture state
    bArchitectureInitialized = false;
    bWorldSystemsReady = false;
    bPlayerSystemsReady = false;
    CurrentGamePhase = EEng_GamePhase::Initialization;
    LastPerformanceUpdateTime = 0.0f;
    SystemIntegrityCheckCount = 0;

    // Set default player controller and pawn classes
    PlayerControllerClass = APlayerController::StaticClass();
    
    // Try to find TranspersonalCharacter class
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (PlayerPawnClassFinder.Class != nullptr)
    {
        DefaultPawnClass = PlayerPawnClassFinder.Class;
        UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Found TranspersonalCharacter class"));
    }
    else
    {
        // Fallback to default pawn
        DefaultPawnClass = APawn::StaticClass();
        UE_LOG(LogTemp, Warning, TEXT("GameModeArchitect: TranspersonalCharacter not found, using default pawn"));
    }

    // Initialize performance metrics
    CurrentPerformanceMetrics.FrameRate = 0.0f;
    CurrentPerformanceMetrics.MemoryUsageMB = 0.0f;
    CurrentPerformanceMetrics.ActiveActorCount = 0;
    CurrentPerformanceMetrics.PhysicsObjectCount = 0;
    CurrentPerformanceMetrics.RenderThreadTimeMs = 0.0f;
    CurrentPerformanceMetrics.GameThreadTimeMs = 0.0f;
}

void AEng_GameModeArchitect::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: BeginPlay - Starting core architecture initialization"));
    
    // Initialize core architecture
    InitializeCoreArchitecture();
    
    // Set up initial game phase
    SetGamePhase(EEng_GamePhase::WorldGeneration);
    
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: BeginPlay complete"));
}

void AEng_GameModeArchitect::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: EndPlay - Shutting down architecture"));
    
    // Clean up registered subsystems
    RegisteredSubsystems.Empty();
    
    // Reset architecture state
    bArchitectureInitialized = false;
    bWorldSystemsReady = false;
    bPlayerSystemsReady = false;
    
    Super::EndPlay(EndPlayReason);
}

void AEng_GameModeArchitect::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: InitGame - Map: %s, Options: %s"), *MapName, *Options);
    
    Super::InitGame(MapName, Options, ErrorMessage);
    
    // Validate system integrity before game starts
    ValidateSystemIntegrity();
}

void AEng_GameModeArchitect::InitGameState()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: InitGameState"));
    
    Super::InitGameState();
    
    // Initialize world systems after game state is ready
    InitializeWorldSystems();
}

void AEng_GameModeArchitect::StartPlay()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: StartPlay"));
    
    Super::StartPlay();
    
    // Initialize player systems
    InitializePlayerSystems();
    
    // Transition to active gameplay phase
    SetGamePhase(EEng_GamePhase::ActiveGameplay);
}

void AEng_GameModeArchitect::PostLogin(APlayerController* NewPlayer)
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: PostLogin - Player joined"));
    
    Super::PostLogin(NewPlayer);
    
    // Ensure player systems are ready
    if (!bPlayerSystemsReady)
    {
        InitializePlayerSystems();
    }
}

void AEng_GameModeArchitect::Logout(AController* Exiting)
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Logout - Player leaving"));
    
    Super::Logout(Exiting);
}

void AEng_GameModeArchitect::InitializeCoreArchitecture()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Initializing core architecture"));
    
    // Set up world partition settings
    SetupWorldPartitionSettings();
    
    // Configure physics settings
    ConfigurePhysicsSettings();
    
    // Initialize audio systems
    InitializeAudioSystems();
    
    // Set up rendering pipeline
    SetupRenderingPipeline();
    
    bArchitectureInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Core architecture initialization complete"));
}

void AEng_GameModeArchitect::InitializeWorldSystems()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Initializing world systems"));
    
    if (!bArchitectureInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeArchitect: Cannot initialize world systems - core architecture not ready"));
        return;
    }
    
    // Initialize biome manager
    UObject* BiomeManager = GetWorld()->GetSubsystem<UGameInstanceSubsystem>();
    if (BiomeManager)
    {
        RegisterSubsystem(TEXT("BiomeManager"), BiomeManager);
        UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: BiomeManager registered"));
    }
    
    bWorldSystemsReady = true;
    
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: World systems initialization complete"));
}

void AEng_GameModeArchitect::InitializePlayerSystems()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Initializing player systems"));
    
    if (!bWorldSystemsReady)
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeArchitect: Cannot initialize player systems - world systems not ready"));
        return;
    }
    
    // Initialize default player class
    InitializeDefaultPlayerClass();
    
    bPlayerSystemsReady = true;
    
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Player systems initialization complete"));
}

void AEng_GameModeArchitect::ValidateSystemIntegrity()
{
    SystemIntegrityCheckCount++;
    SystemValidationErrors.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Validating system integrity (Check #%d)"), SystemIntegrityCheckCount);
    
    // Validate subsystem dependencies
    ValidateSubsystemDependencies();
    
    // Check architecture state
    if (!bArchitectureInitialized)
    {
        SystemValidationErrors.Add(TEXT("Core architecture not initialized"));
    }
    
    // Log validation results
    if (SystemValidationErrors.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: System integrity validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeArchitect: System integrity validation FAILED with %d errors:"), SystemValidationErrors.Num());
        for (const FString& Error : SystemValidationErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
}

bool AEng_GameModeArchitect::IsArchitectureReady() const
{
    return bArchitectureInitialized && bWorldSystemsReady && bPlayerSystemsReady;
}

void AEng_GameModeArchitect::RegisterSubsystem(const FString& SubsystemName, UObject* Subsystem)
{
    if (Subsystem)
    {
        RegisteredSubsystems.Add(SubsystemName, Subsystem);
        UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Registered subsystem: %s"), *SubsystemName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeArchitect: Failed to register subsystem: %s (null object)"), *SubsystemName);
    }
}

UObject* AEng_GameModeArchitect::GetRegisteredSubsystem(const FString& SubsystemName) const
{
    UObject* const* FoundSubsystem = RegisteredSubsystems.Find(SubsystemName);
    return FoundSubsystem ? *FoundSubsystem : nullptr;
}

void AEng_GameModeArchitect::SetGamePhase(EEng_GamePhase NewPhase)
{
    if (CurrentGamePhase != NewPhase)
    {
        EEng_GamePhase OldPhase = CurrentGamePhase;
        CurrentGamePhase = NewPhase;
        
        UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Game phase transition: %d -> %d"), (int32)OldPhase, (int32)NewPhase);
    }
}

EEng_GamePhase AEng_GameModeArchitect::GetCurrentGamePhase() const
{
    return CurrentGamePhase;
}

void AEng_GameModeArchitect::UpdatePerformanceMetrics()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update every second
    if (CurrentTime - LastPerformanceUpdateTime >= 1.0f)
    {
        // Get frame rate
        CurrentPerformanceMetrics.FrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
        
        // Get actor count
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        CurrentPerformanceMetrics.ActiveActorCount = AllActors.Num();
        
        // Update memory usage (basic estimation)
        CurrentPerformanceMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
        
        LastPerformanceUpdateTime = CurrentTime;
    }
}

FEng_PerformanceMetrics AEng_GameModeArchitect::GetCurrentPerformanceMetrics() const
{
    return CurrentPerformanceMetrics;
}

void AEng_GameModeArchitect::LogArchitectureStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("=== GameModeArchitect Status ==="));
    UE_LOG(LogTemp, Log, TEXT("Architecture Initialized: %s"), bArchitectureInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("World Systems Ready: %s"), bWorldSystemsReady ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Player Systems Ready: %s"), bPlayerSystemsReady ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Current Game Phase: %d"), (int32)CurrentGamePhase);
    UE_LOG(LogTemp, Log, TEXT("Registered Subsystems: %d"), RegisteredSubsystems.Num());
    UE_LOG(LogTemp, Log, TEXT("System Integrity Checks: %d"), SystemIntegrityCheckCount);
    UE_LOG(LogTemp, Log, TEXT("Validation Errors: %d"), SystemValidationErrors.Num());
}

void AEng_GameModeArchitect::ValidateSubsystemDependencies()
{
    // Check that critical subsystems are available
    for (const auto& SubsystemPair : RegisteredSubsystems)
    {
        if (!IsValid(SubsystemPair.Value))
        {
            SystemValidationErrors.Add(FString::Printf(TEXT("Subsystem '%s' is invalid"), *SubsystemPair.Key));
        }
    }
}

void AEng_GameModeArchitect::InitializeDefaultPlayerClass()
{
    if (DefaultPawnClass)
    {
        UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Default pawn class set to: %s"), *DefaultPawnClass->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameModeArchitect: No default pawn class set"));
        SystemValidationErrors.Add(TEXT("No default pawn class configured"));
    }
}

void AEng_GameModeArchitect::SetupWorldPartitionSettings()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Setting up World Partition settings"));
    // World Partition setup would go here in a real implementation
}

void AEng_GameModeArchitect::ConfigurePhysicsSettings()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Configuring physics settings"));
    // Physics configuration would go here
}

void AEng_GameModeArchitect::InitializeAudioSystems()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Initializing audio systems"));
    // Audio system initialization would go here
}

void AEng_GameModeArchitect::SetupRenderingPipeline()
{
    UE_LOG(LogTemp, Log, TEXT("GameModeArchitect: Setting up rendering pipeline"));
    // Rendering pipeline setup would go here
}