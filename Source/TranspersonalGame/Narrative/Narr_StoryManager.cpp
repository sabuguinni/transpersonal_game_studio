#include "Narr_StoryManager.h"
#include "Engine/Engine.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    CurrentStoryPhase = ENarr_StoryPhase::Awakening;
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeStoryEvents();
    InitializeCharacterProfiles();
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Story Manager initialized"));
}

void UNarr_StoryManager::AdvanceStoryPhase()
{
    int32 CurrentPhaseValue = static_cast<int32>(CurrentStoryPhase);
    int32 MaxPhaseValue = static_cast<int32>(ENarr_StoryPhase::Legacy);
    
    if (CurrentPhaseValue < MaxPhaseValue)
    {
        CurrentStoryPhase = static_cast<ENarr_StoryPhase>(CurrentPhaseValue + 1);
        UE_LOG(LogTemp, Warning, TEXT("Story phase advanced to: %d"), CurrentPhaseValue + 1);
    }
}

void UNarr_StoryManager::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            if (CheckEventPrerequisites(Event))
            {
                Event.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Story event triggered: %s"), *EventID);
                
                // Trigger narrative context update
                FString ContextUpdate = FString::Printf(TEXT("Event completed: %s - %s"), 
                    *Event.EventTitle, *Event.EventDescription);
                NarrativeContexts.Add(ContextUpdate);
            }
            break;
        }
    }
}

bool UNarr_StoryManager::IsStoryEventCompleted(const FString& EventID) const
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

void UNarr_StoryManager::RegisterCharacter(const FNarr_CharacterProfile& Character)
{
    // Check if character already exists
    for (FNarr_CharacterProfile& ExistingCharacter : CharacterProfiles)
    {
        if (ExistingCharacter.CharacterName == Character.CharacterName)
        {
            ExistingCharacter = Character;
            return;
        }
    }
    
    // Add new character
    CharacterProfiles.Add(Character);
    UE_LOG(LogTemp, Warning, TEXT("Character registered: %s"), *Character.CharacterName);
}

FNarr_CharacterProfile UNarr_StoryManager::GetCharacterProfile(const FString& CharacterName) const
{
    for (const FNarr_CharacterProfile& Character : CharacterProfiles)
    {
        if (Character.CharacterName == CharacterName)
        {
            return Character;
        }
    }
    return FNarr_CharacterProfile();
}

void UNarr_StoryManager::UpdateCharacterTrust(const FString& CharacterName, float TrustDelta)
{
    for (FNarr_CharacterProfile& Character : CharacterProfiles)
    {
        if (Character.CharacterName == CharacterName)
        {
            Character.TrustLevel = FMath::Clamp(Character.TrustLevel + TrustDelta, 0.0f, 100.0f);
            UE_LOG(LogTemp, Warning, TEXT("Character trust updated: %s = %f"), 
                *CharacterName, Character.TrustLevel);
            break;
        }
    }
}

FString UNarr_StoryManager::GetCurrentNarrativeContext() const
{
    if (NarrativeContexts.Num() > 0)
    {
        return NarrativeContexts.Last();
    }
    
    switch (CurrentStoryPhase)
    {
        case ENarr_StoryPhase::Awakening:
            return TEXT("You wake alone in a hostile prehistoric world. Survival is your only priority.");
        case ENarr_StoryPhase::FirstContact:
            return TEXT("Other survivors emerge from the wilderness. Trust must be earned carefully.");
        case ENarr_StoryPhase::TribalGrowth:
            return TEXT("Your growing tribe faces new challenges as territory expands.");
        case ENarr_StoryPhase::GreatHunt:
            return TEXT("Ancient predators threaten everything you've built. The great hunt begins.");
        case ENarr_StoryPhase::Legacy:
            return TEXT("Your legacy shapes the future of human survival in this dangerous world.");
        default:
            return TEXT("The story unfolds...");
    }
}

TArray<FString> UNarr_StoryManager::GetAvailableDialogueOptions(const FString& CharacterName) const
{
    FNarr_CharacterProfile Character = GetCharacterProfile(CharacterName);
    TArray<FString> AvailableOptions;
    
    if (Character.bIsAlive && Character.DialogueLines.Num() > 0)
    {
        // Filter dialogue based on trust level and story phase
        for (const FString& DialogueLine : Character.DialogueLines)
        {
            if (Character.TrustLevel >= 25.0f || CurrentStoryPhase == ENarr_StoryPhase::Awakening)
            {
                AvailableOptions.Add(DialogueLine);
            }
        }
    }
    
    return AvailableOptions;
}

