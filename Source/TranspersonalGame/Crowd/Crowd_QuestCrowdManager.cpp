#include "Crowd_QuestCrowdManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Math/UnrealMathUtility.h"

ACrowd_QuestCrowdManager::ACrowd_QuestCrowdManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Quest Settings
    QuestEventRadius = 2000.0f;
    MaxQuestCrowdSize = 100;
    QuestResponseTime = 30.0f;

    // Story Settings
    StoryMoodDuration = 600.0f;

    // Initialize story gathering points
    StoryGatheringPoints.Add(FVector(0, 0, 0));
    StoryGatheringPoints.Add(FVector(2000, 0, 0));
    StoryGatheringPoints.Add(FVector(-2000, 0, 0));
    StoryGatheringPoints.Add(FVector(0, 2000, 0));
    StoryGatheringPoints.Add(FVector(0, -2000, 0));
}

void ACrowd_QuestCrowdManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Crowd Manager initialized"));
    
    // Create initial story gathering points
    for (const FVector& Point : StoryGatheringPoints)
    {
        CreateStoryGathering(Point, TEXT("Initial Settlement"));
    }
}

void ACrowd_QuestCrowdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    ProcessQuestEvents(DeltaTime);
    UpdateQuestCrowds(DeltaTime);
    HandleStoryIntegration(DeltaTime);
}

void ACrowd_QuestCrowdManager::RegisterQuestEvent(const FCrowd_QuestEvent& QuestEvent)
{
    ActiveQuestEvents.Add(QuestEvent);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered quest event: %s at location %s"), 
           *QuestEvent.EventID, *QuestEvent.EventLocation.ToString());
}

void ACrowd_QuestCrowdManager::TriggerQuestEvent(const FString& EventID)
{
    FCrowd_QuestEvent* Event = FindQuestEvent(EventID);
    if (Event)
    {
        Event->bIsActive = true;
        SpawnQuestNPCs(*Event);
        
        // Create quest response
        FCrowd_QuestResponse Response;
        Response.ResponseID = EventID;
        Response.ResponseBehavior = Event->EventBehavior;
        Response.ResponseDuration = QuestResponseTime;
        Response.ParticipantCount = Event->RequiredCrowdSize;
        
        // Generate gathering points around event location
        for (int32 i = 0; i < 5; i++)
        {
            float Angle = (2.0f * PI * i) / 5.0f;
            FVector GatherPoint = Event->EventLocation + FVector(
                FMath::Cos(Angle) * Event->EventRadius * 0.5f,
                FMath::Sin(Angle) * Event->EventRadius * 0.5f,
                0.0f
            );
            Response.GatheringPoints.Add(GatherPoint);
        }
        
        QuestResponses.Add(Response);
        
        UE_LOG(LogTemp, Warning, TEXT("Triggered quest event: %s"), *EventID);
    }
}

void ACrowd_QuestCrowdManager::CompleteQuestEvent(const FString& EventID)
{
    for (int32 i = ActiveQuestEvents.Num() - 1; i >= 0; i--)
    {
        if (ActiveQuestEvents[i].EventID == EventID)
        {
            ActiveQuestEvents.RemoveAt(i);
            break;
        }
    }
    
    for (int32 i = QuestResponses.Num() - 1; i >= 0; i--)
    {
        if (QuestResponses[i].ResponseID == EventID)
        {
            QuestResponses.RemoveAt(i);
            break;
        }
    }
    
    DismissQuestCrowd(EventID);
    UE_LOG(LogTemp, Warning, TEXT("Completed quest event: %s"), *EventID);
}

FCrowd_QuestResponse ACrowd_QuestCrowdManager::GetQuestResponse(const FString& EventID)
{
    for (const FCrowd_QuestResponse& Response : QuestResponses)
    {
        if (Response.ResponseID == EventID)
        {
            return Response;
        }
    }
    
    return FCrowd_QuestResponse();
}

void ACrowd_QuestCrowdManager::SpawnQuestCrowd(const FVector& Location, int32 CrowdSize, ECrowd_BehaviorType Behavior)
{
    CrowdSize = FMath::Clamp(CrowdSize, 1, MaxQuestCrowdSize);
    
    for (int32 i = 0; i < CrowdSize; i++)
    {
        // Generate random position around location
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(100.0f, QuestEventRadius * 0.3f);
        
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Spawn NPC at location (placeholder for actual NPC spawning)
        UE_LOG(LogTemp, Log, TEXT("Spawning quest NPC %d at %s with behavior %d"), 
               i, *SpawnLocation.ToString(), (int32)Behavior);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned quest crowd of %d NPCs at %s"), 
           CrowdSize, *Location.ToString());
}

void ACrowd_QuestCrowdManager::MoveQuestCrowd(const FString& EventID, const FVector& TargetLocation)
{
    FCrowd_QuestEvent* Event = FindQuestEvent(EventID);
    if (Event)
    {
        Event->EventLocation = TargetLocation;
        UpdateNPCBehavior(*Event);
        
        UE_LOG(LogTemp, Warning, TEXT("Moving quest crowd for event %s to %s"), 
               *EventID, *TargetLocation.ToString());
    }
}

