#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_BuildValidator.generated.h"

UENUM(BlueprintType)
enum class EEng_BuildPhase : uint8
{
    PreBuild,
    Compilation,
    Linking,
    PostBuild,
    Testing,
    Packaging
};

UENUM(BlueprintType)
enum class EEng_BuildResult : uint8
{
    Unknown,
    Success,
    Warning,
    Error,
    Fatal
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BuildReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EEng_BuildPhase Phase;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EEng_BuildResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString SourceFile;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float Timestamp;

    FEng_BuildReport()
    {
        Phase = EEng_BuildPhase::PreBuild;
        Result = EEng_BuildResult::Unknown;
        Message = TEXT("");
        SourceFile = TEXT("");
        LineNumber = 0;
        Timestamp = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_BuildValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Build")
    void StartBuildValidation();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ValidateCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ValidateLinking();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ValidateRuntime();

    UFUNCTION(BlueprintCallable, Category = "Build")
    TArray<FEng_BuildReport> GetBuildReports() const;

    UFUNCTION(BlueprintCallable, Category = "Build")
    bool HasBuildErrors() const;

    UFUNCTION(BlueprintCallable, Category = "Build")
    int32 GetErrorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Build")
    int32 GetWarningCount() const;

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ClearBuildReports();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FEng_BuildReport> BuildReports;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EEng_BuildPhase CurrentPhase;

private:
    void AddBuildReport(EEng_BuildPhase Phase, EEng_BuildResult Result, const FString& Message, const FString& SourceFile = TEXT(""), int32 LineNumber = 0);
    void ValidateSourceFiles();
    void ValidateHeaders();
    void ValidateImplementations();
    void ValidateModuleDependencies();
    void TestClassInstantiation();
    void GenerateBuildSummary();
};

#include "Eng_BuildValidator.generated.h"