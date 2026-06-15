#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "BuildValidationSystem.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"), 
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime Timestamp;

    FBuild_ValidationReport()
    {
        TestName = TEXT("");
        Result = EBuild_ValidationResult::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float HealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    FDateTime LastCheck;

    FBuild_SystemHealth()
    {
        SystemName = TEXT("");
        bIsOperational = true;
        ErrorCount = 0;
        WarningCount = 0;
        HealthScore = 100.0f;
        LastCheck = FDateTime::Now();
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationComplete, const TArray<FBuild_ValidationReport>&, Reports);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSystemHealthChanged, const FString&, SystemName, const FBuild_SystemHealth&, Health);

/**
 * Build Validation System - Comprehensive integration testing and health monitoring
 * Validates cross-system compatibility, performance metrics, and build integrity
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunQuickHealthCheck();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidatePerformanceMetrics();

    // Health monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_SystemHealth GetSystemHealth(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_SystemHealth> GetAllSystemHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    float GetOverallHealthScore() const;

    // Report management
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ValidationReport> GetReportsByResult(EBuild_ValidationResult ResultType) const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ClearValidationReports();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SetValidationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SetAutoValidationInterval(float IntervalSeconds);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Build Validation")
    FOnValidationComplete OnValidationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Build Validation")
    FOnSystemHealthChanged OnSystemHealthChanged;

protected:
    // Internal validation methods
    void ValidateClassLoading();
    void ValidateActorIntegration();
    void ValidateComponentSystems();
    void ValidateWorldGeneration();
    void ValidateAIBehavior();
    void ValidateAudioSystems();
    void ValidateVFXSystems();
    void ValidatePhysicsSystems();
    void ValidateNetworking();
    void ValidateMemoryUsage();

    // Health monitoring
    void UpdateSystemHealth(const FString& SystemName, bool bOperational, int32 Errors, int32 Warnings);
    void CalculateHealthScores();

    // Utility functions
    FBuild_ValidationReport CreateReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message);
    void AddValidationReport(const FBuild_ValidationReport& Report);

private:
    // Validation state
    UPROPERTY()
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY()
    TMap<FString, FBuild_SystemHealth> SystemHealthMap;

    UPROPERTY()
    bool bValidationEnabled;

    UPROPERTY()
    float AutoValidationInterval;

    UPROPERTY()
    FTimerHandle ValidationTimerHandle;

    // Performance tracking
    UPROPERTY()
    float LastValidationTime;

    UPROPERTY()
    int32 TotalValidationRuns;

    UPROPERTY()
    int32 SuccessfulValidationRuns;

    // System monitoring
    void StartAutoValidation();
    void StopAutoValidation();
    void OnAutoValidationTimer();

    // Critical system names
    static const TArray<FString> CoreSystemNames;
};