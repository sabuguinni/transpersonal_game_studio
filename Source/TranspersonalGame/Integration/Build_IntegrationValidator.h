#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Functional  UMETA(DisplayName = "Functional"),
    Degraded    UMETA(DisplayName = "Degraded"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float PerformanceScore;

    FBuild_SystemReport()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Unknown;
        ErrorMessage = TEXT("");
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FunctionalSystemCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float OverallHealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsGameReady;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        FunctionalSystemCount = 0;
        OverallHealthScore = 0.0f;
        bIsGameReady = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UObject
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    FBuild_IntegrationReport ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    FBuild_SystemReport ValidateSystem(const FString& SystemName, UClass* SystemClass);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    int32 GetActorCount();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    float CalculatePerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateCriticalSystems();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalSystemNames;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastReport;

private:
    bool ValidateClassLoading(const FString& ClassName);
    float MeasureSystemPerformance(UClass* SystemClass);
    bool CheckSystemDependencies(const FString& SystemName);
};

#include "Build_IntegrationValidator.generated.h"