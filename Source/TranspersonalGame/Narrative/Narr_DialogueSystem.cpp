#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    // Constructor
}

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue System Initialized"));
    
    // Initialize default dialogues and events
    InitializeDefaultDialogues();
    InitializeNarrativeEvents();
}

void UNarr_DialogueSystem::Deinitialize()
{
    DialogueDatabase.Empty();
    NarrativeEvents.Empty();
    UnlockedLoreEntries.Empty();
    PlayedDialogues.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueSystem::PlayDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (!CanPlayDialogue(DialogueID))
    {
        return;
    }

    const FNarr_DialogueEntry* Entry = DialogueDatabase.Find(DialogueID);
    if (!Entry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue entry not found: %s"), *DialogueID);
        return;
    }

    // Log dialogue playback
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: %s - %s"), *DialogueID, *Entry->DialogueText.ToString());

    // Add to played dialogues if not repeatable
    if (!Entry->bIsRepeatable && !PlayedDialogues.Contains(DialogueID))
    {
        PlayedDialogues.Add(DialogueID);
    }

    // TODO: Integrate with audio system when available
    // For now, just log the dialogue
    if (GEngine)
    {
        FString SpeakerName = UEnum::GetValueAsString(Entry->Speaker);
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), *SpeakerName, *Entry->DialogueText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, Entry->Duration, FColor::Yellow, DisplayText);
    }
}

void UNarr_DialogueSystem::RegisterDialogueEntry(const FNarr_DialogueEntry& Entry)
{
    DialogueDatabase.Add(Entry.DialogueID, Entry);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue entry: %s"), *Entry.DialogueID);
}

bool UNarr_DialogueSystem::CanPlayDialogue(const FString& DialogueID) const
{
    const FNarr_DialogueEntry* Entry = DialogueDatabase.Find(DialogueID);
    if (!Entry)
    {
        return false;
    }

    // Check if already played and not repeatable
    if (!Entry->bIsRepeatable && PlayedDialogues.Contains(DialogueID))
    {
        return false;
    }

    // Check conditions
    return CheckDialogueConditions(*Entry);
}

void UNarr_DialogueSystem::TriggerNarrativeEvent(const FString& EventID, const FVector& PlayerLocation)
{
    FNarr_NarrativeEvent* Event = NarrativeEvents.Find(EventID);
    if (!Event)
    {
        return;
    }

    if (Event->bHasBeenTriggered)
    {
        return;
    }

    // Check if player is within trigger radius
    float Distance = FVector::Dist(PlayerLocation, Event->TriggerLocation);
    if (Distance > Event->TriggerRadius)
    {
        return;
    }

    // Trigger the event
    Event->bHasBeenTriggered = true;
    UE_LOG(LogTemp, Log, TEXT("Triggered narrative event: %s"), *EventID);

    // Play dialogue sequence
    for (const FNarr_DialogueEntry& DialogueEntry : Event->DialogueSequence)
    {
        RegisterDialogueEntry(DialogueEntry);
        PlayDialogue(DialogueEntry.DialogueID);
    }
}

void UNarr_DialogueSystem::RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event)
{
    NarrativeEvents.Add(Event.EventID, Event);
    UE_LOG(LogTemp, Log, TEXT("Registered narrative event: %s"), *Event.EventID);
}

void UNarr_DialogueSystem::TriggerSurvivalTip(const FString& TipCategory)
{
    FString TipDialogueID = FString::Printf(TEXT("SurvivalTip_%s"), *TipCategory);
    PlayDialogue(TipDialogueID);
}

void UNarr_DialogueSystem::TriggerDangerWarning(const FString& ThreatType, const FVector& ThreatLocation)
{
    FString WarningDialogueID = FString::Printf(TEXT("DangerWarning_%s"), *ThreatType);
    PlayDialogue(WarningDialogueID);
}

void UNarr_DialogueSystem::UnlockLoreEntry(const FString& LoreID)
{
    if (!UnlockedLoreEntries.Contains(LoreID))
    {
        UnlockedLoreEntries.Add(LoreID);
        UE_LOG(LogTemp, Log, TEXT("Unlocked lore entry: %s"), *LoreID);
        
        // Play associated lore dialogue
        FString LoreDialogueID = FString::Printf(TEXT("Lore_%s"), *LoreID);
        PlayDialogue(LoreDialogueID);
    }
}

