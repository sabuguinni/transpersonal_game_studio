#include "NarrativeDialogueManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UNarrativeDialogueManager::UNarrativeDialogueManager()
{
    CurrentContext = ENarr_NarrativeContext::Exploration;
    LastNarrativeTime = 0.0f;
    NarrativeCooldown = 5.0f;
}

void UNarrativeDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Initializing narrative system"));
    
    LoadDefaultDialogues();
    LoadDefaultNarrativeEvents();
    
    // Initialize story flags
    StoryFlags.Add("FirstPlay", true);
    StoryFlags.Add("TutorialComplete", false);
    StoryFlags.Add("FirstDinosaurSeen", false);
    StoryFlags.Add("FirstCombat", false);
    StoryFlags.Add("SavanaExplored", false);
    StoryFlags.Add("ForestExplored", false);
}

void UNarrativeDialogueManager::Deinitialize()
{
    DialogueDatabase.Empty();
    NarrativeEvents.Empty();
    StoryFlags.Empty();
    
    Super::Deinitialize();
}

void UNarrativeDialogueManager::PlayDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (!CanPlayNarrative())
    {
        return;
    }

    const FNarr_DialogueEntry* Entry = DialogueDatabase.Find(DialogueID);
    if (!Entry)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Dialogue not found: %s"), *DialogueID);
        return;
    }

    // Check required flags
    for (const FString& Flag : Entry->RequiredFlags)
    {
        if (!GetStoryFlag(Flag))
        {
            UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Dialogue %s blocked by flag: %s"), *DialogueID, *Flag);
            return;
        }
    }

    // Display dialogue text
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *Entry->SpeakerName, *Entry->DialogueText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, Entry->Duration, FColor::Yellow, DisplayText);
    }

    // Play audio if available
    if (!Entry->AudioPath.IsEmpty())
    {
        PlayAudioForDialogue(Entry->AudioPath);
    }

    // Set flags
    for (const FString& Flag : Entry->SetFlags)
    {
        SetStoryFlag(Flag, true);
    }

    LastNarrativeTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Played dialogue: %s"), *DialogueID);
}

void UNarrativeDialogueManager::RegisterDialogueEntry(const FNarr_DialogueEntry& Entry)
{
    DialogueDatabase.Add(Entry.DialogueID, Entry);
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Registered dialogue: %s"), *Entry.DialogueID);
}

bool UNarrativeDialogueManager::HasDialogue(const FString& DialogueID) const
{
    return DialogueDatabase.Contains(DialogueID);
}

void UNarrativeDialogueManager::TriggerNarrativeEvent(const FString& EventID, ENarr_NarrativeContext Context)
{
    if (!CanPlayNarrative())
    {
        return;
    }

    const FNarr_NarrativeEvent* Event = NarrativeEvents.Find(EventID);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Narrative event not found: %s"), *EventID);
        return;
    }

    // Check context relevance
    if (Event->Context != ENarr_NarrativeContext::Any && Event->Context != Context)
    {
        return;
    }

    // Display narrative text
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan, Event->EventText.ToString());
    }

    LastNarrativeTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Triggered narrative event: %s"), *EventID);
}

void UNarrativeDialogueManager::RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event)
{
    NarrativeEvents.Add(Event.EventID, Event);
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Registered narrative event: %s"), *Event.EventID);
}

