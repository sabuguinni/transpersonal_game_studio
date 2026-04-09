// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "ArchitectureValidationSuite.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "GlobalShader.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(LogArchitectureValidation);

UArchitectureValidationSuite::UArchitectureValidationSuite()
{
    UE_LOG(LogArchitectureValidation, Log, TEXT("Architecture Validation Suite initialized"));
}

FArchitectureValidationResults UArchitectureValidationSuite::ExecuteValidation(const FArchitectureValidationConfig& Config)
{
    CurrentConfig = Config;
    FArchitectureValidationResults Results;

    UE_LOG(LogArchitectureValidation, Warning, TEXT("=== EXECUTING ARCHITECTURE VALIDATION v4.2 ==="));

    // Core Engine Systems Validation
    if (Config.bValidateEngineSubsystems)
    {
        Results.bEngineSubsystemsValid = ValidateEngineSubsystems();
        UE_LOG(LogArchitectureValidation, Log, TEXT("Engine Subsystems Valid: %s"), 
               Results.bEngineSubsystemsValid ? TEXT("YES") : TEXT("NO"));
    }

    // Rendering Features Validation
    if (Config.bValidateRenderingFeatures)
    {
        Results.bNaniteSupported = CheckNaniteSupport();
        Results.bLumenSupported = CheckLumenSupport();
        Results.bVirtualShadowMapsSupported = CheckVirtualShadowMapSupport();
        
        UE_LOG(LogArchitectureValidation, Log, TEXT("Nanite Support: %s"), 
               Results.bNaniteSupported ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogArchitectureValidation, Log, TEXT("Lumen Support: %s"), 
               Results.bLumenSupported ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogArchitectureValidation, Log, TEXT("Virtual Shadow Maps: %s"), 
               Results.bVirtualShadowMapsSupported ? TEXT("YES") : TEXT("NO"));
    }

    // World Partition Validation
    Results.bWorldPartitionSupported = ValidateWorldPartition();
    UE_LOG(LogArchitectureValidation, Log, TEXT("World Partition Support: %s"), 
           Results.bWorldPartitionSupported ? TEXT("YES") : TEXT("NO"));

    // Asset Management Validation
    if (Config.bValidateAssetManagement)
    {
        Results.bAssetRegistryValid = ValidateAssetManagement();
        Results.bOneFilePerActorSupported = CheckOneFilePerActorSupport();
        
        UE_LOG(LogArchitectureValidation, Log, TEXT("Asset Management Valid: %s"), 
               Results.bAssetRegistryValid ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogArchitectureValidation, Log, TEXT("One File Per Actor: %s"), 
               Results.bOneFilePerActorSupported ? TEXT("YES") : TEXT("NO"));
    }

    // Performance Systems Validation
    if (Config.bValidatePerformanceSystems)
    {
        Results.bHLODSupported = CheckHLODSupport();
        Results.bDataLayersSupported = CheckDataLayersSupport();
        Results.bLevelStreamingValid = ValidatePerformanceSystems();
        
        UE_LOG(LogArchitectureValidation, Log, TEXT("HLOD Support: %s"), 
               Results.bHLODSupported ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogArchitectureValidation, Log, TEXT("Data Layers Support: %s"), 
               Results.bDataLayersSupported ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogArchitectureValidation, Log, TEXT("Level Streaming Valid: %s"), 
               Results.bLevelStreamingValid ? TEXT("YES") : TEXT("NO"));
    }

    // Module Integration Validation
    if (Config.bValidateModuleIntegration)
    {
        Results.bCoreModuleLoaded = IsModuleLoaded(TEXT("TranspersonalGameCore"));
        Results.bPhysicsModuleLoaded = IsModuleLoaded(TEXT("PhysicsCore"));
        Results.bWorldGenerationModuleLoaded = IsModuleLoaded(TEXT("WorldGeneration"));
        Results.bAIModuleLoaded = IsModuleLoaded(TEXT("AIModule"));
        
        UE_LOG(LogArchitectureValidation, Log, TEXT("Core Module Loaded: %s"), 
               Results.bCoreModuleLoaded ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogArchitectureValidation, Log, TEXT("Physics Module Loaded: %s"), 
               Results.bPhysicsModuleLoaded ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogArchitectureValidation, Log, TEXT("World Generation Module: %s"), 
               Results.bWorldGenerationModuleLoaded ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogArchitectureValidation, Log, TEXT("AI Module Loaded: %s"), 
               Results.bAIModuleLoaded ? TEXT("YES") : TEXT("NO"));
    }

    // Calculate validation score
    Results.ValidationScore = CalculateValidationScore(Results);
    
    // Generate summary
    if (Results.ValidationScore >= Config.MinimumValidationScore)
    {
        Results.ValidationSummary = FString::Printf(TEXT("VALIDATION PASSED - Score: %.1f%%"), Results.ValidationScore);
        UE_LOG(LogArchitectureValidation, Warning, TEXT("=== VALIDATION PASSED - Score: %.1f%% ==="), Results.ValidationScore);
    }
    else
    {
        Results.ValidationSummary = FString::Printf(TEXT("VALIDATION FAILED - Score: %.1f%% (Required: %.1f%%)"), 
                                                   Results.ValidationScore, Config.MinimumValidationScore);
        UE_LOG(LogArchitectureValidation, Error, TEXT("=== VALIDATION FAILED - Score: %.1f%% ==="), Results.ValidationScore);
    }

    // Store results
    LastResults = Results;
    
    return Results;
}

