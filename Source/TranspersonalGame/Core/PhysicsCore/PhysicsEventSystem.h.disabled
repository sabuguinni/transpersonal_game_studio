// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEventSystem.generated.h"

// Forward declarations
class UPhysicalMaterial;

/** Physics event types for different gameplay scenarios */
UENUM(BlueprintType)
enum class ECore_PhysicsEventType_602 : uint8
{
    Collision           UMETA(DisplayName = "Collision Impact"),
    Destruction         UMETA(DisplayName = "Object Destruction"),
    CreatureImpact      UMETA(DisplayName = "Creature Impact"),
    EnvironmentalHit    UMETA(DisplayName = "Environmental Hit"),
    PlayerInteraction   UMETA(DisplayName = "Player Interaction"),
    PredatorAttack      UMETA(DisplayName = "Predator Attack"),
    TerrainDeformation  UMETA(DisplayName = "Terrain Deformation")
};

/** Detailed physics event data */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsEventData
{
    GENERATED_BODY()

    /** Type of physics event */
    UPROPERTY(BlueprintReadOnly)
    ECore_PhysicsEventType_602 EventType = ECore_PhysicsEventType_602::Collision;

    /** Primary actor involved in the event */
    UPROPERTY(BlueprintReadOnly)
    AActor* PrimaryActor = nullptr;

    /** Secondary actor involved in the event */
    UPROPERTY(BlueprintReadOnly)
    AActor* SecondaryActor = nullptr;

    /** World location where the event occurred */
    UPROPERTY(BlueprintReadOnly)
    FVector EventLocation = FVector::ZeroVector;

    /** Impact normal vector */
    UPROPERTY(BlueprintReadOnly)
    FVector ImpactNormal = FVector::ZeroVector;

    /** Magnitude of the impact force */
    UPROPERTY(BlueprintReadOnly)
    float ImpactForce = 0.0f;

    /** Velocity at impact */
    UPROPERTY(BlueprintReadOnly)
    FVector ImpactVelocity = FVector::ZeroVector;

    /** Material involved in the collision */
    UPROPERTY(BlueprintReadOnly)
    UPhysicalMaterial* HitMaterial = nullptr;

    /** Component that was hit */
    UPROPERTY(BlueprintReadOnly)
    UPrimitiveComponent* HitComponent = nullptr;

    /** Bone name for skeletal mesh collisions */
    UPROPERTY(BlueprintReadOnly)
    FName HitBoneName = NAME_None;

    /** Timestamp when the event occurred */
    UPROPERTY(BlueprintReadOnly)
    float EventTimestamp = 0.0f;

    /** Additional event-specific data */
    UPROPERTY(BlueprintReadOnly)
    TMap<FString, FString> AdditionalData;
};

/** Physics event delegate signatures */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPhysicsEventDelegate, const FCore_PhysicsEventData&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPhysicsCollisionEventDelegate, AActor*, ActorA, AActor*, ActorB);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDestructionEventDelegate, AActor*, DestroyedActor, FVector, Location, float, Force);

/**
 * Physics Event System
 * 
 * Centralized system for handling and dispatching physics-related events
 * in the Transpersonal Game. This system provides:
 * 
 * - Event aggregation from multiple physics subsystems
 * - Performance-optimized event filtering and batching
 * - Integration with audio, VFX, and gameplay systems
 * - Event history tracking for debugging and analytics
 * 
 * Key Features:
 * - Collision event processing and filtering
 * - Destruction event coordination
 * - Creature interaction event handling
 * - Environmental impact event processing
 * - Event priority system for performance optimization
 * 
 * @author Core Systems Programmer — Agent #3
 * @version 1.0 — March 2026
 */
UCLASS()
class TRANSPERSONALGAME_API UPhysicsEventSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the physics event system instance */
    UFUNCTION(BlueprintPure, Category = "Physics Events")
    static UPhysicsEventSystem* Get(const UObject* WorldContext);

    /**
     * Register for physics event notifications
     * @param EventType Type of events to listen for
     * @param Callback Function to call when events occur
     */
