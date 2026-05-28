#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitectureValidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitect, Log, All);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleComplianceReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bHasValidHeaders = false;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bHasMatchingCppFiles = false;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bFollowsNamingConvention = false;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bCompilesSuccessfully = false;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> MissingCppFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> CompilationErrors;

    FEng_ModuleComplianceReport()
    {
        ModuleName = TEXT("Unknown");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> RequiredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> OptionalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 Priority = 0;

    FEng_SystemDependency()
    {
        SystemName = TEXT("Unknown");
    }
};

/**
 * Engine Architect validation system that enforces technical architecture rules
 * across all agent-created modules and ensures compilation compliance
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitectureValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation methods
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateModuleCompliance(const FString& ModulePath);

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    FEng_ModuleComplianceReport GenerateComplianceReport(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateHeaderCppPairs();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool EnforceNamingConventions();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    TArray<FString> GetMissingCppFiles();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    void RegisterSystemDependency(const FEng_SystemDependency& Dependency);

    // Compilation validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateCompilation();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    TArray<FString> GetCompilationErrors();

    // Performance architecture rules
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidatePerformanceCompliance();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool EnforceMemoryLimits();

    // Blueprint integration validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateBlueprintExposure();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    TArray<FString> GetUndocumentedBlueprintFunctions();

    // Critical system validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect", CallInEditor)
    void RunFullArchitectureAudit();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_ModuleComplianceReport> ComplianceReports;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_SystemDependency> SystemDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> CriticalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float LastValidationTime = 0.0f;

private:
    // Internal validation helpers
    bool ValidateHeaderFile(const FString& HeaderPath);
    bool ValidateCppFile(const FString& CppPath);
    bool CheckIncludeStructure(const FString& FilePath);
    bool ValidateUPropertyMacros(const FString& FilePath);
    bool ValidateUFunctionMacros(const FString& FilePath);
    
    // Naming convention enforcement
    bool ValidateClassName(const FString& ClassName);
    bool ValidateVariableName(const FString& VariableName);
    bool ValidateFunctionName(const FString& FunctionName);
    
    // Dependency analysis
    void AnalyzeDependencyChain();
    bool DetectCircularDependencies();
    
    // Performance validation
    bool CheckMemoryUsage();
    bool ValidateTickFrequency();
    bool CheckRenderingCompliance();
};