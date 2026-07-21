#include "Eng_GameplayFoundation.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

// UEng_GameplayFoundation Implementation

void UEng_GameplayFoundation::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initializing Gameplay Foundation System"));
    
    // Initialize default configurations
    PlayerMovementConfig.WalkSpeed = 300.0f;
    PlayerMovementConfig.RunSpeed = 600.0f;
    PlayerMovementConfig.JumpHeight = 420.0f;
    PlayerMovementConfig.StaminaDrainRate = 10.0f;
    PlayerMovementConfig.StaminaRegenRate = 15.0f;
    
    WorldInteractionConfig.InteractionRange = 200.0f;
    WorldInteractionConfig.GatheringTime = 2.0f;
    WorldInteractionConfig.CraftingSpeedMultiplier = 1.0f;
    WorldInteractionConfig.MaxInventorySlots = 30;
    
    // Reset metrics
    CurrentMetrics.ActivePlayers = 0;
    CurrentMetrics.ActiveDinosaurs = 0;
    CurrentMetrics.AverageFrameTime = 0.0f;
    CurrentMetrics.WorldLoadProgress = 0.0f;
    CurrentMetrics.bIsGameplayReady = false;
    
    bIsInitialized = true;
    InitializationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Gameplay Foundation System initialized successfully"));
}

void UEng_GameplayFoundation::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Deinitializing Gameplay Foundation System"));
    
    bIsInitialized = false;
    CurrentMetrics.bIsGameplayReady = false;
    
    Super::Deinitialize();
}

bool UEng_GameplayFoundation::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UEng_GameplayFoundation::InitializeGameplaySystems()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Cannot initialize gameplay systems - foundation not initialized"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Cannot initialize gameplay systems - no world"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initializing core gameplay systems"));
    
    // Update initial metrics
    UpdateGameplayMetrics();
    
    // Mark systems as ready
    CurrentMetrics.bIsGameplayReady = true;
    CurrentMetrics.WorldLoadProgress = 1.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Core gameplay systems initialized"));
}

void UEng_GameplayFoundation::UpdateGameplayMetrics()
{
    if (!bIsInitialized)
    {
        return;
    }
    
    UpdatePlayerMetrics();
    UpdateWorldMetrics();
    UpdatePerformanceMetrics();
}

bool UEng_GameplayFoundation::IsGameplaySystemReady() const
{
    return bIsInitialized && CurrentMetrics.bIsGameplayReady;
}

void UEng_GameplayFoundation::SetPlayerMovementConfig(const FEng_PlayerMovementConfig& Config)
{
    PlayerMovementConfig = Config;
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Player movement config updated - Walk: %f, Run: %f"), 
           Config.WalkSpeed, Config.RunSpeed);
}

FEng_PlayerMovementConfig UEng_GameplayFoundation::GetPlayerMovementConfig() const
{
    return PlayerMovementConfig;
}

void UEng_GameplayFoundation::SetWorldInteractionConfig(const FEng_WorldInteractionConfig& Config)
{
    WorldInteractionConfig = Config;
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: World interaction config updated - Range: %f, Slots: %d"), 
           Config.InteractionRange, Config.MaxInventorySlots);
}

FEng_WorldInteractionConfig UEng_GameplayFoundation::GetWorldInteractionConfig() const
{
    return WorldInteractionConfig;
}

FEng_GameplayMetrics UEng_GameplayFoundation::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UEng_GameplayFoundation::LogGameplayStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== Engine Architect: Gameplay Foundation Status ==="));
    UE_LOG(LogTemp, Warning, TEXT("Initialized: %s"), bIsInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Gameplay Ready: %s"), CurrentMetrics.bIsGameplayReady ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Active Players: %d"), CurrentMetrics.ActivePlayers);
    UE_LOG(LogTemp, Warning, TEXT("Active Dinosaurs: %d"), CurrentMetrics.ActiveDinosaurs);
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Time: %f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("World Load Progress: %f%%"), CurrentMetrics.WorldLoadProgress * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("=== End Status ==="));
}

void UEng_GameplayFoundation::ValidateGameplayFoundation()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Validating Gameplay Foundation"));
    
    bool bValidationPassed = true;
    
    // Check initialization
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Validation FAILED: Foundation not initialized"));
        bValidationPassed = false;
    }
    
    // Check world reference
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Validation FAILED: No world reference"));
        bValidationPassed = false;
    }
    
    // Check configuration validity
    if (PlayerMovementConfig.WalkSpeed <= 0.0f || PlayerMovementConfig.RunSpeed <= PlayerMovementConfig.WalkSpeed)
    {
        UE_LOG(LogTemp, Error, TEXT("Validation FAILED: Invalid movement configuration"));
        bValidationPassed = false;
    }
    
    // Check system integrity
    if (!ValidateSystemIntegrity())
    {
        UE_LOG(LogTemp, Error, TEXT("Validation FAILED: System integrity check failed"));
        bValidationPassed = false;
    }
    
    if (bValidationPassed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Gameplay Foundation validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Gameplay Foundation validation FAILED"));
    }
}

