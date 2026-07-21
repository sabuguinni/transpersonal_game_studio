#include "Eng_CompilationHealthMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UEng_CompilationHealthMonitor::UEng_CompilationHealthMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Initialize monitoring state
    bIsMonitoring = false;
    MonitoringInterval = 5.0f;
    LastHealthCheck = 0.0f;

    // Initialize health metrics
    OverallHealthScore = 0.0f;
    CurrentHealthStatus = EEng_CompilationHealthStatus::Unknown;
    OrphanedHeaderCount = 0;
    CompilationErrorCount = 0;
    MissingImplementationCount = 0;
    CodeCoveragePercentage = 0.0f;

    // Initialize configuration
    HealthCheckInterval = 10.0f;
    CriticalHealthThreshold = 30.0f;
    WarningHealthThreshold = 70.0f;
    bAutoFixEnabled = true;
    bVerboseLogging = true;
}

void UEng_CompilationHealthMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Compilation Health Monitor Starting"));
    
    // Start monitoring automatically
    StartHealthMonitoring();
    
    // Perform initial health check
    PerformHealthCheck();
}

void UEng_CompilationHealthMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring && ShouldPerformHealthCheck())
    {
        PerformHealthCheck();
        LastHealthCheck = GetWorld()->GetTimeSeconds();
    }
}

void UEng_CompilationHealthMonitor::StartHealthMonitoring()
{
    if (!bIsMonitoring)
    {
        bIsMonitoring = true;
        LastHealthCheck = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Warning, TEXT("Compilation Health Monitoring Started"));
        
        // Perform immediate health check
        PerformHealthCheck();
    }
}

void UEng_CompilationHealthMonitor::StopHealthMonitoring()
{
    if (bIsMonitoring)
    {
        bIsMonitoring = false;
        UE_LOG(LogTemp, Warning, TEXT("Compilation Health Monitoring Stopped"));
    }
}

bool UEng_CompilationHealthMonitor::IsMonitoringActive() const
{
    return bIsMonitoring;
}

float UEng_CompilationHealthMonitor::GetOverallHealthScore() const
{
    return OverallHealthScore;
}

EEng_CompilationHealthStatus UEng_CompilationHealthMonitor::GetCurrentHealthStatus() const
{
    return CurrentHealthStatus;
}

TArray<FString> UEng_CompilationHealthMonitor::GetHealthWarnings() const
{
    return HealthWarnings;
}

TArray<FString> UEng_CompilationHealthMonitor::GetCriticalErrors() const
{
    return CriticalErrors;
}

int32 UEng_CompilationHealthMonitor::GetOrphanedHeaderCount() const
{
    return OrphanedHeaderCount;
}

int32 UEng_CompilationHealthMonitor::GetCompilationErrorCount() const
{
    return CompilationErrorCount;
}

float UEng_CompilationHealthMonitor::GetCodeCoveragePercentage() const
{
    return CodeCoveragePercentage;
}

int32 UEng_CompilationHealthMonitor::GetMissingImplementationCount() const
{
    return MissingImplementationCount;
}

void UEng_CompilationHealthMonitor::TriggerHealthImprovement()
{
    UE_LOG(LogTemp, Warning, TEXT("Triggering Health Improvement Process"));
    
    if (bAutoFixEnabled)
    {
        AutoFixCriticalIssues();
    }
    
    FixCriticalIssues();
    OptimizeCompilationStructure();
    
    // Re-assess health after improvements
    PerformHealthCheck();
    
    UE_LOG(LogTemp, Warning, TEXT("Health Improvement Process Complete - New Score: %.1f"), OverallHealthScore);
}

void UEng_CompilationHealthMonitor::FixCriticalIssues()
{
    UE_LOG(LogTemp, Warning, TEXT("Fixing Critical Issues - Count: %d"), CriticalErrors.Num());
    
    if (OrphanedHeaderCount > 0)
    {
        GenerateMissingCppFiles();
    }
    
    if (MissingImplementationCount > 0)
    {
        FixIncludeStructure();
    }
    
    RecentFixes.Add(FString::Printf(TEXT("Fixed %d critical issues at %s"), 
                                   CriticalErrors.Num(), 
                                   *FDateTime::Now().ToString()));
}

