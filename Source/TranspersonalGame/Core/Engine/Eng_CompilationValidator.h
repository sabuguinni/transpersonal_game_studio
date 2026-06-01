#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/EngineSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CompilationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bHasErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    int32 ClassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> ErrorMessages;

    FEng_ModuleStatus()
    {
        ModuleName = TEXT("Unknown");
        bIsLoaded = false;
        bHasErrors = false;
        ClassCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    bool bOverallSuccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 TotalModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 LoadedModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 TotalClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FEng_ModuleStatus> ModuleStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    FDateTime ReportTime;

    FEng_CompilationReport()
    {
        bOverallSuccess = false;
        TotalModules = 0;
        LoadedModules = 0;
        TotalClasses = 0;
        ReportTime = FDateTime::Now();
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_CompilationValidator : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    FEng_CompilationReport ValidateProjectCompilation();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    bool ValidateModuleIntegrity(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    TArray<FString> GetMissingCppFiles();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    bool FixCommonCompilationIssues();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    void GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator", CallInEditor = true)
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    FEng_CompilationReport GetLastReport() const { return LastCompilationReport; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compilation Validator")
    FEng_CompilationReport LastCompilationReport;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation Validator")
    TArray<FString> RequiredModules;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation Validator")
    TArray<FString> CriticalClasses;

private:
    void InitializeRequiredModules();
    void InitializeCriticalClasses();
    bool CheckClassExists(const FString& ClassName);
    FEng_ModuleStatus ValidateModule(const FString& ModuleName);
    void LogValidationResults(const FEng_CompilationReport& Report);
};