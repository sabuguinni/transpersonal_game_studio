#include "Core_PhysicsEventSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UEng_PhysicsEventSystem::UEng_PhysicsEventSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize event tracking
    bTrackCollisionEvents = true;
    bTrackBreakEvents = true;
    bTrackSleepEvents = false;
    
    MaxEventHistorySize = 1000;
    EventRetentionTime = 10.0f;
    
    // Performance settings
    MaxEventsPerFrame = 50;
    EventProcessingBudgetMs = 2.0f;
}

void UEng_PhysicsEventSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Clear event history
    CollisionEventHistory.Empty();
    BreakEventHistory.Empty();
    SleepEventHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Event System initialized"));
}

void UEng_PhysicsEventSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Process pending events
    ProcessPendingEvents(DeltaTime);
    
    // Clean up old events
    CleanupOldEvents(DeltaTime);
}

void UEng_PhysicsEventSystem::RegisterCollisionEvent(const FEng_CollisionEventData& EventData)
{
    if (!bTrackCollisionEvents || !IsValid(EventData.Actor1) || !IsValid(EventData.Actor2))
    {
        return;
    }
    
    // Check if we're at capacity
    if (CollisionEventHistory.Num() >= MaxEventHistorySize)
    {
        // Remove oldest event
        CollisionEventHistory.RemoveAt(0);
    }
    
    // Add new event with timestamp
    FEng_CollisionEventData TimestampedEvent = EventData;
    TimestampedEvent.Timestamp = GetWorld()->GetTimeSeconds();
    CollisionEventHistory.Add(TimestampedEvent);
    
    // Broadcast event
    OnCollisionEvent.Broadcast(TimestampedEvent);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Collision event registered: %s vs %s"), 
           *EventData.Actor1->GetName(), *EventData.Actor2->GetName());
}

void UEng_PhysicsEventSystem::RegisterBreakEvent(const FEng_BreakEventData& EventData)
{
    if (!bTrackBreakEvents || !IsValid(EventData.BrokenActor))
    {
        return;
    }
    
    // Check capacity
    if (BreakEventHistory.Num() >= MaxEventHistorySize)
    {
        BreakEventHistory.RemoveAt(0);
    }
    
    // Add timestamped event
    FEng_BreakEventData TimestampedEvent = EventData;
    TimestampedEvent.Timestamp = GetWorld()->GetTimeSeconds();
    BreakEventHistory.Add(TimestampedEvent);
    
    // Broadcast event
    OnBreakEvent.Broadcast(TimestampedEvent);
    
    UE_LOG(LogTemp, Log, TEXT("Break event registered: %s (Force: %f)"), 
           *EventData.BrokenActor->GetName(), EventData.BreakForce);
}

void UEng_PhysicsEventSystem::RegisterSleepEvent(const FEng_SleepEventData& EventData)
{
    if (!bTrackSleepEvents || !IsValid(EventData.Actor))
    {
        return;
    }
    
    // Check capacity
    if (SleepEventHistory.Num() >= MaxEventHistorySize)
    {
        SleepEventHistory.RemoveAt(0);
    }
    
    // Add timestamped event
    FEng_SleepEventData TimestampedEvent = EventData;
    TimestampedEvent.Timestamp = GetWorld()->GetTimeSeconds();
    SleepEventHistory.Add(TimestampedEvent);
    
    // Broadcast event
    OnSleepEvent.Broadcast(TimestampedEvent);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Sleep event registered: %s (Sleeping: %s)"), 
           *EventData.Actor->GetName(), EventData.bIsSleeping ? TEXT("Yes") : TEXT("No"));
}

TArray<FEng_CollisionEventData> UEng_PhysicsEventSystem::GetRecentCollisionEvents(float TimeWindow) const
{
    TArray<FEng_CollisionEventData> RecentEvents;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (const FEng_CollisionEventData& Event : CollisionEventHistory)
    {
        if (CurrentTime - Event.Timestamp <= TimeWindow)
        {
            RecentEvents.Add(Event);
        }
    }
    
    return RecentEvents;
}

TArray<FEng_BreakEventData> UEng_PhysicsEventSystem::GetRecentBreakEvents(float TimeWindow) const
{
    TArray<FEng_BreakEventData> RecentEvents;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (const FEng_BreakEventData& Event : BreakEventHistory)
    {
        if (CurrentTime - Event.Timestamp <= TimeWindow)
        {
            RecentEvents.Add(Event);
        }
    }
    
    return RecentEvents;
}

int32 UEng_PhysicsEventSystem::GetCollisionEventCount(AActor* Actor, float TimeWindow) const
{
    if (!IsValid(Actor))
    {
        return 0;
    }
    
    int32 Count = 0;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (const FEng_CollisionEventData& Event : CollisionEventHistory)
    {
        if (CurrentTime - Event.Timestamp <= TimeWindow)
        {
            if (Event.Actor1 == Actor || Event.Actor2 == Actor)
            {
                Count++;
            }
        }
    }
    
    return Count;
}

void UEng_PhysicsEventSystem::ClearEventHistory()
{
    CollisionEventHistory.Empty();
    BreakEventHistory.Empty();
    SleepEventHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Physics event history cleared"));
}

void UEng_PhysicsEventSystem::ProcessPendingEvents(float DeltaTime)
{
    // Process events within time budget
    double StartTime = FPlatformTime::Seconds();
    double BudgetSeconds = EventProcessingBudgetMs / 1000.0;
    
    int32 EventsProcessed = 0;
    
    // Process collision events
    for (int32 i = 0; i < CollisionEventHistory.Num() && EventsProcessed < MaxEventsPerFrame; ++i)
    {
        if (FPlatformTime::Seconds() - StartTime > BudgetSeconds)
        {
            break;
        }
        
        // Additional processing logic can be added here
        EventsProcessed++;
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processed %d physics events this frame"), EventsProcessed);
}

void UEng_PhysicsEventSystem::CleanupOldEvents(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Clean up collision events
    CollisionEventHistory.RemoveAll([CurrentTime, this](const FEng_CollisionEventData& Event)
    {
        return CurrentTime - Event.Timestamp > EventRetentionTime;
    });
    
    // Clean up break events
    BreakEventHistory.RemoveAll([CurrentTime, this](const FEng_BreakEventData& Event)
    {
        return CurrentTime - Event.Timestamp > EventRetentionTime;
    });
    
    // Clean up sleep events
    SleepEventHistory.RemoveAll([CurrentTime, this](const FEng_SleepEventData& Event)
    {
        return CurrentTime - Event.Timestamp > EventRetentionTime;
    });
}

void UEng_PhysicsEventSystem::SetEventTracking(bool bTrackCollisions, bool bTrackBreaks, bool bTrackSleep)
{
    bTrackCollisionEvents = bTrackCollisions;
    bTrackBreakEvents = bTrackBreaks;
    bTrackSleepEvents = bTrackSleep;
    
    UE_LOG(LogTemp, Log, TEXT("Physics event tracking updated - Collisions: %s, Breaks: %s, Sleep: %s"),
           bTrackCollisions ? TEXT("On") : TEXT("Off"),
           bTrackBreaks ? TEXT("On") : TEXT("Off"),
           bTrackSleep ? TEXT("On") : TEXT("Off"));
}