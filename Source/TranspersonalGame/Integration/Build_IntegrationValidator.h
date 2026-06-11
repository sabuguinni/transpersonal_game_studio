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
    Stable      UMETA(DisplayName = "Stable"),
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
    int32 LoadedClasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalClasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 BinaryFiles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LevelActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ReportTimestamp;

    FBuild_ValidationResult()
    {
        Status = EBuild_IntegrationStatus::Unknown;
        HealthPercentage = 0.0f;
        LoadedClasses = 0;
        TotalClasses = 0;
        BinaryFiles = 0;
        LevelActors = 0;
        ReportTimestamp = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_ValidationResult ValidateProjectIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateClassLoading(const TArray<FString>& ClassNames, TArray<FString>& FailedClasses);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 CountCompiledBinaries();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 CountLevelActors();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus CalculateIntegrationStatus(float HealthPercentage);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport(const FBuild_ValidationResult& Result);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CoreSystemClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float HealthyThreshold = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float StableThreshold = 50.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_ValidationResult LastValidationResult;

private:
    void InitializeCoreSystemClasses();
    bool CheckBinaryCompilation();
    void LogValidationResults(const FBuild_ValidationResult& Result);
};