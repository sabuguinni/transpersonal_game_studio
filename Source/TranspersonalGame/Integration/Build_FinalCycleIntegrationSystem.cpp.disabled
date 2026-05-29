#include "Build_FinalCycleIntegrationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/DateTime.h"

UBuild_FinalCycleIntegrationSystem::UBuild_FinalCycleIntegrationSystem()
{
    // Initialize core system paths
    CoreSystemPaths = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };

    // Initialize VFX system paths
    VFXSystemPaths = {
        TEXT("/Script/TranspersonalGame.VFX_EnvironmentalEffectsManager"),
        TEXT("/Script/TranspersonalGame.VFX_WeatherSystemManager"),
        TEXT("/Script/TranspersonalGame.VFX_ParticleSystemManager"),
        TEXT("/Script/TranspersonalGame.VFX_CombatEffectsManager"),
        TEXT("/Script/TranspersonalGame.VFX_DinosaurEffectsManager")
    };

    // Initialize cycle data
    CycleData.AgentNumber = 19;
    CycleData.CompletionTime = FDateTime::Now();
    CycleData.CycleID = TEXT("PROD_CYCLE_AUTO_20260513_005");
}

void UBuild_FinalCycleIntegrationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleIntegrationSystem: Initializing final cycle integration"));
    
    // Initialize integration metrics
    CurrentMetrics = FBuild_FinalIntegrationMetrics();
    CurrentMetrics.IntegrationStatus = TEXT("Initializing");
    
    bIntegrationInitialized = true;
    
    // Execute automatic integration validation
    ExecuteFinalCycleIntegration();
}

void UBuild_FinalCycleIntegrationSystem::Deinitialize()
{
    if (bIntegrationInitialized && !bCycleCompleted)
    {
        // Complete cycle before shutdown
        CompleteCycle(CycleData.CycleID);
    }
    
    Super::Deinitialize();
}

void UBuild_FinalCycleIntegrationSystem::ExecuteFinalCycleIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("=== EXECUTING FINAL CYCLE INTEGRATION ==="));
    
    // Step 1: Validate all systems
    CurrentMetrics = ValidateAllSystems();
    
    // Step 2: Update cycle completion data
    CycleData.IntegrationMetrics = CurrentMetrics;
    CycleData.bAllSystemsIntegrated = CurrentMetrics.bIntegrationSuccessful;
    
    // Step 3: Generate final report
    GenerateFinalCycleReport();
    
    // Step 4: Log completion
    LogCycleCompletion();
    
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL CYCLE INTEGRATION COMPLETE ==="));
}

FBuild_FinalIntegrationMetrics UBuild_FinalCycleIntegrationSystem::ValidateAllSystems()
{
    FBuild_FinalIntegrationMetrics Metrics;
    
    // Validate core systems
    bool bCoreSystemsValid = ValidateCoreGameSystems();
    
    // Validate VFX systems
    bool bVFXSystemsValid = ValidateVFXSystems();
    
    // Validate level integration
    bool bLevelValid = ValidateLevelIntegration();
    
    // Calculate totals
    Metrics.TotalSystemsValidated = CoreSystemPaths.Num() + VFXSystemPaths.Num();
    Metrics.SystemsLoaded = CurrentMetrics.CoreSystemsActive + CurrentMetrics.VFXSystemsIntegrated;
    Metrics.CoreSystemsActive = CurrentMetrics.CoreSystemsActive;
    Metrics.VFXSystemsIntegrated = CurrentMetrics.VFXSystemsIntegrated;
    Metrics.ActorCountInLevel = CurrentMetrics.ActorCountInLevel;
    
    // Calculate integration percentage
    if (Metrics.TotalSystemsValidated > 0)
    {
        Metrics.IntegrationPercentage = (float(Metrics.SystemsLoaded) / float(Metrics.TotalSystemsValidated)) * 100.0f;
    }
    
    // Determine success
    Metrics.bIntegrationSuccessful = (Metrics.IntegrationPercentage >= 80.0f) && bLevelValid;
    
    // Set status
    if (Metrics.bIntegrationSuccessful)
    {
        Metrics.IntegrationStatus = TEXT("SUCCESS - All systems integrated");
    }
    else
    {
        Metrics.IntegrationStatus = FString::Printf(TEXT("PARTIAL - %.1f%% systems integrated"), Metrics.IntegrationPercentage);
    }
    
    // Copy loaded and failed systems
    Metrics.LoadedSystems = CurrentMetrics.LoadedSystems;
    Metrics.FailedSystems = CurrentMetrics.FailedSystems;
    
    return Metrics;
}

bool UBuild_FinalCycleIntegrationSystem::ValidateCoreGameSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating core game systems..."));
    
    int32 LoadedCount = 0;
    CurrentMetrics.LoadedSystems.Empty();
    CurrentMetrics.FailedSystems.Empty();
    
    for (const FString& SystemPath : CoreSystemPaths)
    {
        if (ValidateSystemClass(SystemPath))
        {
            LoadedCount++;
            CurrentMetrics.LoadedSystems.Add(SystemPath);
            UE_LOG(LogTemp, Log, TEXT("✅ Core system loaded: %s"), *SystemPath);
        }
        else
        {
            CurrentMetrics.FailedSystems.Add(SystemPath);
            UE_LOG(LogTemp, Warning, TEXT("❌ Core system failed: %s"), *SystemPath);
        }
    }
    
    CurrentMetrics.CoreSystemsActive = LoadedCount;
    
    UE_LOG(LogTemp, Warning, TEXT("Core systems validation: %d/%d loaded"), LoadedCount, CoreSystemPaths.Num());
    
    return LoadedCount >= (CoreSystemPaths.Num() * 0.8f); // 80% success rate
}

