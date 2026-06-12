#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SharedTypes.h"
#include "Build_CompilationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 HeaderCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 SourceCount;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EBuild_ValidationStatus CompilationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> CompilationErrors;

    FBuild_ModuleInfo()
    {
        ModuleName = TEXT("");
        HeaderCount = 0;
        SourceCount = 0;
        CompilationStatus = EBuild_ValidationStatus::Unknown;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FBuild_ModuleInfo> ModuleInfos;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 TotalHeaderFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 TotalSourceFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 OrphanedHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bAllModulesCompile;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTime;

    FBuild_CompilationReport()
    {
        TotalHeaderFiles = 0;
        TotalSourceFiles = 0;
        OrphanedHeaders = 0;
        bAllModulesCompile = false;
        CompilationTime = 0.0f;
    }
};

/**
 * Compilation Validator - Ensures all code compiles and follows UE5 standards
 * Validates header/source file pairs and compilation integrity
 */
UCLASS(Config = Game, DefaultConfig, BlueprintType)
class TRANSPERSONALGAME_API UBuild_CompilationValidator : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UBuild_CompilationValidator();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    FBuild_CompilationReport GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateHeaderSourcePairs();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FString> FindOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateUE5Standards();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Compilation")
    void RunCompilationValidation();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FBuild_ModuleInfo> GetModuleInfos();

protected:
    UPROPERTY(Config, BlueprintReadOnly, Category = "Compilation")
    TArray<FString> RequiredModules;

    UPROPERTY(Config, BlueprintReadOnly, Category = "Compilation")
    TArray<FString> OptionalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FBuild_ModuleInfo> CachedModuleInfos;

private:
    void ValidateModule(const FString& ModulePath, FBuild_ModuleInfo& ModuleInfo);
    bool CheckHeaderSourcePair(const FString& HeaderPath);
    TArray<FString> GetCompilationErrors(const FString& ModulePath);
    bool ValidateUE5Macros(const FString& FilePath);
};