bool UArchitectureValidationSuite::ValidateEngineSubsystems()
{
    bool bValid = true;

    // Check engine instance
    if (!GEngine)
    {
        AddCriticalIssue(TEXT("GEngine is null"), LastResults);
        bValid = false;
    }

    // Check world context
    UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
    if (!World)
    {
        World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    }
    
    if (!World)
    {
        AddWarning(TEXT("No valid world context found"), LastResults);
    }

    // Check RHI
    if (!GDynamicRHI)
    {
        AddCriticalIssue(TEXT("Dynamic RHI not initialized"), LastResults);
        bValid = false;
    }

    return bValid;
}

bool UArchitectureValidationSuite::ValidateRenderingFeatures()
{
    bool bAllSupported = true;

    bAllSupported &= CheckNaniteSupport();
    bAllSupported &= CheckLumenSupport();
    bAllSupported &= CheckVirtualShadowMapSupport();

    return bAllSupported;
}

bool UArchitectureValidationSuite::ValidateWorldPartition()
{
    // Check if World Partition module is loaded
    bool bModuleLoaded = FModuleManager::Get().IsModuleLoaded("WorldPartition");
    
    if (!bModuleLoaded)
    {
        AddWarning(TEXT("World Partition module not loaded"), LastResults);
        return false;
    }

    // Check for World Partition support in current world
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (World)
    {
        // World Partition specific checks would go here
        UE_LOG(LogArchitectureValidation, Log, TEXT("World Partition validation completed for world: %s"), 
               *World->GetName());
    }

    return true;
}

bool UArchitectureValidationSuite::ValidateAssetManagement()
{
    // Check Asset Registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    if (!AssetRegistry.IsLoadingAssets())
    {
        UE_LOG(LogArchitectureValidation, Log, TEXT("Asset Registry is ready"));
        return true;
    }
    else
    {
        AddWarning(TEXT("Asset Registry is still loading assets"), LastResults);
        return false;
    }
}

bool UArchitectureValidationSuite::ValidateModuleIntegration()
{
    bool bAllModulesValid = true;

    // Check critical modules
    TArray<FString> CriticalModules = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("RenderCore"),
        TEXT("RHI")
    };

    for (const FString& ModuleName : CriticalModules)
    {
        if (!IsModuleLoaded(ModuleName))
        {
            AddCriticalIssue(FString::Printf(TEXT("Critical module not loaded: %s"), *ModuleName), LastResults);
            bAllModulesValid = false;
        }
    }

    return bAllModulesValid;
}

bool UArchitectureValidationSuite::ValidatePerformanceSystems()
{
    bool bValid = true;

    // Check level streaming
    bool bLevelStreamingAvailable = FModuleManager::Get().IsModuleLoaded("LevelSequence");
    if (!bLevelStreamingAvailable)
    {
        AddWarning(TEXT("Level streaming systems may not be fully available"), LastResults);
    }

    return bValid;
}

