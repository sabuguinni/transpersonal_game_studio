#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Eng_CompilationFixer.generated.h"

/**
 * Engine Architect - Compilation Fixer System
 * Identifies and fixes critical compilation issues across the project
 * Ensures all .h files have matching .cpp implementations
 * Validates UE5.5 API compatibility
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationFixer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationFixer();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Compilation fixing methods
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void FixMissingCppFiles();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateApiCompatibility();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void FixIncludePaths();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RemoveDuplicateFiles();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RunCompilationTest();

    // Status tracking
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    int32 FixedFilesCount;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    int32 RemainingErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bCompilationClean;

private:
    // Internal fixing methods
    void FixHeaderOnlyClasses();
    void FixApiMacros();
    void FixIncludeOrder();
    void ValidateGeneratedHeaders();
    
    // Error tracking
    TArray<FString> CompilationErrors;
    TArray<FString> FixedFiles;
};