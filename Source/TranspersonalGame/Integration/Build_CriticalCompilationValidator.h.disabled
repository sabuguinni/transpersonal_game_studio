#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_CriticalCompilationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_CompilationStatus : uint8
{
    Unknown,
    Compiling,
    Success,
    Failed,
    Timeout,
    CriticalError
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EBuild_CompilationStatus Status = EBuild_CompilationStatus::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> FailedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> MissingHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bHasCriticalErrors = false;

    FBuild_CompilationResult()
    {
        Status = EBuild_CompilationStatus::Unknown;
        ErrorMessage = TEXT("");
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTime = 0.0f;
        bHasCriticalErrors = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleValidationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bHasValidHeaders = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bHasValidImplementations = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 HeaderCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ImplementationCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> MissingImplementations;

    FBuild_ModuleValidationData()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasValidHeaders = false;
        bHasValidImplementations = false;
        HeaderCount = 0;
        ImplementationCount = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_CriticalCompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_CompilationResult ValidateCurrentBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool CheckModuleIntegrity(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ModuleValidationData> ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool PerformCriticalSystemCheck();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsSystemStable();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void LogCriticalErrors();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    FBuild_CompilationResult LastCompilationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    TArray<FBuild_ModuleValidationData> ModuleValidationCache;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    bool bValidationInProgress = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    float LastValidationTime = 0.0f;

private:
    bool ValidateHeaderImplementationPairs();
    bool CheckForOrphanedHeaders();
    bool ValidateModuleDependencies();
    void CacheModuleValidationData();
    FString GenerateValidationTimestamp();
};