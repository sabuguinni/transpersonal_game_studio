#include "Narr_SurvivalStorytellerSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UNarr_SurvivalStorytellerSystem::UNarr_SurvivalStorytellerSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    CurrentTension = 0.3f;
    TimeSinceLastEvent = 0.0f;
}

void UNarr_SurvivalStorytellerSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Survival Storyteller System initialized"));
    
    // Initialize with a welcome event
    FNarr_SurvivalEvent WelcomeEvent;
    WelcomeEvent.EventType = ENarr_SurvivalEventType::EnvironmentalDanger;
    WelcomeEvent.EventTitle = TEXT("Dawn of Survival");
    WelcomeEvent.EventDescription = TEXT("The morning mist reveals ancient tracks in the mud. Something large passed this way during the night.");
    WelcomeEvent.Tone = ENarr_StoryTone::Ominous;
    WelcomeEvent.EventDuration = 45.0f;
    WelcomeEvent.EventLocation = GetOwner()->GetActorLocation();
    
    ActiveEvents.Add(WelcomeEvent);
    OnSurvivalEventTriggered(WelcomeEvent);
}

void UNarr_SurvivalStorytellerSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastEvent += DeltaTime;
    
    UpdateTensionLevel(DeltaTime);
    ProcessActiveEvents(DeltaTime);
    CleanupExpiredEvents();
    
    // Check if we should trigger a new event
    if (ShouldTriggerEvent() && ActiveEvents.Num() < Config.MaxActiveEvents)
    {
        FNarr_SurvivalEvent NewEvent = CreateRandomEvent();
        TriggerSurvivalEvent(NewEvent.EventType, NewEvent.EventLocation);
        TimeSinceLastEvent = 0.0f;
    }
}

void UNarr_SurvivalStorytellerSystem::TriggerSurvivalEvent(ENarr_SurvivalEventType EventType, FVector Location)
{
    FNarr_SurvivalEvent NewEvent;
    NewEvent.EventType = EventType;
    NewEvent.EventLocation = Location;
    NewEvent.EventDuration = FMath::RandRange(30.0f, 120.0f);
    
    switch (EventType)
    {
        case ENarr_SurvivalEventType::DinosaurSighting:
            NewEvent.EventTitle = TEXT("Predator Spotted");
            NewEvent.EventDescription = TEXT("Movement in the dense foliage catches your eye. The bushes part to reveal massive claws and gleaming teeth.");
            NewEvent.Tone = ENarr_StoryTone::Tense;
            NewEvent.bRequiresPlayerAction = true;
            break;
            
        case ENarr_SurvivalEventType::WeatherChange:
            NewEvent.EventTitle = TEXT("Storm Approaching");
            NewEvent.EventDescription = TEXT("Dark clouds gather on the horizon. The wind carries the scent of rain and the distant rumble of thunder.");
            NewEvent.Tone = ENarr_StoryTone::Cautious;
            NewEvent.bRequiresPlayerAction = false;
            break;
            
        case ENarr_SurvivalEventType::ResourceDiscovery:
            NewEvent.EventTitle = TEXT("Valuable Find");
            NewEvent.EventDescription = TEXT("Sharp stones glint in the sunlight, perfect for crafting tools. Fresh water trickles from a hidden spring.");
            NewEvent.Tone = ENarr_StoryTone::Hopeful;
            NewEvent.bRequiresPlayerAction = true;
            break;
            
        case ENarr_SurvivalEventType::HuntSuccess:
            NewEvent.EventTitle = TEXT("Successful Hunt");
            NewEvent.EventDescription = TEXT("The spear finds its mark. Fresh meat will feed the tribe tonight, and the hide will provide warmth.");
            NewEvent.Tone = ENarr_StoryTone::Triumphant;
            NewEvent.bRequiresPlayerAction = false;
            break;
            
        case ENarr_SurvivalEventType::CampThreat:
            NewEvent.EventTitle = TEXT("Camp Under Threat");
            NewEvent.EventDescription = TEXT("Scratches on nearby trees and disturbed earth suggest a large predator has been investigating your shelter.");
            NewEvent.Tone = ENarr_StoryTone::Urgent;
            NewEvent.bRequiresPlayerAction = true;
            break;
            
        case ENarr_SurvivalEventType::AllyEncounter:
            NewEvent.EventTitle = TEXT("Fellow Survivor");
            NewEvent.EventDescription = TEXT("Smoke rises from a distant fire. Another human has survived in this harsh land and may be willing to trade.");
            NewEvent.Tone = ENarr_StoryTone::Hopeful;
            NewEvent.bRequiresPlayerAction = true;
            break;
            
        default:
            NewEvent.EventTitle = TEXT("Unknown Event");
            NewEvent.EventDescription = TEXT("Something stirs in the prehistoric wilderness.");
            NewEvent.Tone = ENarr_StoryTone::Neutral;
            break;
    }
    
    ActiveEvents.Add(NewEvent);
    OnSurvivalEventTriggered(NewEvent);
    
    UE_LOG(LogTemp, Warning, TEXT("Triggered survival event: %s"), *NewEvent.EventTitle);
}

