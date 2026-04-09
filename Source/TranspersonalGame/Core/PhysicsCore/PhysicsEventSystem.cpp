// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsEventSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsEventSystem, Log, All);

void UPhysicsEventSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPhysicsEventSystem, Log, TEXT("PhysicsEventSystem initialized"));
    
    // Initialize event processing timer
    EventProcessingStartTime = FPlatformTime::Seconds();
}

void UPhysicsEventSystem::Deinitialize()
{
    // Clear all event listeners
    EventListeners.Empty();
    
    // Clear event history
    EventHistory.Empty();
    QueuedEvents.Empty();
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EventProcessingTimer);
    }
    
    UE_LOG(LogPhysicsEventSystem, Log, TEXT("PhysicsEventSystem deinitialized"));
    
    Super::Deinitialize();
}

void UPhysicsEventSystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    // Setup event processing timer (process events every frame)
    InWorld.GetTimerManager().SetTimer(
        EventProcessingTimer,
        this,
        &UPhysicsEventSystem::ProcessQueuedEvents,
        0.0f, // Process every frame
        true
    );
    
    UE_LOG(LogPhysicsEventSystem, Log, TEXT("PhysicsEventSystem world begin play"));
}

UPhysicsEventSystem* UPhysicsEventSystem::Get(const UObject* WorldContext)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UPhysicsEventSystem>();
    }
    return nullptr;
}

void UPhysicsEventSystem::RegisterEventListener(EPhysicsEventType EventType, const FPhysicsEventDelegate& Callback)
{
    if (FPhysicsEventDelegate* ExistingDelegate = EventListeners.Find(EventType))
    {
        ExistingDelegate->AddDynamic(Callback.GetUObject(), Callback.GetFunctionName());
    }
    else
    {
        FPhysicsEventDelegate NewDelegate;
        NewDelegate.AddDynamic(Callback.GetUObject(), Callback.GetFunctionName());
        EventListeners.Add(EventType, NewDelegate);
    }
    
    UE_LOG(LogPhysicsEventSystem, Log, TEXT("Registered event listener for event type: %d"), 
           static_cast<int32>(EventType));
}

void UPhysicsEventSystem::UnregisterEventListener(EPhysicsEventType EventType)
{
    if (EventListeners.Contains(EventType))
    {
        EventListeners.Remove(EventType);
        UE_LOG(LogPhysicsEventSystem, Log, TEXT("Unregistered event listener for event type: %d"), 
               static_cast<int32>(EventType));
    }
}

void UPhysicsEventSystem::DispatchPhysicsEvent(const FPhysicsEventData& EventData)
{
    // Add to queue for batch processing
    if (ShouldProcessEvent(EventData))
    {
        QueuedEvents.Add(EventData);
    }
}

void UPhysicsEventSystem::DispatchCollisionEvent(AActor* ActorA, AActor* ActorB, FVector ImpactLocation, 
                                                float ImpactForce, UPrimitiveComponent* HitComponent)
{
    FPhysicsEventData EventData;
    EventData.EventType = EPhysicsEventType::Collision;
    EventData.PrimaryActor = ActorA;
    EventData.SecondaryActor = ActorB;
    EventData.EventLocation = ImpactLocation;
    EventData.ImpactForce = ImpactForce;
    EventData.HitComponent = HitComponent;
    EventData.EventTimestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Get material from hit component
    if (HitComponent)
    {
        EventData.HitMaterial = HitComponent->GetMaterial(0);
    }
    
    DispatchPhysicsEvent(EventData);
}

void UPhysicsEventSystem::DispatchDestructionEvent(AActor* DestroyedActor, FVector DestructionLocation, float DestructionForce)
{
    FPhysicsEventData EventData;
    EventData.EventType = EPhysicsEventType::Destruction;
    EventData.PrimaryActor = DestroyedActor;
    EventData.EventLocation = DestructionLocation;
    EventData.ImpactForce = DestructionForce;
    EventData.EventTimestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    DispatchPhysicsEvent(EventData);
}

void UPhysicsEventSystem::SetEventFiltering(float MinImpactForce, int32 MaxEventsPerFrame)
{
    MinImpactForceThreshold = MinImpactForce;
    MaxEventsPerFrame = MaxEventsPerFrame;
    
    UE_LOG(LogPhysicsEventSystem, Log, TEXT("Event filtering updated: MinForce=%.2f, MaxEvents=%d"), 
           MinImpactForce, MaxEventsPerFrame);
}

