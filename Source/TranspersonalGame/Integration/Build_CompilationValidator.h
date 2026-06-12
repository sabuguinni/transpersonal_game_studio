#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Build_CompilationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_CompilationResult : uint8
{
    Unknown,
    Success,
    Warning,
    Error,
    Fatal
};

USTRUCT(BlueprintType)
struct FBuild_CompilationIssue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EBuild_CompilationResult Severity;

    UPROPERTY(BlueprintReadOnly)
    FString FileName;

    UPROPERTY(BlueprintReadOnly)
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly)
    FString Message;

    UPROPERTY(BlueprintReadOnly)
    FString Category;

    FBuild_CompilationIssue()
    {
        Severity = EBuild_CompilationResult::Unknown;
        FileName = TEXT("");
        LineNumber = 0;
        Message = TEXT("");
        Category = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FBuild_ModuleCompilationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    EBuild_CompilationResult Result;

    UPROPERTY(BlueprintReadOnly)
    TArray<FBuild_CompilationIssue> Issues;

    UPROPERTY(BlueprintReadOnly)
    float CompilationTime;

    UPROPERTY(BlueprintReadOnly)
    int32 SourceFileCount;

    UPROPERTY(BlueprintReadOnly)
    int32 HeaderFileCount;

    FBuild_ModuleCompilationStatus()
    {
        ModuleName = TEXT("");
        Result = EBuild_CompilationResult::Unknown;
        CompilationTime = 0.0f;
        SourceFileCount = 0;
        HeaderFileCount = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_CompilationValidator : public UObject
{
    GENERATED_BODY()

public:
    UBuild_CompilationValidator();

    // Compilation validation
    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static bool ValidateProjectCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static TArray<FBuild_ModuleCompilationStatus> GetModuleCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static bool CheckSourceFileIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static bool ValidateHeaderIncludes();

    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static bool CheckForDuplicateDefinitions();

    // File analysis
    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static TArray<FString> FindOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static TArray<FString> FindMissingImplementations();

    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static bool ValidateUClassMacros();

    // Error reporting
    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static void ReportCompilationIssue(const FBuild_CompilationIssue& Issue);

    UFUNCTION(BlueprintPure, Category = "Build Compilation")
    static TArray<FBuild_CompilationIssue> GetCompilationIssues();

    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static void ClearCompilationIssues();

    // Build system integration
    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static bool TriggerHotReload();

    UFUNCTION(BlueprintCallable, Category = "Build Compilation")
    static bool CheckBuildConfiguration();

    UFUNCTION(BlueprintPure, Category = "Build Compilation")
    static FString GetLastCompilationLog();

protected:
    static TArray<FBuild_CompilationIssue> CompilationIssues;
    static FString LastCompilationLog;

private:
    static bool AnalyzeSourceFile(const FString& FilePath, FBuild_ModuleCompilationStatus& Status);
    static bool ValidateHeaderFile(const FString& FilePath);
    static bool CheckIncludeDependencies(const FString& FilePath);
    static void ParseCompilationOutput(const FString& Output, TArray<FBuild_CompilationIssue>& Issues);
    static EBuild_CompilationResult DetermineOverallResult(const TArray<FBuild_CompilationIssue>& Issues);
};