void UNarr_SurvivalStorytellerSystem::CompleteEvent(int32 EventIndex)
{
    if (ActiveEvents.IsValidIndex(EventIndex))
    {
        FNarr_SurvivalEvent CompletedEvent = ActiveEvents[EventIndex];
        OnEventCompleted(CompletedEvent);
        ActiveEvents.RemoveAt(EventIndex);
        
        // Adjust tension based on event completion
        if (CompletedEvent.bRequiresPlayerAction)
        {
            CurrentTension = FMath::Max(0.1f, CurrentTension - 0.1f);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Completed survival event: %s"), *CompletedEvent.EventTitle);
    }
}

FNarr_SurvivalEvent UNarr_SurvivalStorytellerSystem::CreateRandomEvent()
{
    FNarr_SurvivalEvent RandomEvent;
    
    // Select event type based on current context and tension
    ENarr_SurvivalEventType SelectedType = SelectEventBasedOnContext();
    
    // Generate location near player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    FVector EventLocation = PlayerLocation + FVector(
        FMath::RandRange(-Config.NarrativeRadius, Config.NarrativeRadius),
        FMath::RandRange(-Config.NarrativeRadius, Config.NarrativeRadius),
        0.0f
    );
    
    RandomEvent.EventType = SelectedType;
    RandomEvent.EventLocation = EventLocation;
    RandomEvent.EventDuration = FMath::RandRange(45.0f, 180.0f);
    
    return RandomEvent;
}

void UNarr_SurvivalStorytellerSystem::UpdateTensionLevel(float DeltaTime)
{
    // Gradually increase tension over time
    float TensionIncrease = DeltaTime * 0.01f;
    CurrentTension = FMath::Min(1.0f, CurrentTension + TensionIncrease);
    
    // Active threatening events increase tension faster
    for (const FNarr_SurvivalEvent& Event : ActiveEvents)
    {
        if (Event.Tone == ENarr_StoryTone::Tense || Event.Tone == ENarr_StoryTone::Urgent)
        {
            CurrentTension = FMath::Min(1.0f, CurrentTension + DeltaTime * 0.02f);
        }
    }
    
    // Adapt config based on tension
    if (Config.bAdaptToPlayerActions)
    {
        Config.EventFrequency = FMath::Lerp(180.0f, 60.0f, CurrentTension);
    }
}

bool UNarr_SurvivalStorytellerSystem::ShouldTriggerEvent() const
{
    if (TimeSinceLastEvent < Config.EventFrequency)
    {
        return false;
    }
    
    // Higher tension increases event probability
    float EventProbability = FMath::Lerp(0.3f, 0.8f, CurrentTension);
    return FMath::RandRange(0.0f, 1.0f) < EventProbability;
}

FString UNarr_SurvivalStorytellerSystem::GetEventNarration(const FNarr_SurvivalEvent& Event) const
{
    FString Narration = FString::Printf(TEXT("[%s] %s - %s"), 
        *Event.EventTitle, 
        *UEnum::GetValueAsString(Event.Tone),
        *Event.EventDescription
    );
    
    return Narration;
}

void UNarr_SurvivalStorytellerSystem::ProcessActiveEvents(float DeltaTime)
{
    for (int32 i = ActiveEvents.Num() - 1; i >= 0; i--)
    {
        FNarr_SurvivalEvent& Event = ActiveEvents[i];
        Event.EventDuration -= DeltaTime;
        
        // Auto-complete events that don't require player action
        if (Event.EventDuration <= 0.0f && !Event.bRequiresPlayerAction)
        {
            CompleteEvent(i);
        }
    }
}

void UNarr_SurvivalStorytellerSystem::CleanupExpiredEvents()
{
    for (int32 i = ActiveEvents.Num() - 1; i >= 0; i--)
    {
        if (ActiveEvents[i].EventDuration <= -30.0f) // Grace period for player action
        {
            UE_LOG(LogTemp, Warning, TEXT("Event expired: %s"), *ActiveEvents[i].EventTitle);
            ActiveEvents.RemoveAt(i);
        }
    }
}

ENarr_SurvivalEventType UNarr_SurvivalStorytellerSystem::SelectEventBasedOnContext()
{
    TArray<ENarr_SurvivalEventType> PossibleEvents;
    
    // Add events based on current tension level
    if (CurrentTension > 0.7f)
    {
        PossibleEvents.Add(ENarr_SurvivalEventType::DinosaurSighting);
        PossibleEvents.Add(ENarr_SurvivalEventType::CampThreat);
        PossibleEvents.Add(ENarr_SurvivalEventType::EnvironmentalDanger);
    }
    else if (CurrentTension > 0.4f)
    {
        PossibleEvents.Add(ENarr_SurvivalEventType::WeatherChange);
        PossibleEvents.Add(ENarr_SurvivalEventType::TerritorialDispute);
        PossibleEvents.Add(ENarr_SurvivalEventType::SeasonalMigration);
    }
    else
    {
        PossibleEvents.Add(ENarr_SurvivalEventType::ResourceDiscovery);
        PossibleEvents.Add(ENarr_SurvivalEventType::AllyEncounter);
        PossibleEvents.Add(ENarr_SurvivalEventType::HuntSuccess);
    }
    
    // Always include some variety
    PossibleEvents.Add(ENarr_SurvivalEventType::DinosaurSighting);
    PossibleEvents.Add(ENarr_SurvivalEventType::ResourceDiscovery);
    
    if (PossibleEvents.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, PossibleEvents.Num() - 1);
        return PossibleEvents[RandomIndex];
    }
    
    return ENarr_SurvivalEventType::EnvironmentalDanger;
}

float UNarr_SurvivalStorytellerSystem::CalculateEventProbability(ENarr_SurvivalEventType EventType) const
{
    switch (EventType)
    {
        case ENarr_SurvivalEventType::DinosaurSighting:
            return FMath::Lerp(0.2f, 0.8f, CurrentTension);
        case ENarr_SurvivalEventType::ResourceDiscovery:
            return FMath::Lerp(0.6f, 0.2f, CurrentTension);
        case ENarr_SurvivalEventType::WeatherChange:
            return 0.4f;
        case ENarr_SurvivalEventType::AllyEncounter:
            return FMath::Lerp(0.3f, 0.1f, CurrentTension);
        default:
            return 0.3f;
    }
}