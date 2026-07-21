#include "Build_IntegrationReport.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuild_IntegrationReport::UBuild_IntegrationReport()
{
    ReportTimestamp = FDateTime::Now();
    CycleID = TEXT("UNKNOWN");
    OverallStatus = EBuild_ValidationStatus::Unknown;
}

void UBuild_IntegrationReport::GenerateReport()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationReport: Starting report generation"));
    
    ReportTimestamp = FDateTime::Now();
    CriticalErrors.Empty();
    Warnings.Empty();
    Recommendations.Empty();
    ModuleStatuses.Empty();

    // Validate all modules
    ValidateAllModules();
    
    // Calculate metrics
    CalculateMetrics();
    
    // Check system health
    CheckSystemHealth();
    
    // Generate recommendations
    GenerateRecommendations();

    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationReport: Report generation complete"));
}

bool UBuild_IntegrationReport::ValidateModuleIntegrity(const FString& ModuleName)
{
    FBuild_ModuleStatus ModuleStatus;
    ModuleStatus.ModuleName = ModuleName;
    
    // Check if module classes can be loaded
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
    UClass* TestClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (TestClass)
    {
        ModuleStatus.CompilationStatus = EBuild_ValidationStatus::Passed;
        ModuleStatus.LinkingStatus = EBuild_ValidationStatus::Passed;
        ModuleStatus.RuntimeStatus = EBuild_ValidationStatus::Passed;
        ModuleStatus.ClassCount = 1;
        ModuleStatus.ErrorCount = 0;
    }
    else
    {
        ModuleStatus.CompilationStatus = EBuild_ValidationStatus::Failed;
        ModuleStatus.LinkingStatus = EBuild_ValidationStatus::Failed;
        ModuleStatus.RuntimeStatus = EBuild_ValidationStatus::Failed;
        ModuleStatus.ClassCount = 0;
        ModuleStatus.ErrorCount = 1;
        ModuleStatus.ErrorMessages.Add(FString::Printf(TEXT("Failed to load class: %s"), *ClassPath));
    }

    ModuleStatuses.Add(ModuleStatus);
    return ModuleStatus.CompilationStatus == EBuild_ValidationStatus::Passed;
}

FBuild_ModuleStatus UBuild_IntegrationReport::GetModuleStatus(const FString& ModuleName)
{
    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        if (Status.ModuleName == ModuleName)
        {
            return Status;
        }
    }
    
    // Return default status if not found
    FBuild_ModuleStatus DefaultStatus;
    DefaultStatus.ModuleName = ModuleName;
    DefaultStatus.CompilationStatus = EBuild_ValidationStatus::Unknown;
    return DefaultStatus;
}

void UBuild_IntegrationReport::AddCriticalError(const FString& ErrorMessage)
{
    CriticalErrors.AddUnique(ErrorMessage);
    UE_LOG(LogTemp, Error, TEXT("Build_IntegrationReport: Critical Error - %s"), *ErrorMessage);
}

void UBuild_IntegrationReport::AddWarning(const FString& WarningMessage)
{
    Warnings.AddUnique(WarningMessage);
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationReport: Warning - %s"), *WarningMessage);
}

void UBuild_IntegrationReport::AddRecommendation(const FString& RecommendationMessage)
{
    Recommendations.AddUnique(RecommendationMessage);
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationReport: Recommendation - %s"), *RecommendationMessage);
}

bool UBuild_IntegrationReport::IsSystemHealthy()
{
    return CriticalErrors.Num() == 0 && 
           OverallStatus == EBuild_ValidationStatus::Passed &&
           Metrics.FailedModules == 0;
}

float UBuild_IntegrationReport::GetOverallHealthScore()
{
    if (Metrics.TotalModules == 0)
    {
        return 0.0f;
    }

    float CompilationScore = (float)Metrics.CompiledModules / (float)Metrics.TotalModules;
    float ErrorPenalty = FMath::Min(1.0f, (float)CriticalErrors.Num() * 0.1f);
    float WarningPenalty = FMath::Min(0.5f, (float)Warnings.Num() * 0.05f);

    return FMath::Max(0.0f, CompilationScore - ErrorPenalty - WarningPenalty);
}

void UBuild_IntegrationReport::ValidateAllModules()
{
    TArray<FString> CoreModules = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };

    for (const FString& ModuleName : CoreModules)
    {
        ValidateModuleIntegrity(ModuleName);
    }
}

void UBuild_IntegrationReport::CalculateMetrics()
{
    Metrics.TotalModules = ModuleStatuses.Num();
    Metrics.CompiledModules = 0;
    Metrics.FailedModules = 0;
    Metrics.TotalClasses = 0;
    Metrics.LoadedClasses = 0;

    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        if (Status.CompilationStatus == EBuild_ValidationStatus::Passed)
        {
            Metrics.CompiledModules++;
        }
        else
        {
            Metrics.FailedModules++;
        }

        Metrics.TotalClasses += Status.ClassCount;
        if (Status.RuntimeStatus == EBuild_ValidationStatus::Passed)
        {
            Metrics.LoadedClasses += Status.ClassCount;
        }
    }

    // Get actor count from current world
    if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
    {
        Metrics.TotalActors = World->GetActorCount();
    }

    Metrics.BuildTime = 0.0f; // Would need to be tracked externally
    Metrics.MemoryUsage = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f); // MB
}

void UBuild_IntegrationReport::CheckSystemHealth()
{
    if (Metrics.FailedModules > 0)
    {
        OverallStatus = EBuild_ValidationStatus::Failed;
        AddCriticalError(FString::Printf(TEXT("System has %d failed modules"), Metrics.FailedModules));
    }
    else if (Metrics.CompiledModules == Metrics.TotalModules)
    {
        OverallStatus = EBuild_ValidationStatus::Passed;
    }
    else
    {
        OverallStatus = EBuild_ValidationStatus::InProgress;
        AddWarning(TEXT("Some modules are still being validated"));
    }

    // Check memory usage
    if (Metrics.MemoryUsage > 8192.0f) // 8GB
    {
        AddWarning(FString::Printf(TEXT("High memory usage: %.2f MB"), Metrics.MemoryUsage));
    }

    // Check actor count
    if (Metrics.TotalActors > 20000)
    {
        AddCriticalError(FString::Printf(TEXT("Actor count exceeds limit: %d > 20000"), Metrics.TotalActors));
    }
    else if (Metrics.TotalActors > 15000)
    {
        AddWarning(FString::Printf(TEXT("Actor count approaching limit: %d"), Metrics.TotalActors));
    }
}

void UBuild_IntegrationReport::GenerateRecommendations()
{
    if (Metrics.FailedModules > 0)
    {
        AddRecommendation(TEXT("Review failed module compilation errors and fix missing dependencies"));
    }

    if (Metrics.TotalActors > 15000)
    {
        AddRecommendation(TEXT("Consider implementing actor pooling or LOD systems to reduce actor count"));
    }

    if (Metrics.MemoryUsage > 4096.0f)
    {
        AddRecommendation(TEXT("Optimize memory usage by reducing texture sizes or implementing streaming"));
    }

    if (CriticalErrors.Num() == 0 && Warnings.Num() == 0)
    {
        AddRecommendation(TEXT("System is healthy - continue with planned development"));
    }
}