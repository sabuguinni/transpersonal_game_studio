#include "Build_QAIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UBuild_QAIntegrationValidator::UBuild_QAIntegrationValidator()
{
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
}

void UBuild_QAIntegrationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_QAIntegrationValidator: Subsystem initialized"));
    
    // Initialize validation state
    LastValidationResult = FBuild_QAValidationResult();
    LastMetrics = FBuild_QAIntegrationMetrics();
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
}

void UBuild_QAIntegrationValidator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_QAIntegrationValidator: Subsystem deinitialized"));
    Super::Deinitialize();
}

FBuild_QAValidationResult UBuild_QAIntegrationValidator::ValidateQAResults()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA validation already in progress"));
        return LastValidationResult;
    }

    bValidationInProgress = true;
    FBuild_QAValidationResult Result;
    Result.ValidationTimestamp = FPlatformTime::Seconds();

    UE_LOG(LogTemp, Warning, TEXT("Starting QA Integration validation"));

    // Validate actor limits
    bool bActorLimitsValid = ValidateActorLimits();
    if (!bActorLimitsValid)
    {
        Result.CriticalErrors.Add(TEXT("Actor count exceeds safe limits"));
        Result.ErrorCount++;
    }

    // Validate dinosaur limits
    bool bDinosaurLimitsValid = ValidateDinosaurLimits();
    if (!bDinosaurLimitsValid)
    {
        Result.CriticalErrors.Add(TEXT("Dinosaur count exceeds realistic limits"));
        Result.ErrorCount++;
    }

    // Validate performance metrics
    bool bPerformanceValid = ValidatePerformanceMetrics();
    if (!bPerformanceValid)
    {
        Result.Warnings.Add(TEXT("Performance metrics indicate potential issues"));
        Result.WarningCount++;
    }

    // Validate module integration
    bool bModuleIntegrationValid = ValidateModuleIntegration();
    if (!bModuleIntegrationValid)
    {
        Result.CriticalErrors.Add(TEXT("Module integration validation failed"));
        Result.ErrorCount++;
    }

    // Check critical systems
    bool bCriticalSystemsOnline = CheckCriticalSystemsOnline();
    if (!bCriticalSystemsOnline)
    {
        Result.CriticalErrors.Add(TEXT("Critical systems offline or not responding"));
        Result.ErrorCount++;
    }

    // Validate map integrity
    bool bMapIntegrityValid = ValidateMapIntegrity();
    if (!bMapIntegrityValid)
    {
        Result.CriticalErrors.Add(TEXT("Map integrity validation failed"));
        Result.ErrorCount++;
    }

    // Determine overall validation result
    Result.bValidationPassed = (Result.ErrorCount == 0);
    
    if (Result.bValidationPassed)
    {
        Result.ValidationMessage = FString::Printf(TEXT("QA Integration validation PASSED with %d warnings"), Result.WarningCount);
    }
    else
    {
        Result.ValidationMessage = FString::Printf(TEXT("QA Integration validation FAILED with %d errors and %d warnings"), Result.ErrorCount, Result.WarningCount);
    }

    LastValidationResult = Result;
    LastValidationTime = FPlatformTime::Seconds();
    bValidationInProgress = false;

    LogValidationResults(Result);
    return Result;
}

