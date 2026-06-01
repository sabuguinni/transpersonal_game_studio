#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "BuildValidationManager.generated.h"

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
    EBuild_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationReport()
    {
        Result = EBuild_ValidationResult::Pass;
        TestName = TEXT("");
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
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
    void ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateQAResults();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ValidateCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ValidationReport> GetValidationReports() const { return ValidationReports; }

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsValidationPassing() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ClearValidationReports() { ValidationReports.Empty(); }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 MaxActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 MaxDinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 MaxPropsPerBiome;

private:
    void AddValidationReport(EBuild_ValidationResult Result, const FString& TestName, const FString& Details, float ExecutionTime = 0.0f);
    
    bool ValidateActorLimits();
    bool ValidateBiomeDistribution();
    bool ValidateModuleLoading();
    bool ValidateAssetIntegrity();
    
    FDateTime ValidationStartTime;
};