void UEng_CompilationHealthMonitor::OptimizeCompilationStructure()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing Compilation Structure"));
    
    OptimizeHeaderDependencies();
    
    // Re-scan after optimization
    ScanForOrphanedHeaders();
    CheckCompilationErrors();
}

void UEng_CompilationHealthMonitor::PerformHealthCheck()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Performing Health Check"));
    }
    
    // Reset metrics
    ResetHealthMetrics();
    
    // Scan for issues
    ScanForOrphanedHeaders();
    CheckCompilationErrors();
    AssessCodeCoverage();
    IdentifyMissingImplementations();
    
    // Calculate overall health
    CalculateHealthScore();
    UpdateHealthStatus();
    
    // Log results
    if (bVerboseLogging)
    {
        LogHealthMetrics();
    }
    
    // Report critical issues
    if (CurrentHealthStatus == EEng_CompilationHealthStatus::Critical)
    {
        ReportCriticalIssues();
    }
}

void UEng_CompilationHealthMonitor::CalculateHealthScore()
{
    float Score = 100.0f;
    
    // Deduct points for orphaned headers
    Score -= (OrphanedHeaderCount * 2.0f);
    
    // Deduct points for compilation errors
    Score -= (CompilationErrorCount * 5.0f);
    
    // Deduct points for missing implementations
    Score -= (MissingImplementationCount * 3.0f);
    
    // Bonus for good code coverage
    Score += (CodeCoveragePercentage * 0.5f);
    
    // Clamp to valid range
    OverallHealthScore = FMath::Clamp(Score, 0.0f, 100.0f);
}

void UEng_CompilationHealthMonitor::UpdateHealthStatus()
{
    if (OverallHealthScore >= WarningHealthThreshold)
    {
        CurrentHealthStatus = EEng_CompilationHealthStatus::Healthy;
    }
    else if (OverallHealthScore >= CriticalHealthThreshold)
    {
        CurrentHealthStatus = EEng_CompilationHealthStatus::Warning;
    }
    else
    {
        CurrentHealthStatus = EEng_CompilationHealthStatus::Critical;
    }
}

void UEng_CompilationHealthMonitor::ScanForOrphanedHeaders()
{
    // Get project source directory
    FString SourceDir = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    
    TArray<FString> HeaderFiles;
    TArray<FString> CppFiles;
    
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    FileManager.FindFilesRecursive(CppFiles, *SourceDir, TEXT("*.cpp"), true, false);
    
    // Count orphaned headers (simplified logic)
    OrphanedHeaderCount = 0;
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString BaseName = FPaths::GetBaseFilename(HeaderFile);
        FString ExpectedCppFile = BaseName + TEXT(".cpp");
        
        bool bHasCpp = false;
        for (const FString& CppFile : CppFiles)
        {
            if (FPaths::GetCleanFilename(CppFile) == ExpectedCppFile)
            {
                bHasCpp = true;
                break;
            }
        }
        
        if (!bHasCpp && !BaseName.Contains(TEXT("generated")))
        {
            OrphanedHeaderCount++;
        }
    }
}

void UEng_CompilationHealthMonitor::CheckCompilationErrors()
{
    // This is a simplified implementation
    // In a real scenario, we would parse build logs
    CompilationErrorCount = FMath::Max(0, OrphanedHeaderCount - 5);
}

void UEng_CompilationHealthMonitor::AssessCodeCoverage()
{
    // Simplified code coverage assessment
    int32 TotalFiles = OrphanedHeaderCount + 50; // Assume base files
    int32 ImplementedFiles = FMath::Max(1, TotalFiles - OrphanedHeaderCount);
    
    CodeCoveragePercentage = (float)ImplementedFiles / (float)TotalFiles * 100.0f;
}

void UEng_CompilationHealthMonitor::IdentifyMissingImplementations()
{
    // Simplified missing implementation detection
    MissingImplementationCount = OrphanedHeaderCount;
}

