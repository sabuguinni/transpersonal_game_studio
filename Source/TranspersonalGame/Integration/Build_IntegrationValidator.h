#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Success     UMETA(DisplayName = "Success"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float PerformanceScore;

    FBuild_ValidationReport()
    {
        SystemName = TEXT("");
        Result = EBuild_ValidationResult::Success;
        Message = TEXT("");
        ActorCount = 0;
        PerformanceScore = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationReport> ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationReport ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculatePerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ValidationReport> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastPerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

private:
    FBuild_ValidationReport ValidateCharacterSystem();
    FBuild_ValidationReport ValidateWorldGeneration();
    FBuild_ValidationReport ValidateDinosaurAI();
    FBuild_ValidationReport ValidateCombatSystem();
    FBuild_ValidationReport ValidateAudioSystem();
    FBuild_ValidationReport ValidateVFXSystem();
    
    bool CheckClassLoaded(const FString& ClassName);
    int32 CountActorsInBiome(const FVector& BiomeCenter, float Radius);
};