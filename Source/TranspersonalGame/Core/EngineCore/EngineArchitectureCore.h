#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureCore.generated.h"

/**
 * Engine Architecture Core - Central coordinator for all engine systems
 * Manages system initialization, validation, and cross-system communication
 * This is the technical backbone that ensures all game systems work together
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEngineArchitectureCore : public AActor
{
    GENERATED_BODY()

public:
    AEngineArchitectureCore();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // === SYSTEM REGISTRATION ===
    
    /** Register a new system with the architecture core */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystem(const FString& SystemName, UObject* SystemInstance);
    
    /** Unregister a system */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnregisterSystem(const FString& SystemName);
    
    /** Get a registered system by name */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UObject* GetSystem(const FString& SystemName);

    // === SYSTEM VALIDATION ===
    
    /** Validate all registered systems are functioning correctly */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor)
    bool ValidateAllSystems();
    
    /** Get system health status */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TMap<FString, bool> GetSystemHealthStatus();

    // === PERFORMANCE MONITORING ===
    
    /** Get current frame time */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetCurrentFrameTime() const;
    
    /** Get memory usage statistics */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GetMemoryUsageStats() const;

    // === ARCHITECTURE CONFIGURATION ===
    
    /** Enable/disable system performance monitoring */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architecture")
    bool bEnablePerformanceMonitoring;
    
    /** Enable/disable system validation checks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architecture")
    bool bEnableSystemValidation;
    
    /** Maximum allowed frame time before warning (in milliseconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architecture")
    float MaxFrameTimeMs;
    
    /** Systems update frequency (times per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Architecture")
    float SystemUpdateFrequency;

protected:
    // === CORE COMPONENTS ===
    
    /** Root scene component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // === SYSTEM REGISTRY ===
    
    /** Map of registered systems */
    UPROPERTY()
    TMap<FString, UObject*> RegisteredSystems;
    
    /** System health status cache */
    UPROPERTY()
    TMap<FString, bool> SystemHealthCache;

    // === PERFORMANCE TRACKING ===
    
    /** Frame time accumulator for averaging */
    float FrameTimeAccumulator;
    
    /** Frame count for averaging */
    int32 FrameCount;
    
    /** Last system validation time */
    float LastValidationTime;
    
    /** System update timer */
    float SystemUpdateTimer;

    // === INTERNAL METHODS ===
    
    /** Initialize core architecture systems */
    void InitializeCoreArchitecture();
    
    /** Validate individual system */
    bool ValidateSystem(const FString& SystemName, UObject* SystemInstance);
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics(float DeltaTime);
    
    /** Log system status */
    void LogSystemStatus();
    
    /** Handle system failure */
    void HandleSystemFailure(const FString& SystemName, const FString& ErrorMessage);
};