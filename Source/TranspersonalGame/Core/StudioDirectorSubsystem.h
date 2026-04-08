#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Tickable.h"
#include "StudioDirectorSubsystem.generated.h"

class UPhysicsSystemManager;
class ATranspersonalGameMode;

/**
 * Studio Director Subsystem
 * Central coordination system that manages all game systems and ensures
 * they work together according to the creative vision.
 */
UCLASS()
class TRANSPERSONALGAME_API UStudioDirectorSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UStudioDirectorSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // FTickableGameObject interface
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;

    /** Get the Studio Director subsystem instance */
    UFUNCTION(BlueprintPure, Category = "Studio Director", CallInEditor = true)
    static UStudioDirectorSubsystem* Get(const UObject* WorldContext);

    /** Initialize all game systems in the correct order */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeGameSystems();

    /** Shutdown all game systems in reverse order */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ShutdownGameSystems();

    /** Get current system initialization status */
    UFUNCTION(BlueprintPure, Category = "Studio Director")
    bool AreSystemsInitialized() const { return bSystemsInitialized; }

    /** Force update all systems (for debugging) */
    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void ForceUpdateAllSystems();

    /** Get system performance metrics */
    UFUNCTION(BlueprintPure, Category = "Studio Director")
    FString GetSystemPerformanceReport() const;

    /** Register a system with the director */
    void RegisterSystem(const FString& SystemName, UObject* SystemObject);

    /** Unregister a system from the director */
    void UnregisterSystem(const FString& SystemName);

    /** Check if a specific system is registered and active */
    UFUNCTION(BlueprintPure, Category = "Studio Director")
    bool IsSystemActive(const FString& SystemName) const;

protected:
    /** Initialize core engine systems first */
    void InitializeCoreEngineSystems();

    /** Initialize gameplay systems */
    void InitializeGameplaySystems();

    /** Initialize content and presentation systems */
    void InitializeContentSystems();

    /** Monitor system health and performance */
    void MonitorSystemHealth();

    /** Handle system failures and recovery */
    void HandleSystemFailure(const FString& SystemName, const FString& ErrorMessage);

private:
    /** Are all systems properly initialized */
    UPROPERTY()
    bool bSystemsInitialized = false;

    /** Registry of all active systems */
    UPROPERTY()
    TMap<FString, TObjectPtr<UObject>> RegisteredSystems;

    /** System initialization order */
    TArray<FString> SystemInitializationOrder;

    /** Performance monitoring data */
    TMap<FString, float> SystemPerformanceMetrics;

    /** Last health check time */
    float LastHealthCheckTime = 0.0f;

    /** Health check interval in seconds */
    float HealthCheckInterval = 5.0f;

    /** System failure counts */
    TMap<FString, int32> SystemFailureCounts;

    /** Maximum allowed failures before system shutdown */
    int32 MaxSystemFailures = 3;

    // Core system references
    UPROPERTY()
    TObjectPtr<UPhysicsSystemManager> PhysicsManager;

    /** Studio Director configuration */
    UPROPERTY(EditAnywhere, Category = "Configuration")
    bool bEnableSystemMonitoring = true;

    UPROPERTY(EditAnywhere, Category = "Configuration")
    bool bAutoRecoverFromFailures = true;

    UPROPERTY(EditAnywhere, Category = "Configuration")
    bool bLogSystemPerformance = false;
};