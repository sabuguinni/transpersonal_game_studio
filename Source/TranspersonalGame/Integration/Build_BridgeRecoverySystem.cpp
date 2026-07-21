#include "Build_BridgeRecoverySystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"
#include "EditorLevelLibrary.h"
#include "EditorLoadingAndSavingUtils.h"

UBuild_BridgeRecoverySystem::UBuild_BridgeRecoverySystem()
{
    CurrentHealthMetrics = FBuild_SystemHealthMetrics();
    LastHealthCheckTime = 0.0f;
    RecoveryAttempts = 0;
    bAutoRecoveryEnabled = true;
}

void UBuild_BridgeRecoverySystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_BridgeRecoverySystem: Initializing bridge recovery system"));
    
    // Start health monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            HealthCheckTimer,
            FTimerDelegate::CreateUObject(this, &UBuild_BridgeRecoverySystem::UpdateHealthMetrics),
            HEALTH_CHECK_INTERVAL,
            true
        );
    }
    
    // Initial health check
    UpdateHealthMetrics();
    LogSystemStatus();
}

void UBuild_BridgeRecoverySystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(HealthCheckTimer);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_BridgeRecoverySystem: Shutting down bridge recovery system"));
    Super::Deinitialize();
}

FBuild_SystemHealthMetrics UBuild_BridgeRecoverySystem::GetSystemHealthMetrics() const
{
    return CurrentHealthMetrics;
}

bool UBuild_BridgeRecoverySystem::AttemptBridgeRecovery()
{
    if (RecoveryAttempts >= MAX_RECOVERY_ATTEMPTS)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_BridgeRecoverySystem: Max recovery attempts reached"));
        CurrentHealthMetrics.BridgeStatus = EBuild_BridgeStatus::Failed;
        return false;
    }
    
    RecoveryAttempts++;
    CurrentHealthMetrics.BridgeStatus = EBuild_BridgeStatus::Recovering;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_BridgeRecoverySystem: Attempting recovery %d/%d"), 
           RecoveryAttempts, MAX_RECOVERY_ATTEMPTS);
    
    // Step 1: Force map save to preserve current state
    bool bSaveSuccess = PerformEmergencyMapSave();
    
    // Step 2: Validate actor integrity
    bool bActorsValid = ValidateActorIntegrity();
    
    // Step 3: Update health metrics
    UpdateHealthMetrics();
    
    if (bSaveSuccess && bActorsValid && CurrentHealthMetrics.TotalActorCount > 0)
    {
        CurrentHealthMetrics.BridgeStatus = EBuild_BridgeStatus::Healthy;
        ResetRecoveryState();
        UE_LOG(LogTemp, Warning, TEXT("Build_BridgeRecoverySystem: Recovery successful"));
        return true;
    }
    else
    {
        CurrentHealthMetrics.BridgeStatus = EBuild_BridgeStatus::Degraded;
        UE_LOG(LogTemp, Error, TEXT("Build_BridgeRecoverySystem: Recovery failed"));
        return false;
    }
}

void UBuild_BridgeRecoverySystem::ForceMapSave()
{
    PerformEmergencyMapSave();
}

bool UBuild_BridgeRecoverySystem::ValidateActorIntegrity()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        CurrentHealthMetrics.LastErrorMessage = TEXT("No valid world found");
        return false;
    }
    
    int32 ValidActorCount = 0;
    int32 DinosaurActorCount = 0;
    int32 EnvironmentActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            ValidActorCount++;
            
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Rex")) || ActorName.Contains(TEXT("Raptor")) || 
                ActorName.Contains(TEXT("Dino")) || ActorName.Contains(TEXT("Saur")))
            {
                DinosaurActorCount++;
            }
            else if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) || 
                     ActorName.Contains(TEXT("Foliage")) || ActorName.Contains(TEXT("Environment")))
            {
                EnvironmentActorCount++;
            }
        }
    }
    
    CurrentHealthMetrics.TotalActorCount = ValidActorCount;
    CurrentHealthMetrics.DinosaurCount = DinosaurActorCount;
    CurrentHealthMetrics.EnvironmentActorCount = EnvironmentActorCount;
    
    bool bIntegrityValid = ValidActorCount > 10; // Minimum threshold
    
    if (!bIntegrityValid)
    {
        CurrentHealthMetrics.LastErrorMessage = FString::Printf(
            TEXT("Low actor count: %d (expected > 10)"), ValidActorCount);
    }
    
    return bIntegrityValid;
}

void UBuild_BridgeRecoverySystem::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD BRIDGE RECOVERY SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Bridge Status: %s"), 
           *UEnum::GetValueAsString(CurrentHealthMetrics.BridgeStatus));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentHealthMetrics.TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), CurrentHealthMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Environment Actors: %d"), CurrentHealthMetrics.EnvironmentActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Recovery Attempts: %d/%d"), RecoveryAttempts, MAX_RECOVERY_ATTEMPTS);
    
    if (!CurrentHealthMetrics.LastErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Last Error: %s"), *CurrentHealthMetrics.LastErrorMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END STATUS ==="));
}

void UBuild_BridgeRecoverySystem::UpdateHealthMetrics()
{
    float StartTime = FPlatformTime::Seconds();
    
    CheckBridgeConnectivity();
    ValidateActorIntegrity();
    
    CurrentHealthMetrics.LastResponseTime = FPlatformTime::Seconds() - StartTime;
    LastHealthCheckTime = FPlatformTime::Seconds();
    
    // Auto-recovery if enabled and bridge is degraded
    if (bAutoRecoveryEnabled && 
        CurrentHealthMetrics.BridgeStatus == EBuild_BridgeStatus::Degraded &&
        RecoveryAttempts < MAX_RECOVERY_ATTEMPTS)
    {
        AttemptBridgeRecovery();
    }
}

void UBuild_BridgeRecoverySystem::CheckBridgeConnectivity()
{
    // Simple connectivity check - if we can access the world and actors, bridge is working
    UWorld* World = GetWorld();
    if (!World)
    {
        CurrentHealthMetrics.BridgeStatus = EBuild_BridgeStatus::Failed;
        CurrentHealthMetrics.LastErrorMessage = TEXT("No world access");
        return;
    }
    
    // If we have actors and can iterate them, bridge is functional
    if (CurrentHealthMetrics.TotalActorCount > 0)
    {
        CurrentHealthMetrics.BridgeStatus = EBuild_BridgeStatus::Healthy;
        CurrentHealthMetrics.LastErrorMessage = TEXT("");
    }
    else
    {
        CurrentHealthMetrics.BridgeStatus = EBuild_BridgeStatus::Degraded;
        CurrentHealthMetrics.LastErrorMessage = TEXT("No actors found in world");
    }
}

bool UBuild_BridgeRecoverySystem::PerformEmergencyMapSave()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_BridgeRecoverySystem: Cannot save - no world"));
        return false;
    }
    
    try
    {
        // Use the safe map saving method from brain memories
        FString MapPath = TEXT("/Game/Maps/MinPlayableMap");
        bool bSaveResult = UEditorLoadingAndSavingUtils::SaveMap(World, MapPath);
        
        if (bSaveResult)
        {
            UE_LOG(LogTemp, Warning, TEXT("Build_BridgeRecoverySystem: Emergency map save successful"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Build_BridgeRecoverySystem: Emergency map save failed"));
        }
        
        return bSaveResult;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_BridgeRecoverySystem: Exception during emergency save"));
        return false;
    }
}

void UBuild_BridgeRecoverySystem::ResetRecoveryState()
{
    RecoveryAttempts = 0;
    CurrentHealthMetrics.LastErrorMessage = TEXT("");
}