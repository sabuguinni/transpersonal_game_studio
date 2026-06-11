#include "Narr_StoryManager.h"
#include "Narr_CharacterDialogue.h"
#include "Engine/Engine.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    CurrentStoryPhase = 0;
    SurvivalDaysElapsed = 0.0f;
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeStoryEvents();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_StoryManager initialized"));
}

void UNarr_StoryManager::Deinitialize()
{
    CharacterDialogues.Empty();
    StoryEvents.Empty();
    CompletedEvents.Empty();
    
    Super::Deinitialize();
}

void UNarr_StoryManager::InitializeStoryEvents()
{
    // Initialize core survival story events
    FNarr_StoryEvent FirstContact;
    FirstContact.EventID = TEXT("first_contact");
    FirstContact.EventTitle = FText::FromString(TEXT("First Contact"));
    FirstContact.EventDescription = FText::FromString(TEXT("Meet another survivor in the wilderness"));
    FirstContact.TriggerContext = ESurvivalContext::Normal;
    StoryEvents.Add(FirstContact);

    FNarr_StoryEvent DinosaurEncounter;
    DinosaurEncounter.EventID = TEXT("dinosaur_encounter");
    DinosaurEncounter.EventTitle = FText::FromString(TEXT("Beast Encounter"));
    DinosaurEncounter.EventDescription = FText::FromString(TEXT("Survive your first dinosaur encounter"));
    DinosaurEncounter.TriggerContext = ESurvivalContext::Danger;
    StoryEvents.Add(DinosaurEncounter);

    FNarr_StoryEvent ShelterBuilt;
    ShelterBuilt.EventID = TEXT("shelter_built");
    ShelterBuilt.EventTitle = FText::FromString(TEXT("Safe Haven"));
    ShelterBuilt.EventDescription = FText::FromString(TEXT("Build your first shelter"));
    ShelterBuilt.TriggerContext = ESurvivalContext::Safe;
    StoryEvents.Add(ShelterBuilt);

    FNarr_StoryEvent TribeFormed;
    TribeFormed.EventID = TEXT("tribe_formed");
    TribeFormed.EventTitle = FText::FromString(TEXT("Strength in Numbers"));
    TribeFormed.EventDescription = FText::FromString(TEXT("Form a small tribe with other survivors"));
    TribeFormed.PrerequisiteEvents.Add(TEXT("first_contact"));
    TribeFormed.PrerequisiteEvents.Add(TEXT("shelter_built"));
    TribeFormed.TriggerContext = ESurvivalContext::Safe;
    StoryEvents.Add(TribeFormed);
}

void UNarr_StoryManager::TriggerStoryEvent(const FString& EventID)
{
    if (CompletedEvents.Contains(EventID))
    {
        return; // Event already completed
    }

    // Find the event
    FNarr_StoryEvent* Event = StoryEvents.FindByPredicate([&EventID](const FNarr_StoryEvent& E) {
        return E.EventID == EventID;
    });

    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("Story event not found: %s"), *EventID);
        return;
    }

    // Check prerequisites
    for (const FString& PrereqID : Event->PrerequisiteEvents)
    {
        if (!CompletedEvents.Contains(PrereqID))
        {
            UE_LOG(LogTemp, Warning, TEXT("Prerequisites not met for event: %s"), *EventID);
            return;
        }
    }

    // Mark event as completed
    Event->bIsCompleted = true;
    CompletedEvents.Add(EventID);

    UE_LOG(LogTemp, Log, TEXT("Story event triggered: %s"), *EventID);

    // Check if this should advance the story phase
    CheckEventTriggers();
}

bool UNarr_StoryManager::IsEventCompleted(const FString& EventID) const
{
    return CompletedEvents.Contains(EventID);
}

TArray<FNarr_StoryEvent> UNarr_StoryManager::GetAvailableEvents() const
{
    TArray<FNarr_StoryEvent> AvailableEvents;

    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.bIsCompleted)
        {
            continue;
        }

        // Check prerequisites
        bool bPrerequisitesMet = true;
        for (const FString& PrereqID : Event.PrerequisiteEvents)
        {
            if (!CompletedEvents.Contains(PrereqID))
            {
                bPrerequisitesMet = false;
                break;
            }
        }

        if (bPrerequisitesMet)
        {
            AvailableEvents.Add(Event);
        }
    }

    return AvailableEvents;
}

UNarr_CharacterDialogue* UNarr_StoryManager::GetCharacterDialogue(const FString& CharacterName) const
{
    if (UNarr_CharacterDialogue* const* FoundDialogue = CharacterDialogues.Find(CharacterName))
    {
        return *FoundDialogue;
    }
    return nullptr;
}

void UNarr_StoryManager::RegisterCharacterDialogue(const FString& CharacterName, UNarr_CharacterDialogue* Dialogue)
{
    if (Dialogue)
    {
        CharacterDialogues.Add(CharacterName, Dialogue);
        UE_LOG(LogTemp, Log, TEXT("Registered dialogue for character: %s"), *CharacterName);
    }
}

void UNarr_StoryManager::AdvanceStoryPhase()
{
    CurrentStoryPhase++;
    UE_LOG(LogTemp, Log, TEXT("Story phase advanced to: %d"), CurrentStoryPhase);
}

void UNarr_StoryManager::UpdateSurvivalTime(float DeltaTime)
{
    SurvivalDaysElapsed += DeltaTime / 86400.0f; // Convert seconds to days
}

FText UNarr_StoryManager::GetCurrentStoryContext() const
{
    if (CurrentStoryPhase == 0)
    {
        return FText::FromString(TEXT("Lost and alone in a hostile prehistoric world."));
    }
    else if (CurrentStoryPhase == 1)
    {
        return FText::FromString(TEXT("Learning to survive among the ancient beasts."));
    }
    else if (CurrentStoryPhase == 2)
    {
        return FText::FromString(TEXT("Building alliances with fellow survivors."));
    }
    else
    {
        return FText::FromString(TEXT("Leading a tribe in the dangerous prehistoric wilderness."));
    }
}

void UNarr_StoryManager::CheckEventTriggers()
{
    // Auto-advance story phases based on completed events
    if (IsEventCompleted(TEXT("first_contact")) && IsEventCompleted(TEXT("shelter_built")) && CurrentStoryPhase < 2)
    {
        AdvanceStoryPhase();
    }
    
    if (IsEventCompleted(TEXT("tribe_formed")) && CurrentStoryPhase < 3)
    {
        AdvanceStoryPhase();
    }
}