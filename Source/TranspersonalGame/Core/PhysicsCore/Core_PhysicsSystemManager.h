#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Core_PhysicsSystemManager.generated.h"

// Forward declarations
class UPhysicalMaterial;
class UStaticMeshComponent;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class ECore_PhysicsSystemStatus : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Error           UMETA(DisplayName = "Error"),
    Disabled        UMETA(DisplayName = "Disabled")
};

UENUM(BlueprintType)
enum class ECore_PhysicsMaterialType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Flesh           UMETA(DisplayName = "Flesh"),
    Metal           UMETA(DisplayName = "Metal"),
    Organic         UMETA(DisplayName = "Organic"),
    Water           UMETA(DisplayName = "Water")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActivePhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 SimulatingRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 CollisionPairs;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float TotalPhysicsMemoryMB;

    FCore_PhysicsSystemMetrics()
    {
        ActivePhysicsObjects = 0;
        SimulatingRigidBodies = 0;
        PhysicsFrameTime = 0.0f;
        CollisionPairs = 0;
        TotalPhysicsMemoryMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCore_RagdollConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableRagdollOnDeath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendToRagdollTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bApplyDeathImpulse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float DeathImpulseStrength;

    FCore_RagdollConfiguration()
    {
        bEnableRagdollOnDeath = true;
        RagdollLifetime = 30.0f;
        BlendToRagdollTime = 0.5f;
        bApplyDeathImpulse = true;
        DeathImpulseStrength = 1000.0f;
    }
};

/**
 * Core Physics System Manager
 * Manages all physics simulation, materials, and ragdoll systems
 * Integrates with Engine Architecture for performance monitoring
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Status
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    ECore_PhysicsSystemStatus GetSystemStatus() const { return SystemStatus; }

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FCore_PhysicsSystemMetrics GetPhysicsMetrics() const { return CurrentMetrics; }

    // Physics Materials Management
    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    UPhysicalMaterial* GetPhysicsMaterial(ECore_PhysicsMaterialType MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    void ApplyPhysicsMaterialToActor(AActor* Actor, ECore_PhysicsMaterialType MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    void CreateDefaultPhysicsMaterials();

    // Physics Object Management
    UFUNCTION(BlueprintCallable, Category = "Physics Objects")
    void EnablePhysicsOnActor(AActor* Actor, float Mass = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics Objects")
    void DisablePhysicsOnActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Objects")
    void SetActorMass(AActor* Actor, float NewMass);

    // Ragdoll System
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnableRagdollOnCharacter(ACharacter* Character, const FVector& ImpulseDirection = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DisableRagdollOnCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollConfiguration(const FCore_RagdollConfiguration& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    FCore_RagdollConfiguration GetRagdollConfiguration() const { return RagdollConfig; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePhysicsMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPhysicsPerformanceAcceptable() const;

    // System Integration
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterWithCoreArchitecture();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ValidatePhysicsWorld();

    // Debug and Testing
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void CreatePhysicsTestObjects();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void RunPhysicsStressTest();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPhysicsSystemStatus();

protected:
    // System State
    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    ECore_PhysicsSystemStatus SystemStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Physics System")
    FCore_PhysicsSystemMetrics CurrentMetrics;

    // Physics Materials
    UPROPERTY(BlueprintReadOnly, Category = "Physics Materials")
    TMap<ECore_PhysicsMaterialType, TSoftObjectPtr<UPhysicalMaterial>> PhysicsMaterials;

    // Ragdoll Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FCore_RagdollConfiguration RagdollConfig;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxAcceptablePhysicsFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAcceptablePhysicsObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxAcceptablePhysicsMemoryMB;

    // Timer Handles
    FTimerHandle MetricsUpdateTimer;
    FTimerHandle RagdollCleanupTimer;

private:
    // Internal Methods
    void InitializePhysicsMaterials();
    void SetupPerformanceMonitoring();
    void CleanupRagdolls();
    UPhysicalMaterial* CreatePhysicsMaterial(const FString& MaterialName, float Friction, float Restitution, float Density);
    void ApplyRagdollImpulse(USkeletalMeshComponent* SkeletalMesh, const FVector& ImpulseDirection);
    void ValidateSystemIntegrity();

    // Cached References
    UPROPERTY()
    TArray<TWeakObjectPtr<ACharacter>> ActiveRagdolls;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> PhysicsEnabledActors;

    // System Metrics
    float LastMetricsUpdateTime;
    int32 FramesSinceLastUpdate;
};