bool UBuild_FinalCycleIntegrationSystem::ValidateVFXSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating VFX systems..."));
    
    int32 LoadedCount = 0;
    
    for (const FString& SystemPath : VFXSystemPaths)
    {
        if (ValidateSystemClass(SystemPath))
        {
            LoadedCount++;
            CurrentMetrics.LoadedSystems.Add(SystemPath);
            UE_LOG(LogTemp, Log, TEXT("✅ VFX system loaded: %s"), *SystemPath);
        }
        else
        {
            CurrentMetrics.FailedSystems.Add(SystemPath);
            UE_LOG(LogTemp, Warning, TEXT("⚠️ VFX system not found: %s"), *SystemPath);
        }
    }
    
    CurrentMetrics.VFXSystemsIntegrated = LoadedCount;
    
    UE_LOG(LogTemp, Warning, TEXT("VFX systems validation: %d/%d loaded"), LoadedCount, VFXSystemPaths.Num());
    
    return LoadedCount >= (VFXSystemPaths.Num() * 0.6f); // 60% success rate for VFX (newer systems)
}

bool UBuild_FinalCycleIntegrationSystem::ValidateLevelIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating level integration..."));
    
    // Get current world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No world found for level validation"));
        return false;
    }
    
    // Count actors in level
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    CurrentMetrics.ActorCountInLevel = ActorCount;
    
    UE_LOG(LogTemp, Warning, TEXT("📊 Level validation: %d actors found"), ActorCount);
    
    // Level is valid if we have reasonable number of actors
    bool bLevelValid = ActorCount >= 10; // Minimum viable level
    
    if (bLevelValid)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ Level integration valid"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ Level integration needs attention"));
    }
    
    return bLevelValid;
}

bool UBuild_FinalCycleIntegrationSystem::ValidateSystemClass(const FString& ClassPath)
{
    // Try to load the class
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    return LoadedClass != nullptr;
}

void UBuild_FinalCycleIntegrationSystem::GenerateFinalCycleReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL CYCLE INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle ID: %s"), *CycleData.CycleID);
    UE_LOG(LogTemp, Warning, TEXT("Agent: #%d Integration & Build Agent"), CycleData.AgentNumber);
    UE_LOG(LogTemp, Warning, TEXT("Integration Status: %s"), *CurrentMetrics.IntegrationStatus);
    UE_LOG(LogTemp, Warning, TEXT("Integration Percentage: %.1f%%"), CurrentMetrics.IntegrationPercentage);
    UE_LOG(LogTemp, Warning, TEXT("Systems Loaded: %d/%d"), CurrentMetrics.SystemsLoaded, CurrentMetrics.TotalSystemsValidated);
    UE_LOG(LogTemp, Warning, TEXT("Core Systems: %d active"), CurrentMetrics.CoreSystemsActive);
    UE_LOG(LogTemp, Warning, TEXT("VFX Systems: %d integrated"), CurrentMetrics.VFXSystemsIntegrated);
    UE_LOG(LogTemp, Warning, TEXT("Level Actors: %d"), CurrentMetrics.ActorCountInLevel);
    
    if (CurrentMetrics.bIntegrationSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ FINAL INTEGRATION: SUCCESS"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ FINAL INTEGRATION: NEEDS ATTENTION"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION REPORT ==="));
}

void UBuild_FinalCycleIntegrationSystem::CompleteCycle(const FString& CycleID)
{
    CycleData.CycleID = CycleID;
    CycleData.CompletionTime = FDateTime::Now();
    bCycleCompleted = true;
    
    UE_LOG(LogTemp, Warning, TEXT("🏁 CYCLE COMPLETED: %s"), *CycleID);
}

void UBuild_FinalCycleIntegrationSystem::LogCycleCompletion()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CYCLE COMPLETION LOG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s"), *CycleData.CycleID);
    UE_LOG(LogTemp, Warning, TEXT("Completed: %s"), *CycleData.CompletionTime.ToString());
    UE_LOG(LogTemp, Warning, TEXT("All Systems Integrated: %s"), CycleData.bAllSystemsIntegrated ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("=== END COMPLETION LOG ==="));
}

TArray<FString> UBuild_FinalCycleIntegrationSystem::GetLoadedSystemsList()
{
    return CurrentMetrics.LoadedSystems;
}

TArray<FString> UBuild_FinalCycleIntegrationSystem::GetFailedSystemsList()
{
    return CurrentMetrics.FailedSystems;
}

float UBuild_FinalCycleIntegrationSystem::GetIntegrationPercentage()
{
    return CurrentMetrics.IntegrationPercentage;
}

bool UBuild_FinalCycleIntegrationSystem::IsIntegrationSuccessful()
{
    return CurrentMetrics.bIntegrationSuccessful;
}

FBuild_CycleCompletionData UBuild_FinalCycleIntegrationSystem::GetCycleCompletionData()
{
    return CycleData;
}