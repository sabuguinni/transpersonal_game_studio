#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Subsystems/WorldSubsystem.h"
#include "../Engine/EngineArchitectCore.h"
#include "PhysicsArchitect.generated.h"

UENUM(BlueprintType)
enum class EEng_CollisionProfile : uint8
{
    Default = 0,
    Character = 1,
    Dinosaur = 2,
    Environment = 3,
    Projectile = 4,
    Trigger = 5,
    Debris = 6
};

UENUM(BlueprintType)
enum class EEng_PhysicsQuality : uint8
{
    Low = 0,
    Medium = 1,
    High = 2,
    Ultra = 3
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CollisionSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    EEng_CollisionProfile Profile;

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    bool bEnableCollision;

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    bool bBlockAll;

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    bool bGenerateOverlapEvents;

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    float Mass;

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    float LinearDamping;

    UPROPERTY(BlueprintReadWrite, Category = "Collision")
    float AngularDamping;

    FEng_CollisionSettings()
    {
        Profile = EEng_CollisionProfile::Default;
        bEnableCollision = true;
        bBlockAll = false;
        bGenerateOverlapEvents = false;
        Mass = 1.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    int32 ActiveRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    int32 SleepingRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    int32 CollisionChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    float PhysicsStepTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    float AverageFrameTime;

    FEng_PhysicsMetrics()
    {
        ActiveRigidBodies = 0;
        SleepingRigidBodies = 0;
        CollisionChecks = 0;
        PhysicsStepTime = 0.0f;
        AverageFrameTime = 0.0f;
    }
};

/**
 * Physics Architect System
 * Manages physics settings, collision profiles, and performance optimization
 * Ensures consistent physics behavior across all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsArchitect : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPhysicsArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // Collision Profile Management
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void SetupCollisionProfiles();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void ApplyCollisionProfile(UPrimitiveComponent* Component, EEng_CollisionProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    FEng_CollisionSettings GetCollisionSettings(EEng_CollisionProfile Profile) const;

    // Physics Quality Management
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void SetPhysicsQuality(EEng_PhysicsQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    EEng_PhysicsQuality GetCurrentPhysicsQuality() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    FEng_PhysicsMetrics GetPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void UpdatePhysicsMetrics();

    // Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void OptimizePhysicsForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void SetPhysicsLOD(float Distance, bool bEnablePhysics);

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool ValidatePhysicsSettings();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Architecture")
    void LogPhysicsStatus();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void EnableRagdollForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void DisableRagdollForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void SetActorPhysicsEnabled(AActor* Actor, bool bEnabled);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    TMap<EEng_CollisionProfile, FEng_CollisionSettings> CollisionProfiles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    EEng_PhysicsQuality CurrentQuality;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    float PhysicsLODDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    bool bAutoOptimizePhysics;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    int32 MaxPhysicsObjects;

private:
    FTimerHandle MetricsUpdateTimer;
    UEngineArchitectCore* EngineCore;

    void InitializeCollisionProfiles();
    void ApplyQualitySettings(EEng_PhysicsQuality Quality);
    void RegisterWithEngineCore();
    void CheckPhysicsPerformance();
};

/**
 * Physics Architect Component
 * Provides physics architecture functionality to individual actors
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsArchitectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsArchitectComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void SetCollisionProfile(EEng_CollisionProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void EnablePhysicsLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool IsPhysicsActive() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void ForcePhysicsUpdate();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    EEng_CollisionProfile CollisionProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bUsePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LODDistance;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bPhysicsEnabled;

private:
    UPhysicsArchitect* PhysicsArchitect;
    float LastLODCheck;
    
    void UpdatePhysicsLOD();
    void ApplyCollisionSettings();
};