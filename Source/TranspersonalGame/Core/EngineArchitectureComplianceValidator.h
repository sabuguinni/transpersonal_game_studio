// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/EngineSubsystem.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "EngineArchitectureComplianceValidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineCompliance, Log, All);

/**
 * Engine Architecture Compliance Standards
 * Defines the mandatory standards for UE5 architecture compliance
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEngineComplianceStandards
{
    GENERATED_BODY()

    // World Partition Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    bool bRequireWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    float MinWorldPartitionCellSize = 51200.0f; // 512m default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 MaxWorldPartitionLevels = 22;

    // Nanite Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
    bool bRequireNaniteSupport = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
    int32 MinNaniteTriangleThreshold = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
    bool bEnforceNaniteForHighPolyMeshes = true;

    // Lumen Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bRequireLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bRequireLumenReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float MinLumenSceneViewDistance = 200.0f; // meters

    // Virtual Shadow Maps Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    bool bRequireVirtualShadowMaps = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 MinVSMResolution = 16384; // 16K default

    // Performance Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinTargetFramerate = 60.0f; // PC target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageGB = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bRequireHLOD = true;

    // Asset Management Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    bool bRequireOneFilePerActor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    bool bRequireDataLayers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    int32 MaxAssetsPerDataLayer = 10000;

    FEngineComplianceStandards()
    {
        // Default values set above
    }
};

/**
 * Compliance Violation Severity
 */