void UNarrativeDialogueManager::SetStoryFlag(const FString& FlagName, bool bValue)
{
    StoryFlags.Add(FlagName, bValue);
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Set story flag %s = %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool UNarrativeDialogueManager::GetStoryFlag(const FString& FlagName) const
{
    const bool* Flag = StoryFlags.Find(FlagName);
    return Flag ? *Flag : false;
}

void UNarrativeDialogueManager::ClearStoryFlag(const FString& FlagName)
{
    StoryFlags.Remove(FlagName);
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Cleared story flag: %s"), *FlagName);
}

void UNarrativeDialogueManager::UpdateNarrativeContext(ENarr_NarrativeContext NewContext)
{
    if (CurrentContext != NewContext)
    {
        CurrentContext = NewContext;
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Context changed to: %d"), (int32)NewContext);
    }
}

void UNarrativeDialogueManager::OnPlayerHealthCritical()
{
    TriggerNarrativeEvent("HealthCritical", ENarr_NarrativeContext::Combat);
}

void UNarrativeDialogueManager::OnDinosaurEncounter(const FString& DinosaurType, float Distance)
{
    if (!GetStoryFlag("FirstDinosaurSeen"))
    {
        SetStoryFlag("FirstDinosaurSeen", true);
        TriggerNarrativeEvent("FirstDinosaurSighting", ENarr_NarrativeContext::Exploration);
    }
    
    if (Distance < 500.0f)
    {
        FString EventID = FString::Printf(TEXT("DinosaurClose_%s"), *DinosaurType);
        TriggerNarrativeEvent(EventID, ENarr_NarrativeContext::Combat);
    }
}

void UNarrativeDialogueManager::OnBiomeEntered(const FString& BiomeName)
{
    FString FlagName = FString::Printf(TEXT("%sExplored"), *BiomeName);
    if (!GetStoryFlag(FlagName))
    {
        SetStoryFlag(FlagName, true);
        
        FString EventID = FString::Printf(TEXT("Enter_%s"), *BiomeName);
        TriggerNarrativeEvent(EventID, ENarr_NarrativeContext::Exploration);
    }
}

void UNarrativeDialogueManager::LoadDefaultDialogues()
{
    // Tutorial dialogues
    FNarr_DialogueEntry TutorialStart;
    TutorialStart.DialogueID = "Tutorial_Start";
    TutorialStart.SpeakerName = "Narrator";
    TutorialStart.DialogueText = FText::FromString("Welcome to the Cretaceous period. Your survival depends on understanding this ancient world.");
    TutorialStart.Duration = 4.0f;
    TutorialStart.SetFlags.Add("TutorialStarted");
    RegisterDialogueEntry(TutorialStart);

    // Combat dialogues
    FNarr_DialogueEntry CombatWarning;
    CombatWarning.DialogueID = "Combat_Warning";
    CombatWarning.SpeakerName = "Instinct";
    CombatWarning.DialogueText = FText::FromString("Danger approaches. Trust your instincts and prepare for battle.");
    CombatWarning.Duration = 3.0f;
    RegisterDialogueEntry(CombatWarning);

    // Discovery dialogues
    FNarr_DialogueEntry FirstTool;
    FirstTool.DialogueID = "Discovery_FirstTool";
    FirstTool.SpeakerName = "Narrator";
    FirstTool.DialogueText = FText::FromString("Stone and bone - the building blocks of survival. Craft wisely.");
    FirstTool.Duration = 3.5f;
    RegisterDialogueEntry(FirstTool);
}

void UNarrativeDialogueManager::LoadDefaultNarrativeEvents()
{
    // Exploration events
    FNarr_NarrativeEvent ValleyEntry;
    ValleyEntry.EventID = "Enter_Valley";
    ValleyEntry.EventText = FText::FromString("The ancient valley stretches before you, filled with both wonder and peril.");
    ValleyEntry.Context = ENarr_NarrativeContext::Exploration;
    ValleyEntry.Priority = 1.0f;
    RegisterNarrativeEvent(ValleyEntry);

    // Combat events
    FNarr_NarrativeEvent HealthCritical;
    HealthCritical.EventID = "HealthCritical";
    HealthCritical.EventText = FText::FromString("Your body weakens. Find shelter and tend to your wounds before it's too late.");
    HealthCritical.Context = ENarr_NarrativeContext::Combat;
    HealthCritical.Priority = 3.0f;
    RegisterNarrativeEvent(HealthCritical);

    // Discovery events
    FNarr_NarrativeEvent FirstDinosaur;
    FirstDinosaur.EventID = "FirstDinosaurSighting";
    FirstDinosaur.EventText = FText::FromString("A magnificent creature from Earth's distant past. Observe carefully - knowledge is survival.");
    FirstDinosaur.Context = ENarr_NarrativeContext::Exploration;
    FirstDinosaur.Priority = 2.0f;
    RegisterNarrativeEvent(FirstDinosaur);
}

bool UNarrativeDialogueManager::CanPlayNarrative() const
{
    if (!GetWorld())
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastNarrativeTime) >= NarrativeCooldown;
}

void UNarrativeDialogueManager::PlayAudioForDialogue(const FString& AudioPath)
{
    // Load and play audio - implementation depends on audio system
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Playing audio: %s"), *AudioPath);
    
    // For now, just log the audio path
    // In a full implementation, this would load and play the actual audio file
}