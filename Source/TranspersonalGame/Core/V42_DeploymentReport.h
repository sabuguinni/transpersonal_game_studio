// Copyright Transpersonal Game Studio. All Rights Reserved.
// V42_DeploymentReport.h - Deployment status and health report for v4.2

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "V42_DeploymentReport.generated.h"

/**
 * DEPLOYMENT STATUS ENUMERATION
 */
UENUM(BlueprintType)
enum class EDeploymentStatus : uint8
{
    Unknown     = 0,    // Status not yet determined
    Success     = 1,    // Deployment successful, all systems operational
    Warning     = 2,    // Deployment successful but with warnings
    Failed      = 3     // Deployment failed, critical issues detected
};

/**
 * ISSUE SEVERITY ENUMERATION
 */
UENUM(BlueprintType)
enum class EIssueSeverity : uint8
{
    Info        = 0,    // Informational message
    Warning     = 1,    // Warning that should be addressed
    Critical    = 2     // Critical issue that prevents proper operation
};

/**
 * SYSTEM REPORT STRUCTURE
 * 
 * Contains status information for individual systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemReport
{
    GENERATED_BODY()

    // System identification
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Report")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Report")
    FString ModuleName;

    // System status
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Report")
    bool bIsOnline;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Report")
    float LastUpdateTime;

    // Additional information
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Report")
    TArray<FString> AdditionalInfo;

    FSystemReport()
    {
        SystemName = TEXT("Unknown");
        ModuleName = TEXT("Unknown");
        bIsOnline = false;
        LastUpdateTime = 0.0f;
    }
};

/**
 * PERFORMANCE METRIC STRUCTURE
 * 
 * Contains performance measurement data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetric
{
    GENERATED_BODY()

    // Metric identification
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Metric")
    FString MetricName;

    // Metric value
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Metric")
    float Value;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Metric")
    FString Unit;

    // Target compliance
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Metric")
    bool bIsWithinTarget;

    FPerformanceMetric()
    {
        MetricName = TEXT("Unknown");
        Value = 0.0f;
        Unit = TEXT("");
        bIsWithinTarget = true;
    }
};

/**
 * DEPLOYMENT ISSUE STRUCTURE
 * 
 * Contains information about detected issues
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDeploymentIssue
{
    GENERATED_BODY()

    // Issue classification
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Issue")
    EIssueSeverity Severity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Issue")
    FString Category;

    // Issue details
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Issue")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Issue")
    FString Recommendation;

    FDeploymentIssue()
    {
        Severity = EIssueSeverity::Info;
        Category = TEXT("General");
        Description = TEXT("Unknown issue");
        Recommendation = TEXT("No recommendation available");
    }
};

/**
 * DEPLOYMENT INFORMATION STRUCTURE
 * 
 * Complete deployment status and health information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDeploymentInformation
{
    GENERATED_BODY()

    // Deployment metadata
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Info")
    FString Version;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Info")
    FDateTime BuildDate;

    // Overall status
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Info")
    EDeploymentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Info")
    FString StatusMessage;

    // Detailed reports
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Info")
    TArray<FSystemReport> SystemReports;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Info")
    TArray<FPerformanceMetric> PerformanceMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deployment Info")
    TArray<FDeploymentIssue> Issues;

    FDeploymentInformation()
    {
        Version = TEXT("Unknown");
        BuildDate = FDateTime::Now();
        Status = EDeploymentStatus::Unknown;
        StatusMessage = TEXT("Status not determined");
    }
};

/**
 * V4.2 DEPLOYMENT REPORT COMPONENT
 * 
 * Generates comprehensive deployment status and health reports
 * 
 * FEATURES:
 * - System status monitoring
 * - Performance metrics collection
 * - Issue detection and reporting
 * - Automated health assessment
 * - Report generation and export
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UV42DeploymentReport : public UActorComponent
{
    GENERATED_BODY()

public:
    UV42DeploymentReport();

protected:
    // Component lifecycle
    virtual void BeginPlay() override;

public:
    /**
     * Generate a complete deployment report
     * This analyzes all systems and generates a comprehensive status report
     */
    UFUNCTION(BlueprintCallable, Category = "Deployment Report")
    void GenerateDeploymentReport();

    /**
     * Get the complete deployment information
     */
    UFUNCTION(BlueprintCallable, Category = "Deployment Report")
    FDeploymentInformation GetDeploymentInfo() const;

    /**
     * Check if the deployment is healthy (Success or Warning status)
     */
    UFUNCTION(BlueprintCallable, Category = "Deployment Report")
    bool IsDeploymentHealthy() const;

    /**
     * Get only critical issues
     */
    UFUNCTION(BlueprintCallable, Category = "Deployment Report")
    TArray<FDeploymentIssue> GetCriticalIssues() const;

