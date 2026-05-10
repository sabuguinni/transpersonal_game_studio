#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "Eng_CriticalCompilationFixer.generated.h"

/**
 * Critical Compilation Fixer - Engine Architect Cycle 002
 * Fixes critical compilation issues blocking the playable prototype
 * Ensures TranspersonalCharacter and core classes are properly compiled and accessible
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CriticalCompilationFixer : public UObject
{
    GENERATED_BODY()

public:
    UEng_CriticalCompilationFixer();

    // Critical compilation fixing methods
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool FixTranspersonalCharacterCompilation();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool FixGameModeCompilation();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool FixDinosaurBaseCompilation();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateAllCriticalClasses();

    // Compilation status checking
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsClassCompiledAndAccessible(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetFailedCompilationClasses();

    // Critical system validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePlayablePrototypeReadiness();

protected:
    // Internal compilation fixing methods
    bool FixHeaderIncludes(const FString& ClassName);
    bool FixImplementationStubs(const FString& ClassName);
    bool ValidateClassHierarchy(const FString& ClassName);

    // Compilation status tracking
    UPROPERTY(BlueprintReadOnly, Category = "Compilation Status")
    TArray<FString> FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Status")
    TArray<FString> FixedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation Status")
    bool bPlayablePrototypeReady;
};