// Copyright Transpersonal Game Studio. All Rights Reserved.
// TranspersonalPhysicsSubsystem.h - Core physics system for the game

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "TranspersonalPhysicsSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalPhysics, Log, All);

/**
 * TRANSPERSONAL PHYSICS SUBSYSTEM
 * 
 * This subsystem manages all physics-related functionality in the game.
 * It provides a unified interface for physics operations and ensures
 * consistent physics behavior across all systems.
 * 
 * KEY RESPONSIBILITIES:
 * - Manage Chaos Physics settings
 * - Handle collision detection and response
 * - Manage physics materials and surfaces
 * - Provide physics utilities for gameplay systems
 * - Monitor physics performance
 */

UENUM(BlueprintType)
enum class ETranspersonalPhysicsQuality : uint8
{
    Low,
    Medium,
    High,
    Ultra
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalPhysicsSettings
{
    GENERATED_BODY()

    // Physics quality level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ETranspersonalPhysicsQuality PhysicsQuality = ETranspersonalPhysicsQuality::High;

    // Maximum physics delta time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxPhysicsDeltaTime = 0.033f; // 30fps minimum

    // Physics solver iterations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 SolverIterations = 8;

    // Collision detection accuracy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float CollisionTolerance = 0.1f;

    // Enable continuous collision detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableCCD = true;

    // Enable physics simulation on dedicated server
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableServerPhysics = true;

    // Maximum number of physics bodies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxPhysicsBodies = 10000;

    // Physics tick rate (Hz)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsTickRate = 60.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalCollisionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FName ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> ObjectType = ECC_WorldStatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TArray<TEnumAsByte<ECollisionResponse>> CollisionResponses;
};

UCLASS()
class TRANSPERSONALGAME_API UTranspersonalPhysicsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Get subsystem instance
    UFUNCTION(BlueprintCallable, Category = "Physics", CallInEditor)
    static UTranspersonalPhysicsSubsystem* Get(const UObject* WorldContext);

    // Physics settings management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsSettings(const FTranspersonalPhysicsSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FTranspersonalPhysicsSettings GetPhysicsSettings() const { return PhysicsSettings; }

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsQuality(ETranspersonalPhysicsQuality Quality);

    // Collision management
    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool RegisterCollisionProfile(const FTranspersonalCollisionProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool UnregisterCollisionProfile(const FName& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    FTranspersonalCollisionProfile GetCollisionProfile(const FName& ProfileName) const;

    // Physics utilities
    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool LineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult, 
                   ECollisionChannel TraceChannel = ECC_Visibility) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool SphereTrace(const FVector& Start, const FVector& End, float Radius, 
                     FHitResult& HitResult, ECollisionChannel TraceChannel = ECC_Visibility) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    TArray<FHitResult> MultiLineTrace(const FVector& Start, const FVector& End, 
                                      ECollisionChannel TraceChannel = ECC_Visibility) const;

    // Physics body management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetGlobalGravity(const FVector& NewGravity);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FVector GetGlobalGravity() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void PausePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ResumePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool IsPhysicsSimulationPaused() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPhysicsFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActivePhysicsBodies() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPhysicsMemoryUsage() const;

    // Destruction system
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void EnableDestruction(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool IsDestructionEnabled() const;

    // Ragdoll system
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnableRagdollPhysics(class USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DisableRagdollPhysics(class USkeletalMeshComponent* SkeletalMesh);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsSettingsChanged, const FTranspersonalPhysicsSettings&, NewSettings);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsSettingsChanged OnPhysicsSettingsChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsPerformanceWarning, float, PhysicsTime);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsPerformanceWarning OnPhysicsPerformanceWarning;

protected:
    // Initialize physics systems
    void InitializeChaosPhysics();
    void InitializeCollisionProfiles();
    void InitializePhysicsMaterials();
    void InitializeDestructionSystem();

    // Performance monitoring
    void MonitorPhysicsPerformance();

    // Settings
    UPROPERTY()
    FTranspersonalPhysicsSettings PhysicsSettings;

    // Collision profiles
    UPROPERTY()
    TMap<FName, FTranspersonalCollisionProfile> CollisionProfiles;

    // State tracking
    UPROPERTY()
    bool bPhysicsInitialized = false;

    UPROPERTY()
    bool bPhysicsSimulationPaused = false;

    UPROPERTY()
    bool bDestructionEnabled = true;

    // Performance metrics
    UPROPERTY()
    float LastPhysicsFrameTime = 0.0f;

    UPROPERTY()
    int32 LastActivePhysicsBodies = 0;

    // Timers
    FTimerHandle PhysicsPerformanceTimer;

private:
    // Performance thresholds
    static constexpr float PhysicsPerformanceWarningThreshold = 5.0f; // ms
    static constexpr float PhysicsPerformanceMonitoringInterval = 1.0f; // seconds

    // Default collision profiles
    void SetupDefaultCollisionProfiles();
};