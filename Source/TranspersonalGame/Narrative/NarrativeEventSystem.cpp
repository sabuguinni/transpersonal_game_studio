#include "NarrativeEventSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UNarrativeEventSystem::UNarrativeEventSystem()
{
    bNarrativeEnabled = true;
    bSubtitlesEnabled = true;
    MasterVolume = 1.0f;
    MaxNarrationDistance = 5000.0f;
    bIsProcessingEvents = false;
    NarrationAudioComponent = nullptr;
}

void UNarrativeEventSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeEventSystem: Initializing narrative system"));
    
    // Initialize event counts
    EventCounts.Empty();
    for (int32 i = 0; i < (int32)ENarr_EventType::DayNightTransition + 1; ++i)
    {
        ENarr_EventType EventType = (ENarr_EventType)i;
        EventCounts.Add(EventType, 0);
    }
    
    // Initialize response templates
    InitializeResponseTemplates();
    
    // Start event processing timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(EventProcessingTimer, this, &UNarrativeEventSystem::ProcessEventQueue, 0.1f, true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeEventSystem: Initialization complete"));
}

void UNarrativeEventSystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EventProcessingTimer);
    }
    
    StopCurrentNarration();
    
    Super::Deinitialize();
}

void UNarrativeEventSystem::TriggerNarrativeEvent(ENarr_EventType EventType, const FVector& Location, AActor* TriggeringActor, float Intensity)
{
    if (!bNarrativeEnabled)
    {
        return;
    }
    
    FNarr_EventData EventData;
    EventData.EventType = EventType;
    EventData.EventLocation = Location;
    EventData.TriggeringActor = TriggeringActor;
    EventData.EventIntensity = FMath::Clamp(Intensity, 0.1f, 2.0f);
    EventData.bIsFirstTime = !HasEventOccurred(EventType);
    
    // Generate description based on event type
    switch (EventType)
    {
        case ENarr_EventType::FirstDinosaurSighting:
            EventData.EventDescription = TEXT("First encounter with a prehistoric predator");
            break;
        case ENarr_EventType::FirstKill:
            EventData.EventDescription = TEXT("First successful hunt");
            break;
        case ENarr_EventType::FirstCraft:
            EventData.EventDescription = TEXT("First tool crafted");
            break;
        case ENarr_EventType::FirstFire:
            EventData.EventDescription = TEXT("Fire discovered or created");
            break;
        case ENarr_EventType::DangerWarning:
            EventData.EventDescription = TEXT("Imminent threat detected");
            break;
        case ENarr_EventType::Discovery:
            EventData.EventDescription = TEXT("Important discovery made");
            break;
        case ENarr_EventType::CombatStart:
            EventData.EventDescription = TEXT("Combat initiated");
            break;
        case ENarr_EventType::ResourceFound:
            EventData.EventDescription = TEXT("Valuable resource located");
            break;
        default:
            EventData.EventDescription = TEXT("Narrative event occurred");
            break;
    }
    
    // Add to pending events
    PendingEvents.Add(EventData);
    
    // Update event count
    if (EventCounts.Contains(EventType))
    {
        EventCounts[EventType]++;
    }
    
    // Add to history
    EventHistory.Add(EventData);
    
    // Broadcast event
    OnNarrativeEvent.Broadcast(EventData);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeEventSystem: Triggered event %s at location %s"), 
           *EventData.EventDescription, *Location.ToString());
}

void UNarrativeEventSystem::TriggerCustomEvent(const FString& EventDescription, const FVector& Location, AActor* TriggeringActor)
{
    FNarr_EventData EventData;
    EventData.EventType = ENarr_EventType::Discovery;
    EventData.EventDescription = EventDescription;
    EventData.EventLocation = Location;
    EventData.TriggeringActor = TriggeringActor;
    EventData.EventIntensity = 1.0f;
    EventData.bIsFirstTime = true;
    
    PendingEvents.Add(EventData);
    EventHistory.Add(EventData);
    
    OnNarrativeEvent.Broadcast(EventData);
}

bool UNarrativeEventSystem::HasEventOccurred(ENarr_EventType EventType) const
{
    if (const int32* Count = EventCounts.Find(EventType))
    {
        return *Count > 0;
    }
    return false;
}

int32 UNarrativeEventSystem::GetEventCount(ENarr_EventType EventType) const
{
    if (const int32* Count = EventCounts.Find(EventType))
    {
        return *Count;
    }
    return 0;
}

void UNarrativeEventSystem::MarkEventAsOccurred(ENarr_EventType EventType)
{
    if (EventCounts.Contains(EventType))
    {
        EventCounts[EventType]++;
    }
    else
    {
        EventCounts.Add(EventType, 1);
    }
}

