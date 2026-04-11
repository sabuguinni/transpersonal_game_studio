#include "GameDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UGameDirector::UGameDirector()
    : bIsGameInitialized(false)
    , CurrentSaveSlot(TEXT("DefaultSlot"))
{
    // Constructor
}

void UGameDirector::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Initializing Studio Director System"));
    
    SessionStartTime = FDateTime::Now();
    InitializeCoreSystemsChain();
    SetupPerformanceMonitoring();
    
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Studio Director System Ready"));
}

void UGameDirector::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Shutting down Studio Director System"));
    
    RegisteredSystems.Empty();
    bIsGameInitialized = false;
    
    Super::Deinitialize();
}

void UGameDirector::InitializeGameSession()
{
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Initializing new game session"));
    
    // Validate all required systems are registered
    ValidateSystemDependencies();
    
    // Initialize game state
    bIsGameInitialized = true;
    SessionStartTime = FDateTime::Now();
    
    // Log session start
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Game session initialized at %s"), 
           *SessionStartTime.ToString());
}

void UGameDirector::StartNewGame()
{
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Starting new game"));
    
    if (!bIsGameInitialized)
    {
        InitializeGameSession();
    }
    
    // Reset all systems to initial state
    for (auto& SystemPair : RegisteredSystems)
    {
        UE_LOG(LogTemp, Log, TEXT("GameDirector: Resetting system %s"), *SystemPair.Key);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: New game started successfully"));
}

void UGameDirector::LoadGame(const FString& SaveSlot)
{
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Loading game from slot %s"), *SaveSlot);
    
    CurrentSaveSlot = SaveSlot;
    
    // TODO: Implement save/load system
    // For now, just initialize a new session
    InitializeGameSession();
    
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Game loaded from slot %s"), *SaveSlot);
}

void UGameDirector::SaveGame(const FString& SaveSlot)
{
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Saving game to slot %s"), *SaveSlot);
    
    CurrentSaveSlot = SaveSlot;
    
    // TODO: Implement save system
    // For now, just log the save attempt
    
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Game saved to slot %s"), *SaveSlot);
}

void UGameDirector::RegisterGameSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (!SystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("GameDirector: Cannot register null system %s"), *SystemName);
        return;
    }
    
    RegisteredSystems.Add(SystemName, SystemInstance);
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Registered system %s"), *SystemName);
}

UObject* UGameDirector::GetGameSystem(const FString& SystemName)
{
    if (TObjectPtr<UObject>* FoundSystem = RegisteredSystems.Find(SystemName))
    {
        return *FoundSystem;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: System %s not found"), *SystemName);
    return nullptr;
}

float UGameDirector::GetCurrentFPS()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GEngine->GetGameViewport()->GetWorld()->GetDeltaSeconds();
    }
    return 0.0f;
}

int32 UGameDirector::GetActiveActorCount()
{
    if (UWorld* World = GetWorld())
    {
        return World->GetActorCount();
    }
    return 0;
}

void UGameDirector::RunDiagnostics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR DIAGNOSTICS ==="));
    
    // System status
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    for (const auto& SystemPair : RegisteredSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - %s: %s"), 
               *SystemPair.Key, 
               SystemPair.Value ? TEXT("Active") : TEXT("Inactive"));
    }
    
    // Performance metrics
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.2f"), GetCurrentFPS());
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), GetActiveActorCount());
    
    // Session info
    FTimespan SessionDuration = FDateTime::Now() - SessionStartTime;
    UE_LOG(LogTemp, Warning, TEXT("Session Duration: %s"), *SessionDuration.ToString());
    
    UE_LOG(LogTemp, Warning, TEXT("Game Initialized: %s"), bIsGameInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Current Save Slot: %s"), *CurrentSaveSlot);
    
    UE_LOG(LogTemp, Warning, TEXT("=== DIAGNOSTICS COMPLETE ==="));
}

void UGameDirector::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Current system status"));
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        UE_LOG(LogTemp, Log, TEXT("  System %s: %s"), 
               *SystemPair.Key,
               SystemPair.Value ? TEXT("Running") : TEXT("Stopped"));
    }
}

void UGameDirector::InitializeCoreSystemsChain()
{
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Initializing core systems chain"));
    
    // This will be expanded as other agents create their systems
    // For now, just register the director itself
    RegisterGameSystem(TEXT("GameDirector"), this);
}

void UGameDirector::ValidateSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Validating system dependencies"));
    
    // Check that critical systems are registered
    TArray<FString> RequiredSystems = {
        TEXT("GameDirector")
        // More systems will be added as agents create them
    };
    
    for (const FString& SystemName : RequiredSystems)
    {
        if (!RegisteredSystems.Contains(SystemName))
        {
            UE_LOG(LogTemp, Error, TEXT("GameDirector: Missing required system %s"), *SystemName);
        }
    }
}

void UGameDirector::SetupPerformanceMonitoring()
{
    UE_LOG(LogTemp, Warning, TEXT("GameDirector: Setting up performance monitoring"));
    
    // TODO: Implement performance monitoring system
    // This will track FPS, memory usage, actor counts, etc.
}