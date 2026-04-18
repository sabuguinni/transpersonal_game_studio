#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized"));
    InitializeDefaultContent();
}

void UNarrativeManager::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            if (CheckEventPrerequisites(Event))
            {
                Event.bIsCompleted = true;
                OnStoryEventTriggered.Broadcast(EventID);
                UnlockDependentEvents(EventID);
                
                UE_LOG(LogTemp, Log, TEXT("Story event triggered: %s"), *EventID);
                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Story event prerequisites not met: %s"), *EventID);
                return;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Story event not found or already completed: %s"), *EventID);
}

bool UNarrativeManager::IsStoryEventCompleted(const FString& EventID) const
{
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event.bIsCompleted;
        }
    }
    return false;
}

TArray<FString> UNarrativeManager::GetAvailableStoryEvents() const
{
    TArray<FString> AvailableEvents;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bIsCompleted && CheckEventPrerequisites(Event))
        {
            AvailableEvents.Add(Event.EventID);
        }
    }
    
    return AvailableEvents;
}

void UNarrativeManager::AddStoryEvent(const FNarr_StoryEvent& NewEvent)
{
    // Check if event already exists
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == NewEvent.EventID)
        {
            Event = NewEvent;
            return;
        }
    }
    
    StoryEvents.Add(NewEvent);
}

