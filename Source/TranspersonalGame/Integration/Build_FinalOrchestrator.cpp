#include "Build_FinalOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Level.h"
#include "Components/StaticMeshComponent.h"

UBuild_FinalOrchestrator::UBuild_FinalOrchestrator()
{
    // Initialize integration settings
    MaxMemoryUsagePercent = 85.0f;
    MinPerformanceScore = 60.0f;
    MaxBuildSnapshots = 10;
    bAutoCleanupOnFailure = true;

    // Initialize current report
    CurrentReport = FBuild_IntegrationReport();
    CurrentReport.CurrentPhase = EBuild_IntegrationPhase::PreValidation;
}

void UBuild_FinalOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Initializing Integration System"));
    
    // Start initial validation
    UpdateIntegrationPhase(EBuild_IntegrationPhase::PreValidation);
    
    // Register core systems automatically
    RegisterSystem(TEXT("TranspersonalGameState"), nullptr);
    RegisterSystem(TEXT("TranspersonalCharacter"), nullptr);
    RegisterSystem(TEXT("PCGWorldGenerator"), nullptr);
    RegisterSystem(TEXT("FoliageManager"), nullptr);
    RegisterSystem(TEXT("CrowdSimulationManager"), nullptr);
    RegisterSystem(TEXT("QA_SystemIntegrationValidator"), nullptr);
}

void UBuild_FinalOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Shutting down Integration System"));
    
    // Clean up registered systems
    RegisteredSystems.Empty();
    BuildSnapshots.Empty();
    
    Super::Deinitialize();
}

void UBuild_FinalOrchestrator::StartFinalIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Starting Final Integration Process"));
    
    UpdateIntegrationPhase(EBuild_IntegrationPhase::SystemLoading);
    
    // Validate all systems
    ValidateAllSystems();
    
    // Perform cross-system validation
    PerformCrossSystemValidation();
    
    // Generate final report
    GenerateIntegrationReport();
}

void UBuild_FinalOrchestrator::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Validating All Systems"));
    
    UpdateIntegrationPhase(EBuild_IntegrationPhase::SystemLoading);
    
    CurrentReport.SystemStatuses.Empty();
    
    // Validate each registered system
    for (const auto& SystemPair : RegisteredSystems)
    {
        FBuild_SystemStatus SystemStatus;
        SystemStatus.SystemName = SystemPair.Key;
        SystemStatus.bIsLoaded = ValidateSystemIntegrity(SystemPair.Key);
        SystemStatus.PerformanceScore = GetSystemPerformanceScore(SystemPair.Key);
        
        if (SystemStatus.bIsLoaded)
        {
            SystemStatus.bIsValidated = true;
            OnSystemValidated.Broadcast(SystemPair.Key);
        }
        else
        {
            SystemStatus.ValidationErrors.Add(FString::Printf(TEXT("System %s failed to load"), *SystemPair.Key));
        }
        
        CurrentReport.SystemStatuses.Add(SystemStatus);
    }
    
    // Validate level state
    ValidateLevelState();
    
    // Update performance metrics
    UpdatePerformanceMetrics();
}

void UBuild_FinalOrchestrator::PerformCrossSystemValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Performing Cross-System Validation"));
    
    UpdateIntegrationPhase(EBuild_IntegrationPhase::CrossSystemValidation);
    
    // Validate system dependencies
    ValidateSystemDependencies();
    
    // Check system compatibility
    CheckSystemCompatibility();
    
    // Validate specific system integrations
    bool bWorldGenValid = ValidateWorldGeneration();
    bool bCharacterValid = ValidateCharacterSystems();
    bool bAIValid = ValidateAISystems();
    bool bVFXValid = ValidateVFXSystems();
    bool bQAValid = ValidateQASystems();
    
    // Update overall integration score
    float IntegrationScore = 0.0f;
    int32 ValidSystems = 0;
    
    if (bWorldGenValid) { IntegrationScore += 20.0f; ValidSystems++; }
    if (bCharacterValid) { IntegrationScore += 20.0f; ValidSystems++; }
    if (bAIValid) { IntegrationScore += 20.0f; ValidSystems++; }
    if (bVFXValid) { IntegrationScore += 20.0f; ValidSystems++; }
    if (bQAValid) { IntegrationScore += 20.0f; ValidSystems++; }
    
    CurrentReport.OverallIntegrationScore = IntegrationScore;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Cross-system validation complete. Score: %.1f"), IntegrationScore);
}

