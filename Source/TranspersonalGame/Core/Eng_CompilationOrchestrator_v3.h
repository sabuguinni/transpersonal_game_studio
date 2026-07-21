#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_CompilationOrchestrator_v3.generated.h"

/**
 * Engine Architect - Compilation Orchestrator v3
 * CRITICAL SYSTEM: Manages compilation state and enforces architectural standards
 * Ensures all .h files have corresponding .cpp implementations
 * Validates class loading and prevents orphaned headers
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationOrchestrator_v3 : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationOrchestrator_v3();

    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core compilation management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateCompilationState();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool FixOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateCriticalClasses();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetOrphanedHeaderCount();

    // Architecture enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforceArchitecturalStandards();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleDependencies();

    // Compilation reporting
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsCompilationHealthy();

protected:
    // Critical class validation
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> CriticalClassNames;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TMap<FString, bool> ClassLoadingStatus;

    // Compilation state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    int32 OrphanedHeaderCount;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    int32 FailedClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bCompilationHealthy;

    // Internal validation methods
    bool ValidateClassLoading(const FString& ClassName);
    bool CheckHeaderCppPairs();
    bool ValidateIncludeStructure();
    void UpdateCompilationMetrics();

    // Architecture enforcement helpers
    bool ValidateUPropertyUsage();
    bool ValidateUFunctionImplementations();
    bool CheckGeneratedBodyMacros();
};