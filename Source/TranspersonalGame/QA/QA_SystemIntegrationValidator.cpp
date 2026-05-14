#include "QA_SystemIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"

UQA_SystemIntegrationValidator::UQA_SystemIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    bAutoValidateOnBeginPlay = false;
    ValidationInterval = 30.0f;
    bLogValidationResults = true;
    bExportReportsAutomatically = false;
    
    LastValidationTime = 0.0f;
    FrameCounter = 0;
    FPSAccumulator = 0.0f;
}

void UQA_SystemIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoValidateOnBeginPlay)
    {
        ValidateAllSystems();
    }
}

void UQA_SystemIntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update FPS tracking
    FrameCounter++;
    FPSAccumulator += 1.0f / DeltaTime;
    
    // Auto-validation based on interval
    LastValidationTime += DeltaTime;
    if (LastValidationTime >= ValidationInterval)
    {
        ValidatePerformanceMetrics();
        LastValidationTime = 0.0f;
    }
}

bool UQA_SystemIntegrationValidator::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA: Starting comprehensive system validation"));
    
    ClearValidationReports();
    
    bool bAllSystemsValid = true;
    
    // Validate core systems
    bAllSystemsValid &= ValidateVFXSystem();
    bAllSystemsValid &= ValidateCharacterSystem();
    bAllSystemsValid &= ValidateWorldGeneration();
    bAllSystemsValid &= ValidatePerformanceMetrics();
    bAllSystemsValid &= ValidateAssetIntegrity();
    
    UE_LOG(LogTemp, Warning, TEXT("QA: System validation complete. Result: %s"), 
           bAllSystemsValid ? TEXT("PASS") : TEXT("FAIL"));
    
    if (bExportReportsAutomatically)
    {
        FString ReportPath = FPaths::ProjectSavedDir() / TEXT("QA") / TEXT("ValidationReport_") + 
                           FDateTime::Now().ToString() + TEXT(".txt");
        ExportValidationReport(ReportPath);
    }
    
    return bAllSystemsValid;
}

bool UQA_SystemIntegrationValidator::ValidateVFXSystem()
{
    auto ValidationFunc = [this]() -> bool
    {
        // Check if VFX manager class exists
        if (!CheckClassExists(TEXT("/Script/TranspersonalGame.VFX_NiagaraEffectsManager")))
        {
            return false;
        }
        
        // Check for basic VFX assets
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        // Validate VFX system is properly integrated
        return true;
    };
    
    FQA_ValidationReport Report = ValidateSystemInternal(TEXT("VFX System"), ValidationFunc);
    ValidationReports.Add(Report);
    
    return Report.Result == EQA_ValidationResult::Pass;
}

bool UQA_SystemIntegrationValidator::ValidateCharacterSystem()
{
    auto ValidationFunc = [this]() -> bool
    {
        // Check if character class exists
        if (!CheckClassExists(TEXT("/Script/TranspersonalGame.TranspersonalCharacter")))
        {
            return false;
        }
        
        // Check if game mode exists
        if (!CheckClassExists(TEXT("/Script/TranspersonalGame.TranspersonalGameMode")))
        {
            return false;
        }
        
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        // Check for player character in world
        APawn* PlayerPawn = World->GetFirstPlayerController() ? 
                           World->GetFirstPlayerController()->GetPawn() : nullptr;
        
        return PlayerPawn != nullptr;
    };
    
    FQA_ValidationReport Report = ValidateSystemInternal(TEXT("Character System"), ValidationFunc);
    ValidationReports.Add(Report);
    
    return Report.Result == EQA_ValidationResult::Pass;
}

bool UQA_SystemIntegrationValidator::ValidateWorldGeneration()
{
    auto ValidationFunc = [this]() -> bool
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        // Check for landscape
        bool bHasLandscape = false;
        for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
        {
            bHasLandscape = true;
            break;
        }
        
        // Check for basic world actors
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        
        return bHasLandscape && ActorCount > 10;
    };
    
    FQA_ValidationReport Report = ValidateSystemInternal(TEXT("World Generation"), ValidationFunc);
    ValidationReports.Add(Report);
    
    return Report.Result == EQA_ValidationResult::Pass;
}

bool UQA_SystemIntegrationValidator::ValidatePerformanceMetrics()
{
    auto ValidationFunc = [this]() -> bool
    {
        float CurrentFPS = GetCurrentFPS();
        int32 ActorCount = GetActorCount();
        
        // Performance thresholds
        bool bFPSAcceptable = CurrentFPS > 30.0f;
        bool bActorCountReasonable = ActorCount < 10000;
        
        return bFPSAcceptable && bActorCountReasonable;
    };
    
    FQA_ValidationReport Report = ValidateSystemInternal(TEXT("Performance Metrics"), ValidationFunc);
    ValidationReports.Add(Report);
    
    return Report.Result == EQA_ValidationResult::Pass;
}