// [UHT-FIX2]     UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void RegisterEventListener(ECore_PhysicsEventType_602 EventType, const FPhysicsEventDelegate& Callback);

    /**
     * Unregister from physics event notifications
     * @param EventType Type of events to stop listening for
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void UnregisterEventListener(ECore_PhysicsEventType_602 EventType);

    /**
     * Dispatch a physics event to all registered listeners
     * @param EventData Complete event data to dispatch
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void DispatchPhysicsEvent(const FCore_PhysicsEventData& EventData);

    /**
     * Create and dispatch a collision event
     * @param ActorA First actor in collision
     * @param ActorB Second actor in collision
     * @param ImpactLocation World location of impact
     * @param ImpactForce Magnitude of impact force
     * @param HitComponent Component that was hit
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void DispatchCollisionEvent(AActor* ActorA, AActor* ActorB, FVector ImpactLocation, 
                               float ImpactForce, UPrimitiveComponent* HitComponent);

    /**
     * Create and dispatch a destruction event
     * @param DestroyedActor Actor being destroyed
     * @param DestructionLocation World location of destruction
     * @param DestructionForce Force that caused destruction
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void DispatchDestructionEvent(AActor* DestroyedActor, FVector DestructionLocation, float DestructionForce);

    /**
     * Set event filtering parameters for performance optimization
     * @param MinImpactForce Minimum force required to trigger events
     * @param MaxEventsPerFrame Maximum events to process per frame
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void SetEventFiltering(float MinImpactForce, int32 MaxEventsPerFrame);

    /**
     * Enable/disable event history tracking
     * @param bEnabled Whether to track event history
     * @param MaxHistorySize Maximum number of events to keep in history
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void SetEventHistoryTracking(bool bEnabled, int32 MaxHistorySize = 1000);

    /**
     * Get recent physics events from history
     * @param EventType Type of events to retrieve (or all if not specified)
     * @param MaxEvents Maximum number of events to return
     * @return Array of recent physics events
     */
    UFUNCTION(BlueprintPure, Category = "Physics Events")
    TArray<FCore_PhysicsEventData> GetRecentEvents(ECore_PhysicsEventType_602 EventType, int32 MaxEvents = 10) const;

    /**
     * Clear all event history
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void ClearEventHistory();

    /**
     * Get physics event statistics
     * @param TotalEvents Total number of events processed
     * @param EventsThisFrame Events processed this frame
     * @param AverageEventsPerSecond Average events per second
     */
    UFUNCTION(BlueprintPure, Category = "Physics Events")
    void GetEventStatistics(int32& TotalEvents, int32& EventsThisFrame, float& AverageEventsPerSecond) const;

protected:
    /** Event listeners by event type */
    UPROPERTY()
    TMap<ECore_PhysicsEventType_602, FPhysicsEventDelegate> EventListeners;

    /** Event history for debugging and analytics */
    UPROPERTY()
    TArray<FCore_PhysicsEventData> EventHistory;

    /** Event filtering parameters */
    UPROPERTY(EditAnywhere, Category = "Event Filtering")
    float MinImpactForceThreshold = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Event Filtering")
    int32 MaxEventsPerFrame = 50;

    /** Event history settings */
    UPROPERTY(EditAnywhere, Category = "Event History")
    bool bTrackEventHistory = true;

    UPROPERTY(EditAnywhere, Category = "Event History")
    int32 MaxEventHistorySize = 1000;

    /** Performance statistics */
    UPROPERTY()
    int32 TotalEventsProcessed = 0;

    UPROPERTY()
    int32 EventsProcessedThisFrame = 0;

    UPROPERTY()
    float EventProcessingStartTime = 0.0f;

private:
    /** Process queued events */
    void ProcessQueuedEvents();

    /** Filter event based on current filtering settings */
    bool ShouldProcessEvent(const FCore_PhysicsEventData& EventData) const;

    /** Add event to history if tracking is enabled */
    void AddToEventHistory(const FCore_PhysicsEventData& EventData);

    /** Update performance statistics */
    void UpdateEventStatistics();

    /** Queued events for batch processing */
    TArray<FCore_PhysicsEventData> QueuedEvents;

    /** Timer handle for event processing */
    FTimerHandle EventProcessingTimer;

    /** Frame counter for statistics */
    int32 FrameCounter = 0;
    float LastStatisticsUpdateTime = 0.0f;
    float AverageEventsPerSecond = 0.0f;
};