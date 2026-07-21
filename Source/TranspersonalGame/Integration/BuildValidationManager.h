#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "BuildValidationManager.generated.h"

UENUM(BlueprintType)
enum class EBuildValidationStatus : uint8
{
    Unknown,
    Validating,
    Passed,
    Failed,
    Warning
};

USTRUCT(BlueprintType)
struct FBuildValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString TestName;

    UPROPERTY(BlueprintReadOnly)
    EBuildValidationStatus Status;

    UPROPERTY(BlueprintReadOnly)
    FString Message;

    UPROPERTY(BlueprintReadOnly)
    float ExecutionTime;

    FBuildValidationResult()
        : Status(EBuildValidationStatus::Unknown)
        , ExecutionTime(0.0f)
    {}
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildValidationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateAssetIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateQAResults();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuildValidationResult> GetValidationResults() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    EBuildValidationStatus GetOverallStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ClearValidationResults();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsValidationInProgress() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FBuildValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    EBuildValidationStatus OverallStatus;

private:
    void AddValidationResult(const FString& TestName, EBuildValidationStatus Status, const FString& Message, float ExecutionTime = 0.0f);
    void UpdateOverallStatus();
    
    FDateTime ValidationStartTime;
    int32 TotalTests;
    int32 CompletedTests;
};