#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_ModuleValidator.generated.h"

USTRUCT(BlueprintType)
struct FEng_ModuleValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> ValidationWarnings;

    UPROPERTY(BlueprintReadOnly)
    float ValidationTime;

    FEng_ModuleValidationResult()
    {
        ModuleName = TEXT("Unknown");
        bIsValid = false;
        ValidationTime = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UEng_ModuleValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    FEng_ModuleValidationResult ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    TArray<FEng_ModuleValidationResult> ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    void RunCompilationTest();

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    bool CheckClassAvailability(const FString& ClassName);

private:
    void ValidateTranspersonalGameModule(FEng_ModuleValidationResult& Result);
    void ValidateEngineArchitecture(FEng_ModuleValidationResult& Result);
    void CheckRequiredClasses(FEng_ModuleValidationResult& Result);
};