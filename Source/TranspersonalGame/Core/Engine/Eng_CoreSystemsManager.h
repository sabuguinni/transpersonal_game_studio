#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Eng_CoreSystemsManager.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Uninitialized,
    Initializing,
    Active,
    Paused,
    Error,
    Shutdown
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EEng_SystemStatus Status;

    FEng_SystemMetrics()
    {
        FrameTime = 0.0f;
        ActorCount = 0;
        MemoryUsageMB = 0.0f;
        Status = EEng_SystemStatus::Uninitialized;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CoreSystemsManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CoreSystemsManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core system management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeCoreSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownCoreSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_SystemStatus GetSystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_SystemMetrics GetSystemMetrics() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSystemPerformance();

    // System registration
    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    void UnregisterCoreSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Registry")
    UObject* GetRegisteredSystem(const FString& SystemName);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    EEng_SystemStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FEng_SystemMetrics CurrentMetrics;

    UPROPERTY()
    TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

    UPROPERTY()
    FTimerHandle MetricsUpdateTimer;

private:
    void InternalMetricsUpdate();
    void ValidateRegisteredSystems();
    bool CheckSystemDependencies();
};