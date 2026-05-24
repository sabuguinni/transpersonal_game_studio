#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CompilationValidator.generated.h"

/**
 * Engine Architect - Compilation Validation System
 * Validates that all required .cpp files exist and compile correctly
 * Enforces the "Every .h MUST have a matching .cpp" rule
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> MissingCppFiles;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> CompilationErrors;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly)
    bool bAllClassesLoaded = false;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalHeaderFiles = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalCppFiles = 0;

    FEng_CompilationReport()
    {
        MissingCppFiles.Empty();
        CompilationErrors.Empty();
        LoadedClasses.Empty();
        bAllClassesLoaded = false;
        TotalHeaderFiles = 0;
        TotalCppFiles = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Validates that all .h files have corresponding .cpp files
     * Returns detailed report of missing files and compilation status
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    FEng_CompilationReport ValidateCompilation();

    /**
     * Checks if a specific class is properly loaded in the engine
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsClassLoaded(const FString& ClassName);

    /**
     * Forces recompilation of the TranspersonalGame module
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    bool ForceRecompile();

    /**
     * Gets the current compilation status summary
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GetCompilationSummary();

    /**
     * Validates that all core game classes are properly loaded
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateCoreClasses();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    FEng_CompilationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> RequiredCoreClasses;

    /**
     * Scans the source directory for .h files without matching .cpp
     */
    TArray<FString> FindOrphanedHeaders();

    /**
     * Checks if a .cpp file exists for the given .h file
     */
    bool DoesCppExist(const FString& HeaderPath);

    /**
     * Gets all classes that should be loaded from TranspersonalGame module
     */
    void PopulateRequiredClasses();

private:
    bool bIsInitialized = false;
    FDateTime LastValidationTime;
};