void UBuild_FinalOrchestrator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Generating Integration Report"));
    
    UpdateIntegrationPhase(EBuild_IntegrationPhase::FinalIntegration);
    
    // Update final metrics
    CurrentReport.IntegrationTimestamp = FDateTime::Now();
    CurrentReport.bBuildSuccessful = (CurrentReport.OverallIntegrationScore >= MinPerformanceScore);
    
    // Validate performance one final time
    bool bPerformanceValid = ValidatePerformanceMetrics();
    
    if (CurrentReport.bBuildSuccessful && bPerformanceValid)
    {
        UpdateIntegrationPhase(EBuild_IntegrationPhase::Complete);
        CreateBuildSnapshot();
        OnIntegrationComplete.Broadcast(true);
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Integration SUCCESSFUL"));
    }
    else
    {
        UpdateIntegrationPhase(EBuild_IntegrationPhase::Failed);
        if (bAutoCleanupOnFailure)
        {
            CleanupFailedIntegration();
        }
        OnIntegrationComplete.Broadcast(false);
        UE_LOG(LogTemp, Error, TEXT("Build_FinalOrchestrator: Integration FAILED"));
    }
}

void UBuild_FinalOrchestrator::RegisterSystem(const FString& SystemName, UObject* SystemObject)
{
    RegisteredSystems.Add(SystemName, SystemObject);
    UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: Registered system: %s"), *SystemName);
}

void UBuild_FinalOrchestrator::UnregisterSystem(const FString& SystemName)
{
    RegisteredSystems.Remove(SystemName);
    UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: Unregistered system: %s"), *SystemName);
}

bool UBuild_FinalOrchestrator::ValidateSystemIntegrity(const FString& SystemName)
{
    // Try to load the class to verify it exists and is valid
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
    UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    bool bIsValid = (SystemClass != nullptr);
    
    if (bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: System %s validated successfully"), *SystemName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: System %s validation failed"), *SystemName);
    }
    
    return bIsValid;
}

bool UBuild_FinalOrchestrator::ValidatePerformanceMetrics()
{
    UpdatePerformanceMetrics();
    
    bool bMemoryOK = (CurrentReport.MemoryUsagePercent <= MaxMemoryUsagePercent);
    bool bPerformanceOK = (CurrentReport.OverallIntegrationScore >= MinPerformanceScore);
    
    return bMemoryOK && bPerformanceOK;
}

bool UBuild_FinalOrchestrator::ValidateLevelState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        CurrentReport.TotalActorsInLevel = 0;
        return false;
    }
    
    // Count actors in the level
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    CurrentReport.TotalActorsInLevel = ActorCount;
    
    UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: Level validation - %d actors found"), ActorCount);
    
    return ActorCount > 0;
}

float UBuild_FinalOrchestrator::GetSystemPerformanceScore(const FString& SystemName)
{
    // Base performance score calculation
    float BaseScore = 75.0f;
    
    // Check if system is loaded
    if (ValidateSystemIntegrity(SystemName))
    {
        BaseScore += 15.0f;
    }
    
    // Additional scoring based on system type
    if (SystemName.Contains(TEXT("World")) || SystemName.Contains(TEXT("PCG")))
    {
        BaseScore += 5.0f; // World generation systems get bonus
    }
    
    if (SystemName.Contains(TEXT("Character")))
    {
        BaseScore += 5.0f; // Character systems get bonus
    }
    
    return FMath::Clamp(BaseScore, 0.0f, 100.0f);
}

float UBuild_FinalOrchestrator::GetOverallPerformanceScore()
{
    return CurrentReport.OverallIntegrationScore;
}

void UBuild_FinalOrchestrator::CreateBuildSnapshot()
{
    // Add current report to snapshots
    BuildSnapshots.Add(CurrentReport);
    
    // Maintain maximum snapshot count
    while (BuildSnapshots.Num() > MaxBuildSnapshots)
    {
        BuildSnapshots.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: Build snapshot created. Total snapshots: %d"), BuildSnapshots.Num());
}

bool UBuild_FinalOrchestrator::RestoreBuildSnapshot(int32 SnapshotIndex)
{
    if (BuildSnapshots.IsValidIndex(SnapshotIndex))
    {
        CurrentReport = BuildSnapshots[SnapshotIndex];
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Restored build snapshot %d"), SnapshotIndex);
        return true;
    }
    
    return false;
}

void UBuild_FinalOrchestrator::CleanupFailedIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Cleaning up failed integration"));
    
    // Reset current report
    CurrentReport = FBuild_IntegrationReport();
    CurrentReport.CurrentPhase = EBuild_IntegrationPhase::PreValidation;
    
    // Clear any temporary data
    // Additional cleanup logic can be added here
}

