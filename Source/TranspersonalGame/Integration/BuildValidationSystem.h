#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
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
struct FBuild_ValidationReport
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

    FBuild_ValidationReport()
    {
        TestName = TEXT("");
        Result = EBuild_ValidationResult::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildValidationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool HasCriticalErrors() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float LastValidationTime;

private:
    void AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime);
    void ValidateWorldState();
    void ValidateGameSystems();
    void ValidateMemoryUsage();
    void ValidateDinosaurPopulation();
    void ValidatePlayerSystems();
    void ValidateEnvironmentSystems();
    void ValidateAudioSystems();
    void ValidateRenderingSystems();
    void ValidateNetworkingSystems();
    void ValidateInputSystems();
};