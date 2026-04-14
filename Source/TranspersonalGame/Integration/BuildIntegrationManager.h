#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "UObject/ObjectMacros.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildStatusChanged, EBuild_BuildStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleValidated, FString, ModuleName, bool, bIsValid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrationComplete, bool, bSuccess);

/**
 * Build Integration Manager - Orchestrates the integration of all game systems
 * Handles compilation validation, module dependency resolution, and build status tracking
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
    bool IsBuildReady() const;

    // Module Validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetFailedModules() const { return FailedModules; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetValidatedModules() const { return ValidatedModules; }

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateBuildReport();

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ResolveDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckDependencyConflicts();

    // Build Artifacts
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CleanBuildArtifacts();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ArchiveBuild(const FString& BuildVersion);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildStatusChanged OnBuildStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnModuleValidated OnModuleValidated;

    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnIntegrationComplete OnIntegrationComplete;

protected:
    // Build Status
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    EBuild_BuildStatus CurrentBuildStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    float BuildProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    FString CurrentBuildVersion;

    // Module Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ValidatedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> FailedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> PendingModules;

    // Integration Test Results
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_IntegrationTestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    int32 FailedTests;

    // Build Configuration
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    EBuild_Configuration BuildConfiguration;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    EBuild_Platform TargetPlatform;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    bool bEnableDebugMode;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    bool bEnablePerformanceProfiling;

private:
    // Internal validation methods
    bool ValidateEngineIntegration();
    bool ValidateGameplayIntegration();
    bool ValidateRenderingIntegration();
    bool ValidateAudioIntegration();
    bool ValidatePhysicsIntegration();
    bool ValidateAIIntegration();
    bool ValidateWorldGenerationIntegration();
    bool ValidateCharacterIntegration();

    // Dependency resolution
    void BuildDependencyGraph();
    void SortModulesByDependency();
    bool DetectCircularDependencies();

    // Build utilities
    void UpdateBuildProgress(float Progress);
    void LogBuildEvent(const FString& Event, EBuild_LogLevel LogLevel);
    void NotifyBuildStatusChange();

    // Test execution
    void ExecuteModuleTests();
    void ExecuteIntegrationTests();
    void ExecutePerformanceTests();

    FTimerHandle ValidationTimerHandle;
    FTimerHandle IntegrationTimerHandle;
};