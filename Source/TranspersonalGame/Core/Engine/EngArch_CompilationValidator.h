#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EngArch_CompilationValidator.generated.h"

/**
 * Engine Architect Compilation Validator
 * Validates compilation status and module loading for all game systems
 * Ensures architectural compliance across the entire codebase
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngArch_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Validate that all core modules are properly loaded and compiled */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleCompilation();

    /** Check if a specific class is properly loaded and accessible */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateClassLoading(const FString& ClassName);

    /** Validate architectural compliance for the entire project */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateArchitecturalCompliance();

    /** Get compilation status report */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GetCompilationStatusReport();

    /** Force recompilation of specific modules */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    void ForceModuleRecompilation();

protected:
    /** List of critical classes that must be available */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    TArray<FString> CriticalClasses;

    /** Last validation timestamp */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FDateTime LastValidationTime;

    /** Current validation status */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsValidationPassing;

    /** Detailed validation results */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TMap<FString, bool> ValidationResults;

private:
    /** Internal validation helper functions */
    bool ValidateTranspersonalGameModule();
    bool ValidateCorePhysicsSystems();
    bool ValidateCharacterSystems();
    bool ValidateDinosaurSystems();
    bool ValidateWorldGeneration();
    
    /** Log validation results */
    void LogValidationResults();
};