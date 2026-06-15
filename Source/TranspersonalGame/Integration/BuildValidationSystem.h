#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "BuildValidationSystem.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    Unknown,
    Passed,
    Failed,
    Warning,
    Critical
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationResult()
    {
        TestName = TEXT("");
        Status = EBuild_ValidationStatus::Unknown;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    int32 LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    TArray<FString> Dependencies;

    FBuild_SystemHealth()
    {
        SystemName = TEXT("");
        bIsOperational = false;
        LoadedClasses = 0;
        ActiveActors = 0;
    }
};

/**
 * Build Validation System - Integration Agent #19
 * Comprehensive validation framework for all game systems
 * Monitors compilation status, class loading, and cross-system integration
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildValidationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateMapIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateCompilationStatus();

    // Health monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_SystemHealth> GetSystemHealthReport();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsSystemOperational(const FString& SystemName);

    // Results access
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ValidationResult> GetValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    int32 GetPassedTestsCount() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    int32 GetFailedTestsCount() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    float GetOverallHealthScore() const;

    // Automated monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void StartContinuousMonitoring(float IntervalSeconds = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void StopContinuousMonitoring();

protected:
    // Internal validation methods
    void ValidateWorldGeneration();
    void ValidateCharacterSystems();
    void ValidateAISystems();
    void ValidateEnvironmentSystems();
    void ValidateQuestSystems();
    void ValidateAudioSystems();
    void ValidateVFXSystems();

    // Helper methods
    void AddValidationResult(const FString& TestName, EBuild_ValidationStatus Status, const FString& Message, float ExecutionTime = 0.0f);
    bool CheckClassExists(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);

private:
    UPROPERTY()
    TArray<FBuild_ValidationResult> ValidationResults;

    UPROPERTY()
    TArray<FBuild_SystemHealth> SystemHealthData;

    UPROPERTY()
    FTimerHandle MonitoringTimerHandle;

    UPROPERTY()
    bool bIsContinuousMonitoringActive;

    // Core system class names for validation
    TArray<FString> CoreSystemClasses;
    
    // System dependency mapping
    TMap<FString, TArray<FString>> SystemDependencies;
};