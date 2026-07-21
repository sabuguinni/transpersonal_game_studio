#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_ArchitecturalFramework.h"
#include "Eng_CompilationManager.generated.h"

// ============================================================================
// COMPILATION MANAGEMENT SYSTEM - AGENT #2
// Handles orphaned headers, missing implementations, and build validation
// ============================================================================

UENUM(BlueprintType)
enum class EEng_BuildTarget : uint8
{
    Development,    // Development build for testing
    Shipping,       // Final shipping build
    Editor,         // Editor-only build
    Test           // Test/QA build
};

UENUM(BlueprintType)
enum class EEng_CompilationPhase : uint8
{
    PreBuild,       // Before compilation starts
    HeaderCheck,    // Checking header dependencies
    Implementation, // Compiling source files
    Linking,        // Linking phase
    PostBuild,      // After compilation complete
    Validation      // Post-build validation
};

USTRUCT(BlueprintType)
struct FEng_CompilationError
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ErrorCode;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EEng_CompilationPhase Phase;

    FEng_CompilationError()
    {
        FileName = TEXT("");
        LineNumber = 0;
        ErrorMessage = TEXT("");
        ErrorCode = TEXT("");
        Phase = EEng_CompilationPhase::PreBuild;
    }
};

USTRUCT(BlueprintType)
struct FEng_OrphanedHeader
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Orphaned")
    FString HeaderPath;

    UPROPERTY(BlueprintReadOnly, Category = "Orphaned")
    FString ExpectedCppPath;

    UPROPERTY(BlueprintReadOnly, Category = "Orphaned")
    bool bHasUCLASS;

    UPROPERTY(BlueprintReadOnly, Category = "Orphaned")
    bool bHasUSTRUCT;

    UPROPERTY(BlueprintReadOnly, Category = "Orphaned")
    bool bHasUENUM;

    UPROPERTY(BlueprintReadOnly, Category = "Orphaned")
    TArray<FString> DeclaredClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Orphaned")
    TArray<FString> DeclaredFunctions;

    FEng_OrphanedHeader()
    {
        HeaderPath = TEXT("");
        ExpectedCppPath = TEXT("");
        bHasUCLASS = false;
        bHasUSTRUCT = false;
        bHasUENUM = false;
        DeclaredClasses.Empty();
        DeclaredFunctions.Empty();
    }
};

USTRUCT(BlueprintType)
struct FEng_BuildReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EEng_BuildTarget BuildTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bBuildSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float BuildTimeSeconds;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 CompiledFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FEng_CompilationError> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FEng_OrphanedHeader> OrphanedHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime BuildTimestamp;

    FEng_BuildReport()
    {
        BuildTarget = EEng_BuildTarget::Development;
        bBuildSuccessful = false;
        BuildTimeSeconds = 0.0f;
        TotalFiles = 0;
        CompiledFiles = 0;
        ErrorCount = 0;
        WarningCount = 0;
        Errors.Empty();
        OrphanedHeaders.Empty();
        BuildTimestamp = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // ORPHANED HEADER MANAGEMENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    void ScanForOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    TArray<FEng_OrphanedHeader> GetOrphanedHeaders() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Compilation Manager")
    void GenerateStubImplementations();

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    bool CreateCppStubForHeader(const FString& HeaderPath);

    // ========================================================================
    // BUILD VALIDATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    bool ValidateBuildConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    FEng_BuildReport RunBuildValidation(EEng_BuildTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    bool CheckModuleDependencies();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Compilation Manager")
    void FixCommonCompilationIssues();

    // ========================================================================
    // ERROR ANALYSIS
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    void AnalyzeCompilationErrors(const TArray<FString>& ErrorLog);

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    TArray<FEng_CompilationError> GetRecentErrors() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    bool AutoFixKnownErrors();

    // ========================================================================
    // HEADER ANALYSIS
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    void AnalyzeHeaderFile(const FString& HeaderPath, FEng_OrphanedHeader& OutAnalysis);

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    bool ValidateHeaderIncludes(const FString& HeaderPath);

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    TArray<FString> GetMissingIncludes(const FString& HeaderPath);

    // ========================================================================
    // STUB GENERATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    FString GenerateBasicCppStub(const FEng_OrphanedHeader& HeaderInfo);

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    FString GenerateConstructorStub(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    FString GenerateFunctionStubs(const TArray<FString>& FunctionDeclarations);

    // ========================================================================
    // AGENT COORDINATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    bool ValidateAgentFiles(int32 AgentID, const TArray<FString>& CreatedFiles);

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    void RegisterAgentOutput(int32 AgentID, const TArray<FString>& Files);

    UFUNCTION(BlueprintCallable, Category = "Compilation Manager")
    TArray<FString> GetAgentOrphanedHeaders(int32 AgentID) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Compilation Manager")
    TArray<FEng_OrphanedHeader> OrphanedHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Manager")
    TArray<FEng_CompilationError> RecentErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Manager")
    TMap<int32, TArray<FString>> AgentFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Manager")
    FEng_BuildReport LastBuildReport;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Manager")
    bool bManagerInitialized;

private:
    void ScanDirectory(const FString& DirectoryPath, TArray<FString>& OutHeaderFiles);
    void ParseHeaderFile(const FString& FilePath, FEng_OrphanedHeader& OutInfo);
    bool CheckCppExists(const FString& HeaderPath);
    FString GetCppPathFromHeader(const FString& HeaderPath);
    void ExtractClassDeclarations(const FString& FileContent, TArray<FString>& OutClasses);
    void ExtractFunctionDeclarations(const FString& FileContent, TArray<FString>& OutFunctions);
    bool HasUnrealMacros(const FString& FileContent, bool& bHasUCLASS, bool& bHasUSTRUCT, bool& bHasUENUM);
    FString CreateBasicIncludeStatement(const FString& HeaderPath);
    FString CreateBasicConstructorImplementation(const FString& ClassName);
    FString CreateBasicFunctionImplementation(const FString& FunctionDeclaration);
};

#include "Eng_CompilationManager.generated.h"