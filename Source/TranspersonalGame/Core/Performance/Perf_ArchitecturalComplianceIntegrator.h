#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../Physics/Core_ArchitecturalCompliance.h"
#include "Perf_ArchitecturalComplianceIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CompliancePerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance")
    float ComplianceCheckTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance")
    float PerformanceValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance")
    float GovernanceIntegrationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance")
    float SystemRegistrationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance")
    float ViolationReportingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance")
    int32 ActiveComplianceChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance")
    int32 PerformanceViolations = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance")
    float AverageComplianceScore = 0.0f;

    FPerf_CompliancePerformanceMetrics()
    {
        ComplianceCheckTime = 0.0f;
        PerformanceValidationTime = 0.0f;
        GovernanceIntegrationTime = 0.0f;
        SystemRegistrationTime = 0.0f;
        ViolationReportingTime = 0.0f;
        ActiveComplianceChecks = 0;
        PerformanceViolations = 0;
        AverageComplianceScore = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ComplianceOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Compliance", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float MaxComplianceCheckTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Compliance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float MaxPerformanceValidationTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Compliance", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float MaxGovernanceIntegrationTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Compliance", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxConcurrentComplianceChecks = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Compliance", meta = (ClampMin = "50.0", ClampMax = "100.0"))
    float MinComplianceScore = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Compliance")
    bool bEnableAdaptiveCompliance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Compliance")
    bool bEnableComplianceOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Compliance")
    bool bEnablePerformanceThrottling = true;

    FPerf_ComplianceOptimizationSettings()
    {
        MaxComplianceCheckTime = 2.0f;
        MaxPerformanceValidationTime = 1.0f;
        MaxGovernanceIntegrationTime = 0.5f;
        MaxConcurrentComplianceChecks = 10;
        MinComplianceScore = 85.0f;
        bEnableAdaptiveCompliance = true;
        bEnableComplianceOptimization = true;
        bEnablePerformanceThrottling = true;
    }
};

UENUM(BlueprintType)
enum class EPerf_ComplianceOptimizationState : uint8
{
    Optimal         UMETA(DisplayName = "Optimal"),
    Monitoring      UMETA(DisplayName = "Monitoring"),
    Optimizing      UMETA(DisplayName = "Optimizing"),
    Throttling      UMETA(DisplayName = "Throttling"),
    Critical        UMETA(DisplayName = "Critical"),
    Disabled        UMETA(DisplayName = "Disabled")
};

/**
 * Performance optimizer specifically designed to integrate with the Core_ArchitecturalCompliance system
 * Monitors and optimizes architectural compliance performance to maintain 60fps PC / 30fps console targets
 * Provides comprehensive compliance performance metrics and adaptive optimization strategies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_ArchitecturalComplianceIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_ArchitecturalComplianceIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    void StartCompliancePerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    void StopCompliancePerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    void UpdateCompliancePerformanceMetrics(float DeltaTime);

    // Compliance Integration
    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    void IntegrateWithComplianceSystem();

    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    void ValidateCompliancePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    void OptimizeComplianceOperations();

    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    float AnalyzeComplianceCheckPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    float AnalyzeGovernanceIntegrationPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    bool IsCompliancePerformanceOptimal() const;

    // Optimization Control
    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    void SetComplianceOptimizationLevel(int32 Level);

    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    void EnableAdaptiveComplianceOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance|Compliance")
    void ThrottleComplianceOperations(float ThrottleAmount);

    // Metrics Access
    UFUNCTION(BlueprintPure, Category = "Performance|Compliance")
    FPerf_CompliancePerformanceMetrics GetCompliancePerformanceMetrics() const { return ComplianceMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance|Compliance")
    EPerf_ComplianceOptimizationState GetOptimizationState() const { return OptimizationState; }

    UFUNCTION(BlueprintPure, Category = "Performance|Compliance")
    float GetCompliancePerformanceScore() const;

    // Blueprint Events
    UPROPERTY(BlueprintAssignable, Category = "Performance|Compliance")
    FOnPerformanceStateChanged OnComplianceOptimizationStateChanged;

protected:
    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    FPerf_CompliancePerformanceMetrics ComplianceMetrics;

    // Optimization Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    FPerf_ComplianceOptimizationSettings OptimizationSettings;

    // State Management
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    EPerf_ComplianceOptimizationState OptimizationState;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoringActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    float LastOptimizationTime;

    // Compliance System Reference
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    UCore_ArchitecturalCompliance* ComplianceSystem;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    TArray<float> ComplianceCheckTimes;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    TArray<float> PerformanceValidationTimes;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    float AverageComplianceCheckTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Compliance", meta = (AllowPrivateAccess = "true"))
    float AverageValidationTime;

private:
    // Internal optimization methods
    void UpdateOptimizationState();
    void PerformAdaptiveOptimization();
    void ApplyPerformanceThrottling();
    void ResetPerformanceMetrics();

    // Timing utilities
    double ComplianceCheckStartTime;
    double ValidationStartTime;
    double GovernanceIntegrationStartTime;

    // Performance thresholds
    static constexpr float OPTIMAL_COMPLIANCE_TIME = 1.0f;
    static constexpr float WARNING_COMPLIANCE_TIME = 2.0f;
    static constexpr float CRITICAL_COMPLIANCE_TIME = 3.0f;
};