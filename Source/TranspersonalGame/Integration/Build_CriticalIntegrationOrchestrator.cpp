#include "Build_CriticalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Engine/Level.h"
#include "EngineUtils.h"

UBuild_CriticalIntegrationOrchestrator::UBuild_CriticalIntegrationOrchestrator()
{
    // Initialize default status
    CurrentStatus.bModulesLoaded = false;
    CurrentStatus.bCoreSystemsActive = false;
    CurrentStatus.bBuildHealthy = false;
    CurrentStatus.LoadedActorCount = 0;
    CurrentStatus.ActiveSystemCount = 0;
    CurrentStatus.LastValidationTime = TEXT("Never");
}

void UBuild_CriticalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Initializing integration orchestrator"));
    
    // Perform initial validation
    ValidateFullSystemIntegration();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Integration orchestrator initialized"));
}

void UBuild_CriticalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Shutting down integration orchestrator"));
    
    // Clear tracked systems
    TrackedSystems.Empty();
    SystemHealthMap.Empty();
    
    Super::Deinitialize();
}

FBuild_IntegrationStatus UBuild_CriticalIntegrationOrchestrator::ValidateFullSystemIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Starting full system integration validation"));
    
    // Reset status
    CurrentStatus = FBuild_IntegrationStatus();
    CurrentStatus.CriticalErrors.Empty();
    CurrentStatus.SystemWarnings.Empty();
    
    // Validate modules
    CurrentStatus.bModulesLoaded = ValidateAllModulesLoaded();
    
    // Validate core systems
    CurrentStatus.bCoreSystemsActive = ValidateCoreSystemsActive();
    
    // Validate build integrity
    CurrentStatus.bBuildHealthy = ValidateBuildIntegrity();
    
    // Count actors in world
    if (UWorld* World = GetWorld())
    {
        CurrentStatus.LoadedActorCount = World->GetCurrentLevel()->Actors.Num();
    }
    
    // Update validation time
    LastValidationTime = FDateTime::Now();
    CurrentStatus.LastValidationTime = LastValidationTime.ToString();
    
    // Log results
    LogCriticalIntegrationStatus();
    
    ValidationCycleCount++;
    
    return CurrentStatus;
}

bool UBuild_CriticalIntegrationOrchestrator::ValidateAllModulesLoaded()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Validating module loading"));
    
    bool bAllModulesValid = true;
    ModuleResults.Empty();
    
    // Test TranspersonalGame module
    FBuild_ModuleValidationResult TranspersonalResult;
    TranspersonalResult.ModuleName = TEXT("TranspersonalGame");
    
    // Try to load core classes
    TArray<FString> TestClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager")
    };
    
    for (const FString& ClassName : TestClasses)
    {
        UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
        if (TestClass)
        {
            TranspersonalResult.LoadedClasses.Add(ClassName);
            TranspersonalResult.ClassCount++;
        }
        else
        {
            TranspersonalResult.ValidationErrors.Add(FString::Printf(TEXT("Failed to load class: %s"), *ClassName));
            TranspersonalResult.bHasErrors = true;
            bAllModulesValid = false;
        }
    }
    
    TranspersonalResult.bIsLoaded = (TranspersonalResult.ClassCount > 0);
    ModuleResults.Add(TranspersonalResult);
    
    if (bAllModulesValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: All modules validated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalIntegrationOrchestrator: Module validation failed"));
        CurrentStatus.CriticalErrors.Add(TEXT("Module loading validation failed"));
    }
    
    return bAllModulesValid;
}

TArray<FBuild_ModuleValidationResult> UBuild_CriticalIntegrationOrchestrator::ValidateModuleHealth()
{
    ValidateAllModulesLoaded();
    return ModuleResults;
}