FBuild_QAIntegrationMetrics UBuild_QAIntegrationValidator::CollectIntegrationMetrics()
{
    FBuild_QAIntegrationMetrics Metrics;

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot collect metrics - World is null"));
        return Metrics;
    }

    // Count all actors
    int32 TotalActors = 0;
    int32 DinosaurActors = 0;
    int32 VFXActors = 0;
    int32 AudioActors = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        TotalActors++;

        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();

        // Count dinosaurs
        TArray<FString> DinoKeywords = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
        for (const FString& Keyword : DinoKeywords)
        {
            if (ActorName.Contains(Keyword) || ActorLabel.Contains(Keyword))
            {
                DinosaurActors++;
                break;
            }
        }

        // Count VFX actors
        if (Actor->FindComponentByClass<UParticleSystemComponent>())
        {
            VFXActors++;
        }

        // Count audio actors
        if (Actor->FindComponentByClass<UAudioComponent>())
        {
            AudioActors++;
        }
    }

    Metrics.TotalActorsInMap = TotalActors;
    Metrics.DinosaurCount = DinosaurActors;
    Metrics.VFXActorCount = VFXActors;
    Metrics.AudioActorCount = AudioActors;

    // Performance metrics (simplified)
    Metrics.FrameRate = 1.0f / FApp::GetDeltaTime();
    Metrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);

    // Check performance limits
    Metrics.bPerformanceWithinLimits = true;
    if (Metrics.TotalActorsInMap > 20000)
    {
        Metrics.PerformanceWarnings.Add(FString::Printf(TEXT("Total actors (%d) exceeds limit of 20,000"), Metrics.TotalActorsInMap));
        Metrics.bPerformanceWithinLimits = false;
    }

    if (Metrics.DinosaurCount > 150)
    {
        Metrics.PerformanceWarnings.Add(FString::Printf(TEXT("Dinosaur count (%d) exceeds limit of 150"), Metrics.DinosaurCount));
        Metrics.bPerformanceWithinLimits = false;
    }

    if (Metrics.FrameRate < 30.0f)
    {
        Metrics.PerformanceWarnings.Add(FString::Printf(TEXT("Frame rate (%.1f) below minimum of 30 FPS"), Metrics.FrameRate));
        Metrics.bPerformanceWithinLimits = false;
    }

    LastMetrics = Metrics;
    return Metrics;
}

bool UBuild_QAIntegrationValidator::ValidateActorLimits()
{
    FBuild_QAIntegrationMetrics Metrics = CollectIntegrationMetrics();
    
    bool bWithinLimits = true;
    
    if (Metrics.TotalActorsInMap > 20000)
    {
        LogCriticalError(FString::Printf(TEXT("Actor count %d exceeds maximum limit of 20,000"), Metrics.TotalActorsInMap));
        bWithinLimits = false;
    }
    else if (Metrics.TotalActorsInMap > 15000)
    {
        LogWarning(FString::Printf(TEXT("Actor count %d approaching limit of 20,000"), Metrics.TotalActorsInMap));
    }

    return bWithinLimits;
}

bool UBuild_QAIntegrationValidator::ValidateDinosaurLimits()
{
    FBuild_QAIntegrationMetrics Metrics = CollectIntegrationMetrics();
    
    bool bWithinLimits = true;
    
    if (Metrics.DinosaurCount > 150)
    {
        LogCriticalError(FString::Printf(TEXT("Dinosaur count %d exceeds maximum limit of 150"), Metrics.DinosaurCount));
        bWithinLimits = false;
    }
    else if (Metrics.DinosaurCount > 120)
    {
        LogWarning(FString::Printf(TEXT("Dinosaur count %d approaching limit of 150"), Metrics.DinosaurCount));
    }

    return bWithinLimits;
}

bool UBuild_QAIntegrationValidator::ValidatePerformanceMetrics()
{
    FBuild_QAIntegrationMetrics Metrics = CollectIntegrationMetrics();
    return Metrics.bPerformanceWithinLimits;
}

bool UBuild_QAIntegrationValidator::ValidateModuleIntegration()
{
    // Check if core TranspersonalGame classes are available
    bool bModulesValid = true;

    // This is a simplified check - in a real implementation we would
    // verify that all expected modules are loaded and functional
    UWorld* World = GetWorld();
    if (!World)
    {
        LogCriticalError(TEXT("World reference is null - core module integration failed"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Module integration validation passed"));
    return bModulesValid;
}

bool UBuild_QAIntegrationValidator::CheckCriticalSystemsOnline()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogCriticalError(TEXT("World system is offline"));
        return false;
    }

    // Check for essential actors
    bool bPlayerStartFound = false;
    bool bLightingFound = false;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        FString ActorName = Actor->GetClass()->GetName();
        
        if (ActorName.Contains(TEXT("PlayerStart")))
        {
            bPlayerStartFound = true;
        }
        
        if (ActorName.Contains(TEXT("Light")))
        {
            bLightingFound = true;
        }
    }

    if (!bPlayerStartFound)
    {
        LogCriticalError(TEXT("No PlayerStart found in map"));
        return false;
    }

    if (!bLightingFound)
    {
        LogWarning(TEXT("No lighting actors found in map"));
    }

    return true;
}

