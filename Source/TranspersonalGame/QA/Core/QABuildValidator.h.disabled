#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Misc/DateTime.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "QABuildValidator.generated.h"

/**
 * Build Validation System for Transpersonal Game Studio
 * 
 * Validates that all modules compile correctly, dependencies are resolved,
 * and the build meets quality standards before integration.
 * 
 * This is the Integration Agent's primary tool for ensuring build stability.
 */

DECLARE_LOG_CATEGORY_EXTERN(LogQABuildValidator, Log, All);

/**
 * Build validation result structure
 */
USTRUCT()
struct TRANSPERSONALGAME_API FQA_BuildValidationResult
{
    GENERATED_BODY()

    UPROPERTY()
    bool bIsValid = false;

    UPROPERTY()
    FString ModuleName;

    UPROPERTY()
    FString ErrorMessage;

    UPROPERTY()
    FDateTime ValidationTime;

    UPROPERTY()
    float CompilationTimeSeconds = 0.0f;

    UPROPERTY()
    TArray<FString> Warnings;

    UPROPERTY()
    TArray<FString> Dependencies;
};

/**
 * Module dependency information
 */
USTRUCT()
struct TRANSPERSONALGAME_API FModuleDependencyInfo
{
    GENERATED_BODY()

    UPROPERTY()
    FString ModuleName;

    UPROPERTY()
    TArray<FString> RequiredModules;

    UPROPERTY()
    TArray<FString> OptionalModules;

    UPROPERTY()
    bool bIsLoaded = false;

    UPROPERTY()
    bool bHasErrors = false;
};

/**
 * Build configuration validation
 */
USTRUCT()
struct TRANSPERSONALGAME_API FBuildConfigValidation
{
    GENERATED_BODY()

    UPROPERTY()
    bool bDebugSymbolsPresent = false;

    UPROPERTY()
    bool bOptimizationEnabled = false;

    UPROPERTY()
    FString BuildConfiguration;

    UPROPERTY()
    FString TargetPlatform;

    UPROPERTY()
    int32 BinarySize = 0;

    UPROPERTY()
    TArray<FString> MissingAssets;
};

/**
 * Main build validator class
 */
UCLASS()
class TRANSPERSONALGAME_API UQABuildValidator : public UObject
{
    GENERATED_BODY()

public:
    UQABuildValidator();

    /**
     * Validate entire project build
     */
// [UHT-FIX2]     UFUNCTION(BlueprintCallable, Category = "QA|Build")
    bool ValidateProjectBuild(TArray<FQA_BuildValidationResult>& OutResults);

    /**
     * Validate specific module compilation
     */
// [UHT-FIX2]     UFUNCTION(BlueprintCallable, Category = "QA|Build")
    bool ValidateModuleCompilation(const FString& ModuleName, FQA_BuildValidationResult& OutResult);

    /**
     * Validate module dependencies
     */
// [UHT-FIX2]     UFUNCTION(BlueprintCallable, Category = "QA|Build")
    bool ValidateModuleDependencies(TArray<FModuleDependencyInfo>& OutDependencies);

    /**
     * Validate build configuration
     */
// [UHT-FIX2]     UFUNCTION(BlueprintCallable, Category = "QA|Build")
    bool ValidateBuildConfiguration(FBuildConfigValidation& OutValidation);

    /**
     * Check for duplicate symbols or conflicting definitions
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Build")
    bool CheckForSymbolConflicts(TArray<FString>& OutConflicts);

    /**
     * Validate asset references and packaging
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Build")
    bool ValidateAssetReferences(TArray<FString>& OutMissingAssets);

    /**
     * Performance validation for build times
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Build")
    bool ValidateBuildPerformance(float& OutTotalBuildTime, TArray<FString>& OutSlowModules);

    /**
     * Generate build report
     */
// [UHT-FIX2]     UFUNCTION(BlueprintCallable, Category = "QA|Build")
    FString GenerateBuildReport(const TArray<FQA_BuildValidationResult>& Results);

    /**
     * Check if build is ready for integration
     */
    UFUNCTION(BlueprintCallable, Category = "QA|Build")
    bool IsBuildReadyForIntegration();

protected:
    /**
     * Internal validation methods
     */
    bool ValidateModuleHeaders(const FString& ModuleName);
    bool ValidateModuleImplementation(const FString& ModuleName);
    bool CheckIncludePaths(const FString& ModuleName);
    bool ValidateUE5Compatibility(const FString& ModuleName);
    bool CheckForMemoryLeaks(const FString& ModuleName);
    bool ValidateThreadSafety(const FString& ModuleName);

    /**
     * Dependency resolution
     */
    void BuildDependencyGraph();
    bool ResolveDependencyOrder(TArray<FString>& OutModuleOrder);
    bool DetectCircularDependencies(TArray<FString>& OutCircularDeps);

    /**
     * Platform-specific validation
     */
    bool ValidateWindowsBuild();
    bool ValidateConsoleBuild();
    bool ValidateMobileBuild();

private:
    UPROPERTY()
    TMap<FString, FModuleDependencyInfo> ModuleDependencies;

    UPROPERTY()
    TArray<FString> KnownModules;

    UPROPERTY()
    FDateTime LastValidationTime;

    UPROPERTY()
    bool bValidationInProgress = false;

    // Build validation thresholds
    static constexpr float MAX_MODULE_COMPILE_TIME = 30.0f; // seconds
    static constexpr int32 MAX_WARNINGS_PER_MODULE = 10;
    static constexpr int32 MAX_TOTAL_BUILD_TIME = 300; // 5 minutes
};

/**
 * Build validation automation test
 */
class TRANSPERSONALGAME_API FBuildValidationTest : public FAutomationTestBase
{
public:
    FBuildValidationTest(const FString& InName)
        : FAutomationTestBase(InName, false)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
    virtual EAutomationTestFlags GetTestFlags() const override { return EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter; }
    virtual FString GetBeautifiedTestName() const override { return TEXT("Build Validation Test"); }
};