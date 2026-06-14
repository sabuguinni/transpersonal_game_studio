#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"
#include "TranspersonalGame/TranspersonalGame.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    TotalActorCount = 0;
    DinosaurCount = 0;
    LastValidationTime = 0.0f;
    bSystemHealthy = true;
}

void UBuild_IntegrationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTranspersonalGame, Log, TEXT("Build Integration Validator initialized"));
    
    // Run initial validation
    RunFullValidationSuite();
}

void UBuild_IntegrationValidator::Deinitialize()
{
    ValidationResults.Empty();
    Super::Deinitialize();
}

void UBuild_IntegrationValidator::RunFullValidationSuite()
{
    double StartTime = FPlatformTime::Seconds();
    ValidationResults.Empty();
    bSystemHealthy = true;

    UE_LOG(LogTranspersonalGame, Log, TEXT("Starting full validation suite"));

    // Core validation tests
    ValidateModuleCompilation();
    ValidateActorIntegrity();
    ValidatePerformanceMetrics();
    ValidateSystemDependencies();
    
    // Cleanup and enforcement
    CleanupOrphanedActors();
    EnforceActorLimits();

    LastValidationTime = FPlatformTime::Seconds() - StartTime;
    
    // Check overall health
    for (const FBuild_ValidationReport& Report : ValidationResults)
    {
        if (Report.Result == EBuild_ValidationResult::Critical || Report.Result == EBuild_ValidationResult::Fail)
        {
            bSystemHealthy = false;
            break;
        }
    }

    UE_LOG(LogTranspersonalGame, Log, TEXT("Validation suite completed in %.3f seconds. System healthy: %s"), 
           LastValidationTime, bSystemHealthy ? TEXT("YES") : TEXT("NO"));
}

void UBuild_IntegrationValidator::ValidateModuleCompilation()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Check if TranspersonalGame module is loaded
    FModuleManager& ModuleManager = FModuleManager::Get();
    bool bModuleLoaded = ModuleManager.IsModuleLoaded("TranspersonalGame");
    
    if (bModuleLoaded)
    {
        AddValidationResult(TEXT("Module Compilation"), EBuild_ValidationResult::Pass, 
                          TEXT("TranspersonalGame module loaded successfully"), 
                          FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationResult(TEXT("Module Compilation"), EBuild_ValidationResult::Critical, 
                          TEXT("TranspersonalGame module failed to load"), 
                          FPlatformTime::Seconds() - StartTime);
    }
}

void UBuild_IntegrationValidator::ValidateActorIntegrity()
{
    double StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("Actor Integrity"), EBuild_ValidationResult::Critical, 
                          TEXT("No valid world found"), 
                          FPlatformTime::Seconds() - StartTime);
        return;
    }

    CountActorsByType();
    
    int32 ValidActors = 0;
    int32 InvalidActors = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            ValidActors++;
        }
        else
        {
            InvalidActors++;
        }
    }
    
    FString Details = FString::Printf(TEXT("Valid: %d, Invalid: %d, Total: %d, Dinosaurs: %d"), 
                                    ValidActors, InvalidActors, TotalActorCount, DinosaurCount);
    
    EBuild_ValidationResult Result = (InvalidActors > 10) ? EBuild_ValidationResult::Warning : EBuild_ValidationResult::Pass;
    
    AddValidationResult(TEXT("Actor Integrity"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

void UBuild_IntegrationValidator::ValidatePerformanceMetrics()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Get current FPS
    float CurrentFPS = 0.0f;
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentFPS = 1.0f / FApp::GetDeltaTime();
    }
    
    // Memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    FString Details = FString::Printf(TEXT("FPS: %.1f, Memory: %.1f MB, Actors: %d"), 
                                    CurrentFPS, MemoryUsageMB, TotalActorCount);
    
    EBuild_ValidationResult Result = EBuild_ValidationResult::Pass;
    if (CurrentFPS < 30.0f)
    {
        Result = EBuild_ValidationResult::Warning;
    }
    if (CurrentFPS < 15.0f || MemoryUsageMB > 4096.0f)
    {
        Result = EBuild_ValidationResult::Fail;
    }
    
    AddValidationResult(TEXT("Performance Metrics"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

void UBuild_IntegrationValidator::ValidateSystemDependencies()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Check critical subsystems
    TArray<FString> MissingSubsystems;
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Check for game mode
        if (!World->GetAuthGameMode())
        {
            MissingSubsystems.Add(TEXT("GameMode"));
        }
        
        // Check for player controller
        if (!World->GetFirstPlayerController())
        {
            MissingSubsystems.Add(TEXT("PlayerController"));
        }
    }
    
    FString Details = MissingSubsystems.Num() > 0 ? 
                     FString::Printf(TEXT("Missing: %s"), *FString::Join(MissingSubsystems, TEXT(", "))) :
                     TEXT("All critical subsystems present");
    
    EBuild_ValidationResult Result = MissingSubsystems.Num() > 0 ? EBuild_ValidationResult::Warning : EBuild_ValidationResult::Pass;
    
    AddValidationResult(TEXT("System Dependencies"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

void UBuild_IntegrationValidator::CountActorsByType()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TotalActorCount = 0;
    DinosaurCount = 0;
    
    TArray<FString> DinosaurLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
                                     TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            TotalActorCount++;
            
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            for (const FString& DinoLabel : DinosaurLabels)
            {
                if (ActorLabel.Contains(DinoLabel))
                {
                    DinosaurCount++;
                    break;
                }
            }
        }
    }
}

