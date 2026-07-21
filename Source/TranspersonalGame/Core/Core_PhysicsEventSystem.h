#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Core_PhysicsEventSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsEventType : uint8
{
    Collision       UMETA(DisplayName = "Collision"),
    Impact          UMETA(DisplayName = "Impact"),
    Break           UMETA(DisplayName = "Break"),
    Wake            UMETA(DisplayName = "Wake"),
    Sleep           UMETA(DisplayName = "Sleep"),
    Constraint      UMETA(DisplayName = "Constraint"),
    Destruction     UMETA(DisplayName = "Destruction")
};

UENUM(BlueprintType)
enum class ECore_PhysicsEventPriority : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Normal      UMETA(DisplayName = "Normal"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsEventData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    ECore_PhysicsEventType EventType;

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    ECore_PhysicsEventPriority Priority;

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    TWeakObjectPtr<AActor> PrimaryActor;

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    TWeakObjectPtr<AActor> SecondaryActor;

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    FVector Normal;

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    float ImpactForce;

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    float Timestamp;

    UPROPERTY(BlueprintReadWrite, Category = "Physics Event")
    FString EventDescription;

    FCore_PhysicsEventData()
    {
        EventType = ECore_PhysicsEventType::Collision;
        Priority = ECore_PhysicsEventPriority::Normal;
        PrimaryActor = nullptr;
        SecondaryActor = nullptr;
        Location = FVector::ZeroVector;
        Normal = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        ImpactForce = 0.0f;
        Timestamp = 0.0f;
        EventDescription = TEXT("");
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCore_PhysicsEventDelegate, const FCore_PhysicsEventData&, EventData);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsEventFilter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Filter")
    TArray<ECore_PhysicsEventType> AllowedEventTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Filter")
    ECore_PhysicsEventPriority MinimumPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Filter")
    float MinimumImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Filter")
    TArray<TSubclassOf<AActor>> AllowedActorClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Filter")
    bool bFilterByLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Filter", meta = (EditCondition = "bFilterByLocation"))
    FVector FilterCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Filter", meta = (EditCondition = "bFilterByLocation"))
    float FilterRadius;

    FCore_PhysicsEventFilter()
    {
        AllowedEventTypes.Add(ECore_PhysicsEventType::Collision);
        AllowedEventTypes.Add(ECore_PhysicsEventType::Impact);
        MinimumPriority = ECore_PhysicsEventPriority::Low;
        MinimumImpactForce = 0.0f;
        bFilterByLocation = false;
        FilterCenter = FVector::ZeroVector;
        FilterRadius = 1000.0f;
    }
};

/**
 * Core Physics Event System - Manages physics events and notifications
 * Handles collision detection, impact events, destruction events, and physics state changes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsEventSystem : public UObject
{
    GENERATED_BODY()

public:
    UCore_PhysicsEventSystem();

    // Event System Management
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void InitializeEventSystem(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void ShutdownEventSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void UpdateEventSystem(float DeltaTime);

    // Event Registration
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void RegisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void UnregisterPhysicsActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void RegisterAllPhysicsActors();

    // Event Broadcasting
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void BroadcastPhysicsEvent(const FCore_PhysicsEventData& EventData);

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void CreateCollisionEvent(AActor* ActorA, AActor* ActorB, const FVector& Location, const FVector& Normal, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void CreateImpactEvent(AActor* Actor, const FVector& Location, const FVector& Velocity, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void CreateBreakEvent(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void CreateWakeEvent(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void CreateSleepEvent(AActor* Actor);

    // Event Filtering
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void SetEventFilter(const FCore_PhysicsEventFilter& Filter);

    UFUNCTION(BlueprintPure, Category = "Physics Events")
    FCore_PhysicsEventFilter GetEventFilter() const { return CurrentFilter; }

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    bool PassesEventFilter(const FCore_PhysicsEventData& EventData) const;

    // Event History
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void EnableEventHistory(bool bEnable, int32 MaxHistorySize = 1000);

    UFUNCTION(BlueprintPure, Category = "Physics Events")
    TArray<FCore_PhysicsEventData> GetEventHistory() const { return EventHistory; }

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void ClearEventHistory();

    UFUNCTION(BlueprintPure, Category = "Physics Events")
    int32 GetEventHistoryCount() const { return EventHistory.Num(); }

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    int32 GetEventCount(ECore_PhysicsEventType EventType) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    float GetAverageImpactForce() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void ResetEventStatistics();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Physics Events", CallInEditor = true)
    void DebugDrawEvents(float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void LogEventStatistics();

    UFUNCTION(BlueprintCallable, Category = "Physics Events")
    void SetDebugVisualization(bool bEnable);

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FCore_PhysicsEventDelegate OnPhysicsEvent;

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FCore_PhysicsEventDelegate OnCollisionEvent;

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FCore_PhysicsEventDelegate OnImpactEvent;

    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FCore_PhysicsEventDelegate OnBreakEvent;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Physics Events")
    FCore_PhysicsEventFilter CurrentFilter;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Events")
    TArray<FCore_PhysicsEventData> EventHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Events")
    TArray<TWeakObjectPtr<AActor>> RegisteredActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Events")
    bool bEnableEventHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Events")
    int32 MaxEventHistorySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Events")
    bool bEnableDebugVisualization;

    UPROPERTY()
    TWeakObjectPtr<UWorld> EventWorld;

    // Internal Methods
    void ProcessPendingEvents();
    void UpdateRegisteredActors();
    void BindActorEvents(AActor* Actor);
    void UnbindActorEvents(AActor* Actor);
    void AddEventToHistory(const FCore_PhysicsEventData& EventData);
    void TrimEventHistory();

    // Event Handlers
    UFUNCTION()
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

private:
    TArray<FCore_PhysicsEventData> PendingEvents;
    TMap<ECore_PhysicsEventType, int32> EventCounts;
    float TotalImpactForce;
    int32 ImpactEventCount;
    float LastUpdateTime;
};