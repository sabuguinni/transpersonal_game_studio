#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Eng_CompilationHealthMonitor.generated.h"

/**
 * Engine Architect - Compilation Health Monitor
 * Real-time monitoring system for compilation health and module integrity.
 * Provides continuous assessment of code quality and compilation status.
 * 
 * CYCLE 005 FOCUS: Monitor orphaned headers and enforce compilation standards
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_CompilationHealthMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_CompilationHealthMonitor();

    // UActorComponent interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Health Monitoring Interface
    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    void StartHealthMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    void StopHealthMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    bool IsMonitoringActive() const;

    // Health Assessment
    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    float GetOverallHealthScore() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    EEng_CompilationHealthStatus GetCurrentHealthStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    TArray<FString> GetHealthWarnings() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    TArray<FString> GetCriticalErrors() const;

    // Real-time Metrics
    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    int32 GetOrphanedHeaderCount() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    int32 GetCompilationErrorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    float GetCodeCoveragePercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    int32 GetMissingImplementationCount() const;

    // Health Improvement
    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    void TriggerHealthImprovement();

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    void FixCriticalIssues();

    UFUNCTION(BlueprintCallable, Category = "Compilation Health")
    void OptimizeCompilationStructure();

protected:
    // Monitoring State
    UPROPERTY(BlueprintReadOnly, Category = "Health State")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Health State")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Health State")
    float LastHealthCheck;

    // Health Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Health Metrics")
    float OverallHealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "Health Metrics")
    EEng_CompilationHealthStatus CurrentHealthStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Health Metrics")
    int32 OrphanedHeaderCount;

    UPROPERTY(BlueprintReadOnly, Category = "Health Metrics")
    int32 CompilationErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Health Metrics")
    int32 MissingImplementationCount;

    UPROPERTY(BlueprintReadOnly, Category = "Health Metrics")
    float CodeCoveragePercentage;

    // Health Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Health Tracking")
    TArray<FString> HealthWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Health Tracking")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Health Tracking")
    TArray<FString> RecentFixes;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Configuration")
    float HealthCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Configuration")
    float CriticalHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Configuration")
    float WarningHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Configuration")
    bool bAutoFixEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Configuration")
    bool bVerboseLogging;

private:
    // Internal monitoring methods
    void PerformHealthCheck();
    void CalculateHealthScore();
    void UpdateHealthStatus();
    void ScanForOrphanedHeaders();
    void CheckCompilationErrors();
    void AssessCodeCoverage();
    void IdentifyMissingImplementations();
    
    // Health improvement methods
    void AutoFixCriticalIssues();
    void GenerateMissingCppFiles();
    void FixIncludeStructure();
    void OptimizeHeaderDependencies();
    
    // Logging and reporting
    void LogHealthMetrics();
    void ReportCriticalIssues();
    void UpdateHealthWarnings();
    
    // Utility methods
    bool ShouldPerformHealthCheck() const;
    FString GetHealthStatusString() const;
    void ResetHealthMetrics();
};