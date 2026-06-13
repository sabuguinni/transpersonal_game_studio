#include "Narr_SurvivalNarrator.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_SurvivalNarrator::UNarr_SurvivalNarrator()
{
    PrimaryComponentTick.bCanEverTick = false;
    NarrativeDuration = 8.0f;
    bAutoTriggerEvents = true;
}

void UNarr_SurvivalNarrator::BeginPlay()
{
    Super::BeginPlay();
    InitializeSurvivalEvents();
}

void UNarr_SurvivalNarrator::InitializeSurvivalEvents()
{
    EventNarratives.Empty();
    
    EventNarratives.Add(ENarr_SurvivalEvent::FirstHunt, 
        TEXT("The scent of prey fills your nostrils. Your first hunt begins now. Move carefully - one mistake could mean death."));
    
    EventNarratives.Add(ENarr_SurvivalEvent::PackEncounter, 
        TEXT("You hear the low growls of a pack nearby. They've caught your scent. Fight or flight - choose wisely."));
    
    EventNarratives.Add(ENarr_SurvivalEvent::WaterFound, 
        TEXT("Fresh water! Your parched throat rejoices, but remember - predators gather at watering holes."));
    
    EventNarratives.Add(ENarr_SurvivalEvent::ShelterBuilt, 
        TEXT("Your shelter stands complete. Tonight you'll sleep protected from the elements and prowling beasts."));
    
    EventNarratives.Add(ENarr_SurvivalEvent::FireDiscovered, 
        TEXT("Fire! The dancing flames will keep the darkness at bay and cook your meat. You've mastered a primal force."));
    
    EventNarratives.Add(ENarr_SurvivalEvent::TerritoryMarked, 
        TEXT("This land is yours now. You've marked your territory and claimed your place in this harsh world."));
    
    EventNarratives.Add(ENarr_SurvivalEvent::AlphaChallenge, 
        TEXT("The alpha approaches. This is your moment - prove your dominance or submit to a stronger will."));
    
    EventNarratives.Add(ENarr_SurvivalEvent::StormSurvived, 
        TEXT("The storm passes. You've weathered nature's fury and emerged stronger. The weak perish, but you endure."));
}

void UNarr_SurvivalNarrator::TriggerSurvivalEvent(ENarr_SurvivalEvent EventType, float Intensity)
{
    if (EventType == ENarr_SurvivalEvent::None)
    {
        return;
    }

    // Clear any existing narrative timer
    if (GetWorld() && NarrativeClearTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(NarrativeClearTimer);
    }

    // Create new survival moment
    CurrentMoment.EventType = EventType;
    CurrentMoment.Intensity = FMath::Clamp(Intensity, 0.1f, 2.0f);
    CurrentMoment.NarrativeText = GenerateNarrativeForEvent(EventType, CurrentMoment.Intensity);
    CurrentMoment.bIsTriggered = true;

    // Add to history
    EventHistory.Add(CurrentMoment);
    if (EventHistory.Num() > 20)
    {
        EventHistory.RemoveAt(0);
    }

    // Set timer to clear narrative
    if (GetWorld())
    {
        float ClearDelay = NarrativeDuration * CurrentMoment.Intensity;
        GetWorld()->GetTimerManager().SetTimer(NarrativeClearTimer, this, 
            &UNarr_SurvivalNarrator::ClearCurrentNarrative, ClearDelay, false);
    }
}

FString UNarr_SurvivalNarrator::GenerateNarrativeForEvent(ENarr_SurvivalEvent EventType, float Intensity)
{
    FString BaseNarrative = EventNarratives.FindRef(EventType);
    
    if (BaseNarrative.IsEmpty())
    {
        return TEXT("Something stirs in the wilderness...");
    }

    // Modify narrative based on intensity
    if (Intensity > 1.5f)
    {
        BaseNarrative = TEXT("URGENT: ") + BaseNarrative;
    }
    else if (Intensity < 0.5f)
    {
        BaseNarrative = TEXT("Quietly: ") + BaseNarrative;
    }

    return BaseNarrative;
}

FString UNarr_SurvivalNarrator::GetCurrentNarrative() const
{
    if (CurrentMoment.bIsTriggered)
    {
        return CurrentMoment.NarrativeText;
    }
    return TEXT("");
}

bool UNarr_SurvivalNarrator::HasActiveNarrative() const
{
    return CurrentMoment.bIsTriggered && !CurrentMoment.NarrativeText.IsEmpty();
}

void UNarr_SurvivalNarrator::ClearCurrentNarrative()
{
    CurrentMoment.bIsTriggered = false;
    CurrentMoment.NarrativeText = TEXT("");
    CurrentMoment.EventType = ENarr_SurvivalEvent::None;
    CurrentMoment.Intensity = 1.0f;

    if (GetWorld() && NarrativeClearTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(NarrativeClearTimer);
    }
}