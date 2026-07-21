#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "Eng_CompilationRules.generated.h"

/**
 * Engine Architect Compilation Rules System
 * Defines mandatory compilation standards and validation rules for all agents
 * Enforces UE5 C++ best practices and prevents common compilation errors
 */

UENUM(BlueprintType)
enum class EEng_CompilationSeverity : uint8
{
    Info        UMETA(DisplayName = "Info"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EEng_RuleCategory : uint8
{
    HeaderStructure     UMETA(DisplayName = "Header Structure"),
    MacroUsage         UMETA(DisplayName = "Macro Usage"),
    TypeDefinitions    UMETA(DisplayName = "Type Definitions"),
    ModuleDependencies UMETA(DisplayName = "Module Dependencies"),
    NamingConventions  UMETA(DisplayName = "Naming Conventions"),
    MemoryManagement   UMETA(DisplayName = "Memory Management"),
    BlueprintIntegration UMETA(DisplayName = "Blueprint Integration")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    FString RuleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    EEng_CompilationSeverity Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    EEng_RuleCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    FString Example;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    bool bIsEnforced;

    FEng_CompilationRule()
    {
        RuleID = TEXT("");
        RuleName = TEXT("");
        Description = TEXT("");
        Severity = EEng_CompilationSeverity::Warning;
        Category = EEng_RuleCategory::HeaderStructure;
        Example = TEXT("");
        bIsEnforced = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsValid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FString> Errors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FString> Warnings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FString> Info;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 TotalRulesChecked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 RulesPassed;

    FEng_ValidationResult()
    {
        bIsValid = false;
        TotalRulesChecked = 0;
        RulesPassed = 0;
    }
};

/**
 * Engine Architect Compilation Rules Manager
 * Central authority for all compilation standards and validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationRules : public UObject
{
    GENERATED_BODY()

public:
    UEng_CompilationRules();

    // Core compilation rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rules")
    TArray<FEng_CompilationRule> CompilationRules;

    // Validation methods
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationResult ValidateHeaderFile(const FString& HeaderContent);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationResult ValidateSourceFile(const FString& SourceContent);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationResult ValidateModuleDependencies(const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool CheckUSTRUCTPlacement(const FString& FileContent);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool CheckGeneratedInclude(const FString& FileContent);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool CheckNamingConventions(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Standards")
    void InitializeStandardRules();

    UFUNCTION(BlueprintCallable, Category = "Standards")
    TArray<FString> GetMandatoryIncludes();

    UFUNCTION(BlueprintCallable, Category = "Standards")
    TArray<FString> GetForbiddenPatterns();

    // Rule enforcement
    UFUNCTION(BlueprintCallable, Category = "Enforcement")
    void EnforceRule(const FString& RuleID, bool bEnforce);

    UFUNCTION(BlueprintCallable, Category = "Enforcement")
    bool IsRuleEnforced(const FString& RuleID);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GenerateValidationReport(const FEng_ValidationResult& Result);

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogValidationResult(const FEng_ValidationResult& Result);

private:
    void SetupCoreRules();
    void SetupHeaderRules();
    void SetupMacroRules();
    void SetupTypeRules();
    void SetupModuleRules();
    void SetupNamingRules();
    void SetupMemoryRules();
    void SetupBlueprintRules();

    bool ValidateUSTRUCTRule(const FString& Content);
    bool ValidateGeneratedBodyRule(const FString& Content);
    bool ValidateIncludeOrderRule(const FString& Content);
    bool ValidateAPIExportRule(const FString& Content);
    bool ValidateForwardDeclarationRule(const FString& Content);
};