#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BuildIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Success UMETA(DisplayName = "Success"),
    Warning UMETA(DisplayName = "Warning"),
    Error UMETA(DisplayName = "Error"),
    Critical UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    EBuild_ValidationResult ValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString ErrorMessage;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsCompiled = false;
        ValidationResult = EBuild_ValidationResult::Error;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bBuildHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FDateTime ValidationTimestamp;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        DinosaurCount = 0;
        PerformanceScore = 0.0f;
        bBuildHealthy = false;
        ValidationTimestamp = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationValidator();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport ValidateFullBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSystemIntegration(const FString& SystemName, const FString& ClassPath);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float CalculatePerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_SystemStatus> GetCriticalSystemStatuses();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, FString> EssentialSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 MaxDinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 MaxTotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float MinPerformanceThreshold;

private:
    void InitializeEssentialSystems();
    bool ValidateClassLoading(const FString& ClassPath);
    void LogValidationResult(const FString& SystemName, EBuild_ValidationResult Result, const FString& Message);
};