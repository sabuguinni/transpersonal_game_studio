#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "../SharedTypes.h"
#include "IntegrationSystemManager.generated.h"

USTRUCT(BlueprintType)
struct FInteg_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastCheckTime = 0.0f;

    FInteg_SystemStatus()
    {
        bIsHealthy = false;
        SystemName = TEXT("Unknown");
        LastError = TEXT("");
        LastCheckTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FInteg_ActorInventory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DirectionalLights = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 SkyLights = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 AtmosphereComponents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FogComponents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 PlayerStarts = 0;

    FInteg_ActorInventory()
    {
        TotalActors = 0;
        DirectionalLights = 0;
        SkyLights = 0;
        AtmosphereComponents = 0;
        FogComponents = 0;
        DinosaurActors = 0;
        PlayerStarts = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UIntegrationSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UIntegrationSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FInteg_ActorInventory GetActorInventory();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FInteg_SystemStatus> GetSystemStatuses();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemHealthy(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ForceSystemValidation(const FString& SystemName);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetCurrentFramerate();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetActorCount();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OptimizeLevel();

protected:
    // System validation
    void ValidateWorldGeneration();
    void ValidateCharacterSystems();
    void ValidateAISystems();
    void ValidateLightingSystems();
    void ValidateVFXSystems();

    // Actor management
    void CleanupLightingActors();
    void CleanupAtmosphereActors();
    void ValidateEssentialActors();

    // Performance tracking
    void UpdatePerformanceMetrics();

private:
    UPROPERTY()
    TMap<FString, FInteg_SystemStatus> SystemStatuses;

    UPROPERTY()
    FInteg_ActorInventory CachedInventory;

    UPROPERTY()
    float LastValidationTime;

    UPROPERTY()
    float ValidationInterval;

    UPROPERTY()
    bool bAutoCleanupEnabled;

    // Performance metrics
    UPROPERTY()
    float AverageFramerate;

    UPROPERTY()
    int32 LastActorCount;

    UPROPERTY()
    float LastOptimizationTime;
};