void UNarrativeManager::ModifyCharacterTrust(const FString& CharacterID, int32 TrustChange)
{
    for (FNarr_CharacterProfile& Profile : CharacterProfiles)
    {
        if (Profile.CharacterID == CharacterID)
        {
            int32 OldTrust = Profile.TrustLevel;
            Profile.TrustLevel = FMath::Clamp(Profile.TrustLevel + TrustChange, -100, 100);
            
            if (Profile.TrustLevel != OldTrust)
            {
                OnCharacterTrustChanged.Broadcast(CharacterID, Profile.TrustLevel);
                UE_LOG(LogTemp, Log, TEXT("Character trust changed: %s (%d -> %d)"), 
                       *CharacterID, OldTrust, Profile.TrustLevel);
            }
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Character profile not found: %s"), *CharacterID);
}

int32 UNarrativeManager::GetCharacterTrust(const FString& CharacterID) const
{
    for (const FNarr_CharacterProfile& Profile : CharacterProfiles)
    {
        if (Profile.CharacterID == CharacterID)
        {
            return Profile.TrustLevel;
        }
    }
    return 0;
}

void UNarrativeManager::AddCharacterProfile(const FNarr_CharacterProfile& NewProfile)
{
    // Check if profile already exists
    for (FNarr_CharacterProfile& Profile : CharacterProfiles)
    {
        if (Profile.CharacterID == NewProfile.CharacterID)
        {
            Profile = NewProfile;
            return;
        }
    }
    
    CharacterProfiles.Add(NewProfile);
}

FNarr_CharacterProfile UNarrativeManager::GetCharacterProfile(const FString& CharacterID) const
{
    for (const FNarr_CharacterProfile& Profile : CharacterProfiles)
    {
        if (Profile.CharacterID == CharacterID)
        {
            return Profile;
        }
    }
    
    return FNarr_CharacterProfile();
}

void UNarrativeManager::DiscoverLore(const FString& LoreID)
{
    for (FNarr_WorldLore& Lore : WorldLore)
    {
        if (Lore.LoreID == LoreID && !Lore.bIsDiscovered)
        {
            Lore.bIsDiscovered = true;
            OnLoreDiscovered.Broadcast(LoreID);
            UE_LOG(LogTemp, Log, TEXT("Lore discovered: %s"), *LoreID);
            return;
        }
    }
}

bool UNarrativeManager::IsLoreDiscovered(const FString& LoreID) const
{
    for (const FNarr_WorldLore& Lore : WorldLore)
    {
        if (Lore.LoreID == LoreID)
        {
            return Lore.bIsDiscovered;
        }
    }
    return false;
}

TArray<FNarr_WorldLore> UNarrativeManager::GetDiscoveredLore() const
{
    TArray<FNarr_WorldLore> DiscoveredLore;
    
    for (const FNarr_WorldLore& Lore : WorldLore)
    {
        if (Lore.bIsDiscovered)
        {
            DiscoveredLore.Add(Lore);
        }
    }
    
    return DiscoveredLore;
}

void UNarrativeManager::AddWorldLore(const FNarr_WorldLore& NewLore)
{
    // Check if lore already exists
    for (FNarr_WorldLore& Lore : WorldLore)
    {
        if (Lore.LoreID == NewLore.LoreID)
        {
            Lore = NewLore;
            return;
        }
    }
    
    WorldLore.Add(NewLore);
}

void UNarrativeManager::SaveNarrativeState()
{
    // TODO: Implement save system integration
    UE_LOG(LogTemp, Log, TEXT("Narrative state saved"));
}

void UNarrativeManager::LoadNarrativeState()
{
    // TODO: Implement save system integration
    UE_LOG(LogTemp, Log, TEXT("Narrative state loaded"));
}

void UNarrativeManager::InitializeDefaultContent()
{
    // Initialize default story events
    FNarr_StoryEvent FirstContact;
    FirstContact.EventID = TEXT("first_contact");
    FirstContact.EventName = TEXT("First Contact");
    FirstContact.EventDescription = FText::FromString(TEXT("The player encounters their first tribal settlement"));
    FirstContact.bIsCompleted = false;
    StoryEvents.Add(FirstContact);

    FNarr_StoryEvent FirstHunt;
    FirstHunt.EventID = TEXT("first_hunt");
    FirstHunt.EventName = TEXT("First Hunt");
    FirstHunt.EventDescription = FText::FromString(TEXT("The player participates in their first dinosaur hunt"));
    FirstHunt.Prerequisites.Add(TEXT("first_contact"));
    StoryEvents.Add(FirstHunt);

    FNarr_StoryEvent AlphaChallenge;
    AlphaChallenge.EventID = TEXT("alpha_challenge");
    AlphaChallenge.EventName = TEXT("Alpha Challenge");
    AlphaChallenge.EventDescription = FText::FromString(TEXT("The player faces the alpha predator of the region"));
    AlphaChallenge.Prerequisites.Add(TEXT("first_hunt"));
    StoryEvents.Add(AlphaChallenge);

    // Initialize default character profiles
    FNarr_CharacterProfile TribalElder;
    TribalElder.CharacterID = TEXT("tribal_elder");
    TribalElder.CharacterName = TEXT("Kael the Elder");
    TribalElder.BackgroundStory = FText::FromString(TEXT("An experienced hunter who has survived countless encounters with the great beasts"));
    TribalElder.PersonalityTraits.Add(TEXT("Wise"));
    TribalElder.PersonalityTraits.Add(TEXT("Cautious"));
    TribalElder.PersonalityTraits.Add(TEXT("Protective"));
    TribalElder.TrustLevel = 10;
    CharacterProfiles.Add(TribalElder);

    FNarr_CharacterProfile YoungScout;
    YoungScout.CharacterID = TEXT("young_scout");
    YoungScout.CharacterName = TEXT("Zara the Swift");
    YoungScout.BackgroundStory = FText::FromString(TEXT("A quick and agile scout who knows the dangerous paths through dinosaur territory"));
    YoungScout.PersonalityTraits.Add(TEXT("Brave"));
    YoungScout.PersonalityTraits.Add(TEXT("Impulsive"));
    YoungScout.PersonalityTraits.Add(TEXT("Loyal"));
    YoungScout.TrustLevel = 5;
    CharacterProfiles.Add(YoungScout);

    // Initialize default world lore
    FNarr_WorldLore CarnotaurusLore;
    CarnotaurusLore.LoreID = TEXT("carnotaurus_behavior");
    CarnotaurusLore.LoreCategory = TEXT("Predator Knowledge");
    CarnotaurusLore.LoreTitle = FText::FromString(TEXT("The Horned Death"));
    CarnotaurusLore.LoreContent = FText::FromString(TEXT("The Carnotaurus is a cunning predator that hunts by ambush. Its speed and intelligence make it one of the most dangerous threats in the valley."));
    WorldLore.Add(CarnotaurusLore);

    FNarr_WorldLore TribalHistory;
    TribalHistory.LoreID = TEXT("tribal_origins");
    TribalHistory.LoreCategory = TEXT("Tribal History");
    TribalHistory.LoreTitle = FText::FromString(TEXT("The First Hunters"));
    TribalHistory.LoreContent = FText::FromString(TEXT("Long ago, the first humans learned to survive alongside the great beasts through cunning, cooperation, and respect for the natural order."));
    WorldLore.Add(TribalHistory);

    UE_LOG(LogTemp, Log, TEXT("Default narrative content initialized"));
}

bool UNarrativeManager::CheckEventPrerequisites(const FNarr_StoryEvent& Event) const
{
    for (const FString& PrereqID : Event.Prerequisites)
    {
        if (!IsStoryEventCompleted(PrereqID))
        {
            return false;
        }
    }
    return true;
}

void UNarrativeManager::UnlockDependentEvents(const FString& CompletedEventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.Prerequisites.Contains(CompletedEventID))
        {
            UE_LOG(LogTemp, Log, TEXT("Event unlocked: %s"), *Event.EventID);
        }
    }
}