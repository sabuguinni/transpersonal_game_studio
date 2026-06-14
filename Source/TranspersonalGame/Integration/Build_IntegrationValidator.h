#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    NotTested       UMETA(DisplayName = "Not Tested"),
    Pass           UMETA(DisplayName = "Pass"),
    Fail           UMETA(DisplayName = "Fail"),
    Warning        UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EBuild_ValidationStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 ActorCount;

    FBuild_SystemValidation()
    {
        SystemName = TEXT("");
        Status = EBuild_ValidationStatus::NotTested;
        ErrorMessage = TEXT("");
        ActorCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Validation results
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Results")
    TArray<FBuild_SystemValidation> SystemValidations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Results")
    int32 TotalActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Results")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Results")
    bool bPerformanceWithinLimits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Results")
    FDateTime LastValidationTime;

public:
    // Validation functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void ValidateActorCounts();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void ValidatePerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void GenerateValidationReport();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_ValidationStatus GetOverallStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GetValidationSummary() const;

private:
    void AddValidationResult(const FString& SystemName, EBuild_ValidationStatus Status, const FString& ErrorMessage = TEXT(""), int32 ActorCount = 0);
    void ClearValidationResults();
    bool ValidateClassLoading(const FString& ClassName, const FString& ClassPath);
};