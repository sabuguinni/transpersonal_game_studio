#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ModuleValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationWarnings;

    FEng_ModuleValidationResult()
    {
        ModuleName = TEXT("");
        bIsValid = false;
        ValidationScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ModuleValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    FEng_ModuleValidationResult ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    TArray<FEng_ModuleValidationResult> ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    bool IsModuleArchitectureCompliant(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    void GenerateArchitectureReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TMap<FString, FEng_ModuleValidationResult> ModuleValidationCache;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float OverallArchitectureScore;

private:
    bool ValidateModuleStructure(const FString& ModuleName, FEng_ModuleValidationResult& Result);
    bool ValidateModuleDependencies(const FString& ModuleName, FEng_ModuleValidationResult& Result);
    bool ValidateModulePerformance(const FString& ModuleName, FEng_ModuleValidationResult& Result);
    void UpdateArchitectureScore();
};