// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "ChaosPhysicsManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogChaosPhysicsManager, Log, All);

/**
 * Chaos Physics Configuration for different simulation types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChaosPhysicsConfig
{
    GENERATED_BODY()

    /** Maximum number of physics iterations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Config")
    int32 MaxIterations = 8;

    /** Collision pair iterations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Config")
    int32 CollisionPairIterations = 2;

    /** Push out iterations for depenetration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Config")
    int32 PushOutIterations = 4;

    /** Collision margin fraction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Config")
    float CollisionMarginFraction = 0.04f;

    /** Maximum collision margin */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Config")
    float CollisionMarginMax = 1.0f;

    /** Collision cull distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Config")
    float CollisionCullDistance = 5.0f;

    /** Enable continuous collision detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Config")
    bool bEnableCCD = true;

    /** Enable enhanced determinism */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Config")
    bool bEnhancedDeterminism = false;
};

/**
 * Physics simulation profile for different object types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPhysicsSimulationProfile
{
    GENERATED_BODY()

    /** Profile name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString ProfileName;

    /** Mass scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    float MassScale = 1.0f;

    /** Linear damping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    float LinearDamping = 0.01f;

    /** Angular damping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    float AngularDamping = 0.0f;

    /** Maximum linear velocity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    float MaxLinearVelocity = 3000.0f;

    /** Maximum angular velocity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    float MaxAngularVelocity = 3600.0f;

    /** Sleep threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    float SleepThreshold = 0.1f;

    /** Stabilization threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    float StabilizationThreshold = 0.05f;
};

/**
 * Chaos Physics Manager
 * Manages Chaos Physics system configuration and optimization
 * Handles physics profiles for different object types (dinosaurs, environment, debris)
 */
UCLASS()
class TRANSPERSONALGAME_API UChaosPhysicsManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the Chaos physics manager instance */
    UFUNCTION(BlueprintPure, Category = "Chaos Physics")
    static UChaosPhysicsManager* Get(const UObject* WorldContext);

    /** Initialize Chaos Physics with optimized settings */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void InitializeChaosPhysics();

    /** Configure Chaos settings for large-scale simulation */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void ConfigureLargeScaleSimulation();

    /** Apply physics profile to an actor */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void ApplyPhysicsProfile(AActor* Actor, const FString& ProfileName);

    /** Create a new physics profile */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void CreatePhysicsProfile(const FString& ProfileName, const FPhysicsSimulationProfile& Profile);

    /** Get physics profile by name */
    UFUNCTION(BlueprintPure, Category = "Chaos Physics")
    FPhysicsSimulationProfile GetPhysicsProfile(const FString& ProfileName) const;

    /** Enable/disable physics simulation for performance */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void SetPhysicsSimulationEnabled(bool bEnabled);

    /** Set Chaos physics configuration */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void SetChaosPhysicsConfig(const FChaosPhysicsConfig& Config);

    /** Get current Chaos physics configuration */
    UFUNCTION(BlueprintPure, Category = "Chaos Physics")
    FChaosPhysicsConfig GetChaosPhysicsConfig() const;

    /** Optimize physics settings for dinosaur simulation */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void OptimizeForDinosaurSimulation();

    /** Configure physics for environmental destruction */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void ConfigureEnvironmentalDestruction();

    /** Set up physics LOD system */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void SetupPhysicsLOD();

    /** Force physics update for all actors */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void ForcePhysicsUpdate();

    /** Get physics simulation statistics */
    UFUNCTION(BlueprintPure, Category = "Chaos Physics")
    void GetPhysicsStatistics(int32& ActiveBodies, int32& SleepingBodies, float& SimulationTime) const;

    /** Enable/disable physics debugging */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void SetPhysicsDebuggingEnabled(bool bEnabled);

    /** Reset all physics actors in the world */
    UFUNCTION(BlueprintCallable, Category = "Chaos Physics")
    void ResetAllPhysicsActors();

protected:
    /** Current Chaos physics configuration */
    UPROPERTY(BlueprintReadOnly, Category = "Chaos Physics")
    FChaosPhysicsConfig CurrentConfig;

    /** Physics simulation profiles */
    UPROPERTY(BlueprintReadOnly, Category = "Chaos Physics")
    TMap<FString, FPhysicsSimulationProfile> PhysicsProfiles;

    /** Whether physics simulation is enabled */
    UPROPERTY(BlueprintReadOnly, Category = "Chaos Physics")
    bool bPhysicsSimulationEnabled = true;

    /** Whether physics debugging is enabled */
    UPROPERTY(BlueprintReadOnly, Category = "Chaos Physics")
    bool bPhysicsDebuggingEnabled = false;

    /** Physics LOD distances */
    UPROPERTY(EditAnywhere, Category = "Chaos Physics")
    TArray<float> PhysicsLODDistances = {500.0f, 1000.0f, 2000.0f};

    /** Maximum number of active physics bodies */
    UPROPERTY(EditAnywhere, Category = "Chaos Physics")
    int32 MaxActivePhysicsBodies = 1000;

private:
    /** Initialize default physics profiles */
    void InitializeDefaultProfiles();

    /** Apply configuration to Chaos physics system */
    void ApplyChaosConfiguration();

    /** Update physics LOD for all actors */
    void UpdatePhysicsLOD();

    /** Get distance to nearest player */
    float GetDistanceToNearestPlayer(const FVector& Location) const;

    /** Timer handle for physics LOD updates */
    FTimerHandle PhysicsLODTimerHandle;

    /** Cached physics scene reference */
    FPhysScene* CachedPhysicsScene = nullptr;
};