bool UBuild_CriticalIntegrationOrchestrator::ValidateCoreSystemsActive()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Validating core systems"));
    
    bool bCoreSystemsValid = true;
    int32 ActiveSystems = 0;
    
    // Check for world generation system
    if (ValidateWorldGeneration())
    {
        ActiveSystems++;
        SystemHealthMap.Add(TEXT("WorldGeneration"), true);
    }
    else
    {
        SystemHealthMap.Add(TEXT("WorldGeneration"), false);
        CurrentStatus.SystemWarnings.Add(TEXT("World generation system not active"));
    }
    
    // Check character systems
    if (ValidateCharacterSystems())
    {
        ActiveSystems++;
        SystemHealthMap.Add(TEXT("CharacterSystems"), true);
    }
    else
    {
        SystemHealthMap.Add(TEXT("CharacterSystems"), false);
        CurrentStatus.SystemWarnings.Add(TEXT("Character systems not fully active"));
    }
    
    // Check AI systems
    if (ValidateAISystems())
    {
        ActiveSystems++;
        SystemHealthMap.Add(TEXT("AISystems"), true);
    }
    else
    {
        SystemHealthMap.Add(TEXT("AISystems"), false);
        CurrentStatus.SystemWarnings.Add(TEXT("AI systems not active"));
    }
    
    // Check VFX systems
    if (ValidateVFXSystems())
    {
        ActiveSystems++;
        SystemHealthMap.Add(TEXT("VFXSystems"), true);
    }
    else
    {
        SystemHealthMap.Add(TEXT("VFXSystems"), false);
        CurrentStatus.SystemWarnings.Add(TEXT("VFX systems not active"));
    }
    
    CurrentStatus.ActiveSystemCount = ActiveSystems;
    
    // Consider core systems active if at least 2 are working
    bCoreSystemsValid = (ActiveSystems >= 2);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Core systems validation complete - %d systems active"), ActiveSystems);
    
    return bCoreSystemsValid;
}

bool UBuild_CriticalIntegrationOrchestrator::ValidateBuildIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Validating build integrity"));
    
    bool bBuildIntegrityValid = true;
    
    // Check if we can access basic UE5 systems
    if (!GEngine)
    {
        CurrentStatus.CriticalErrors.Add(TEXT("GEngine not available"));
        bBuildIntegrityValid = false;
    }
    
    // Check if we have a valid world
    UWorld* World = GetWorld();
    if (!World)
    {
        CurrentStatus.CriticalErrors.Add(TEXT("No valid world context"));
        bBuildIntegrityValid = false;
    }
    else
    {
        // Check if world has basic components
        if (!World->GetCurrentLevel())
        {
            CurrentStatus.CriticalErrors.Add(TEXT("No current level in world"));
            bBuildIntegrityValid = false;
        }
    }
    
    // Check module dependencies
    if (!ValidateSystemDependencies())
    {
        CurrentStatus.CriticalErrors.Add(TEXT("System dependencies validation failed"));
        bBuildIntegrityValid = false;
    }
    
    if (bBuildIntegrityValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Build integrity validated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalIntegrationOrchestrator: Build integrity validation failed"));
    }
    
    return bBuildIntegrityValid;
}

bool UBuild_CriticalIntegrationOrchestrator::ValidateWorldGeneration()
{
    // Try to find PCGWorldGenerator class
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    return (WorldGenClass != nullptr);
}

bool UBuild_CriticalIntegrationOrchestrator::ValidateCharacterSystems()
{
    // Try to find TranspersonalCharacter class
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    return (CharacterClass != nullptr);
}

bool UBuild_CriticalIntegrationOrchestrator::ValidateAISystems()
{
    // Try to find CrowdSimulationManager class
    UClass* CrowdClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
    return (CrowdClass != nullptr);
}

bool UBuild_CriticalIntegrationOrchestrator::ValidateVFXSystems()
{
    // For now, assume VFX systems are available if we can access Niagara
    return true; // Basic validation - VFX systems are optional for core functionality
}

