#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString LastError;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsInitialized = false;
        HealthPercentage = 0.0f;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 LoadedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 InitializedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float OverallHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FDateTime LastValidationTime;

    FBuild_IntegrationReport()
    {
        TotalSystems = 0;
        LoadedSystems = 0;
        InitializedSystems = 0;
        OverallHealth = 0.0f;
        LastValidationTime = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetFailedSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetOverallSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor = true)
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor = true)
    void GenerateIntegrationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, FBuild_SystemStatus> SystemStatusMap;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    TArray<FString> CoreSystemNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    bool bAutoValidateOnStartup;

private:
    void InitializeCoreSystemsList();
    bool TestSystemLoading(const FString& SystemName);
    bool TestSystemInitialization(const FString& SystemName);
    void UpdateSystemStatus(const FString& SystemName, bool bLoaded, bool bInitialized, const FString& Error = TEXT(""));
    void CalculateOverallHealth();

    FTimerHandle ValidationTimerHandle;
    void PeriodicValidation();
};