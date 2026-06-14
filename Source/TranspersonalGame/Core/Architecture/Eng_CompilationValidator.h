#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_CompilationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ClassValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ClassName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly)
    bool bHasValidCDO;

    UPROPERTY(BlueprintReadOnly)
    int32 PropertyCount;

    UPROPERTY(BlueprintReadOnly)
    int32 FunctionCount;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    FEng_ClassValidation()
        : bIsLoaded(false)
        , bHasValidCDO(false)
        , PropertyCount(0)
        , FunctionCount(0)
    {}
};

UCLASS()
class TRANSPERSONALGAME_API UEng_CompilationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    void ValidateAllClasses();

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    FEng_ClassValidation ValidateClass(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    TArray<FEng_ClassValidation> GetValidationResults() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    bool AreAllClassesValid() const;

    UFUNCTION(BlueprintCallable, Category = "Compilation Validator")
    void GenerateValidationReport();

private:
    UPROPERTY()
    TArray<FEng_ClassValidation> ValidationResults;

    TArray<FString> GetKnownClasses() const;
    bool ValidateClassProperties(UClass* Class, FEng_ClassValidation& Validation);
    bool ValidateClassFunctions(UClass* Class, FEng_ClassValidation& Validation);
};

#include "Eng_CompilationValidator.generated.h"