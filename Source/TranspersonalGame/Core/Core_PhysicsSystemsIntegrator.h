#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Core_PhysicsSystemsIntegrator.generated.h"

class UCore_PhysicsManager;
class UCore_CollisionSystem;
class UCore_DestructionSystem;
class UCore_RagdollSystem;
class UCore_TerrainPhysics;
class UCore_PhysicsPerformanceMonitor;

/**
 * Core Physics Systems Integrator
 * Coordinates all physics subsystems and ensures proper initialization order
 * Manages physics system lifecycle and inter-system communication
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsSystemsIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Initialize all physics subsystems in correct order */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSystems();

    /** Shutdown all physics subsystems safely */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ShutdownPhysicsSystems();

    /** Update all physics systems for current frame */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsSystems(float DeltaTime);

    /** Get physics performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    FString GetPhysicsPerformanceReport() const;

    /** Enable/disable physics system debugging */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetPhysicsDebugging(bool bEnabled);

    /** Check if all physics systems are healthy */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool ArePhysicsSystemsHealthy() const;

protected:
    /** Physics Manager - Core physics simulation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsManager> PhysicsManager;

    /** Collision System - Handles collision detection and response */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_CollisionSystem> CollisionSystem;

    /** Destruction System - Manages destructible objects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_DestructionSystem> DestructionSystem;

    /** Ragdoll System - Character physics and death states */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_RagdollSystem> RagdollSystem;

    /** Terrain Physics - Ground interaction and deformation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_TerrainPhysics> TerrainPhysics;

    /** Performance Monitor - Tracks physics performance metrics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsPerformanceMonitor> PerformanceMonitor;

    /** Whether physics systems are currently initialized */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsSystemsInitialized;

    /** Whether physics debugging is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsDebuggingEnabled;

    /** Physics update frequency (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    float PhysicsUpdateFrequency;

    /** Time accumulator for fixed physics updates */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    float PhysicsTimeAccumulator;

private:
    /** Initialize individual physics subsystem */
    bool InitializeSubsystem(UObject* Subsystem, const FString& SubsystemName);

    /** Shutdown individual physics subsystem */
    void ShutdownSubsystem(UObject* Subsystem, const FString& SubsystemName);

    /** Validate physics system health */
    bool ValidateSystemHealth() const;

    /** Log physics system status */
    void LogSystemStatus(const FString& Message, bool bIsError = false) const;
};