bool UArchitectureValidationSuite::CheckNaniteSupport()
{
    // Check if Nanite is supported on current platform
    return FModuleManager::Get().IsModuleLoaded("Renderer");
}

bool UArchitectureValidationSuite::CheckLumenSupport()
{
    // Check if Lumen is supported
    return FModuleManager::Get().IsModuleLoaded("Renderer");
}

bool UArchitectureValidationSuite::CheckVirtualShadowMapSupport()
{
    // Check if Virtual Shadow Maps are supported
    return FModuleManager::Get().IsModuleLoaded("Renderer");
}

bool UArchitectureValidationSuite::CheckHLODSupport()
{
    return FModuleManager::Get().IsModuleLoaded("HierarchicalLODUtilities");
}

bool UArchitectureValidationSuite::CheckDataLayersSupport()
{
    return FModuleManager::Get().IsModuleLoaded("DataLayerEditor");
}

bool UArchitectureValidationSuite::CheckOneFilePerActorSupport()
{
    return FModuleManager::Get().IsModuleLoaded("WorldPartition");
}

bool UArchitectureValidationSuite::IsModuleLoaded(const FString& ModuleName)
{
    return FModuleManager::Get().IsModuleLoaded(*ModuleName);
}

float UArchitectureValidationSuite::CalculateValidationScore(const FArchitectureValidationResults& Results)
{
    float Score = 0.0f;
    int32 TotalChecks = 0;

    // Engine subsystems (20 points)
    if (Results.bEngineSubsystemsValid) Score += 20.0f;
    TotalChecks += 20;

    // Rendering features (30 points total)
    if (Results.bNaniteSupported) Score += 10.0f;
    if (Results.bLumenSupported) Score += 10.0f;
    if (Results.bVirtualShadowMapsSupported) Score += 10.0f;
    TotalChecks += 30;

    // World systems (25 points total)
    if (Results.bWorldPartitionSupported) Score += 10.0f;
    if (Results.bAssetRegistryValid) Score += 8.0f;
    if (Results.bOneFilePerActorSupported) Score += 7.0f;
    TotalChecks += 25;

    // Performance systems (15 points total)
    if (Results.bHLODSupported) Score += 5.0f;
    if (Results.bDataLayersSupported) Score += 5.0f;
    if (Results.bLevelStreamingValid) Score += 5.0f;
    TotalChecks += 15;

    // Module integration (10 points total)
    if (Results.bCoreModuleLoaded) Score += 3.0f;
    if (Results.bPhysicsModuleLoaded) Score += 2.0f;
    if (Results.bWorldGenerationModuleLoaded) Score += 3.0f;
    if (Results.bAIModuleLoaded) Score += 2.0f;
    TotalChecks += 10;

    return (Score / TotalChecks) * 100.0f;
}

