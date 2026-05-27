#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BuildIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Operational UMETA(DisplayName = "Operational"),
    Failed      UMETA(DisplayName = "Failed"),
    Error       UMETA(DisplayName = "Error")
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
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationTime;

    FBuild_SystemReport()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Unknown;
        Details = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 OperationalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildReady;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ValidationTimestamp;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        OperationalSystems = 0;
        TotalSystems = 0;
        bBuildReady = false;
        ValidationTimestamp = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationValidator();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_SystemStatus ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_SystemStatus ValidateGameStateSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_SystemStatus ValidateWorldGenSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_SystemStatus ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformMemoryOptimization();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool SaveCurrentMap();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CoreSystemClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxActorCountWarning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxActorCountCritical;

private:
    EBuild_SystemStatus ValidateSystemClass(const FString& ClassPath);
    void LogSystemStatus(const FString& SystemName, EBuild_SystemStatus Status);
};

#include "BuildIntegrationValidator.generated.h"