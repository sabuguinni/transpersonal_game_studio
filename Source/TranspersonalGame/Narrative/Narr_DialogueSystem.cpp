#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    bDialogueEnabled = true;
    DialogueCooldownTime = 3.0f;
    LastDialogueTime = 0.0f;
    DialogueDataTable = nullptr;
}

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized"));
    
    // Load default dialogues
    LoadDefaultDialogues();
}

void UNarr_DialogueSystem::Deinitialize()
{
    DialogueEntries.Empty();
    PlayedDialogueIDs.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueSystem::TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FString& ContextData)
{
    if (!bDialogueEnabled)
    {
        return;
    }

    // Get current time
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    float CurrentTime = World->GetTimeSeconds();
    
    // Check cooldown
    if (CurrentTime - LastDialogueTime < DialogueCooldownTime)
    {
        return;
    }

    // Find matching dialogues
    TArray<FNarr_DialogueEntry> MatchingDialogues = GetDialoguesForTrigger(TriggerType);
    
    if (MatchingDialogues.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogues found for trigger type"));
        return;
    }

    // Filter by playability
    TArray<FNarr_DialogueEntry> PlayableDialogues;
    for (const FNarr_DialogueEntry& Entry : MatchingDialogues)
    {
        if (CanPlayDialogue(Entry))
        {
            PlayableDialogues.Add(Entry);
        }
    }

    if (PlayableDialogues.Num() == 0)
    {
        return;
    }

    // Sort by priority (highest first)
    PlayableDialogues.Sort([](const FNarr_DialogueEntry& A, const FNarr_DialogueEntry& B)
    {
        return A.Priority > B.Priority;
    });

    // Play the highest priority dialogue
    PlayDialogueEntry(PlayableDialogues[0]);
    LastDialogueTime = CurrentTime;
}

void UNarr_DialogueSystem::PlayDialogueEntry(const FNarr_DialogueEntry& DialogueEntry)
{
    if (!bDialogueEnabled)
    {
        return;
    }

    // Log the dialogue
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: %s - %s"), 
           *DialogueEntry.DialogueID, 
           *DialogueEntry.DialogueText.ToString());

    // Mark as played if not repeatable
    if (!DialogueEntry.bIsRepeatable)
    {
        MarkDialogueAsPlayed(DialogueEntry.DialogueID);
    }

    // TODO: Integrate with audio system to play actual voice lines
    // For now, just display the text
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), 
                                            *UEnum::GetValueAsString(DialogueEntry.SpeakerType),
                                            *DialogueEntry.DialogueText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
    }
}

void UNarr_DialogueSystem::RegisterDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    // Check if dialogue with this ID already exists
    for (int32 i = 0; i < DialogueEntries.Num(); i++)
    {
        if (DialogueEntries[i].DialogueID == NewEntry.DialogueID)
        {
            // Update existing entry
            DialogueEntries[i] = NewEntry;
            UE_LOG(LogTemp, Log, TEXT("Updated dialogue entry: %s"), *NewEntry.DialogueID);
            return;
        }
    }

    // Add new entry
    DialogueEntries.Add(NewEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered new dialogue entry: %s"), *NewEntry.DialogueID);
}

TArray<FNarr_DialogueEntry> UNarr_DialogueSystem::GetDialoguesForTrigger(ENarr_DialogueTrigger TriggerType)
{
    TArray<FNarr_DialogueEntry> MatchingDialogues;
    
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.TriggerCondition == TriggerType)
        {
            MatchingDialogues.Add(Entry);
        }
    }
    
    return MatchingDialogues;
}

void UNarr_DialogueSystem::SetDialogueEnabled(bool bEnabled)
{
    bDialogueEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Dialogue system %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UNarr_DialogueSystem::LoadDefaultDialogues()
{
    // Create default dialogue entries based on the voice lines generated
    FNarr_DialogueEntry NarratorEntry;
    NarratorEntry.DialogueID = "narrator_ancient_hunt";
    NarratorEntry.SpeakerType = ENarr_DialogueType::Narrator;
    NarratorEntry.TriggerCondition = ENarr_DialogueTrigger::OnQuestStart;
    NarratorEntry.DialogueText = FText::FromString("The ancient ones spoke of the Great Hunt - when the earth trembled beneath the Thunder Lizard's feet and the sky darkened with their breath.");
    NarratorEntry.AudioAssetPath = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778842332378_AncientNarrator.mp3";
    NarratorEntry.Priority = 5.0f;
    NarratorEntry.bIsRepeatable = false;
    RegisterDialogueEntry(NarratorEntry);

    FNarr_DialogueEntry WarningEntry;
    WarningEntry.DialogueID = "scout_trex_warning";
    WarningEntry.SpeakerType = ENarr_DialogueType::Warning;
    WarningEntry.TriggerCondition = ENarr_DialogueTrigger::OnDinosaurSighted;
    WarningEntry.DialogueText = FText::FromString("Warning! Territorial predator ahead. That roar means a Tyrannosaurus has claimed this valley as its hunting ground.");
    WarningEntry.AudioAssetPath = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778842338450_ScoutWarning.mp3";
    WarningEntry.Priority = 8.0f;
    WarningEntry.bIsRepeatable = true;
    RegisterDialogueEntry(WarningEntry);

    FNarr_DialogueEntry HuntEntry;
    HuntEntry.DialogueID = "leader_hunt_strategy";
    HuntEntry.SpeakerType = ENarr_DialogueType::HuntLeader;
    HuntEntry.TriggerCondition = ENarr_DialogueTrigger::OnHuntSuccess;
    HuntEntry.DialogueText = FText::FromString("The herd moves at dawn. Follow the river stones, avoid the thorned paths, and remember - when the ground shakes, it means the giants are feeding.");
    HuntEntry.AudioAssetPath = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778842343769_HuntLeader.mp3";
    HuntEntry.Priority = 6.0f;
    HuntEntry.bIsRepeatable = false;
    RegisterDialogueEntry(HuntEntry);

    FNarr_DialogueEntry WisdomEntry;
    WisdomEntry.DialogueID = "elder_survival_wisdom";
    WisdomEntry.SpeakerType = ENarr_DialogueType::ElderWisdom;
    WisdomEntry.TriggerCondition = ENarr_DialogueTrigger::OnSurvivalTip;
    WisdomEntry.DialogueText = FText::FromString("Fire keeps the night hunters away, but smoke draws the day stalkers. Build your shelter in the rocks, not the open plains.");
    WisdomEntry.AudioAssetPath = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778842348847_ElderWisdom.mp3";
    WisdomEntry.Priority = 4.0f;
    WisdomEntry.bIsRepeatable = true;
    RegisterDialogueEntry(WisdomEntry);

    UE_LOG(LogTemp, Log, TEXT("Loaded %d default dialogue entries"), DialogueEntries.Num());
}

bool UNarr_DialogueSystem::CanPlayDialogue(const FNarr_DialogueEntry& Entry)
{
    // Check if dialogue is repeatable or hasn't been played yet
    if (Entry.bIsRepeatable)
    {
        return true;
    }

    return !PlayedDialogueIDs.Contains(Entry.DialogueID);
}

void UNarr_DialogueSystem::MarkDialogueAsPlayed(const FString& DialogueID)
{
    if (!PlayedDialogueIDs.Contains(DialogueID))
    {
        PlayedDialogueIDs.Add(DialogueID);
        UE_LOG(LogTemp, Log, TEXT("Marked dialogue as played: %s"), *DialogueID);
    }
}