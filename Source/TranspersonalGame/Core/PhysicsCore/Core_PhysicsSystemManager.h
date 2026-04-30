#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/HitResult.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

// Physics simulation quality levels for performance scaling
UENUM(BlueprintType)
enum class ECore_PhysicsQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

// Physics material types for different surfaces
UENUM(BlueprintType)
enum class ECore_SurfaceType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Water       UMETA(DisplayName = "Water"),
    Wood        UMETA(DisplayName = "Wood"),
    Bone        UMETA(DisplayName = "Bone"),
    Metal       UMETA(DisplayName = "Metal")
};

// Physics interaction data for collision responses
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_SurfaceType SurfaceType = ECore_SurfaceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float ImpactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector ImpactNormal = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bShouldCreateDebris = false;

    FCore_PhysicsInteraction()
    {
        SurfaceType = ECore_SurfaceType::Rock;
        ImpactForce = 0.0f;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::ZeroVector;
        bShouldCreateDebris = false;
    }
};

// Ragdoll configuration for character physics
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float PhysicsBlendWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bSimulatePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableGravity = true;

    FCore_RagdollConfig()
    {
        BlendTime = 0.2f;
        PhysicsBlendWeight = 1.0f;
        bSimulatePhysics = true;
        bEnableGravity = true;
    }
};

/**
 * Core Physics System Manager - Handles all physics simulation, collision detection,
 * ragdoll physics, and destruction systems for the prehistoric survival game.
 * Optimized for 60fps on PC and 30fps on console with scalable quality settings.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics quality management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsQuality(ECore_PhysicsQuality Quality);

    UFUNCTION(BlueprintPure, Category = "Physics System")
    ECore_PhysicsQuality GetPhysicsQuality() const { return CurrentPhysicsQuality; }

    // Collision and impact handling
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void HandleImpact(const FHitResult& HitResult, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FCore_PhysicsInteraction ProcessCollision(AActor* ActorA, AActor* ActorB, const FVector& ImpactPoint, float Force);

    // Ragdoll physics
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnableRagdoll(AActor* Actor, const FCore_RagdollConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void DisableRagdoll(AActor* Actor, float BlendOutTime = 0.2f);

    // Destruction system
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float DestructionForce);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void CreateDebris(const FVector& Location, ECore_SurfaceType SurfaceType, int32 DebrisCount = 5);

    // Physics simulation control
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsSimulation(AActor* Actor, bool bSimulate);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyImpulse(AActor* Actor, const FVector& Impulse, const FVector& Location = FVector::ZeroVector);

    // Performance monitoring
    UFUNCTION(BlueprintPure, Category = "Physics System")
    int32 GetActivePhysicsActors() const { return ActivePhysicsActors; }

    UFUNCTION(BlueprintPure, Category = "Physics System")
    float GetPhysicsFrameTime() const { return LastPhysicsFrameTime; }

    // Surface material detection
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    ECore_SurfaceType GetSurfaceType(const FHitResult& HitResult);

protected:
    // Physics quality settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    ECore_PhysicsQuality CurrentPhysicsQuality = ECore_PhysicsQuality::High;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastPhysicsFrameTime = 0.0f;

    // Physics configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float MaxPhysicsDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    int32 MaxSimulatedActors = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float CollisionImpactThreshold = 500.0f;

    // Ragdoll management
    UPROPERTY()
    TArray<AActor*> RagdollActors;

    // Internal methods
    void UpdatePhysicsSettings();
    void OptimizePhysicsPerformance();
    void CleanupInactivePhysics();
    
    // Timer handles
    FTimerHandle PhysicsOptimizationTimer;
    FTimerHandle PerformanceMonitorTimer;
};

/**
 * Physics Component for individual actors that need custom physics behavior
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Physics properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_SurfaceType SurfaceType = ECore_SurfaceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bCanBeDestroyed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DestructionThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableRagdoll = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FCore_RagdollConfig RagdollConfig;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Events")
    void OnPhysicsImpact(const FCore_PhysicsInteraction& Interaction);

    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Events")
    void OnDestruction(const FVector& ImpactPoint, float Force);

    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Events")
    void OnRagdollEnabled();

    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Events")
    void OnRagdollDisabled();

protected:
    UPROPERTY()
    UCore_PhysicsSystemManager* PhysicsManager;

    void InitializePhysicsComponent();
};