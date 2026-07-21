#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_CompilationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationError
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorType;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    ESeverity_Level Severity;

    FBuild_CompilationError()
    {
        ErrorType = TEXT("");
        FileName = TEXT("");
        LineNumber = 0;
        ErrorMessage = TEXT("");
        Severity = ESeverity_Level::Info;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FBuild_CompilationError> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FBuild_CompilationError> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bCompilationSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 FilesCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString BuildConfiguration;

    FBuild_CompilationReport()
    {
        bCompilationSuccessful = false;
        CompilationTime = 0.0f;
        FilesCompiled = 0;
        BuildConfiguration = TEXT("Development");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_CompilationValidator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void RunCompilationValidation();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    FBuild_CompilationReport GetLastCompilationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    bool ValidateHeaderImplementationPairs();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void CheckForDuplicateTypes();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void ValidateSharedTypesUsage();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    TArray<FString> GetMissingImplementations();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void GenerateCompilationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compilation Validation")
    FBuild_CompilationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Validation")
    TArray<FString> SourceDirectories;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Validation")
    bool bValidationInProgress;

private:
    void ScanSourceFiles();
    void ValidateIncludeStructure();
    void CheckUHTCompliance();
    void ParseCompilationLogs();
    bool IsHeaderFile(const FString& FilePath);
    bool IsSourceFile(const FString& FilePath);
    FString GetCorrespondingFile(const FString& FilePath);
    void AddCompilationError(const FString& ErrorType, const FString& FileName, int32 LineNumber, const FString& Message, ESeverity_Level Severity);
};