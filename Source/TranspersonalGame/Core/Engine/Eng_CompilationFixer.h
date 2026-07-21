#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/Object.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Eng_CompilationFixer.generated.h"

// Forward declarations
class UEngArch_BiomeManager;
class AEng_DinosaurBase;

/**
 * Compilation Fixer - Resolves missing implementations and duplicate definitions
 * Cycle 009 - Engine Architect
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationFixer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationFixer();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Fix missing .cpp implementations
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void FixMissingImplementations();

    // Remove duplicate files
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void RemoveDuplicateDefinitions();

    // Validate compilation status
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateCompilationStatus();

    // Fix include paths
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void FixIncludePaths();

    // Fix API compatibility issues
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void FixAPICompatibility();

protected:
    // Track compilation issues
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TArray<FString> CompilationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TArray<FString> FixedFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bCompilationClean;

    // Fix specific file types
    void FixHeaderOnlyClasses();
    void FixEmptyImplementations();
    void FixDuplicateTypes();
    void FixMissingIncludes();
    void FixUE5Compatibility();

private:
    // Internal tracking
    TMap<FString, FString> FilePathMap;
    TArray<FString> ProcessedFiles;
    
    // Validation helpers
    bool IsFileEmpty(const FString& FilePath);
    bool HasMatchingImplementation(const FString& HeaderPath);
    void LogCompilationFix(const FString& Message);
};