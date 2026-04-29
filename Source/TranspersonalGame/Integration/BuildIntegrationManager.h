#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TranspersonalGame/SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Build Integration Manager
 * 
 * Responsible for validating and monitoring the integration status of all game systems.
 * Performs continuous health checks on:
 * - Core class loading and registration
 * - Map functionality and actor spawning
 * - Compilation artifacts and module loading
 * - Cross-system dependencies and compatibility
 * 
 * Provides real-time build health scoring and critical error detection.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildIntegrationManager : public AActor
{
    GENERATED_BODY()

public:
    ABuildIntegrationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // === VALIDATION FUNCTIONS ===
    
    /** Perform comprehensive build validation */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void PerformBuildValidation();
    
    /** Validate project loading status */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateProjectLoading();
    
    /** Validate core class loading */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateCoreClasses();
    
    /** Validate map functionality */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateMapFunctionality();
    
    /** Validate compilation artifacts */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateCompilationArtifacts();
    
    /** Calculate overall build health score */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CalculateBuildHealthScore();
    
    /** Update build status based on validation results */
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void UpdateBuildStatus();

    // === STATUS REPORTING ===
    
    /** Get current build status as string */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    FString GetBuildStatusString() const;
    
    /** Get list of critical errors */
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    TArray<FString> GetCriticalErrors() const;
    
    /** Log comprehensive build report */
    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor)
    void LogBuildReport() const;

    // === BUILD STATUS PROPERTIES ===
    
    /** Current build status */
    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    EBuild_BuildStatus BuildStatus;
    
    /** Overall build health score (0-100) */
    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    float BuildHealthScore;
    
    /** Number of core classes successfully loaded */
    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    int32 LoadedClassCount;
    
    /** Total number of core classes to validate */
    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    int32 TotalClassCount;
    
    /** Number of active actors in current scene */
    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    int32 ActiveActorCount;
    
    /** Number of critical errors detected */
    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    int32 ErrorCount;

    // === BUILD FLAGS ===
    
    /** Whether project is properly loaded */
    UPROPERTY(BlueprintReadOnly, Category = "Build Flags")
    bool bProjectLoaded;
    
    /** Whether map is functional */
    UPROPERTY(BlueprintReadOnly, Category = "Build Flags")
    bool bMapFunctional;
    
    /** Whether compilation artifacts were found */
    UPROPERTY(BlueprintReadOnly, Category = "Build Flags")
    bool bCompilationArtifactsFound;
    
    /** Whether critical errors were detected */
    UPROPERTY(BlueprintReadOnly, Category = "Build Flags")
    bool bCriticalErrorsDetected;

    // === VALIDATION SETTINGS ===
    
    /** Time since last validation */
    UPROPERTY(BlueprintReadOnly, Category = "Validation Settings")
    float LastValidationTime;
    
    /** Interval between automatic validations (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    float ValidationInterval;

private:
    /** List of critical errors detected during validation */
    UPROPERTY()
    TArray<FString> CriticalErrors;
};