protected:
    // Report generation methods
    void CollectSystemInformation();
    void CollectPerformanceMetrics();
    void AnalyzeSystemHealth();
    void DetermineDeploymentStatus();
    void GenerateReportOutput();

    // Utility methods
    void SaveReportToFile(const FString& ReportContent);
    FString GetDeploymentStatusString(EDeploymentStatus Status) const;
    FString GetIssueSeverityString(EIssueSeverity Severity) const;

    // Deployment information storage
    UPROPERTY(BlueprintReadOnly, Category = "Deployment Info")
    FDeploymentInformation DeploymentInfo;
};

/**
 * DEPLOYMENT REPORT ACTOR
 * 
 * Actor that can be placed in levels to generate deployment reports
 */
UCLASS()
class TRANSPERSONALGAME_API AV42DeploymentReportActor : public AActor
{
    GENERATED_BODY()

public:
    AV42DeploymentReportActor()
    {
        PrimaryActorTick.bCanEverTick = false;

        // Create deployment report component
        DeploymentReportComponent = CreateDefaultSubobject<UV42DeploymentReport>(TEXT("DeploymentReportComponent"));
    }

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();

        if (bGenerateReportOnBeginPlay)
        {
            DeploymentReportComponent->GenerateDeploymentReport();
        }
    }

    // Deployment report component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UV42DeploymentReport* DeploymentReportComponent;

public:
    /**
     * Generate deployment report manually
     */
    UFUNCTION(BlueprintCallable, Category = "Deployment Report")
    void GenerateReport()
    {
        if (DeploymentReportComponent)
        {
            DeploymentReportComponent->GenerateDeploymentReport();
        }
    }

    /**
     * Get deployment information
     */
    UFUNCTION(BlueprintCallable, Category = "Deployment Report")
    FDeploymentInformation GetDeploymentInformation() const
    {
        if (DeploymentReportComponent)
        {
            return DeploymentReportComponent->GetDeploymentInfo();
        }
        return FDeploymentInformation();
    }

protected:
    // Auto-generate report on begin play
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bGenerateReportOnBeginPlay = true;
};

/**
 * DEPLOYMENT REPORT SUBSYSTEM
 * 
 * Global subsystem for managing deployment reports across the project
 */
UCLASS()
class TRANSPERSONALGAME_API UV42DeploymentReportSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override
    {
        Super::Initialize(Collection);
        UE_LOG(LogTemp, Warning, TEXT("V4.2 Deployment Report Subsystem initialized"));
    }

    virtual void Deinitialize() override
    {
        Super::Deinitialize();
        UE_LOG(LogTemp, Warning, TEXT("V4.2 Deployment Report Subsystem deinitialized"));
    }

    /**
     * Generate a global deployment report
     */
    UFUNCTION(BlueprintCallable, Category = "Deployment Report")
    void GenerateGlobalDeploymentReport()
    {
        UE_LOG(LogTemp, Warning, TEXT("Generating global deployment report..."));
        
        // Create a temporary component to generate the report
        if (UWorld* World = GetWorld())
        {
            if (AActor* TempActor = World->SpawnActor<AActor>())
            {
                UV42DeploymentReport* ReportComponent = NewObject<UV42DeploymentReport>(TempActor);
                ReportComponent->RegisterComponent();
                ReportComponent->GenerateDeploymentReport();
                
                LastGlobalReport = ReportComponent->GetDeploymentInfo();
                
                // Clean up
                TempActor->Destroy();
            }
        }
    }

    /**
     * Get the last global deployment report
     */
    UFUNCTION(BlueprintCallable, Category = "Deployment Report")
    FDeploymentInformation GetLastGlobalReport() const
    {
        return LastGlobalReport;
    }

protected:
    // Last global report storage
    UPROPERTY()
    FDeploymentInformation LastGlobalReport;
};