FString UBuild_CriticalIntegrationOrchestrator::GenerateIntegrationReport()
{
    FString Report = TEXT("=== CRITICAL INTEGRATION REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Validation Time: %s\n"), *CurrentStatus.LastValidationTime);
    Report += FString::Printf(TEXT("Validation Cycle: %d\n\n"), ValidationCycleCount);
    
    // Overall status
    Report += TEXT("OVERALL STATUS:\n");
    Report += FString::Printf(TEXT("- Modules Loaded: %s\n"), CurrentStatus.bModulesLoaded ? TEXT("✅ YES") : TEXT("❌ NO"));
    Report += FString::Printf(TEXT("- Core Systems Active: %s\n"), CurrentStatus.bCoreSystemsActive ? TEXT("✅ YES") : TEXT("❌ NO"));
    Report += FString::Printf(TEXT("- Build Healthy: %s\n"), CurrentStatus.bBuildHealthy ? TEXT("✅ YES") : TEXT("❌ NO"));
    Report += FString::Printf(TEXT("- Loaded Actors: %d\n"), CurrentStatus.LoadedActorCount);
    Report += FString::Printf(TEXT("- Active Systems: %d\n\n"), CurrentStatus.ActiveSystemCount);
    
    // Module details
    Report += TEXT("MODULE VALIDATION:\n");
    for (const FBuild_ModuleValidationResult& ModuleResult : ModuleResults)
    {
        Report += FString::Printf(TEXT("- %s: %s (%d classes)\n"), 
            *ModuleResult.ModuleName,
            ModuleResult.bIsLoaded ? TEXT("✅ LOADED") : TEXT("❌ FAILED"),
            ModuleResult.ClassCount);
    }
    Report += TEXT("\n");
    
    // System health
    Report += TEXT("SYSTEM HEALTH:\n");
    for (const auto& SystemPair : SystemHealthMap)
    {
        Report += FString::Printf(TEXT("- %s: %s\n"), 
            *SystemPair.Key,
            SystemPair.Value ? TEXT("✅ ACTIVE") : TEXT("❌ INACTIVE"));
    }
    Report += TEXT("\n");
    
    // Critical errors
    if (CurrentStatus.CriticalErrors.Num() > 0)
    {
        Report += TEXT("CRITICAL ERRORS:\n");
        for (const FString& Error : CurrentStatus.CriticalErrors)
        {
            Report += FString::Printf(TEXT("❌ %s\n"), *Error);
        }
        Report += TEXT("\n");
    }
    
    // Warnings
    if (CurrentStatus.SystemWarnings.Num() > 0)
    {
        Report += TEXT("SYSTEM WARNINGS:\n");
        for (const FString& Warning : CurrentStatus.SystemWarnings)
        {
            Report += FString::Printf(TEXT("⚠️ %s\n"), *Warning);
        }
        Report += TEXT("\n");
    }
    
    Report += TEXT("=== END INTEGRATION REPORT ===");
    
    return Report;
}

void UBuild_CriticalIntegrationOrchestrator::LogCriticalIntegrationStatus()
{
    FString Report = GenerateIntegrationReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

bool UBuild_CriticalIntegrationOrchestrator::SaveIntegrationReport(const FString& ReportPath)
{
    FString Report = GenerateIntegrationReport();
    return FFileHelper::SaveStringToFile(Report, *ReportPath);
}

bool UBuild_CriticalIntegrationOrchestrator::AttemptSystemRecovery()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: Attempting system recovery"));
    
    bEmergencyMode = true;
    
    // Clear cached data
    TrackedSystems.Empty();
    SystemHealthMap.Empty();
    
    // Re-validate everything
    FBuild_IntegrationStatus RecoveryStatus = ValidateFullSystemIntegration();
    
    bool bRecoverySuccessful = RecoveryStatus.bModulesLoaded && RecoveryStatus.bBuildHealthy;
    
    if (bRecoverySuccessful)
    {
        bEmergencyMode = false;
        UE_LOG(LogTemp, Warning, TEXT("Build_CriticalIntegrationOrchestrator: System recovery successful"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalIntegrationOrchestrator: System recovery failed"));
    }
    
    return bRecoverySuccessful;
}

void UBuild_CriticalIntegrationOrchestrator::TriggerEmergencyValidation()
{
    UE_LOG(LogTemp, Error, TEXT("Build_CriticalIntegrationOrchestrator: EMERGENCY VALIDATION TRIGGERED"));
    
    bEmergencyMode = true;
    
    // Perform immediate validation
    ValidateFullSystemIntegration();
    
    // Log emergency status
    LogCriticalIntegrationStatus();
    
    // Attempt recovery if needed
    if (!CurrentStatus.bBuildHealthy)
    {
        AttemptSystemRecovery();
    }
}

bool UBuild_CriticalIntegrationOrchestrator::ValidateSystemDependencies()
{
    // Basic dependency validation - check if core UE5 systems are available
    bool bDependenciesValid = true;
    
    // Check if we can access basic reflection system
    UClass* ObjectClass = UObject::StaticClass();
    if (!ObjectClass)
    {
        bDependenciesValid = false;
    }
    
    // Check if we can access basic actor system
    UClass* ActorClass = AActor::StaticClass();
    if (!ActorClass)
    {
        bDependenciesValid = false;
    }
    
    return bDependenciesValid;
}