void ACrowd_QuestCrowdManager::DismissQuestCrowd(const FString& EventID)
{
    UE_LOG(LogTemp, Warning, TEXT("Dismissing quest crowd for event: %s"), *EventID);
    
    // Placeholder for actual NPC cleanup
    // In a full implementation, this would remove NPCs associated with the event
}

void ACrowd_QuestCrowdManager::CreateStoryGathering(const FVector& Location, const FString& StoryContext)
{
    // Create a story-driven gathering
    FCrowd_QuestEvent StoryEvent;
    StoryEvent.EventID = FString::Printf(TEXT("Story_%s_%d"), *StoryContext, FMath::RandRange(1000, 9999));
    StoryEvent.EventLocation = Location;
    StoryEvent.EventRadius = 1500.0f;
    StoryEvent.EventBehavior = ECrowd_BehaviorType::Gathering;
    StoryEvent.RequiredCrowdSize = FMath::RandRange(15, 40);
    StoryEvent.bIsActive = true;
    
    RegisterQuestEvent(StoryEvent);
    SpawnQuestCrowd(Location, StoryEvent.RequiredCrowdSize, ECrowd_BehaviorType::Gathering);
    
    UE_LOG(LogTemp, Warning, TEXT("Created story gathering '%s' at %s with %d NPCs"), 
           *StoryContext, *Location.ToString(), StoryEvent.RequiredCrowdSize);
}

void ACrowd_QuestCrowdManager::TriggerStoryReaction(const FString& StoryEvent, float IntensityLevel)
{
    IntensityLevel = FMath::Clamp(IntensityLevel, 0.0f, 1.0f);
    
    // Determine behavior based on story event and intensity
    ECrowd_BehaviorType ReactionBehavior = ECrowd_BehaviorType::Gathering;
    
    if (StoryEvent.Contains(TEXT("Danger")) || StoryEvent.Contains(TEXT("Attack")))
    {
        ReactionBehavior = ECrowd_BehaviorType::Fleeing;
    }
    else if (StoryEvent.Contains(TEXT("Celebration")) || StoryEvent.Contains(TEXT("Victory")))
    {
        ReactionBehavior = ECrowd_BehaviorType::Gathering;
    }
    else if (StoryEvent.Contains(TEXT("Hunt")) || StoryEvent.Contains(TEXT("Expedition")))
    {
        ReactionBehavior = ECrowd_BehaviorType::Patrolling;
    }
    
    // Apply reaction to all active quest events
    for (FCrowd_QuestEvent& Event : ActiveQuestEvents)
    {
        if (Event.bIsActive)
        {
            Event.EventBehavior = ReactionBehavior;
            UpdateNPCBehavior(Event);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Triggered story reaction '%s' with intensity %.2f, behavior %d"), 
           *StoryEvent, IntensityLevel, (int32)ReactionBehavior);
}

void ACrowd_QuestCrowdManager::SetStoryMood(ECrowd_BehaviorType MoodType, float Duration)
{
    // Set global mood for all crowds
    for (FCrowd_QuestEvent& Event : ActiveQuestEvents)
    {
        if (Event.bIsActive)
        {
            Event.EventBehavior = MoodType;
            UpdateNPCBehavior(Event);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Set story mood to %d for %.2f seconds"), 
           (int32)MoodType, Duration);
}

void ACrowd_QuestCrowdManager::ProcessQuestEvents(float DeltaTime)
{
    // Update quest event timers and states
    for (FCrowd_QuestEvent& Event : ActiveQuestEvents)
    {
        if (Event.bIsActive)
        {
            // Process event logic here
            // This is where you'd handle event-specific behavior updates
        }
    }
}

void ACrowd_QuestCrowdManager::UpdateQuestCrowds(float DeltaTime)
{
    // Update crowd behaviors for active quest events
    for (const FCrowd_QuestEvent& Event : ActiveQuestEvents)
    {
        if (Event.bIsActive)
        {
            UpdateNPCBehavior(Event);
        }
    }
}

void ACrowd_QuestCrowdManager::HandleStoryIntegration(float DeltaTime)
{
    // Handle ongoing story integration updates
    // This could include mood changes, event transitions, etc.
}

FCrowd_QuestEvent* ACrowd_QuestCrowdManager::FindQuestEvent(const FString& EventID)
{
    for (FCrowd_QuestEvent& Event : ActiveQuestEvents)
    {
        if (Event.EventID == EventID)
        {
            return &Event;
        }
    }
    return nullptr;
}

void ACrowd_QuestCrowdManager::SpawnQuestNPCs(const FCrowd_QuestEvent& QuestEvent)
{
    SpawnQuestCrowd(QuestEvent.EventLocation, QuestEvent.RequiredCrowdSize, QuestEvent.EventBehavior);
}

void ACrowd_QuestCrowdManager::UpdateNPCBehavior(const FCrowd_QuestEvent& QuestEvent)
{
    // Update NPC behavior based on quest event
    UE_LOG(LogTemp, Log, TEXT("Updating NPC behavior for event %s to behavior %d"), 
           *QuestEvent.EventID, (int32)QuestEvent.EventBehavior);
}