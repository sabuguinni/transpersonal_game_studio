#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildStatusChanged, EBuild_BuildStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleValidated, FString, ModuleName, bool, bIsValid);

/**
 * Build Integration Manager - Orchestrates compilation, validation, and integration
 * of all game systems. Ensures cross-module compatibility and build health.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build Status Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_BuildStatus GetCurrentBuildStatus() const { return CurrentBuildStatus; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetBuildStatus(EBuild_BuildStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsSystemHealthy() const;

    // Module Validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetFailedModules() const { return FailedModules; }

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestCrossModuleCompatibility();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateBuildReport();

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CheckModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ResolveDependencyConflicts();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_PerformanceMetrics GetPerformanceMetrics() const { return PerformanceMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void UpdatePerformanceMetrics();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildStatusChanged OnBuildStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnModuleValidated OnModuleValidated;

protected:
    // Core Properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Status")
    EBuild_BuildStatus CurrentBuildStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Status")
    TArray<FString> ValidatedModules;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Status")
    TArray<FString> FailedModules;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build Status")
    FBuild_PerformanceMetrics PerformanceMetrics;

    // Integration Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bAutoValidateOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    int32 MaxRetryAttempts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bGenerateDetailedReports;

    // Module Dependencies
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dependencies")
    TMap<FString, TArray<FString>> ModuleDependencies;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> CriticalModules;

    // Validation State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bValidationInProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    FDateTime LastValidationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    int32 ValidationAttempts;

private:
    // Internal Methods
    void InitializeModuleDependencies();
    void ValidateModuleInternal(const FString& ModuleName);
    void UpdateBuildMetrics();
    void LogBuildStatus(const FString& Message, bool bIsError = false);
    bool CheckClassAvailability(const FString& ClassName);
    void PerformSystemHealthCheck();

    // Timers
    FTimerHandle ValidationTimerHandle;
    FTimerHandle MetricsUpdateHandle;
};