UENUM(BlueprintType)
enum class EComplianceViolationSeverity : uint8
{
    Info        UMETA(DisplayName = "Info"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * Compliance Violation Report
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FComplianceViolation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Violation")
    EComplianceViolationSeverity Severity = EComplianceViolationSeverity::Info;

    UPROPERTY(BlueprintReadOnly, Category = "Violation")
    FString ViolationType;

    UPROPERTY(BlueprintReadOnly, Category = "Violation")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Violation")
    FString RecommendedAction;

    UPROPERTY(BlueprintReadOnly, Category = "Violation")
    FString AffectedAsset;

    UPROPERTY(BlueprintReadOnly, Category = "Violation")
    float ImpactScore = 0.0f;

    FComplianceViolation()
    {
    }

    FComplianceViolation(EComplianceViolationSeverity InSeverity, 
                        const FString& InType, 
                        const FString& InDescription, 
                        const FString& InAction = TEXT(""),
                        const FString& InAsset = TEXT(""))
        : Severity(InSeverity)
        , ViolationType(InType)
        , Description(InDescription)
        , RecommendedAction(InAction)
        , AffectedAsset(InAsset)
    {
        // Calculate impact score based on severity
        switch (Severity)
        {
            case EComplianceViolationSeverity::Critical: ImpactScore = 10.0f; break;
            case EComplianceViolationSeverity::Error: ImpactScore = 5.0f; break;
            case EComplianceViolationSeverity::Warning: ImpactScore = 2.0f; break;
            case EComplianceViolationSeverity::Info: ImpactScore = 0.5f; break;
        }
    }
};

/**
 * Engine Compliance Report
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEngineComplianceReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float ComplianceScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    bool bIsCompliant = false;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FComplianceViolation> Violations;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString GeneratedTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString EngineVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString ProjectVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 TotalChecksPerformed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 PassedChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 FailedChecks = 0;

    FEngineComplianceReport()
    {
        Violations.Empty();
        GeneratedTimestamp = FDateTime::Now().ToString();
    }
};

/**
 * Engine Architecture Compliance Validator
 * Validates that the engine setup meets Transpersonal Game Studio standards
 * Enforces UE5 best practices for large-scale game development
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureComplianceValidator : public UObject
{
    GENERATED_BODY()

public:
    UEngineArchitectureComplianceValidator();

    /**
     * Execute comprehensive compliance validation
     * @param Standards Compliance standards to validate against
     * @return Detailed compliance report
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    FEngineComplianceReport ValidateCompliance(const FEngineComplianceStandards& Standards);

    /**
     * Validate World Partition configuration
     * @param Standards World Partition standards
     * @return True if compliant
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    bool ValidateWorldPartitionCompliance(const FEngineComplianceStandards& Standards);

    /**
     * Validate Nanite configuration and usage
     * @param Standards Nanite standards
     * @return True if compliant
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    bool ValidateNaniteCompliance(const FEngineComplianceStandards& Standards);

    /**
     * Validate Lumen configuration
     * @param Standards Lumen standards
     * @return True if compliant
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    bool ValidateLumenCompliance(const FEngineComplianceStandards& Standards);

    /**
     * Validate Virtual Shadow Maps configuration
     * @param Standards VSM standards
     * @return True if compliant
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    bool ValidateVSMCompliance(const FEngineComplianceStandards& Standards);

    /**
     * Validate performance requirements
     * @param Standards Performance standards
     * @return True if compliant
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    bool ValidatePerformanceCompliance(const FEngineComplianceStandards& Standards);

    /**
     * Validate asset management setup
     * @param Standards Asset management standards
     * @return True if compliant
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    bool ValidateAssetManagementCompliance(const FEngineComplianceStandards& Standards);

    /**
     * Generate compliance report as formatted string
     * @param Report Compliance report to format
     * @return Formatted report string
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    FString GenerateComplianceReportString(const FEngineComplianceReport& Report);

    /**
     * Get default compliance standards for Transpersonal Game Studio
     * @return Default standards configuration
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    static FEngineComplianceStandards GetDefaultStandards();

    /**
     * Calculate compliance score from violations
     * @param Violations List of compliance violations
     * @param TotalChecks Total number of checks performed
     * @return Score from 0-100
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    float CalculateComplianceScore(const TArray<FComplianceViolation>& Violations, int32 TotalChecks);

protected:
    // Internal validation methods
    void ValidateWorldPartitionSettings(const FEngineComplianceStandards& Standards, FEngineComplianceReport& Report);
    void ValidateNaniteSettings(const FEngineComplianceStandards& Standards, FEngineComplianceReport& Report);
    void ValidateLumenSettings(const FEngineComplianceStandards& Standards, FEngineComplianceReport& Report);
    void ValidateVSMSettings(const FEngineComplianceStandards& Standards, FEngineComplianceReport& Report);
    void ValidatePerformanceSettings(const FEngineComplianceStandards& Standards, FEngineComplianceReport& Report);
    void ValidateAssetSettings(const FEngineComplianceStandards& Standards, FEngineComplianceReport& Report);

    // Utility methods
    void AddViolation(FEngineComplianceReport& Report, const FComplianceViolation& Violation);
    bool CheckConsoleVariable(const FString& CVarName, const FString& ExpectedValue);
    bool CheckProjectSetting(const FString& SettingPath, const FString& ExpectedValue);
    
    // Platform-specific validation
    bool ValidatePlatformSupport();
    bool ValidateHardwareRequirements();

private:
    // Current validation state
    FEngineComplianceStandards CurrentStandards;
    FEngineComplianceReport CurrentReport;

    // Validation metrics
    int32 TotalValidationChecks = 0;
    int32 PassedValidationChecks = 0;
};

/**
 * Engine Compliance Subsystem
 * Manages continuous compliance monitoring
 */
UCLASS()
class TRANSPERSONALGAME_API UEngineComplianceSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Get the compliance validator instance
     * @return Validator object
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    UEngineArchitectureComplianceValidator* GetComplianceValidator();

    /**
     * Execute compliance check on engine startup
     * @return True if engine is compliant
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    bool ExecuteStartupComplianceCheck();

    /**
     * Schedule periodic compliance monitoring
     * @param IntervalMinutes Interval between checks in minutes
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    void SchedulePeriodicCompliance(float IntervalMinutes = 30.0f);

    /**
     * Get the last compliance report
     * @return Most recent compliance report
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Compliance")
    FEngineComplianceReport GetLastComplianceReport() const { return LastComplianceReport; }

protected:
    UPROPERTY()
    UEngineArchitectureComplianceValidator* ComplianceValidator;

    FEngineComplianceReport LastComplianceReport;
    FTimerHandle ComplianceTimerHandle;
    
    void PeriodicComplianceCheck();
    void OnComplianceViolationDetected(const FComplianceViolation& Violation);
};