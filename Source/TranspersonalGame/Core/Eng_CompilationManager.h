#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CompilationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngCompilation, Log, All);

/**
 * Engine Architect's Compilation Management System
 * Ensures all .h files have corresponding .cpp implementations
 * Validates module loading and class registration
 * Prevents ghost headers from breaking builds
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Validate all .h files have .cpp implementations */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    bool ValidateHeaderCppPairs();

    /** Check if TranspersonalGame module classes are loadable */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    bool ValidateModuleLoading();

    /** Clean up ghost headers (headers without implementations) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    int32 CleanupGhostHeaders();

    /** Force recompile TranspersonalGame module */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    bool ForceModuleRecompile();

    /** Get compilation status report */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    FString GetCompilationStatusReport();

protected:
    /** List of critical classes that must be loadable */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    TArray<FString> CriticalClasses;

    /** Directories to scan for header/cpp pairs */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
    TArray<FString> SourceDirectories;

    /** Last validation timestamp */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FDateTime LastValidationTime;

    /** Number of ghost headers found in last scan */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 GhostHeaderCount;

    /** Number of missing cpp files */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 MissingCppCount;

private:
    /** Scan directory for .h/.cpp pairs */
    void ScanDirectoryForPairs(const FString& Directory, TArray<FString>& OutMissingCpp);

    /** Check if a class can be loaded */
    bool CanLoadClass(const FString& ClassName);

    /** Get project source directory */
    FString GetProjectSourceDirectory() const;
};