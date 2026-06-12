#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Eng_CompilationFixManager.generated.h"

/**
 * CYCLE 004 - ENGINE ARCHITECT COMPILATION FIX MANAGER
 * 
 * Critical system to identify and resolve compilation issues across the TranspersonalGame module.
 * This manager detects duplicate type definitions, missing dependencies, and cross-module conflicts
 * that prevent successful compilation and class loading.
 * 
 * ARCHITECTURE RULES ENFORCED:
 * 1. No duplicate USTRUCT/UENUM definitions across modules
 * 2. All cross-module dependencies must be declared in Build.cs
 * 3. SharedTypes.h is the single source of truth for shared types
 * 4. All UCLASS types must have matching .cpp implementations
 * 5. Forward declarations used for types from non-dependent modules
 */

UENUM(BlueprintType)
enum class EEng_CompilationIssueType : uint8
{
    None                    UMETA(DisplayName = "No Issues"),
    DuplicateTypeDefinition UMETA(DisplayName = "Duplicate Type Definition"),
    MissingDependency      UMETA(DisplayName = "Missing Module Dependency"),
    MissingImplementation  UMETA(DisplayName = "Missing .cpp Implementation"),
    CircularDependency     UMETA(DisplayName = "Circular Module Dependency"),
    InvalidForwardDecl     UMETA(DisplayName = "Invalid Forward Declaration"),
    SharedTypesConflict    UMETA(DisplayName = "SharedTypes.h Conflict")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationIssue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation")
    FString FileName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation")
    FString TypeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation")
    EEng_CompilationIssueType IssueType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation")
    FString SuggestedFix;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation")
    int32 Priority; // 1=Critical, 2=High, 3=Medium, 4=Low

    FEng_CompilationIssue()
    {
        FileName = TEXT("");
        TypeName = TEXT("");
        IssueType = EEng_CompilationIssueType::None;
        Description = TEXT("");
        SuggestedFix = TEXT("");
        Priority = 4;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleDependency
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dependencies")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> RequiredByFiles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dependencies")
    bool bIsInBuildCS;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dependencies")
    bool bIsCritical;

    FEng_ModuleDependency()
    {
        ModuleName = TEXT("");
        bIsInBuildCS = false;
        bIsCritical = false;
    }
};

/**
 * Compilation Fix Manager - Detects and resolves compilation issues
 * 
 * This subsystem runs compilation diagnostics and provides automated fixes
 * for common compilation problems in the TranspersonalGame module.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationFixManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationFixManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Run comprehensive compilation diagnostics
     * @return Array of detected compilation issues
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    TArray<FEng_CompilationIssue> RunCompilationDiagnostics();

    /**
     * Scan for duplicate type definitions across all .h files
     * @return Array of duplicate type issues
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    TArray<FEng_CompilationIssue> ScanForDuplicateTypes();

    /**
     * Validate SharedTypes.h consistency
     * @return Array of SharedTypes-related issues
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    TArray<FEng_CompilationIssue> ValidateSharedTypes();

    /**
     * Check for missing .cpp implementations
     * @return Array of missing implementation issues
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    TArray<FEng_CompilationIssue> CheckMissingImplementations();

    /**
     * Analyze module dependencies and detect missing Build.cs entries
     * @return Array of dependency issues
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    TArray<FEng_ModuleDependency> AnalyzeModuleDependencies();

    /**
     * Generate automated fixes for detected issues
     * @param Issues Array of issues to fix
     * @return Number of issues successfully fixed
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    int32 ApplyAutomatedFixes(const TArray<FEng_CompilationIssue>& Issues);

    /**
     * Test class loading for all UCLASS types in the module
     * @return Array of classes that failed to load
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    TArray<FString> TestClassLoading();

    /**
     * Get compilation health score (0-100)
     * @return Health score based on detected issues
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", BlueprintPure)
    float GetCompilationHealthScore() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation State")
    TArray<FEng_CompilationIssue> DetectedIssues;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation State")
    TArray<FEng_ModuleDependency> ModuleDependencies;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation State")
    float LastHealthScore;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation State")
    FDateTime LastDiagnosticsRun;

private:
    // Internal helper methods
    bool ScanHeaderFile(const FString& FilePath, TArray<FEng_CompilationIssue>& OutIssues);
    bool ValidateTypeDefinition(const FString& TypeName, const FString& FilePath);
    TArray<FString> ExtractIncludedModules(const FString& FilePath);
    bool CheckBuildCSForModule(const FString& ModuleName);
    FString GenerateFixSuggestion(const FEng_CompilationIssue& Issue);
};