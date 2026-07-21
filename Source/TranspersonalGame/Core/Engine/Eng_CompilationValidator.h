#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "UObject/Class.h"
#include "Eng_CompilationValidator.generated.h"

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    Unknown,
    Compiling,
    Success,
    Failed,
    Warning
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ClassValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ClassName;

    UPROPERTY(BlueprintReadOnly)
    bool bClassLoaded;

    UPROPERTY(BlueprintReadOnly)
    bool bCDOConstructed;

    UPROPERTY(BlueprintReadOnly)
    bool bPropertiesAccessible;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    FEng_ClassValidationResult()
        : bClassLoaded(false)
        , bCDOConstructed(false)
        , bPropertiesAccessible(false)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EEng_CompilationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalClassesTested;

    UPROPERTY(BlueprintReadOnly)
    int32 ClassesLoaded;

    UPROPERTY(BlueprintReadOnly)
    int32 CDOsConstructed;

    UPROPERTY(BlueprintReadOnly)
    TArray<FEng_ClassValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> MissingCppFiles;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> DuplicateTypes;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> APICompatibilityIssues;

    FEng_CompilationReport()
        : OverallStatus(EEng_CompilationStatus::Unknown)
        , TotalClassesTested(0)
        , ClassesLoaded(0)
        , CDOsConstructed(0)
    {}
};

/**
 * Engine Architect's Compilation Validator
 * Validates C++ compilation, class loading, and runtime functionality
 * Identifies missing .cpp files, duplicate types, and API compatibility issues
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_CompilationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_ClassValidationResult ValidateClass(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> FindMissingCppFiles();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> FindDuplicateTypes();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool FixCompilationIssues();

    // Runtime testing
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool TestClassInstantiation(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool TestSubsystemAccess(const FString& SubsystemName);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_CompilationStatus GetCurrentCompilationStatus() const;

protected:
    UPROPERTY()
    FEng_CompilationReport LastValidationReport;

    UPROPERTY()
    EEng_CompilationStatus CurrentStatus;

    // Internal validation helpers
    bool ValidateClassLoading(const FString& ClassName, FEng_ClassValidationResult& Result);
    bool ValidateCDOConstruction(UClass* Class, FEng_ClassValidationResult& Result);
    bool ValidatePropertyAccess(UClass* Class, FEng_ClassValidationResult& Result);
    
    // File system validation
    void ScanForMissingImplementations();
    void ScanForDuplicateDefinitions();
    void CheckAPICompatibility();

    // Auto-fixing capabilities
    bool CreateMissingCppStub(const FString& HeaderPath);
    bool FixIncludePaths(const FString& FilePath);
    bool UpdateAPICompatibility(const FString& FilePath);

private:
    // Core classes that must always work
    TArray<FString> CoreClassNames;
    
    // Known API compatibility issues in UE5.5
    TMap<FString, FString> APIReplacements;
    
    void InitializeCoreClassList();
    void InitializeAPIReplacements();
};

#include "Eng_CompilationValidator.generated.h"