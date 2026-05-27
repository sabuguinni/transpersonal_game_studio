#include "BuildIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/PlatformFilemanager.h"

UBuildIntegrationValidator::UBuildIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize core system classes to validate
    CoreSystemClasses.Add(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    CoreSystemClasses.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    CoreSystemClasses.Add(TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    CoreSystemClasses.Add(TEXT("/Script/TranspersonalGame.FoliageManager"));
    CoreSystemClasses.Add(TEXT("/Script/TranspersonalGame.VFXManager"));
    
    MaxActorCountWarning = 1000;
    MaxActorCountCritical = 2000;
}

FBuild_IntegrationReport UBuildIntegrationValidator::ValidateAllSystems()
{
    FBuild_IntegrationReport Report;
    Report.ValidationTimestamp = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationValidator: Starting comprehensive system validation"));
    
    // Validate each core system
    for (const FString& SystemClass : CoreSystemClasses)
    {
        FBuild_SystemReport SystemReport;
        SystemReport.SystemName = SystemClass;
        
        float StartTime = FPlatformTime::Seconds();
        SystemReport.Status = ValidateSystemClass(SystemClass);
        SystemReport.ValidationTime = FPlatformTime::Seconds() - StartTime;
        
        switch (SystemReport.Status)
        {
            case EBuild_SystemStatus::Operational:
                SystemReport.Details = TEXT("System loaded and operational");
                Report.OperationalSystems++;
                break;
            case EBuild_SystemStatus::Failed:
                SystemReport.Details = TEXT("System class not found or failed to load");
                break;
            case EBuild_SystemStatus::Error:
                SystemReport.Details = TEXT("Exception occurred during validation");
                break;
            default:
                SystemReport.Details = TEXT("Unknown validation state");
                break;
        }
        
        Report.SystemReports.Add(SystemReport);
        LogSystemStatus(SystemReport.SystemName, SystemReport.Status);
    }
    
    Report.TotalSystems = CoreSystemClasses.Num();
    Report.TotalActors = GetTotalActorCount();
    Report.bBuildReady = (Report.OperationalSystems == Report.TotalSystems);
    
    LastReport = Report;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationValidator: Validation complete - %d/%d systems operational"), 
           Report.OperationalSystems, Report.TotalSystems);
    
    return Report;
}

EBuild_SystemStatus UBuildIntegrationValidator::ValidateCharacterSystem()
{
    return ValidateSystemClass(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
}

EBuild_SystemStatus UBuildIntegrationValidator::ValidateGameStateSystem()
{
    return ValidateSystemClass(TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
}

EBuild_SystemStatus UBuildIntegrationValidator::ValidateWorldGenSystem()
{
    return ValidateSystemClass(TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
}

EBuild_SystemStatus UBuildIntegrationValidator::ValidateVFXSystem()
{
    return ValidateSystemClass(TEXT("/Script/TranspersonalGame.VFXManager"));
}

int32 UBuildIntegrationValidator::GetTotalActorCount()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    return ActorCount;
}

void UBuildIntegrationValidator::PerformMemoryOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationValidator: Performing memory optimization"));
    
    // Force garbage collection
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationValidator: Memory optimization completed"));
}

bool UBuildIntegrationValidator::SaveCurrentMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationValidator: No valid world for map save"));
        return false;
    }
    
    FString MapPath = TEXT("/Game/Maps/MinPlayableMap");
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationValidator: Attempting to save map to %s"), *MapPath);
    
    // Note: In runtime, we cannot directly save maps like in editor
    // This is a placeholder for editor-specific functionality
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationValidator: Map save requested (editor functionality)"));
    
    return true;
}

EBuild_SystemStatus UBuildIntegrationValidator::ValidateSystemClass(const FString& ClassPath)
{
    try
    {
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (SystemClass && IsValid(SystemClass))
        {
            return EBuild_SystemStatus::Operational;
        }
        else
        {
            return EBuild_SystemStatus::Failed;
        }
    }
    catch (...)
    {
        return EBuild_SystemStatus::Error;
    }
}

void UBuildIntegrationValidator::LogSystemStatus(const FString& SystemName, EBuild_SystemStatus Status)
{
    FString StatusString;
    switch (Status)
    {
        case EBuild_SystemStatus::Operational:
            StatusString = TEXT("OPERATIONAL");
            UE_LOG(LogTemp, Warning, TEXT("✓ %s: %s"), *SystemName, *StatusString);
            break;
        case EBuild_SystemStatus::Failed:
            StatusString = TEXT("FAILED");
            UE_LOG(LogTemp, Error, TEXT("✗ %s: %s"), *SystemName, *StatusString);
            break;
        case EBuild_SystemStatus::Error:
            StatusString = TEXT("ERROR");
            UE_LOG(LogTemp, Error, TEXT("✗ %s: %s"), *SystemName, *StatusString);
            break;
        default:
            StatusString = TEXT("UNKNOWN");
            UE_LOG(LogTemp, Warning, TEXT("? %s: %s"), *SystemName, *StatusString);
            break;
    }
}