TArray<FString> UNarr_DialogueSystem::GetUnlockedLore() const
{
    return UnlockedLoreEntries;
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Survival Tips
    FNarr_DialogueEntry SurvivalTip1;
    SurvivalTip1.DialogueID = TEXT("SurvivalTip_Water");
    SurvivalTip1.Speaker = ENarr_SpeakerType::Wise_Elder;
    SurvivalTip1.Type = ENarr_DialogueType::Survival_Tip;
    SurvivalTip1.DialogueText = FText::FromString(TEXT("Fresh water flows from the high rocks. Follow the sound of rushing water, but beware - predators also gather near the streams."));
    SurvivalTip1.Duration = 6.0f;
    RegisterDialogueEntry(SurvivalTip1);

    FNarr_DialogueEntry SurvivalTip2;
    SurvivalTip2.DialogueID = TEXT("SurvivalTip_Shelter");
    SurvivalTip2.Speaker = ENarr_SpeakerType::Experienced_Hunter;
    SurvivalTip2.Type = ENarr_DialogueType::Survival_Tip;
    SurvivalTip2.DialogueText = FText::FromString(TEXT("The caves offer protection from the great storms, but check for claw marks first. Many beasts claim these shelters as their own."));
    SurvivalTip2.Duration = 7.0f;
    RegisterDialogueEntry(SurvivalTip2);

    // Danger Warnings
    FNarr_DialogueEntry DangerWarning1;
    DangerWarning1.DialogueID = TEXT("DangerWarning_TRex");
    DangerWarning1.Speaker = ENarr_SpeakerType::Tribal_Warrior;
    DangerWarning1.Type = ENarr_DialogueType::Warning;
    DangerWarning1.DialogueText = FText::FromString(TEXT("The Thunder Lizard's roar echoes through the valley! Its massive jaws can crush stone. Seek high ground and remain still until it passes."));
    DangerWarning1.Duration = 8.0f;
    RegisterDialogueEntry(DangerWarning1);

    FNarr_DialogueEntry DangerWarning2;
    DangerWarning2.DialogueID = TEXT("DangerWarning_Raptors");
    DangerWarning2.Speaker = ENarr_SpeakerType::Young_Scout;
    DangerWarning2.Type = ENarr_DialogueType::Warning;
    DangerWarning2.DialogueText = FText::FromString(TEXT("Pack hunters move in the shadows! They communicate with clicks and chirps. Never turn your back on one - there are always more nearby."));
    DangerWarning2.Duration = 7.5f;
    RegisterDialogueEntry(DangerWarning2);

    // Lore Entries
    FNarr_DialogueEntry Lore1;
    Lore1.DialogueID = TEXT("Lore_GreatMigration");
    Lore1.Speaker = ENarr_SpeakerType::Ancient_Storyteller;
    Lore1.Type = ENarr_DialogueType::Lore;
    Lore1.DialogueText = FText::FromString(TEXT("In the time of the great migration, the long-necks follow paths carved by their ancestors. These ancient routes lead to the sacred feeding grounds."));
    Lore1.Duration = 9.0f;
    RegisterDialogueEntry(Lore1);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d default dialogue entries"), DialogueDatabase.Num());
}

void UNarr_DialogueSystem::InitializeNarrativeEvents()
{
    // Valley Entrance Event
    FNarr_NarrativeEvent ValleyEntrance;
    ValleyEntrance.EventID = TEXT("ValleyEntrance");
    ValleyEntrance.EventTitle = FText::FromString(TEXT("Entering the Ancient Valley"));
    ValleyEntrance.EventDescription = FText::FromString(TEXT("The player discovers the main valley area"));
    ValleyEntrance.TriggerLocation = FVector(0.0f, 0.0f, 0.0f);
    ValleyEntrance.TriggerRadius = 2000.0f;

    FNarr_DialogueEntry ValleyWelcome;
    ValleyWelcome.DialogueID = TEXT("ValleyEntrance_Welcome");
    ValleyWelcome.Speaker = ENarr_SpeakerType::Ancient_Storyteller;
    ValleyWelcome.Type = ENarr_DialogueType::Lore;
    ValleyWelcome.DialogueText = FText::FromString(TEXT("Welcome to the ancient valley, young survivor. Here, time moves differently, and the great lizards rule as they have for countless seasons."));
    ValleyWelcome.Duration = 8.0f;

    ValleyEntrance.DialogueSequence.Add(ValleyWelcome);
    RegisterNarrativeEvent(ValleyEntrance);

    // Predator Territory Event
    FNarr_NarrativeEvent PredatorTerritory;
    PredatorTerritory.EventID = TEXT("PredatorTerritory");
    PredatorTerritory.EventTitle = FText::FromString(TEXT("Entering Predator Territory"));
    PredatorTerritory.EventDescription = FText::FromString(TEXT("Warning about dangerous predator area"));
    PredatorTerritory.TriggerLocation = FVector(5000.0f, 5000.0f, 0.0f);
    PredatorTerritory.TriggerRadius = 1500.0f;

    FNarr_DialogueEntry PredatorWarning;
    PredatorWarning.DialogueID = TEXT("PredatorTerritory_Warning");
    PredatorWarning.Speaker = ENarr_SpeakerType::Experienced_Hunter;
    PredatorWarning.Type = ENarr_DialogueType::Warning;
    PredatorWarning.DialogueText = FText::FromString(TEXT("You enter the hunting grounds of the apex predators. Move carefully and watch for signs - broken branches, deep claw marks, and the silence of smaller creatures."));
    PredatorWarning.Duration = 9.0f;

    PredatorTerritory.DialogueSequence.Add(PredatorWarning);
    RegisterNarrativeEvent(PredatorTerritory);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d narrative events"), NarrativeEvents.Num());
}

bool UNarr_DialogueSystem::CheckDialogueConditions(const FNarr_DialogueEntry& Entry) const
{
    // For now, all dialogues are available
    // TODO: Implement condition checking based on game state
    return true;
}