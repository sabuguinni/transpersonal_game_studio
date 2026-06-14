#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 WarningCount = 0;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_SystemValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FEng_ValidationResult> ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationResult ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool IsSystemValid(const FString& SystemName);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void RegisterSystem(const FString& SystemName, UObject* SystemObject);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void UnregisterSystem(const FString& SystemName);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FEng_ValidationResult> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float LastValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalSystemsValidated = 0;

    FEng_ValidationResult ValidateGameMode();
    FEng_ValidationResult ValidatePlayerController();
    FEng_ValidationResult ValidateCharacter();
    FEng_ValidationResult ValidateWorldSystems();
    FEng_ValidationResult ValidatePerformance();

private:
    bool ValidateObjectExists(UObject* Object, const FString& ObjectName, FString& OutError);
    bool ValidateComponentSetup(AActor* Actor, const FString& ActorName, FString& OutError);
    void LogValidationResult(const FEng_ValidationResult& Result);
};