#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
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
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString LastError;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsCompiled = false;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomeDistribution
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FVector CenterLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float PerformanceScore;

    FBuild_BiomeDistribution()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        ActorCount = 0;
        PerformanceScore = 1.0f;
    }
};

/**
 * Build Integration Manager - Coordinates all systems and validates integration
 * Agent #19 - Integration & Build Agent
 */
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
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckSystemCompilation(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SaveMapSafely();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_SystemStatus> GetSystemStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_BiomeDistribution> GetBiomeDistributionReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void PerformIntegrationTest();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsGamePlayable();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_BiomeDistribution> BiomeDistributions;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bAllSystemsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime;

private:
    void InitializeBiomeData();
    void CheckCoreClasses();
    void ValidateActorDistribution();
    void CheckPerformanceLimits();
};

#include "BuildIntegrationManager.generated.h"