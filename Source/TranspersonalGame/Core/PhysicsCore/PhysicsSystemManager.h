// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "TranspersonalGameTypes.h"
#include "PhysicsSystemManager.generated.h"

class UTranspersonalRagdollComponent;
class UTranspersonalDestructionComponent;
class UTranspersonalVehicleComponent;

/**
 * @brief Core physics orchestration system for Transpersonal Game
 * 
 * Manages all physics subsystems including:
 * - Chaos Physics integration
 * - Ragdoll system coordination
 * - Destruction system management
 * - Vehicle physics oversight
 * - Performance monitoring and LOD
 * 
 * Design Philosophy (Casey Muratori): 
 * "Physics that can't be tested in isolation will fail when it matters most"
 * 
 * Performance Philosophy (Mike Acton):
 * "Performance is not a feature - it's a requirement"
 * 
 * @author Core Systems Programmer #03
 * @version 1.0
 */
UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

    //~ Begin UActorComponent Interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End UActorComponent Interface

    /**
     * @brief Initialize the physics system with world context
     * @param InWorld The world to initialize physics for
     * @return True if initialization succeeded
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool InitializePhysicsSystem(UWorld* InWorld);

    /**
     * @brief Register a ragdoll component with the system
     * @param RagdollComponent The component to register
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterRagdollComponent(UTranspersonalRagdollComponent* RagdollComponent);

    /**
     * @brief Register a destruction component with the system
     * @param DestructionComponent The component to register
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterDestructionComponent(UTranspersonalDestructionComponent* DestructionComponent);

    /**
     * @brief Register a vehicle component with the system
     * @param VehicleComponent The component to register
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterVehicleComponent(UTranspersonalVehicleComponent* VehicleComponent);

    /**
     * @brief Get current physics performance metrics
     * @return Physics performance data
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FTranspersonalPhysicsMetrics GetPhysicsMetrics() const;

    /**
     * @brief Set physics quality level for performance scaling
     * @param QualityLevel 0=Low, 1=Medium, 2=High, 3=Ultra
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsQualityLevel(int32 QualityLevel);

    /**
     * @brief Emergency physics reset - use when physics goes unstable
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EmergencyPhysicsReset();

protected:
    /**
     * @brief Initialize Chaos Physics settings
     */
    void InitializeChaosPhysics();

    /**
     * @brief Update physics performance monitoring
     */
    void UpdatePerformanceMetrics(float DeltaTime);

    /**
     * @brief Apply physics LOD based on distance and performance
     */
    void ApplyPhysicsLOD();

    /**
     * @brief Validate physics system integrity
     * @return True if all systems are functioning correctly
     */
    bool ValidatePhysicsIntegrity() const;

private:
    /** Current world context */
    UPROPERTY()
    TWeakObjectPtr<UWorld> WorldContext;

    /** Registered ragdoll components */
    UPROPERTY()
    TArray<TWeakObjectPtr<UTranspersonalRagdollComponent>> RegisteredRagdolls;

    /** Registered destruction components */
    UPROPERTY()
    TArray<TWeakObjectPtr<UTranspersonalDestructionComponent>> RegisteredDestructionComponents;

    /** Registered vehicle components */
    UPROPERTY()
    TArray<TWeakObjectPtr<UTranspersonalVehicleComponent>> RegisteredVehicles;

    /** Current physics metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    FTranspersonalPhysicsMetrics CurrentMetrics;

    /** Physics quality level (0-3) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    int32 PhysicsQualityLevel = 2;

    /** Maximum physics objects before LOD kicks in */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    int32 MaxPhysicsObjects = 500;

    /** Physics update frequency (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    float PhysicsUpdateFrequency = 60.0f;

    /** Performance monitoring interval */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics System", meta = (AllowPrivateAccess = "true"))
    float MetricsUpdateInterval = 1.0f;

    /** Last metrics update time */
    float LastMetricsUpdate = 0.0f;

    /** Physics system initialized flag */
    bool bPhysicsSystemInitialized = false;

    /** Emergency reset cooldown */
    float EmergencyResetCooldown = 0.0f;
};