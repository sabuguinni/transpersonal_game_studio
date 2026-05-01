#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemManager.generated.h"

/**
 * Core System Manager - Central architecture coordinator for all game systems
 * Manages initialization order, system dependencies, and performance monitoring
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_SystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemInitialized(const FString& SystemName) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemPerformanceMetric(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogSystemStatus() const;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, int32 InitializationPriority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystem(const FString& SystemName);

protected:
    // System tracking
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TMap<FString, bool> SystemInitializationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TMap<FString, int32> SystemPriorities;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TMap<FString, float> SystemPerformanceMetrics;

    // Core systems list
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> CoreSystems;

    // Initialization state
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bAllSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    float LastPerformanceCheck;

private:
    // Internal system management
    void InitializeSystemByName(const FString& SystemName);
    void UpdatePerformanceMetrics();
    void ValidateSystemDependencies();

    // Performance monitoring
    FDateTime LastMetricsUpdate;
    static constexpr float METRICS_UPDATE_INTERVAL = 1.0f;
};