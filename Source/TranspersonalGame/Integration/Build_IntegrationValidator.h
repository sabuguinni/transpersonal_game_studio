#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Healthy     UMETA(DisplayName = "Healthy"),
    Moderate    UMETA(DisplayName = "Moderate"), 
    Critical    UMETA(DisplayName = "Critical"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status = EBuild_IntegrationStatus::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float HealthPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 PassedChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 BinaryCount = 0;

    FBuild_ValidationResult()
    {
        Status = EBuild_IntegrationStatus::Unknown;
        HealthPercentage = 0.0f;
        PassedChecks = 0;
        TotalChecks = 0;
        ActorCount = 0;
        BinaryCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    FBuild_ValidationResult ValidateIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateCriticalClasses();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateBinaryCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateLevelContent();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateQAIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetCurrentIntegrationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetIntegrationHealthPercentage() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_ValidationResult LastValidationResult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CriticalClassNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> QAClassNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MinimumActorCount = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float HealthyThreshold = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ModerateThreshold = 50.0f;

private:
    void InitializeCriticalClasses();
    void InitializeQAClasses();
    EBuild_IntegrationStatus CalculateStatus(float HealthPercentage) const;
};