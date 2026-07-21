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
    Stable      UMETA(DisplayName = "Stable"),
    Functional  UMETA(DisplayName = "Functional"),
    Unstable    UMETA(DisplayName = "Unstable"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float HealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastError;

    FBuild_SystemHealth()
    {
        SystemName = TEXT("");
        bIsOperational = false;
        HealthScore = 0.0f;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 WorkingSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemHealth> SystemHealthData;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString PerformanceStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString Recommendations;

    FBuild_IntegrationReport()
    {
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        TotalActors = 0;
        WorkingSystems = 0;
        TotalSystems = 0;
        IntegrationScore = 0.0f;
        PerformanceStatus = TEXT("Unknown");
        Recommendations = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemHealth(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void EnforceActorCaps();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetLoadedModules() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus CalculateOverallStatus() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxDinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float TimeSinceLastValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CoreSystemsToValidate;

private:
    void ValidateAllSystems();
    FBuild_SystemHealth ValidateIndividualSystem(const FString& SystemName);
    void CleanupExcessActors();
    void LogIntegrationStatus(const FBuild_IntegrationReport& Report);
};