void UNarr_StoryManager::InitializeStoryEvents()
{
    StoryEvents.Empty();
    
    // Awakening phase events
    FNarr_StoryEvent FirstShelter;
    FirstShelter.EventID = TEXT("first_shelter");
    FirstShelter.EventTitle = TEXT("First Shelter");
    FirstShelter.EventDescription = TEXT("Build your first shelter to survive the night");
    FirstShelter.RequiredPhase = ENarr_StoryPhase::Awakening;
    FirstShelter.ExperienceReward = 100;
    StoryEvents.Add(FirstShelter);
    
    FNarr_StoryEvent FirstHunt;
    FirstHunt.EventID = TEXT("first_hunt");
    FirstHunt.EventTitle = TEXT("First Hunt");
    FirstHunt.EventDescription = TEXT("Successfully hunt your first prey for food");
    FirstHunt.RequiredPhase = ENarr_StoryPhase::Awakening;
    FirstHunt.ExperienceReward = 150;
    StoryEvents.Add(FirstHunt);
    
    // FirstContact phase events
    FNarr_StoryEvent MeetSurvivor;
    MeetSurvivor.EventID = TEXT("meet_survivor");
    MeetSurvivor.EventTitle = TEXT("First Contact");
    MeetSurvivor.EventDescription = TEXT("Encounter another survivor in the wilderness");
    MeetSurvivor.RequiredPhase = ENarr_StoryPhase::FirstContact;
    MeetSurvivor.Prerequisites.Add(TEXT("first_shelter"));
    MeetSurvivor.ExperienceReward = 200;
    StoryEvents.Add(MeetSurvivor);
    
    // TribalGrowth phase events
    FNarr_StoryEvent FormTribe;
    FormTribe.EventID = TEXT("form_tribe");
    FormTribe.EventTitle = TEXT("Tribal Formation");
    FormTribe.EventDescription = TEXT("Establish a permanent tribal settlement");
    FormTribe.RequiredPhase = ENarr_StoryPhase::TribalGrowth;
    FormTribe.Prerequisites.Add(TEXT("meet_survivor"));
    FormTribe.ExperienceReward = 300;
    StoryEvents.Add(FormTribe);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d story events"), StoryEvents.Num());
}

void UNarr_StoryManager::InitializeCharacterProfiles()
{
    CharacterProfiles.Empty();
    
    // Tribal Elder
    FNarr_CharacterProfile Elder;
    Elder.CharacterName = TEXT("Kael the Elder");
    Elder.CharacterRole = TEXT("Tribal Leader");
    Elder.BackgroundStory = TEXT("A weathered survivor who has seen many seasons. His knowledge of the land and beasts is unmatched.");
    Elder.DialogueLines.Add(TEXT("The great beasts follow ancient paths. Learn them, and you will survive."));
    Elder.DialogueLines.Add(TEXT("Fire keeps the darkness at bay, but wisdom keeps death from your door."));
    Elder.DialogueLines.Add(TEXT("Trust is earned through shared hardship, young one."));
    Elder.TrustLevel = 50.0f;
    CharacterProfiles.Add(Elder);
    
    // Scout
    FNarr_CharacterProfile Scout;
    Scout.CharacterName = TEXT("Nira the Swift");
    Scout.CharacterRole = TEXT("Scout");
    Scout.BackgroundStory = TEXT("A nimble hunter who knows every trail and hiding spot in the territory.");
    Scout.DialogueLines.Add(TEXT("I've seen tracks near the river. Large ones. We should avoid that area."));
    Scout.DialogueLines.Add(TEXT("The herds are moving south. Good hunting if we move quickly."));
    Scout.DialogueLines.Add(TEXT("Stay low, move quiet. The predators hunt by sound."));
    Scout.TrustLevel = 25.0f;
    CharacterProfiles.Add(Scout);
    
    // Crafter
    FNarr_CharacterProfile Crafter;
    Crafter.CharacterName = TEXT("Thom the Maker");
    Crafter.CharacterRole = TEXT("Tool Crafter");
    Crafter.BackgroundStory = TEXT("A skilled artisan who can shape stone and bone into tools of survival.");
    Crafter.DialogueLines.Add(TEXT("Good stone makes good tools. Bad tools make dead hunters."));
    Crafter.DialogueLines.Add(TEXT("Bring me bone from your kills. I'll make weapons that bite deep."));
    Crafter.DialogueLines.Add(TEXT("Every tool has a purpose. Learn them all, or learn to die."));
    Crafter.TrustLevel = 30.0f;
    CharacterProfiles.Add(Crafter);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d character profiles"), CharacterProfiles.Num());
}

bool UNarr_StoryManager::CheckEventPrerequisites(const FNarr_StoryEvent& Event) const
{
    // Check story phase requirement
    if (Event.RequiredPhase != CurrentStoryPhase)
    {
        return false;
    }
    
    // Check all prerequisites are completed
    for (const FString& Prerequisite : Event.Prerequisites)
    {
        if (!IsStoryEventCompleted(Prerequisite))
        {
            return false;
        }
    }
    
    return true;
}