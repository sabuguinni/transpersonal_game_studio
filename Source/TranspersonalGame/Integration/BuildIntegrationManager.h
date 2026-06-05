#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown,
    Validating,
    Passed,
    Failed,
    Critical
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsFunctional;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsFunctional = false;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 SystemsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 SystemsTotal;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> SystemResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ValidationTimestamp;

    FBuild_IntegrationReport()
    {
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        SystemsLoaded = 0;
        SystemsTotal = 0;
        IntegrationPercentage = 0.0f;
        ValidationTimestamp = FDateTime::Now();
    }
};

/**
 * Build Integration Manager - Validates and integrates all game systems
 * Ensures cross-module compatibility and functional integration
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

    // Core integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateMapIntegrity();

    // System-specific validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateCoreSystem(const FString& SystemClassName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateQASystem(const FString& QAClassName);

    // Integration reporting
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GetLastIntegrationReport() const { return LastIntegrationReport; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetCurrentIntegrationStatus() const { return CurrentStatus; }

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CreateBuildSnapshot(const FString& SnapshotName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool RestoreBuildSnapshot(const FString& SnapshotName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetAvailableSnapshots() const;

protected:
    // Internal validation methods
    bool ValidateSystemClass(const FString& ClassName, FBuild_SystemValidationResult& OutResult);
    bool TestSystemFunctionality(UClass* SystemClass, FBuild_SystemValidationResult& OutResult);
    void UpdateIntegrationStatus();

    // Critical system class paths
    UPROPERTY(EditDefaultsOnly, Category = "Integration")
    TArray<FString> CoreSystemClasses;

    UPROPERTY(EditDefaultsOnly, Category = "Integration")
    TArray<FString> QASystemClasses;

    // Integration state
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastIntegrationReport;

    // Build snapshots
    UPROPERTY()
    TMap<FString, FString> BuildSnapshots;

    // Validation timing
    UPROPERTY(EditDefaultsOnly, Category = "Integration")
    float ValidationInterval;

    FTimerHandle ValidationTimerHandle;

private:
    void PerformPeriodicValidation();
    void LogIntegrationResults(const FBuild_IntegrationReport& Report);
};