FNarr_NarrativeResponse UNarrativeEventSystem::GenerateResponseForEvent(const FNarr_EventData& EventData)
{
    FNarr_NarrativeResponse Response;
    
    // Select appropriate narrator based on event type
    switch (EventData.EventType)
    {
        case ENarr_EventType::FirstDinosaurSighting:
        case ENarr_EventType::DangerWarning:
            Response.NarratorType = ENarr_NarratorType::InternalThought;
            Response.NarrativeText = TEXT("The ground trembles. Something massive approaches. Every instinct screams danger.");
            Response.DelayBeforePlay = 0.5f;
            break;
            
        case ENarr_EventType::FirstKill:
            Response.NarratorType = ENarr_NarratorType::AncestralWisdom;
            Response.NarrativeText = TEXT("The hunt is complete. Your ancestors smile upon this victory. Survival demands such choices.");
            Response.DelayBeforePlay = 1.0f;
            break;
            
        case ENarr_EventType::FirstCraft:
            Response.NarratorType = ENarr_NarratorType::SurvivalGuide;
            Response.NarrativeText = TEXT("Stone shaped by will. Tool born from necessity. This is how civilization begins.");
            Response.DelayBeforePlay = 0.3f;
            break;
            
        case ENarr_EventType::FirstFire:
            Response.NarratorType = ENarr_NarratorType::AncestralWisdom;
            Response.NarrativeText = TEXT("Fire! The gift that separates us from beasts. Guard it well, for it guards you.");
            Response.DelayBeforePlay = 0.8f;
            break;
            
        case ENarr_EventType::CombatStart:
            Response.NarratorType = ENarr_NarratorType::CombatInstructor;
            Response.NarrativeText = TEXT("Strike fast, strike true. Hesitation is death. Trust your training.");
            Response.DelayBeforePlay = 0.2f;
            break;
            
        case ENarr_EventType::Discovery:
            Response.NarratorType = ENarr_NarratorType::InternalThought;
            Response.NarrativeText = TEXT("Something important lies here. The land reveals its secrets to those who observe.");
            Response.DelayBeforePlay = 0.5f;
            break;
            
        case ENarr_EventType::ResourceFound:
            Response.NarratorType = ENarr_NarratorType::SurvivalGuide;
            Response.NarrativeText = TEXT("Resources gathered. Each stone, each branch brings you closer to mastery over this harsh world.");
            Response.DelayBeforePlay = 0.3f;
            break;
            
        default:
            Response.NarratorType = ENarr_NarratorType::InternalThought;
            Response.NarrativeText = TEXT("The world speaks to those who listen. What story does this moment tell?");
            Response.DelayBeforePlay = 0.5f;
            break;
    }
    
    Response.AudioVolume = MasterVolume * EventData.EventIntensity;
    Response.bShowSubtitles = bSubtitlesEnabled;
    
    return Response;
}

void UNarrativeEventSystem::PlayNarrativeResponse(const FNarr_NarrativeResponse& Response, const FVector& Location)
{
    if (!bNarrativeEnabled)
    {
        return;
    }
    
    // Stop current narration if playing
    StopCurrentNarration();
    
    // Create audio component if needed
    if (!NarrationAudioComponent)
    {
        if (UWorld* World = GetWorld())
        {
            AActor* DummyActor = World->SpawnActor<AActor>();
            if (DummyActor)
            {
                NarrationAudioComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudio"));
            }
        }
    }
    
    // Broadcast response
    FNarr_EventData DummyEventData; // For delegate compatibility
    OnNarrativeResponse.Broadcast(DummyEventData, Response);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeEventSystem: Playing narrative response: %s"), *Response.NarrativeText);
}

void UNarrativeEventSystem::StopCurrentNarration()
{
    if (NarrationAudioComponent && NarrationAudioComponent->IsPlaying())
    {
        NarrationAudioComponent->Stop();
    }
}

bool UNarrativeEventSystem::IsNarrationPlaying() const
{
    return NarrationAudioComponent && NarrationAudioComponent->IsPlaying();
}

void UNarrativeEventSystem::SetNarrativeEnabled(bool bEnabled)
{
    bNarrativeEnabled = bEnabled;
    if (!bEnabled)
    {
        StopCurrentNarration();
    }
}

void UNarrativeEventSystem::SetSubtitlesEnabled(bool bEnabled)
{
    bSubtitlesEnabled = bEnabled;
}

void UNarrativeEventSystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
}

void UNarrativeEventSystem::InitializeResponseTemplates()
{
    // Initialize response templates for different event types
    ResponseTemplates.Empty();
    
    // This would typically be loaded from data tables or config files
    // For now, using hardcoded responses for core events
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeEventSystem: Response templates initialized"));
}

void UNarrativeEventSystem::ProcessEventQueue()
{
    if (bIsProcessingEvents || PendingEvents.Num() == 0)
    {
        return;
    }
    
    bIsProcessingEvents = true;
    
    // Process oldest event first
    if (PendingEvents.Num() > 0)
    {
        FNarr_EventData EventData = PendingEvents[0];
        PendingEvents.RemoveAt(0);
        
        // Generate and play response
        FNarr_NarrativeResponse Response = GenerateResponseForEvent(EventData);
        
        // Delay before playing if specified
        if (Response.DelayBeforePlay > 0.0f)
        {
            if (UWorld* World = GetWorld())
            {
                FTimerHandle DelayTimer;
                World->GetTimerManager().SetTimer(DelayTimer, [this, Response, EventData]()
                {
                    PlayNarrativeResponse(Response, EventData.EventLocation);
                }, Response.DelayBeforePlay, false);
            }
        }
        else
        {
            PlayNarrativeResponse(Response, EventData.EventLocation);
        }
    }
    
    bIsProcessingEvents = false;
}

FNarr_NarrativeResponse UNarrativeEventSystem::SelectBestResponse(const FNarr_EventData& EventData)
{
    // Advanced response selection logic would go here
    // For now, use the basic generation method
    return GenerateResponseForEvent(EventData);
}

void UNarrativeEventSystem::PlayAudioAtLocation(const FString& AudioPath, const FVector& Location, float Volume)
{
    if (UWorld* World = GetWorld())
    {
        // This would load and play audio at the specified location
        // Implementation depends on audio asset management system
        UE_LOG(LogTemp, Log, TEXT("NarrativeEventSystem: Playing audio %s at location %s"), 
               *AudioPath, *Location.ToString());
    }
}