#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "Eng_ArchitectureValidator.generated.h"

UENUM(BlueprintType)
enum class EEng_ValidationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Valid           UMETA(DisplayName = "Valid"),
    Warning         UMETA(DisplayName = "Warning"),
    Critical        UMETA(DisplayName = "Critical"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EEng_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FEng_ModuleValidationResult()
    {
        ModuleName = TEXT("");
        Status = EEng_ValidationStatus::Unknown;
        ErrorMessage = TEXT("");
        ClassCount = 0;
        ValidationTime = 0.0f;
    }
};

/**
 * Engine Architecture Validator - Ensures compilation integrity and cross-module dependencies
 * Validates all C++ modules, checks for missing implementations, and prevents conflicts
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureValidator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    bool ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    FEng_ModuleValidationResult ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckCompilationIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetMissingImplementations();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetConflictingTypes();

    // Status Queries
    UFUNCTION(BlueprintPure, Category = "Architecture")
    EEng_ValidationStatus GetOverallStatus() const { return OverallStatus; }

    UFUNCTION(BlueprintPure, Category = "Architecture")
    TArray<FEng_ModuleValidationResult> GetValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintPure, Category = "Architecture")
    int32 GetTotalClassCount() const { return TotalClassCount; }

    UFUNCTION(BlueprintPure, Category = "Architecture")
    float GetLastValidationTime() const { return LastValidationTime; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    EEng_ValidationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_ModuleValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 TotalClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> KnownModules;

private:
    void ValidateModuleClasses(const FString& ModuleName, FEng_ModuleValidationResult& Result);
    void CheckHeaderCppPairs(const FString& ModuleName, FEng_ModuleValidationResult& Result);
    void ValidateSharedTypes();
    void LogValidationResults();
};