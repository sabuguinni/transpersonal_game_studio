#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Eng_CompilationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngCompilationManager, Log, All);

/**
 * Compilation Management System
 * 
 * Central authority for managing compilation processes, build validation,
 * and ensuring code quality across the entire TranspersonalGame project.
 * Coordinates with compilation fixer and validator systems.
 */
UCLASS(ClassGroup=(Engine), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_CompilationManager();

protected:
    virtual void BeginPlay() override;

public:
    // === COMPILATION STATE ===
    
    /** Current compilation status */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation State")
    bool bIsCompilationValid;
    
    /** Build system is operational */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation State")
    bool bBuildSystemReady;
    
    /** All modules are properly linked */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation State")
    bool bModulesLinked;
    
    /** Code quality standards met */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation State")
    bool bCodeQualityValid;
    
    // === COMPILATION METRICS ===
    
    /** Number of successfully compiled modules */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation Metrics")
    int32 CompiledModules;
    
    /** Number of compilation errors found */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation Metrics")
    int32 CompilationErrors;
    
    /** Number of compilation warnings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation Metrics")
    int32 CompilationWarnings;
    
    /** Build time in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compilation Metrics")
    float BuildTimeSeconds;
    
    // === MODULE MANAGEMENT ===
    
    /** List of active modules */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module Management")
    TArray<FString> ActiveModules;
    
    /** List of failed modules */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module Management")
    TArray<FString> FailedModules;
    
    /** Module dependency graph */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module Management")
    TArray<FString> ModuleDependencies;
    
    /** Critical modules that must compile */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module Management")
    TArray<FString> CriticalModules;
    
    // === BUILD CONFIGURATION ===
    
    /** Current build configuration (Debug/Development/Shipping) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Configuration")
    FString BuildConfiguration;
    
    /** Target platform for compilation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Configuration")
    FString TargetPlatform;
    
    /** Optimization level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Configuration")
    int32 OptimizationLevel;
    
    /** Enable debug symbols */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Configuration")
    bool bEnableDebugSymbols;
    
    // === ERROR TRACKING ===
    
    /** Recent compilation error messages */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Error Tracking")
    TArray<FString> RecentErrors;
    
    /** Recent compilation warnings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Error Tracking")
    TArray<FString> RecentWarnings;
    
    /** Files with compilation issues */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Error Tracking")
    TArray<FString> ProblematicFiles;
    
    /** Error categories */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Error Tracking")
    TArray<FString> ErrorCategories;

public:
    // === COMPILATION MANAGEMENT FUNCTIONS ===
    
    /** Initialize compilation management system */
    UFUNCTION(BlueprintCallable, Category = "Compilation Management")
    void InitializeCompilationSystem();
    
    /** Start full project compilation */
    UFUNCTION(BlueprintCallable, Category = "Compilation Management")
    bool StartCompilation();
    
    /** Validate compilation state */
    UFUNCTION(BlueprintCallable, Category = "Compilation Management")
    bool ValidateCompilation();
    
    /** Fix compilation issues */
    UFUNCTION(BlueprintCallable, Category = "Compilation Management")
    void FixCompilationIssues();
    
    // === MODULE MANAGEMENT FUNCTIONS ===
    
    /** Register a new module */
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool RegisterModule(const FString& ModuleName);
    
    /** Validate module dependencies */
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool ValidateModuleDependencies();
    
    /** Compile specific module */
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool CompileModule(const FString& ModuleName);
    
    /** Check if module is critical */
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool IsModuleCritical(const FString& ModuleName) const;
    
    // === BUILD MANAGEMENT FUNCTIONS ===
    
    /** Set build configuration */
    UFUNCTION(BlueprintCallable, Category = "Build Management")
    void SetBuildConfiguration(const FString& Configuration);
    
    /** Clean build artifacts */
    UFUNCTION(BlueprintCallable, Category = "Build Management")
    void CleanBuild();
    
    /** Generate build report */
    UFUNCTION(BlueprintCallable, Category = "Build Management")
    FString GenerateBuildReport() const;
    
    /** Check build system health */
    UFUNCTION(BlueprintCallable, Category = "Build Management")
    bool CheckBuildSystemHealth();
    
    // === ERROR MANAGEMENT FUNCTIONS ===
    
    /** Add compilation error */
    UFUNCTION(BlueprintCallable, Category = "Error Management")
    void AddCompilationError(const FString& ErrorMessage, const FString& FileName);
    
    /** Add compilation warning */
    UFUNCTION(BlueprintCallable, Category = "Error Management")
    void AddCompilationWarning(const FString& WarningMessage, const FString& FileName);
    
    /** Clear error history */
    UFUNCTION(BlueprintCallable, Category = "Error Management")
    void ClearErrorHistory();
    
    /** Get error summary */
    UFUNCTION(BlueprintCallable, Category = "Error Management")
    FString GetErrorSummary() const;
    
    // === QUALITY ASSURANCE FUNCTIONS ===
    
    /** Validate code quality standards */
    UFUNCTION(BlueprintCallable, Category = "Quality Assurance")
    bool ValidateCodeQuality();
    
    /** Check coding standards compliance */
    UFUNCTION(BlueprintCallable, Category = "Quality Assurance")
    bool CheckCodingStandards();
    
    /** Analyze code complexity */
    UFUNCTION(BlueprintCallable, Category = "Quality Assurance")
    float AnalyzeCodeComplexity();
    
    /** Generate quality report */
    UFUNCTION(BlueprintCallable, Category = "Quality Assurance")
    FString GenerateQualityReport() const;

private:
    // === INTERNAL FUNCTIONS ===
    
    /** Initialize default modules */
    void InitializeDefaultModules();
    
    /** Setup build configuration */
    void SetupBuildConfiguration();
    
    /** Validate system requirements */
    bool ValidateSystemRequirements();
    
    /** Process compilation results */
    void ProcessCompilationResults();
    
    /** Update compilation metrics */
    void UpdateCompilationMetrics();
    
    /** Categorize errors */
    void CategorizeErrors();
    
    /** Check module health */
    bool CheckModuleHealth(const FString& ModuleName);
    
    /** Resolve module dependencies */
    bool ResolveModuleDependencies(const FString& ModuleName);
};