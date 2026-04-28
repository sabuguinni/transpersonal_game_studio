#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

/**
 * Engine Architecture Manager - Core architectural oversight system
 * Manages module dependencies, performance monitoring, and system integration
 * Enforces architectural rules across all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemModule(const FString& ModuleName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemModuleRegistered(const FString& ModuleName) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReport() const;

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    float GetMemoryUsageMB() const;

    // System Integration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void NotifySystemEvent(const FString& SystemName, const FString& EventType);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetActiveSystemModules() const;

protected:
    // Module Registry
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, int32> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> SystemEventLog;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LastActorCount;

    // Architecture Rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    float MaxAllowedFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    int32 MaxAllowedActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    float MaxMemoryUsageMB;

private:
    // Internal validation
    void ValidatePerformanceThresholds();
    void UpdatePerformanceMetrics();
    void LogArchitectureEvent(const FString& Event);

    // Timer handles
    FTimerHandle PerformanceUpdateTimer;
    FTimerHandle ValidationTimer;
};