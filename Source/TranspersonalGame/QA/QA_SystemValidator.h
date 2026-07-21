#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_SystemValidator.generated.h"

/**
 * QA System Validator - Automated testing and validation system
 * Provides runtime validation of game systems and integration points
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_SystemValidator : public UObject
{
    GENERATED_BODY()

public:
    UQA_SystemValidator();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateCoreSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateQuestSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateLevelIntegrity();

    // Comprehensive validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    float RunFullValidationSuite();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    int32 GetDinosaurCount();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    TArray<FString> GetSystemHealthReport();

protected:
    // Validation helper functions
    bool ValidateClassLoading(const FString& ClassName, const FString& ClassPath);
    bool ValidateActorSpawning(UClass* ActorClass, const FVector& Location);
    bool ValidateComponentIntegration(AActor* Actor, UClass* ComponentClass);

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    int32 MaxActorCount = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    int32 MaxDinosaurCount = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    float MinSystemHealthPercentage = 80.0f;

    // Validation results
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TMap<FString, bool> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    float LastValidationScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    FDateTime LastValidationTime;
};