#include "NarrativeDialogueLibrary.h"
#include "Engine/Engine.h"

TArray<FNarr_DialogueEntry> UNarrativeDialogueLibrary::DialogueDatabase;
TArray<FNarr_NarrativeEvent> UNarrativeDialogueLibrary::EventDatabase;
bool UNarrativeDialogueLibrary::bIsInitialized = false;

UNarrativeDialogueLibrary::UNarrativeDialogueLibrary()
{
    if (!bIsInitialized)
    {
        InitializeNarrativeDatabase();
    }
}

void UNarrativeDialogueLibrary::InitializeNarrativeDatabase()
{
    if (bIsInitialized)
    {
        return;
    }

    DialogueDatabase.Empty();
    EventDatabase.Empty();

    LoadExplorationDialogue();
    LoadCombatDialogue();
    LoadSurvivalDialogue();

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Narrative Database initialized with %d dialogue entries"), DialogueDatabase.Num());
}

void UNarrativeDialogueLibrary::LoadExplorationDialogue()
{
    FNarr_DialogueEntry Entry;

    // Valley exploration warning
    Entry.DialogueID = TEXT("valley_warning_001");
    Entry.SpeakerName = TEXT("Ancient Narrator");
    Entry.DialogueText = FText::FromString(TEXT("Ancient winds carry the scent of danger. The great hunters stalk these lands, their eyes ever watchful for the unwary traveler."));
    Entry.TriggerContext = ENarr_SurvivalContext::Exploration;
    Entry.Priority = 8.0f;
    Entry.bIsRepeatable = false;
    DialogueDatabase.Add(Entry);

    // Forest entry
    Entry.DialogueID = TEXT("forest_entry_001");
    Entry.SpeakerName = TEXT("Forest Guide");
    Entry.DialogueText = FText::FromString(TEXT("The thick canopy blocks the sun, but also hides the predators. Move carefully through the undergrowth."));
    Entry.TriggerContext = ENarr_SurvivalContext::Exploration;
    Entry.Priority = 6.0f;
    Entry.bIsRepeatable = true;
    DialogueDatabase.Add(Entry);

    // Desert warning
    Entry.DialogueID = TEXT("desert_warning_001");
    Entry.SpeakerName = TEXT("Desert Wanderer");
    Entry.DialogueText = FText::FromString(TEXT("The burning sands hide bone-crushing jaws beneath. Watch for movement in the dunes."));
    Entry.TriggerContext = ENarr_SurvivalContext::Exploration;
    Entry.Priority = 7.0f;
    Entry.bIsRepeatable = false;
    DialogueDatabase.Add(Entry);
}

void UNarrativeDialogueLibrary::LoadCombatDialogue()
{
    FNarr_DialogueEntry Entry;

    // Raptor pack warning
    Entry.DialogueID = TEXT("raptor_pack_warning");
    Entry.SpeakerName = TEXT("Survival Mentor");
    Entry.DialogueText = FText::FromString(TEXT("Stay low, move quiet. The pack hunters work together - when you see one raptor, two more circle behind you."));
    Entry.TriggerContext = ENarr_SurvivalContext::Combat;
    Entry.Priority = 9.0f;
    Entry.bIsRepeatable = true;
    DialogueDatabase.Add(Entry);

    // T-Rex encounter
    Entry.DialogueID = TEXT("trex_encounter_001");
    Entry.SpeakerName = TEXT("Battle Veteran");
    Entry.DialogueText = FText::FromString(TEXT("The earth shakes with each step. Do not run - it sees movement. Stand still, breathe shallow, pray to the ancestors."));
    Entry.TriggerContext = ENarr_SurvivalContext::Combat;
    Entry.Priority = 10.0f;
    Entry.bIsRepeatable = false;
    DialogueDatabase.Add(Entry);

    // Combat victory
    Entry.DialogueID = TEXT("combat_victory_001");
    Entry.SpeakerName = TEXT("Tribal Warrior");
    Entry.DialogueText = FText::FromString(TEXT("You have proven your strength. The beast falls, but its pack remembers. Be ready."));
    Entry.TriggerContext = ENarr_SurvivalContext::Combat;
    Entry.Priority = 5.0f;
    Entry.bIsRepeatable = true;
    DialogueDatabase.Add(Entry);
}