bool UBuild_QAIntegrationValidator::ValidateMapIntegrity()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Basic map integrity checks
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }

    if (ActorCount == 0)
    {
        LogCriticalError(TEXT("Map appears to be empty - no actors found"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Map integrity validation passed with %d actors"), ActorCount);
    return true;
}

bool UBuild_QAIntegrationValidator::ValidateAssetReferences()
{
    // Simplified asset reference validation
    // In a full implementation, this would check for broken asset references
    return true;
}

FString UBuild_QAIntegrationValidator::GenerateIntegrationReport()
{
    FString Report;
    Report += TEXT("=== QA INTEGRATION VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += TEXT("\n");

    FBuild_QAIntegrationMetrics Metrics = CollectIntegrationMetrics();
    
    Report += TEXT("METRICS:\n");
    Report += FString::Printf(TEXT("- Total Actors: %d\n"), Metrics.TotalActorsInMap);
    Report += FString::Printf(TEXT("- Dinosaurs: %d\n"), Metrics.DinosaurCount);
    Report += FString::Printf(TEXT("- VFX Actors: %d\n"), Metrics.VFXActorCount);
    Report += FString::Printf(TEXT("- Audio Actors: %d\n"), Metrics.AudioActorCount);
    Report += FString::Printf(TEXT("- Frame Rate: %.1f FPS\n"), Metrics.FrameRate);
    Report += FString::Printf(TEXT("- Memory Usage: %.1f MB\n"), Metrics.MemoryUsageMB);
    Report += TEXT("\n");

    Report += TEXT("VALIDATION RESULTS:\n");
    Report += FString::Printf(TEXT("- Actor Limits: %s\n"), ValidateActorLimits() ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("- Dinosaur Limits: %s\n"), ValidateDinosaurLimits() ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("- Performance: %s\n"), ValidatePerformanceMetrics() ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("- Module Integration: %s\n"), ValidateModuleIntegration() ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("- Critical Systems: %s\n"), CheckCriticalSystemsOnline() ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("- Map Integrity: %s\n"), ValidateMapIntegrity() ? TEXT("PASS") : TEXT("FAIL"));

    if (Metrics.PerformanceWarnings.Num() > 0)
    {
        Report += TEXT("\nWARNINGS:\n");
        for (const FString& Warning : Metrics.PerformanceWarnings)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Warning);
        }
    }

    Report += TEXT("\n=== END REPORT ===");
    
    return Report;
}

void UBuild_QAIntegrationValidator::LogValidationResults(const FBuild_QAValidationResult& Results)
{
    if (Results.bValidationPassed)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Integration Validation: %s"), *Results.ValidationMessage);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA Integration Validation: %s"), *Results.ValidationMessage);
        
        for (const FString& Error : Results.CriticalErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("Critical Error: %s"), *Error);
        }
    }

    for (const FString& Warning : Results.Warnings)
    {
        UE_LOG(LogTemp, Warning, TEXT("Warning: %s"), *Warning);
    }
}

bool UBuild_QAIntegrationValidator::ValidateActorCounts()
{
    return ValidateActorLimits();
}

bool UBuild_QAIntegrationValidator::ValidateSystemReferences()
{
    return ValidateModuleIntegration();
}

bool UBuild_QAIntegrationValidator::ValidatePerformanceLimits()
{
    return ValidatePerformanceMetrics();
}

void UBuild_QAIntegrationValidator::CollectPerformanceData()
{
    CollectIntegrationMetrics();
}

void UBuild_QAIntegrationValidator::LogCriticalError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("QA Integration Critical Error: %s"), *ErrorMessage);
}

void UBuild_QAIntegrationValidator::LogWarning(const FString& WarningMessage)
{
    UE_LOG(LogTemp, Warning, TEXT("QA Integration Warning: %s"), *WarningMessage);
}