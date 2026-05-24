#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EngineArchitecturalFramework.generated.h"

// Engine Architect System Priority Levels
UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High           UMETA(DisplayName = "High"),
    Medium         UMETA(DisplayName = "Medium"),
    Low            UMETA(DisplayName = "Low"),
    Background     UMETA(DisplayName = "Background")
};

// Engine Performance Monitoring Categories
UENUM(BlueprintType)
enum class EEng_PerformanceCategory : uint8
{
    Physics        UMETA(DisplayName = "Physics"),
    Rendering      UMETA(DisplayName = "Rendering"),
    AI             UMETA(DisplayName = "AI"),
    Audio          UMETA(DisplayName = "Audio"),
    Networking     UMETA(DisplayName = "Networking"),
    Memory         UMETA(DisplayName = "Memory"),
    IO             UMETA(DisplayName = "Input/Output")
};

// Architectural Compliance Status
UENUM(BlueprintType)
enum class EEng_ComplianceStatus : uint8
{
    Compliant      UMETA(DisplayName = "Compliant"),
    Warning        UMETA(DisplayName = "Warning"),
    Violation      UMETA(DisplayName = "Violation"),
    Critical       UMETA(DisplayName = "Critical"),
    Unknown        UMETA(DisplayName = "Unknown")
};

// System Registration Information
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemRegistration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority = EEng_SystemPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString AgentResponsible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float LastUpdateTime = 0.0f;

    FEng_SystemRegistration()
    {
        SystemName = TEXT("UnknownSystem");
        ModuleName = TEXT("TranspersonalGame");
        AgentResponsible = TEXT("Unknown");
    }
};

// Performance Metrics Structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_PerformanceCategory Category = EEng_PerformanceCategory::Physics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FString MetricName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIsWithinLimits = true;

    FEng_PerformanceMetrics()
    {
        MetricName = TEXT("UnknownMetric");
    }
};

// Architectural Validation Result
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EEng_ComplianceStatus Status = EEng_ComplianceStatus::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FString> Recommendations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ValidationTime = 0.0f;

    FEng_ValidationResult()
    {
        SystemName = TEXT("UnknownSystem");
        ValidationMessage = TEXT("No validation performed");
    }
};

/**
 * Engine Architectural Framework - Core system that defines and enforces
 * architectural standards across all game systems. Manages system registration,
 * performance monitoring, and architectural compliance validation.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitecturalFramework();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystem(const FEng_SystemRegistration& SystemInfo);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemRegistration> GetRegisteredSystems() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdatePerformanceMetric(const FEng_PerformanceMetrics& Metric);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetPerformanceMetric(const FString& MetricName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_PerformanceMetrics> GetAllPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsPerformanceWithinLimits() const;

    // Architectural Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_ValidationResult ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_ValidationResult> ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_ComplianceStatus GetOverallComplianceStatus() const;

    // System Health Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdateSystemHealth(const FString& SystemName, float HealthValue);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemHealth(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool AreAllSystemsHealthy() const;

    // Architectural Standards Enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforceArchitecturalStandards();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetArchitecturalViolations() const;

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void GenerateArchitecturalReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GetFrameworkStatus() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void RunArchitecturalDiagnostics();

protected:
    // Registered Systems Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    TArray<FEng_SystemRegistration> RegisteredSystems;

    // Performance Metrics Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    TArray<FEng_PerformanceMetrics> PerformanceMetrics;

    // Validation Results Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    TArray<FEng_ValidationResult> ValidationResults;

    // System Health Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    TMap<FString, float> SystemHealthMap;

    // Framework Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePerformanceMonitoring = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableArchitecturalValidation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ValidationFrequency = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PerformanceMonitoringFrequency = 1.0f;

private:
    // Internal validation methods
    bool ValidateSystemInternal(const FString& SystemName, FEng_ValidationResult& OutResult);
    void UpdateFrameworkMetrics();
    void CheckArchitecturalCompliance();

    // Timer handles for periodic operations
    FTimerHandle ValidationTimerHandle;
    FTimerHandle PerformanceTimerHandle;
};