void UEng_GameplayFoundation::UpdatePlayerMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count active players
    TArray<AActor*> PlayerControllers;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerController::StaticClass(), PlayerControllers);
    CurrentMetrics.ActivePlayers = PlayerControllers.Num();
}

void UEng_GameplayFoundation::UpdateWorldMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count dinosaurs (actors with "Dinosaur" in their name)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            DinosaurCount++;
        }
    }
    CurrentMetrics.ActiveDinosaurs = DinosaurCount;
}

void UEng_GameplayFoundation::UpdatePerformanceMetrics()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        // Get frame time from engine
        CurrentMetrics.AverageFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    }
}

bool UEng_GameplayFoundation::ValidateSystemIntegrity() const
{
    // Basic integrity checks
    if (PlayerMovementConfig.WalkSpeed <= 0.0f)
    {
        return false;
    }
    
    if (WorldInteractionConfig.InteractionRange <= 0.0f)
    {
        return false;
    }
    
    if (WorldInteractionConfig.MaxInventorySlots <= 0)
    {
        return false;
    }
    
    return true;
}

// UEng_GameplayFoundationComponent Implementation

UEng_GameplayFoundationComponent::UEng_GameplayFoundationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second
    
    bAutoInitialize = true;
    bEnableMetricsLogging = true;
    MetricsUpdateInterval = 1.0f;
}

void UEng_GameplayFoundationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Gameplay Foundation Component starting"));
    
    FoundationStartupTime = GetWorld()->GetTimeSeconds();
    
    if (bAutoInitialize)
    {
        InitializeFoundationSystems();
    }
}

void UEng_GameplayFoundationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ShutdownFoundationSystems();
    Super::EndPlay(EndPlayReason);
}

void UEng_GameplayFoundationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bFoundationInitialized && bEnableMetricsLogging)
    {
        LastMetricsUpdate += DeltaTime;
        if (LastMetricsUpdate >= MetricsUpdateInterval)
        {
            UpdateFoundationMetrics();
            LastMetricsUpdate = 0.0f;
        }
    }
}

void UEng_GameplayFoundationComponent::InitializeFoundationSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Cannot initialize foundation - no world"));
        return;
    }
    
    UEng_GameplayFoundation* FoundationSubsystem = World->GetSubsystem<UEng_GameplayFoundation>();
    if (FoundationSubsystem)
    {
        FoundationSubsystem->InitializeGameplaySystems();
        bFoundationInitialized = true;
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Foundation systems initialized via component"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Could not find Gameplay Foundation subsystem"));
    }
}

void UEng_GameplayFoundationComponent::ShutdownFoundationSystems()
{
    if (bFoundationInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Shutting down foundation systems"));
        bFoundationInitialized = false;
    }
}

bool UEng_GameplayFoundationComponent::AreFoundationSystemsReady() const
{
    if (!bFoundationInitialized)
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    UEng_GameplayFoundation* FoundationSubsystem = World->GetSubsystem<UEng_GameplayFoundation>();
    return FoundationSubsystem && FoundationSubsystem->IsGameplaySystemReady();
}

void UEng_GameplayFoundationComponent::ApplyDefaultConfiguration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UEng_GameplayFoundation* FoundationSubsystem = World->GetSubsystem<UEng_GameplayFoundation>();
    if (FoundationSubsystem)
    {
        // Apply default movement config
        FEng_PlayerMovementConfig DefaultMovement;
        DefaultMovement.WalkSpeed = 300.0f;
        DefaultMovement.RunSpeed = 600.0f;
        DefaultMovement.JumpHeight = 420.0f;
        DefaultMovement.StaminaDrainRate = 10.0f;
        DefaultMovement.StaminaRegenRate = 15.0f;
        FoundationSubsystem->SetPlayerMovementConfig(DefaultMovement);
        
        // Apply default interaction config
        FEng_WorldInteractionConfig DefaultInteraction;
        DefaultInteraction.InteractionRange = 200.0f;
        DefaultInteraction.GatheringTime = 2.0f;
        DefaultInteraction.CraftingSpeedMultiplier = 1.0f;
        DefaultInteraction.MaxInventorySlots = 30;
        FoundationSubsystem->SetWorldInteractionConfig(DefaultInteraction);
        
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Default configuration applied"));
    }
}

void UEng_GameplayFoundationComponent::SaveConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Configuration save requested (placeholder)"));
    // TODO: Implement configuration saving to file/registry
}

void UEng_GameplayFoundationComponent::LoadConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Configuration load requested (placeholder)"));
    // TODO: Implement configuration loading from file/registry
}

void UEng_GameplayFoundationComponent::UpdateFoundationMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UEng_GameplayFoundation* FoundationSubsystem = World->GetSubsystem<UEng_GameplayFoundation>();
    if (FoundationSubsystem)
    {
        FoundationSubsystem->UpdateGameplayMetrics();
        
        if (bEnableMetricsLogging)
        {
            FEng_GameplayMetrics Metrics = FoundationSubsystem->GetCurrentMetrics();
            UE_LOG(LogTemp, Log, TEXT("Engine Architect Metrics - Players: %d, Dinosaurs: %d, Frame: %f ms"), 
                   Metrics.ActivePlayers, Metrics.ActiveDinosaurs, Metrics.AverageFrameTime);
        }
    }
}