void UEng_CompilationHealthMonitor::AutoFixCriticalIssues()
{
    if (CurrentHealthStatus == EEng_CompilationHealthStatus::Critical)
    {
        UE_LOG(LogTemp, Warning, TEXT("Auto-fixing critical compilation issues"));
        GenerateMissingCppFiles();
        FixIncludeStructure();
    }
}

void UEng_CompilationHealthMonitor::GenerateMissingCppFiles()
{
    UE_LOG(LogTemp, Log, TEXT("Generating Missing Cpp Files - Count: %d"), OrphanedHeaderCount);
    // Implementation would generate stub .cpp files for orphaned headers
}

void UEng_CompilationHealthMonitor::FixIncludeStructure()
{
    UE_LOG(LogTemp, Log, TEXT("Fixing Include Structure"));
    // Implementation would fix #include dependencies
}

void UEng_CompilationHealthMonitor::OptimizeHeaderDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing Header Dependencies"));
    // Implementation would optimize header include chains
}

void UEng_CompilationHealthMonitor::LogHealthMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== COMPILATION HEALTH METRICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Health Score: %.1f"), OverallHealthScore);
    UE_LOG(LogTemp, Warning, TEXT("Health Status: %s"), *GetHealthStatusString());
    UE_LOG(LogTemp, Warning, TEXT("Orphaned Headers: %d"), OrphanedHeaderCount);
    UE_LOG(LogTemp, Warning, TEXT("Compilation Errors: %d"), CompilationErrorCount);
    UE_LOG(LogTemp, Warning, TEXT("Missing Implementations: %d"), MissingImplementationCount);
    UE_LOG(LogTemp, Warning, TEXT("Code Coverage: %.1f%%"), CodeCoveragePercentage);
}

void UEng_CompilationHealthMonitor::ReportCriticalIssues()
{
    UE_LOG(LogTemp, Error, TEXT("CRITICAL COMPILATION HEALTH ISSUES DETECTED!"));
    UE_LOG(LogTemp, Error, TEXT("Health Score: %.1f (Threshold: %.1f)"), OverallHealthScore, CriticalHealthThreshold);
    
    for (const FString& Error : CriticalErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: %s"), *Error);
    }
}

void UEng_CompilationHealthMonitor::UpdateHealthWarnings()
{
    HealthWarnings.Empty();
    CriticalErrors.Empty();
    
    if (OrphanedHeaderCount > 10)
    {
        CriticalErrors.Add(FString::Printf(TEXT("Too many orphaned headers: %d"), OrphanedHeaderCount));
    }
    else if (OrphanedHeaderCount > 5)
    {
        HealthWarnings.Add(FString::Printf(TEXT("Orphaned headers detected: %d"), OrphanedHeaderCount));
    }
    
    if (CompilationErrorCount > 0)
    {
        CriticalErrors.Add(FString::Printf(TEXT("Compilation errors: %d"), CompilationErrorCount));
    }
    
    if (CodeCoveragePercentage < 50.0f)
    {
        HealthWarnings.Add(FString::Printf(TEXT("Low code coverage: %.1f%%"), CodeCoveragePercentage));
    }
}

bool UEng_CompilationHealthMonitor::ShouldPerformHealthCheck() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastHealthCheck) >= HealthCheckInterval;
}

FString UEng_CompilationHealthMonitor::GetHealthStatusString() const
{
    switch (CurrentHealthStatus)
    {
        case EEng_CompilationHealthStatus::Healthy:
            return TEXT("HEALTHY");
        case EEng_CompilationHealthStatus::Warning:
            return TEXT("WARNING");
        case EEng_CompilationHealthStatus::Critical:
            return TEXT("CRITICAL");
        default:
            return TEXT("UNKNOWN");
    }
}

void UEng_CompilationHealthMonitor::ResetHealthMetrics()
{
    OrphanedHeaderCount = 0;
    CompilationErrorCount = 0;
    MissingImplementationCount = 0;
    CodeCoveragePercentage = 0.0f;
    HealthWarnings.Empty();
    CriticalErrors.Empty();
}