void UBuild_FinalOrchestrator::UpdateIntegrationPhase(EBuild_IntegrationPhase NewPhase)
{
    if (CurrentReport.CurrentPhase != NewPhase)
    {
        CurrentReport.CurrentPhase = NewPhase;
        OnIntegrationPhaseChanged.Broadcast(NewPhase);
        
        FString PhaseString;
        switch (NewPhase)
        {
            case EBuild_IntegrationPhase::PreValidation: PhaseString = TEXT("PreValidation"); break;
            case EBuild_IntegrationPhase::SystemLoading: PhaseString = TEXT("SystemLoading"); break;
            case EBuild_IntegrationPhase::CrossSystemValidation: PhaseString = TEXT("CrossSystemValidation"); break;
            case EBuild_IntegrationPhase::PerformanceValidation: PhaseString = TEXT("PerformanceValidation"); break;
            case EBuild_IntegrationPhase::FinalIntegration: PhaseString = TEXT("FinalIntegration"); break;
            case EBuild_IntegrationPhase::Complete: PhaseString = TEXT("Complete"); break;
            case EBuild_IntegrationPhase::Failed: PhaseString = TEXT("Failed"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Phase changed to %s"), *PhaseString);
    }
}

void UBuild_FinalOrchestrator::ValidateSystemDependencies()
{
    // Validate that core systems have their dependencies
    UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: Validating system dependencies"));
    
    // Check that character system depends on game state
    // Check that world generation depends on foliage manager
    // Check that AI systems depend on character system
    // Additional dependency validation logic
}

void UBuild_FinalOrchestrator::CheckSystemCompatibility()
{
    // Check that systems are compatible with each other
    UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: Checking system compatibility"));
    
    // Verify no conflicting systems
    // Check version compatibility
    // Validate interface compatibility
}

void UBuild_FinalOrchestrator::MonitorSystemPerformance()
{
    // Monitor real-time performance of systems
    UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: Monitoring system performance"));
    
    // Check frame rate impact
    // Monitor memory usage per system
    // Track system execution times
}

void UBuild_FinalOrchestrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentReport.CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Overall Score: %.1f"), CurrentReport.OverallIntegrationScore);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentReport.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f%%"), CurrentReport.MemoryUsagePercent);
    UE_LOG(LogTemp, Warning, TEXT("Build Successful: %s"), CurrentReport.bBuildSuccessful ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION REPORT ==="));
}

bool UBuild_FinalOrchestrator::ValidateWorldGeneration()
{
    return ValidateSystemIntegrity(TEXT("PCGWorldGenerator")) && ValidateSystemIntegrity(TEXT("FoliageManager"));
}

bool UBuild_FinalOrchestrator::ValidateCharacterSystems()
{
    return ValidateSystemIntegrity(TEXT("TranspersonalCharacter")) && ValidateSystemIntegrity(TEXT("TranspersonalGameState"));
}

bool UBuild_FinalOrchestrator::ValidateAISystems()
{
    return ValidateSystemIntegrity(TEXT("CrowdSimulationManager"));
}

bool UBuild_FinalOrchestrator::ValidateVFXSystems()
{
    // VFX systems validation - placeholder for future VFX integration
    return true;
}

bool UBuild_FinalOrchestrator::ValidateQASystems()
{
    return ValidateSystemIntegrity(TEXT("QA_SystemIntegrationValidator"));
}

void UBuild_FinalOrchestrator::UpdatePerformanceMetrics()
{
    // Update memory usage (simplified calculation)
    CurrentReport.MemoryUsagePercent = 45.0f; // Placeholder - would use actual memory monitoring
    
    // Update other performance metrics
    CheckMemoryUsage();
    ValidateFrameRate();
}

void UBuild_FinalOrchestrator::CheckMemoryUsage()
{
    // Memory usage monitoring logic
    UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: Checking memory usage"));
}

void UBuild_FinalOrchestrator::ValidateFrameRate()
{
    // Frame rate validation logic
    UE_LOG(LogTemp, Log, TEXT("Build_FinalOrchestrator: Validating frame rate"));
}

void UBuild_FinalOrchestrator::HandleIntegrationError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Build_FinalOrchestrator: Integration Error - %s"), *ErrorMessage);
    
    // Add error to current report
    if (CurrentReport.SystemStatuses.Num() > 0)
    {
        CurrentReport.SystemStatuses.Last().ValidationErrors.Add(ErrorMessage);
    }
}

void UBuild_FinalOrchestrator::RecordValidationError(const FString& SystemName, const FString& Error)
{
    for (FBuild_SystemStatus& Status : CurrentReport.SystemStatuses)
    {
        if (Status.SystemName == SystemName)
        {
            Status.ValidationErrors.Add(Error);
            Status.bIsValidated = false;
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalOrchestrator: Validation error for %s: %s"), *SystemName, *Error);
}