void UBuild_IntegrationValidator::EnforceActorLimits()
{
    double StartTime = FPlatformTime::Seconds();
    
    EnforceDinosaurLimit();
    EnforceTotalActorLimit();
    
    AddValidationResult(TEXT("Actor Limits"), EBuild_ValidationResult::Pass, 
                      FString::Printf(TEXT("Enforced limits - Total: %d, Dinosaurs: %d"), TotalActorCount, DinosaurCount),
                      FPlatformTime::Seconds() - StartTime);
}

void UBuild_IntegrationValidator::EnforceDinosaurLimit()
{
    if (DinosaurCount <= 150) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> DinosaurActors;
    TArray<FString> DinosaurLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
                                     TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            for (const FString& DinoLabel : DinosaurLabels)
            {
                if (ActorLabel.Contains(DinoLabel))
                {
                    DinosaurActors.Add(Actor);
                    break;
                }
            }
        }
    }
    
    // Remove excess dinosaurs randomly
    while (DinosaurActors.Num() > 150)
    {
        int32 RandomIndex = FMath::RandRange(0, DinosaurActors.Num() - 1);
        AActor* ActorToRemove = DinosaurActors[RandomIndex];
        if (ActorToRemove)
        {
            ActorToRemove->Destroy();
        }
        DinosaurActors.RemoveAt(RandomIndex);
    }
    
    DinosaurCount = DinosaurActors.Num();
}

void UBuild_IntegrationValidator::EnforceTotalActorLimit()
{
    if (TotalActorCount <= 8000) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> NonEssentialActors;
    TArray<FString> EssentialLabels = {TEXT("playerstart"), TEXT("directionallight"), TEXT("skylight"), 
                                      TEXT("skyatmosphere"), TEXT("fog")};
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            bool bIsEssential = false;
            
            for (const FString& EssentialLabel : EssentialLabels)
            {
                if (ActorLabel.Contains(EssentialLabel))
                {
                    bIsEssential = true;
                    break;
                }
            }
            
            if (!bIsEssential)
            {
                NonEssentialActors.Add(Actor);
            }
        }
    }
    
    // Remove excess non-essential actors
    int32 ActorsToRemove = TotalActorCount - 7000;
    while (NonEssentialActors.Num() > 0 && ActorsToRemove > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, NonEssentialActors.Num() - 1);
        AActor* ActorToRemove = NonEssentialActors[RandomIndex];
        if (ActorToRemove)
        {
            ActorToRemove->Destroy();
            ActorsToRemove--;
        }
        NonEssentialActors.RemoveAt(RandomIndex);
    }
    
    // Recount after cleanup
    CountActorsByType();
}

void UBuild_IntegrationValidator::CleanupOrphanedActors()
{
    double StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 OrphanedCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !IsValid(Actor))
        {
            OrphanedCount++;
        }
    }
    
    AddValidationResult(TEXT("Orphaned Cleanup"), EBuild_ValidationResult::Pass, 
                      FString::Printf(TEXT("Found %d orphaned actors"), OrphanedCount),
                      FPlatformTime::Seconds() - StartTime);
}

void UBuild_IntegrationValidator::GenerateIntegrationReport()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("=== INTEGRATION REPORT ==="));
    UE_LOG(LogTranspersonalGame, Log, TEXT("Total Actors: %d"), TotalActorCount);
    UE_LOG(LogTranspersonalGame, Log, TEXT("Dinosaur Count: %d"), DinosaurCount);
    UE_LOG(LogTranspersonalGame, Log, TEXT("System Healthy: %s"), bSystemHealthy ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTranspersonalGame, Log, TEXT("Last Validation Time: %.3f seconds"), LastValidationTime);
    
    for (const FBuild_ValidationReport& Report : ValidationResults)
    {
        FString ResultStr;
        switch (Report.Result)
        {
            case EBuild_ValidationResult::Pass: ResultStr = TEXT("PASS"); break;
            case EBuild_ValidationResult::Warning: ResultStr = TEXT("WARN"); break;
            case EBuild_ValidationResult::Fail: ResultStr = TEXT("FAIL"); break;
            case EBuild_ValidationResult::Critical: ResultStr = TEXT("CRIT"); break;
        }
        
        UE_LOG(LogTranspersonalGame, Log, TEXT("[%s] %s: %s (%.3fs)"), 
               *ResultStr, *Report.TestName, *Report.Details, Report.ExecutionTime);
    }
    UE_LOG(LogTranspersonalGame, Log, TEXT("=== END REPORT ==="));
}

TArray<FBuild_ValidationReport> UBuild_IntegrationValidator::GetValidationResults() const
{
    return ValidationResults;
}

bool UBuild_IntegrationValidator::IsSystemHealthy() const
{
    return bSystemHealthy;
}

void UBuild_IntegrationValidator::AddValidationResult(const FString& TestName, EBuild_ValidationResult Result, const FString& Details, float ExecutionTime)
{
    FBuild_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Details = Details;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationResults.Add(Report);
}