void UNarrativeDialogueLibrary::LoadSurvivalDialogue()
{
    FNarr_DialogueEntry Entry;

    // Hunger warning
    Entry.DialogueID = TEXT("hunger_warning_001");
    Entry.SpeakerName = TEXT("Survival Guide");
    Entry.DialogueText = FText::FromString(TEXT("Your stomach gnaws at your ribs. Find berries, hunt small game, or risk becoming prey yourself."));
    Entry.TriggerContext = ENarr_SurvivalContext::Survival;
    Entry.Priority = 8.0f;
    Entry.bIsRepeatable = true;
    DialogueDatabase.Add(Entry);

    // Thirst critical
    Entry.DialogueID = TEXT("thirst_critical_001");
    Entry.SpeakerName = TEXT("Desert Survivor");
    Entry.DialogueText = FText::FromString(TEXT("Your lips crack like dried earth. Water flows near the great stones - but beware what drinks there."));
    Entry.TriggerContext = ENarr_SurvivalContext::Survival;
    Entry.Priority = 9.0f;
    Entry.bIsRepeatable = false;
    DialogueDatabase.Add(Entry);

    // Shelter needed
    Entry.DialogueID = TEXT("shelter_needed_001");
    Entry.SpeakerName = TEXT("Cave Dweller");
    Entry.DialogueText = FText::FromString(TEXT("Night brings the cold and the hunters. Find shelter among the rocks, away from the paths of giants."));
    Entry.TriggerContext = ENarr_SurvivalContext::Survival;
    Entry.Priority = 7.0f;
    Entry.bIsRepeatable = true;
    DialogueDatabase.Add(Entry);
}

FNarr_DialogueEntry UNarrativeDialogueLibrary::GetDialogueByContext(ENarr_SurvivalContext Context, float PlayerFear)
{
    if (!bIsInitialized)
    {
        InitializeNarrativeDatabase();
    }

    TArray<FNarr_DialogueEntry> ContextDialogue;
    for (const FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.TriggerContext == Context)
        {
            // Adjust priority based on player fear level
            float AdjustedPriority = Entry.Priority;
            if (PlayerFear > 0.7f && Context == ENarr_SurvivalContext::Combat)
            {
                AdjustedPriority += 2.0f; // Boost combat dialogue when player is afraid
            }
            
            ContextDialogue.Add(Entry);
        }
    }

    if (ContextDialogue.Num() > 0)
    {
        // Sort by priority and return highest
        ContextDialogue.Sort([](const FNarr_DialogueEntry& A, const FNarr_DialogueEntry& B) {
            return A.Priority > B.Priority;
        });
        
        return ContextDialogue[0];
    }

    // Return empty entry if nothing found
    return FNarr_DialogueEntry();
}

TArray<FNarr_DialogueEntry> UNarrativeDialogueLibrary::GetAllDialogueForContext(ENarr_SurvivalContext Context)
{
    if (!bIsInitialized)
    {
        InitializeNarrativeDatabase();
    }

    TArray<FNarr_DialogueEntry> ContextDialogue;
    for (const FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.TriggerContext == Context)
        {
            ContextDialogue.Add(Entry);
        }
    }

    return ContextDialogue;
}

void UNarrativeDialogueLibrary::TriggerNarrativeEvent(const FString& EventID)
{
    for (FNarr_NarrativeEvent& Event : EventDatabase)
    {
        if (Event.EventID == EventID)
        {
            Event.bIsTriggered = true;
            UE_LOG(LogTemp, Warning, TEXT("Narrative Event Triggered: %s"), *EventID);
            break;
        }
    }
}

bool UNarrativeDialogueLibrary::IsEventTriggered(const FString& EventID)
{
    for (const FNarr_NarrativeEvent& Event : EventDatabase)
    {
        if (Event.EventID == EventID)
        {
            return Event.bIsTriggered;
        }
    }
    return false;
}