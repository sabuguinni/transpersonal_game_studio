#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_CompilationValidator.generated.h"

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    NotTested,
    Compiling,
    Success,
    Failed,
    Warning
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ClassValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ClassName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EEng_CompilationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FEng_ClassValidationResult()
    {
        ClassName = TEXT("");
        Status = EEng_CompilationStatus::NotTested;
        ErrorMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void ValidateAllClasses();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateClass(const FString& ClassPath);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_ClassValidationResult> GetValidationResults() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    int32 GetFailedClassCount() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    float GetOverallValidationTime() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FEng_ClassValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float TotalValidationTime;

private:
    void ValidateTranspersonalClasses();
    void ValidateEngineClasses();
    FEng_ClassValidationResult TestClassAvailability(const FString& ClassPath);
    void LogValidationSummary();
};

#include "Eng_CompilationValidator.generated.h"