void UPhysicsEventSystem::SetEventHistoryTracking(bool bEnabled, int32 MaxHistorySize)
{
    bTrackEventHistory = bEnabled;
    MaxEventHistorySize = MaxHistorySize;
    
    if (!bEnabled)
    {
        EventHistory.Empty();
    }
    
    UE_LOG(LogPhysicsEventSystem, Log, TEXT("Event history tracking %s, MaxSize=%d"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"), MaxHistorySize);
}

TArray<FPhysicsEventData> UPhysicsEventSystem::GetRecentEvents(EPhysicsEventType EventType, int32 MaxEvents) const
{
    TArray<FPhysicsEventData> FilteredEvents;
    
    // Get events in reverse order (most recent first)
    for (int32 i = EventHistory.Num() - 1; i >= 0 && FilteredEvents.Num() < MaxEvents; --i)
    {
        const FPhysicsEventData& Event = EventHistory[i];
        if (Event.EventType == EventType)
        {
            FilteredEvents.Add(Event);
        }
    }
    
    return FilteredEvents;
}

void UPhysicsEventSystem::ClearEventHistory()
{
    EventHistory.Empty();
    UE_LOG(LogPhysicsEventSystem, Log, TEXT("Event history cleared"));
}

void UPhysicsEventSystem::GetEventStatistics(int32& TotalEvents, int32& EventsThisFrame, float& AverageEventsPerSecond) const
{
    TotalEvents = TotalEventsProcessed;
    EventsThisFrame = EventsProcessedThisFrame;
    AverageEventsPerSecond = this->AverageEventsPerSecond;
}

void UPhysicsEventSystem::ProcessQueuedEvents()
{
    EventsProcessedThisFrame = 0;
    
    // Process events up to the frame limit
    int32 EventsToProcess = FMath::Min(QueuedEvents.Num(), MaxEventsPerFrame);
    
    for (int32 i = 0; i < EventsToProcess; ++i)
    {
        const FPhysicsEventData& EventData = QueuedEvents[i];
        
        // Dispatch to appropriate listeners
        if (FPhysicsEventDelegate* Delegate = EventListeners.Find(EventData.EventType))
        {
            Delegate->Broadcast(EventData);
        }
        
        // Add to history if tracking is enabled
        AddToEventHistory(EventData);
        
        EventsProcessedThisFrame++;
        TotalEventsProcessed++;
    }
    
    // Remove processed events from queue
    if (EventsToProcess > 0)
    {
        QueuedEvents.RemoveAt(0, EventsToProcess);
    }
    
    // Update statistics
    UpdateEventStatistics();
    
    // Log performance warning if queue is backing up
    if (QueuedEvents.Num() > MaxEventsPerFrame * 2)
    {
        UE_LOG(LogPhysicsEventSystem, Warning, 
               TEXT("Physics event queue backing up: %d events queued"), QueuedEvents.Num());
    }
}

bool UPhysicsEventSystem::ShouldProcessEvent(const FPhysicsEventData& EventData) const
{
    // Filter by impact force threshold
    if (EventData.ImpactForce < MinImpactForceThreshold)
    {
        return false;
    }
    
    // Additional filtering logic can be added here
    // For example, distance-based filtering, actor type filtering, etc.
    
    return true;
}

void UPhysicsEventSystem::AddToEventHistory(const FPhysicsEventData& EventData)
{
    if (!bTrackEventHistory)
    {
        return;
    }
    
    EventHistory.Add(EventData);
    
    // Trim history if it exceeds maximum size
    if (EventHistory.Num() > MaxEventHistorySize)
    {
        int32 EventsToRemove = EventHistory.Num() - MaxEventHistorySize;
        EventHistory.RemoveAt(0, EventsToRemove);
    }
}

void UPhysicsEventSystem::UpdateEventStatistics()
{
    FrameCounter++;
    
    // Update average events per second every second
    float CurrentTime = FPlatformTime::Seconds();
    if (CurrentTime - LastStatisticsUpdateTime >= 1.0f)
    {
        float TimeDelta = CurrentTime - LastStatisticsUpdateTime;
        float EventsInPeriod = static_cast<float>(TotalEventsProcessed);
        
        if (EventProcessingStartTime > 0.0f)
        {
            float TotalTime = CurrentTime - EventProcessingStartTime;
            AverageEventsPerSecond = EventsInPeriod / TotalTime;
        }
        
        LastStatisticsUpdateTime = CurrentTime;
    }
}