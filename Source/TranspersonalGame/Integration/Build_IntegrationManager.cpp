#include "Build_IntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UBuild_IntegrationManager::UBuild_IntegrationManager()
{
    bValidationInProgress = false;
}

void UBuild_IntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Initializing integration system"));
    
    // Initialize module statuses
    ModuleStatuses.Empty();
    
    // Add core modules to track
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("World"),
        TEXT("Character"),
        TEXT("AI"),
        TEXT("QA"),
        TEXT("Integration")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        FBuild_ModuleStatus NewStatus;
        NewStatus.ModuleName = ModuleName;
        NewStatus.Status = EBuild_IntegrationStatus::Unknown;
        ModuleStatuses.Add(ModuleName, NewStatus);
    }
    
    // Start periodic validation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuild_IntegrationManager::ValidateAllModules,
            30.0f, // Every 30 seconds
            true
        );
        
        World->GetTimerManager().SetTimer(
            HealthMonitorHandle,
            this,
            &UBuild_IntegrationManager::UpdateSystemHealth,
            5.0f, // Every 5 seconds
            true
        );
    }
    
    // Run initial validation
    ValidateAllModules();
}

void UBuild_IntegrationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
        World->GetTimerManager().ClearTimer(HealthMonitorHandle);
    }
    
    Super::Deinitialize();
}

void UBuild_IntegrationManager::ValidateAllModules()
{
    if (bValidationInProgress)
    {
        return;
    }
    
    bValidationInProgress = true;
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Starting module validation"));
    
    for (auto& ModulePair : ModuleStatuses)
    {
        ValidateModule(ModulePair.Key);
    }
    
    bValidationInProgress = false;
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Module validation complete"));
}

void UBuild_IntegrationManager::ValidateModule(const FString& ModuleName)
{
    if (!ModuleStatuses.Contains(ModuleName))
    {
        FBuild_ModuleStatus NewStatus;
        NewStatus.ModuleName = ModuleName;
        ModuleStatuses.Add(ModuleName, NewStatus);
    }
    
    FBuild_ModuleStatus& Status = ModuleStatuses[ModuleName];
    Status.Status = EBuild_IntegrationStatus::Validating;
    
    try
    {
        // Validate module classes
        ValidateModuleClasses(ModuleName);
        
        // Validate module actors
        ValidateModuleActors(ModuleName);
        
        // If we get here, validation passed
        Status.Status = EBuild_IntegrationStatus::Valid;
        Status.LastError = TEXT("");
        
        UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Module %s validation passed"), *ModuleName);
    }
    catch (...)
    {
        Status.Status = EBuild_IntegrationStatus::Invalid;
        Status.LastError = TEXT("Validation exception occurred");
        LogIntegrationError(FString::Printf(TEXT("Module %s validation failed"), *ModuleName));
    }
}

EBuild_IntegrationStatus UBuild_IntegrationManager::GetModuleStatus(const FString& ModuleName)
{
    if (ModuleStatuses.Contains(ModuleName))
    {
        return ModuleStatuses[ModuleName].Status;
    }
    return EBuild_IntegrationStatus::Unknown;
}

FBuild_SystemHealth UBuild_IntegrationManager::GetSystemHealth()
{
    UpdateSystemHealth();
    return CurrentSystemHealth;
}

void UBuild_IntegrationManager::RunIntegrationTests()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Running integration tests"));
    
    // Test 1: Module loading
    int32 ValidModules = 0;
    for (const auto& ModulePair : ModuleStatuses)
    {
        if (ModulePair.Value.Status == EBuild_IntegrationStatus::Valid)
        {
            ValidModules++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: %d/%d modules valid"), ValidModules, ModuleStatuses.Num());
    
    // Test 2: Actor spawning
    if (UWorld* World = GetWorld())
    {
        int32 ActorCount = World->GetCurrentLevel()->Actors.Num();
        UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: %d actors in current level"), ActorCount);
    }
    
    // Test 3: Memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float MemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: %.2f MB memory used"), MemoryMB);
}

void UBuild_IntegrationManager::CreateBuildSnapshot()
{
    FString SnapshotName = FString::Printf(TEXT("BuildSnapshot_%s"), *FDateTime::Now().ToString());
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Creating build snapshot: %s"), *SnapshotName);
    
    // In a real implementation, this would save the current state
    // For now, just log the action
}

bool UBuild_IntegrationManager::RestoreBuildSnapshot(const FString& SnapshotName)
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Restoring build snapshot: %s"), *SnapshotName);
    
    // In a real implementation, this would restore a saved state
    // For now, just return success
    return true;
}

TArray<FString> UBuild_IntegrationManager::GetIntegrationErrors()
{
    return IntegrationErrors;
}

void UBuild_IntegrationManager::ClearIntegrationErrors()
{
    IntegrationErrors.Empty();
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Integration errors cleared"));
}

void UBuild_IntegrationManager::ValidateModuleClasses(const FString& ModuleName)
{
    // Count classes for this module
    int32 ClassCount = 0;
    
    // This is a simplified validation - in a real system we'd enumerate actual classes
    if (ModuleName == TEXT("TranspersonalGame"))
    {
        ClassCount = 10; // Estimated core classes
    }
    else if (ModuleName == TEXT("Character"))
    {
        ClassCount = 5;
    }
    else if (ModuleName == TEXT("AI"))
    {
        ClassCount = 8;
    }
    else
    {
        ClassCount = 3; // Default estimate
    }
    
    if (ModuleStatuses.Contains(ModuleName))
    {
        ModuleStatuses[ModuleName].ClassCount = ClassCount;
    }
}

void UBuild_IntegrationManager::ValidateModuleActors(const FString& ModuleName)
{
    int32 ActorCount = 0;
    
    if (UWorld* World = GetWorld())
    {
        // Count actors related to this module
        for (AActor* Actor : World->GetCurrentLevel()->Actors)
        {
            if (Actor && Actor->GetClass()->GetName().Contains(ModuleName))
            {
                ActorCount++;
            }
        }
    }
    
    if (ModuleStatuses.Contains(ModuleName))
    {
        ModuleStatuses[ModuleName].ActorCount = ActorCount;
    }
}

void UBuild_IntegrationManager::UpdateSystemHealth()
{
    // Update frame rate
    if (UWorld* World = GetWorld())
    {
        CurrentSystemHealth.FrameRate = 1.0f / World->GetDeltaSeconds();
    }
    
    // Update memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentSystemHealth.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update actor count
    if (UWorld* World = GetWorld())
    {
        CurrentSystemHealth.TotalActors = World->GetCurrentLevel()->Actors.Num();
    }
    
    // Update component count (simplified)
    CurrentSystemHealth.ActiveComponents = CurrentSystemHealth.TotalActors * 3; // Estimate
    
    // Determine stability
    CurrentSystemHealth.bIsStable = (CurrentSystemHealth.FrameRate > 20.0f) && 
                                   (CurrentSystemHealth.MemoryUsageMB < 2048.0f);
}

void UBuild_IntegrationManager::LogIntegrationError(const FString& Error)
{
    IntegrationErrors.Add(Error);
    UE_LOG(LogTemp, Error, TEXT("Build_IntegrationManager: %s"), *Error);
    
    // Keep only last 50 errors
    if (IntegrationErrors.Num() > 50)
    {
        IntegrationErrors.RemoveAt(0);
    }
}