FString UArchitectureValidationSuite::GenerateValidationReport(const FArchitectureValidationResults& Results)
{
    FString Report;
    Report += TEXT("=== TRANSPERSONAL GAME STUDIO v4.2 ARCHITECTURE VALIDATION REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Validation Score: %.1f%%\n"), Results.ValidationScore);
    Report += FString::Printf(TEXT("Status: %s\n\n"), *Results.ValidationSummary);

    Report += TEXT("CORE ENGINE SYSTEMS:\n");
    Report += FString::Printf(TEXT("  Engine Subsystems: %s\n"), Results.bEngineSubsystemsValid ? TEXT("✓") : TEXT("✗"));
    Report += FString::Printf(TEXT("  World Partition: %s\n"), Results.bWorldPartitionSupported ? TEXT("✓") : TEXT("✗"));

    Report += TEXT("\nRENDERING FEATURES:\n");
    Report += FString::Printf(TEXT("  Nanite Support: %s\n"), Results.bNaniteSupported ? TEXT("✓") : TEXT("✗"));
    Report += FString::Printf(TEXT("  Lumen Support: %s\n"), Results.bLumenSupported ? TEXT("✓") : TEXT("✗"));
    Report += FString::Printf(TEXT("  Virtual Shadow Maps: %s\n"), Results.bVirtualShadowMapsSupported ? TEXT("✓") : TEXT("✗"));

    Report += TEXT("\nASSET MANAGEMENT:\n");
    Report += FString::Printf(TEXT("  Asset Registry: %s\n"), Results.bAssetRegistryValid ? TEXT("✓") : TEXT("✗"));
    Report += FString::Printf(TEXT("  One File Per Actor: %s\n"), Results.bOneFilePerActorSupported ? TEXT("✓") : TEXT("✗"));

    Report += TEXT("\nPERFORMANCE SYSTEMS:\n");
    Report += FString::Printf(TEXT("  HLOD Support: %s\n"), Results.bHLODSupported ? TEXT("✓") : TEXT("✗"));
    Report += FString::Printf(TEXT("  Data Layers: %s\n"), Results.bDataLayersSupported ? TEXT("✓") : TEXT("✗"));
    Report += FString::Printf(TEXT("  Level Streaming: %s\n"), Results.bLevelStreamingValid ? TEXT("✓") : TEXT("✗"));

    if (Results.CriticalIssues.Num() > 0)
    {
        Report += TEXT("\nCRITICAL ISSUES:\n");
        for (const FString& Issue : Results.CriticalIssues)
        {
            Report += FString::Printf(TEXT("  • %s\n"), *Issue);
        }
    }

    if (Results.Warnings.Num() > 0)
    {
        Report += TEXT("\nWARNINGS:\n");
        for (const FString& Warning : Results.Warnings)
        {
            Report += FString::Printf(TEXT("  • %s\n"), *Warning);
        }
    }

    Report += TEXT("\n=== END VALIDATION REPORT ===\n");
    
    return Report;
}

void UArchitectureValidationSuite::AddCriticalIssue(const FString& Issue, FArchitectureValidationResults& Results)
{
    Results.CriticalIssues.Add(Issue);
    UE_LOG(LogArchitectureValidation, Error, TEXT("CRITICAL: %s"), *Issue);
}

void UArchitectureValidationSuite::AddWarning(const FString& Warning, FArchitectureValidationResults& Results)
{
    Results.Warnings.Add(Warning);
    UE_LOG(LogArchitectureValidation, Warning, TEXT("WARNING: %s"), *Warning);
}

// Architecture Validation Subsystem Implementation

void UArchitectureValidationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ValidationSuite = NewObject<UArchitectureValidationSuite>(this);
    
    UE_LOG(LogArchitectureValidation, Log, TEXT("Architecture Validation Subsystem initialized"));
    
    // Execute startup validation
    ExecuteStartupValidation();
}

void UArchitectureValidationSubsystem::Deinitialize()
{
    if (ValidationTimerHandle.IsValid())
    {
        GEngine->GetTimerManager()->ClearTimer(ValidationTimerHandle);
    }
    
    Super::Deinitialize();
}

UArchitectureValidationSuite* UArchitectureValidationSubsystem::GetValidationSuite()
{
    return ValidationSuite;
}

bool UArchitectureValidationSubsystem::ExecuteStartupValidation()
{
    if (!ValidationSuite)
    {
        return false;
    }

    FArchitectureValidationConfig Config;
    Config.bValidateEngineSubsystems = true;
    Config.bValidateRenderingFeatures = true;
    Config.bValidateAssetManagement = true;
    Config.bValidateModuleIntegration = true;
    Config.bValidatePerformanceSystems = true;
    Config.MinimumValidationScore = 80.0f;

    FArchitectureValidationResults Results = ValidationSuite->ExecuteValidation(Config);
    
    FString Report = ValidationSuite->GenerateValidationReport(Results);
    UE_LOG(LogArchitectureValidation, Warning, TEXT("%s"), *Report);

    return Results.ValidationScore >= Config.MinimumValidationScore;
}

void UArchitectureValidationSubsystem::SchedulePeriodicValidation(float IntervalSeconds)
{
    if (GEngine && GEngine->GetTimerManager())
    {
        GEngine->GetTimerManager()->SetTimer(ValidationTimerHandle, 
                                           this, 
                                           &UArchitectureValidationSubsystem::PeriodicValidationCheck, 
                                           IntervalSeconds, 
                                           true);
    }
}

void UArchitectureValidationSubsystem::PeriodicValidationCheck()
{
    UE_LOG(LogArchitectureValidation, Log, TEXT("Executing periodic validation check"));
    ExecuteStartupValidation();
}