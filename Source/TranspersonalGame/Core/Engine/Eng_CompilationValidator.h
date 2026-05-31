#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CompilationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationError
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorType;

    FEng_CompilationError()
    {
        FileName = TEXT("");
        LineNumber = 0;
        ErrorMessage = TEXT("");
        ErrorType = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleCompilationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bCompiledSuccessfully;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FEng_CompilationError> Errors;

    FEng_ModuleCompilationStatus()
    {
        ModuleName = TEXT("");
        bCompiledSuccessfully = false;
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Compilation validation
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    FEng_ModuleCompilationStatus ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_ModuleCompilationStatus> GetCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool CheckClassAvailability(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FString> GetMissingClasses();

    // Code quality checks
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateHeaderIncludes();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateUPropertyMacros();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateUFunctionMacros();

    // Build system integration
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool TriggerHotReload();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool CheckBuildConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void GenerateCompilationReport();

    // Error tracking
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void LogCompilationError(const FString& FileName, int32 LineNumber, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void ClearCompilationErrors();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_CompilationError> GetRecentErrors() const { return RecentErrors; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FEng_ModuleCompilationStatus> ModuleStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FEng_CompilationError> RecentErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float LastValidationTime;

private:
    void ValidateTranspersonalGameModule();
    void CheckRequiredClasses();
    void ValidateSharedTypes();
    bool TestClassInstantiation(const FString& ClassName);
    void ScanForCompilationIssues();
};