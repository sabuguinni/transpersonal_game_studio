#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Eng_CompilationFixer.generated.h"

/**
 * Engine Architect's Compilation Fixer
 * Supreme authority for fixing compilation issues across the entire project
 * Detects and resolves common C++ compilation problems automatically
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationFixer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationFixer();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Compilation fixing functions
    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Compilation")
    void FixAllCompilationIssues();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Compilation")
    void FixIncludePaths();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Compilation")
    void FixMissingHeaders();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Compilation")
    void FixAPICompatibility();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Compilation")
    void RemoveDuplicateFiles();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Compilation")
    void ImplementMissingStubs();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Compilation")
    bool ValidateProjectCompilation();

    // Diagnostic functions
    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Diagnostics")
    TArray<FString> GetCompilationErrors();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Diagnostics")
    TArray<FString> GetMissingIncludes();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect|Diagnostics")
    TArray<FString> GetDuplicateDefinitions();

protected:
    // Internal fixing methods
    void FixHeaderIncludeOrder();
    void FixGeneratedIncludeOrder();
    void FixUPropertyMacros();
    void FixUFunctionMacros();
    void FixForwardDeclarations();
    void FixModuleDependencies();

    // Validation state
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architect")
    bool bCompilationValid;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architect")
    int32 FixedIssuesCount;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architect")
    TArray<FString> LastCompilationErrors;
};