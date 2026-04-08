// TranspersonalPhysicsManager.h
// Core Physics Manager for Transpersonal Game
// Handles Chaos Physics integration and custom physics systems

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "TranspersonalPhysicsManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsEvent, AActor*, Actor, FVector, ImpactPoint);

/**
 * @class UTranspersonalPhysicsManager
 * @brief Central physics management system for the transpersonal game
 * 
 * Manages all physics interactions, destruction events, and performance optimization
 * for the open-world RPG environment. Integrates with Chaos Physics for realistic
 * simulation while maintaining 60fps target on consoles.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalPhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UTranspersonalPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Initialize physics systems for the current world
     * @param World Target world to initialize physics for
     * @return true if initialization successful
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    bool InitializePhysicsWorld(UWorld* World);

    /**
     * @brief Register an actor for advanced physics processing
     * @param Actor Actor to register
     * @param PhysicsType Type of physics processing required
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void RegisterPhysicsActor(AActor* Actor, EPhysicsType PhysicsType);

    /**
     * @brief Unregister an actor from physics processing
     * @param Actor Actor to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void UnregisterPhysicsActor(AActor* Actor);

    /**
     * @brief Apply impulse with performance optimization
     * @param Actor Target actor
     * @param Impulse Impulse vector
     * @param Location Application point
     * @param bVelChange Whether to treat as velocity change
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void ApplyOptimizedImpulse(AActor* Actor, FVector Impulse, FVector Location, bool bVelChange = false);

    /**
     * @brief Enable/disable physics simulation for performance
     * @param Actor Target actor
     * @param bEnable Whether to enable physics
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void SetPhysicsEnabled(AActor* Actor, bool bEnable);

    /**
     * @brief Get current physics performance metrics
     * @return Physics performance data
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    FPhysicsPerformanceData GetPhysicsPerformanceData() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsEvent OnMajorImpact;

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsEvent OnDestructionEvent;

protected:
    /**
     * @brief Update physics LOD based on distance and importance
     */
    void UpdatePhysicsLOD();

    /**
     * @brief Process queued physics operations
     */
    void ProcessPhysicsQueue();

    /**
     * @brief Handle physics collision events
     */
    UFUNCTION()
    void OnPhysicsCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
    // Performance tracking
    UPROPERTY(VisibleAnywhere, Category = "Performance")
    float PhysicsFrameTime;

    UPROPERTY(VisibleAnywhere, Category = "Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float MaxPhysicsFrameTime = 2.0f; // 2ms budget

    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxActivePhysicsActors = 100;

    // Physics actor registry
    UPROPERTY()
    TMap<AActor*, EPhysicsType> RegisteredActors;

    // LOD system
    UPROPERTY(EditAnywhere, Category = "LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float LowDetailDistance = 5000.0f;

    // Performance optimization
    TQueue<FPhysicsOperation> PhysicsOperationQueue;
    FTimerHandle PhysicsUpdateTimer;
};

/**
 * @enum EPhysicsType
 * @brief Types of physics processing for different actors
 */
UENUM(BlueprintType)
enum class EPhysicsType : uint8
{
    Standard        UMETA(DisplayName = "Standard Physics"),
    HighPrecision   UMETA(DisplayName = "High Precision"),
    Destructible    UMETA(DisplayName = "Destructible"),
    Vehicle         UMETA(DisplayName = "Vehicle Physics"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Environmental   UMETA(DisplayName = "Environmental")
};

/**
 * @struct FPhysicsPerformanceData
 * @brief Performance metrics for physics system
 */
USTRUCT(BlueprintType)
struct FPhysicsPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage = 0.0f;
};

/**
 * @struct FPhysicsOperation
 * @brief Queued physics operation for performance optimization
 */
USTRUCT()
struct FPhysicsOperation
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* TargetActor = nullptr;

    UPROPERTY()
    FVector ImpulseVector = FVector::ZeroVector;

    UPROPERTY()
    FVector Location = FVector::ZeroVector;

    UPROPERTY()
    float Priority = 1.0f;

    UPROPERTY()
    float Timestamp = 0.0f;
};