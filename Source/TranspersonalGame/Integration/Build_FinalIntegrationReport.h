#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Build_FinalIntegrationReport.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Testing         UMETA(DisplayName = "Testing"),
    Validating      UMETA(DisplayName = "Validating"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed")
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
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationScore;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsCompiled = false;
        ErrorMessage = TEXT("");
        ValidationScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedSystemCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FailedSystemCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float OverallBuildScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus BuildStatus;

    FBuild_IntegrationMetrics()
    {
        TotalActorCount = 0;
        DinosaurActorCount = 0;
        LoadedSystemCount = 0;
        FailedSystemCount = 0;
        OverallBuildScore = 0.0f;
        BuildStatus = EBuild_IntegrationStatus::Unknown;
    }
};

/**
 * Final Integration Report System
 * Provides comprehensive build validation and system health monitoring
 * Used by Integration Agent #19 to validate all agent outputs
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationReport : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationReport();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunFullIntegrationValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationMetrics GetCurrentBuildMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemValidationResult> GetSystemValidationResults() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemClass(const FString& ClassPath);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculateBuildScore() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalSystemPaths;

private:
    void InitializeCriticalSystems();
    void ValidateActorCounts();
    void ValidateSystemLoading();
    void UpdateBuildStatus();
    void LogIntegrationResults();
};

#include "Build_FinalIntegrationReport.generated.h"