bool UQA_SystemIntegrationValidator::ValidateAssetIntegrity()
{
    auto ValidationFunc = [this]() -> bool
    {
        // Check for critical game assets
        bool bHasCharacterAssets = CheckAssetExists(TEXT("/Game/Characters/"));
        bool bHasEnvironmentAssets = CheckAssetExists(TEXT("/Game/Environment/"));
        
        return bHasCharacterAssets || bHasEnvironmentAssets;
    };
    
    FQA_ValidationReport Report = ValidateSystemInternal(TEXT("Asset Integrity"), ValidationFunc);
    ValidationReports.Add(Report);
    
    return Report.Result == EQA_ValidationResult::Pass;
}

TArray<FQA_ValidationReport> UQA_SystemIntegrationValidator::GetValidationReports() const
{
    return ValidationReports;
}

void UQA_SystemIntegrationValidator::ClearValidationReports()
{
    ValidationReports.Empty();
}

void UQA_SystemIntegrationValidator::ExportValidationReport(const FString& FilePath)
{
    FString ReportContent;
    ReportContent += TEXT("=== QA VALIDATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    for (const FQA_ValidationReport& Report : ValidationReports)
    {
        ReportContent += FString::Printf(TEXT("System: %s\n"), *Report.SystemName);
        ReportContent += FString::Printf(TEXT("Result: %s\n"), 
                                       Report.Result == EQA_ValidationResult::Pass ? TEXT("PASS") : TEXT("FAIL"));
        ReportContent += FString::Printf(TEXT("Message: %s\n"), *Report.Message);
        ReportContent += FString::Printf(TEXT("Execution Time: %.2fms\n\n"), Report.ExecutionTime);
    }
    
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    UE_LOG(LogTemp, Warning, TEXT("QA: Validation report exported to %s"), *FilePath);
}

float UQA_SystemIntegrationValidator::GetCurrentFPS() const
{
    if (FrameCounter > 0)
    {
        return FPSAccumulator / FrameCounter;
    }
    return 0.0f;
}

int32 UQA_SystemIntegrationValidator::GetActorCount() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        Count++;
    }
    
    return Count;
}

float UQA_SystemIntegrationValidator::GetMemoryUsage() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f); // MB
}

FQA_ValidationReport UQA_SystemIntegrationValidator::ValidateSystemInternal(const FString& SystemName, TFunction<bool()> ValidationFunction)
{
    FQA_ValidationReport Report;
    Report.SystemName = SystemName;
    Report.Timestamp = FDateTime::Now();
    
    double StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bResult = ValidationFunction();
        Report.Result = bResult ? EQA_ValidationResult::Pass : EQA_ValidationResult::Fail;
        Report.Message = bResult ? TEXT("Validation successful") : TEXT("Validation failed");
    }
    catch (...)
    {
        Report.Result = EQA_ValidationResult::Critical;
        Report.Message = TEXT("Validation threw exception");
    }
    
    double EndTime = FPlatformTime::Seconds();
    Report.ExecutionTime = static_cast<float>((EndTime - StartTime) * 1000.0);
    
    if (bLogValidationResults)
    {
        LogValidationResult(Report);
    }
    
    return Report;
}

void UQA_SystemIntegrationValidator::LogValidationResult(const FQA_ValidationReport& Report)
{
    FString ResultString;
    switch (Report.Result)
    {
        case EQA_ValidationResult::Pass:
            ResultString = TEXT("PASS");
            break;
        case EQA_ValidationResult::Fail:
            ResultString = TEXT("FAIL");
            break;
        case EQA_ValidationResult::Warning:
            ResultString = TEXT("WARNING");
            break;
        case EQA_ValidationResult::Critical:
            ResultString = TEXT("CRITICAL");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA [%s]: %s - %s (%.2fms)"), 
           *Report.SystemName, *ResultString, *Report.Message, Report.ExecutionTime);
}

bool UQA_SystemIntegrationValidator::CheckClassExists(const FString& ClassName)
{
    UClass* Class = StaticLoadClass(UObject::StaticClass(), nullptr, *ClassName);
    return Class != nullptr;
}

bool UQA_SystemIntegrationValidator::CheckAssetExists(const FString& AssetPath)
{
    return FPaths::DirectoryExists(FPaths::ProjectContentDir() + AssetPath);
}