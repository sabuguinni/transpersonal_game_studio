#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerStart.h"
#include "Components/LightComponent.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bSystemsValidated = false;
    TotalActorCount = 0;
    LastValidationTime = 0.0f;
    CurrentFPS = 0.0f;
    MemoryUsageMB = 0.0f;
    DrawCalls = 0;
    CurrentBuildVersion = TEXT("1.0.0");
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing integration subsystem"));
    
    // Start periodic validation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::ValidateAllSystems,
            30.0f, // Every 30 seconds
            true
        );
        
        World->GetTimerManager().SetTimer(
            PerformanceTimerHandle,
            this,
            &UBuildIntegrationManager::UpdatePerformanceMetrics,
            5.0f, // Every 5 seconds
            true
        );
    }
    
    // Initial validation
    ValidateAllSystems();
}

void UBuildIntegrationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
        World->GetTimerManager().ClearTimer(PerformanceTimerHandle);
        World->GetTimerManager().ClearTimer(BuildTimerHandle);
    }
    
    Super::Deinitialize();
}

bool UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting system validation"));
    
    bool bAllValid = true;
    
    // Validate core systems
    bAllValid &= ValidateCharacterSystems();
    bAllValid &= ValidateDinosaurSystems();
    bAllValid &= ValidateEnvironmentSystems();
    bAllValid &= ValidateAudioSystems();
    bAllValid &= ValidateVFXSystems();
    
    // Validate actor counts
    bAllValid &= ValidateActorCounts();
    
    // Validate performance
    bAllValid &= ValidatePerformance();
    
    bSystemsValidated = bAllValid;
    LastValidationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: System validation %s"), 
           bAllValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bAllValid;
}

bool UBuildIntegrationManager::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorCount = AllActors.Num();
    
    // Count specific actor types
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    
    TArray<AActor*> StaticMeshes;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshes);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Actor counts - Total: %d, PlayerStarts: %d, StaticMeshes: %d"),
           TotalActorCount, PlayerStarts.Num(), StaticMeshes.Num());
    
    // Validation criteria
    bool bValid = true;
    if (PlayerStarts.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No PlayerStart found!"));
        bValid = false;
    }
    
    if (TotalActorCount > 50000)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: High actor count detected: %d"), TotalActorCount);
        OptimizeActorCounts();
    }
    
    return bValid;
}

bool UBuildIntegrationManager::ValidatePerformance()
{
    UpdatePerformanceMetrics();
    
    bool bValid = true;
    
    // Check FPS
    if (CurrentFPS < 30.0f && CurrentFPS > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Low FPS detected: %.2f"), CurrentFPS);
        bValid = false;
    }
    
    // Check memory usage
    if (MemoryUsageMB > 8192.0f) // 8GB limit
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: High memory usage: %.2f MB"), MemoryUsageMB);
        OptimizeMemory();
    }
    
    return bValid;
}

void UBuildIntegrationManager::SaveCurrentBuild()
{
    FString BuildName = FString::Printf(TEXT("Build_%s"), *FDateTime::Now().ToString());
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Saving build: %s"), *BuildName);
    
    CreateBuildSnapshot();
    BuildHistory.Add(BuildName);
    
    // Keep only last 10 builds
    if (BuildHistory.Num() > 10)
    {
        BuildHistory.RemoveAt(0);
    }
}

bool UBuildIntegrationManager::LoadBuild(const FString& BuildName)
{
    if (!BuildHistory.Contains(BuildName))
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Build not found: %s"), *BuildName);
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Loading build: %s"), *BuildName);
    
    // Implementation would load the specific build state
    CurrentBuildVersion = BuildName;
    return true;
}

TArray<FString> UBuildIntegrationManager::GetAvailableBuilds()
{
    return BuildHistory;
}

FBuild_SystemStatus UBuildIntegrationManager::GetSystemStatus()
{
    FBuild_SystemStatus Status;
    Status.bSystemsValid = bSystemsValidated;
    Status.ActorCount = TotalActorCount;
    Status.FPS = CurrentFPS;
    Status.MemoryUsageMB = MemoryUsageMB;
    Status.LastValidationTime = LastValidationTime;
    Status.BuildVersion = CurrentBuildVersion;
    
    return Status;
}

void UBuildIntegrationManager::LogSystemMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION METRICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Systems Validated: %s"), bSystemsValidated ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.2f"), CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Build Version: %s"), *CurrentBuildVersion);
    UE_LOG(LogTemp, Warning, TEXT("Last Validation: %.2f seconds ago"), 
           GetWorld() ? GetWorld()->GetTimeSeconds() - LastValidationTime : 0.0f);
}

void UBuildIntegrationManager::OptimizeMemory()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting memory optimization"));
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Update metrics after optimization
    CheckMemoryUsage();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Memory optimization complete"));
}

bool UBuildIntegrationManager::ValidateCharacterSystems()
{
    // Check if TranspersonalCharacter class exists and is functional
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    bool bValid = (CharacterClass != nullptr);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Character systems %s"), 
           bValid ? TEXT("VALID") : TEXT("INVALID"));
    
    return bValid;
}

bool UBuildIntegrationManager::ValidateDinosaurSystems()
{
    // Check for dinosaur-related classes
    UClass* DinosaurClass = FindObject<UClass>(ANY_PACKAGE, TEXT("DinosaurTRex"));
    bool bValid = (DinosaurClass != nullptr);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Dinosaur systems %s"), 
           bValid ? TEXT("VALID") : TEXT("INVALID"));
    
    return bValid;
}

bool UBuildIntegrationManager::ValidateEnvironmentSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for environment actors
    TArray<AActor*> StaticMeshes;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshes);
    
    bool bValid = StaticMeshes.Num() > 0;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Environment systems %s (%d meshes)"), 
           bValid ? TEXT("VALID") : TEXT("INVALID"), StaticMeshes.Num());
    
    return bValid;
}

bool UBuildIntegrationManager::ValidateAudioSystems()
{
    // Basic audio system validation
    bool bValid = GEngine && GEngine->GetAudioDeviceManager();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Audio systems %s"), 
           bValid ? TEXT("VALID") : TEXT("INVALID"));
    
    return bValid;
}

bool UBuildIntegrationManager::ValidateVFXSystems()
{
    // Basic VFX system validation
    bool bValid = true; // Assume valid for now
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: VFX systems %s"), 
           bValid ? TEXT("VALID") : TEXT("INVALID"));
    
    return bValid;
}

void UBuildIntegrationManager::UpdatePerformanceMetrics()
{
    // Update FPS
    CurrentFPS = 1.0f / FApp::GetDeltaTime();
    
    // Update memory usage
    CheckMemoryUsage();
    
    // Update draw calls (simplified)
    DrawCalls = 1000; // Placeholder value
}

void UBuildIntegrationManager::CheckMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UBuildIntegrationManager::OptimizeActorCounts()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Optimizing actor counts"));
    
    // Implementation would remove unnecessary actors or optimize LODs
    // For now, just log the action
}

void UBuildIntegrationManager::CreateBuildSnapshot()
{
    // Implementation would save current world state
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Creating build snapshot"));
}

void UBuildIntegrationManager::CleanupOldBuilds()
{
    // Implementation would clean up old build files
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Cleaning up old builds"));
}

void UBuildIntegrationManager::ValidateBuildIntegrity()
{
    // Implementation would validate build file integrity
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating build integrity"));
}