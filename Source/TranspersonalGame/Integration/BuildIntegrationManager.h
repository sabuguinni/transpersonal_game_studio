#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bHasErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float PerformanceScore;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsActive = false;
        bHasErrors = false;
        ActorCount = 0;
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bWithinActorLimits;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bBiomeDistributionValid;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float OverallHealthScore;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        TotalDinosaurs = 0;
        bWithinActorLimits = true;
        bBiomeDistributionValid = true;
        OverallHealthScore = 100.0f;
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
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CleanupExcessActors();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_SystemStatus> GetSystemStatuses();

    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor = true)
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor = true)
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float CalculatePerformanceScore();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> KnownSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 MaxActorLimit;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 MaxDinosaurLimit;

private:
    void InitializeKnownSystems();
    FBuild_SystemStatus CheckSystemStatus(const FString& SystemName);
    bool IsActorInBiome(AActor* Actor, EBiomeType BiomeType);
    void LogIntegrationStatus(const FBuild_IntegrationReport& Report);
};