#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    bDialogueEnabled = true;
    bCurrentlyPlayingDialogue = false;
    CurrentDialogueEndTime = 0.0f;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDialogueLines();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue Manager initialized"));
}

void UNarr_DialogueManager::Deinitialize()
{
    RegisteredEvents.Empty();
    EventCooldowns.Empty();
    PlayedOnceEvents.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueManager::InitializeDialogueLines()
{
    // Initialize Tribal Elder lines
    FNarr_DialogueLine ElderLine1;
    ElderLine1.DialogueText = TEXT("The great beasts roam these lands with ancient purpose. Learn their patterns, young hunter, or become prey to their savage instincts.");
    ElderLine1.SpeakerType = ENarr_DialogueType::TribalElder;
    ElderLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778886916995_TribalElder.mp3");
    ElderLine1.Duration = 16.0f;
    ElderLine1.TriggerCondition = ENarr_TriggerCondition::FirstTime;
    TribalElderLines.Add(ElderLine1);

    FNarr_DialogueLine ElderLine2;
    ElderLine2.DialogueText = TEXT("Listen well, young hunter. The great predators of this land follow ancient patterns. When the Thunder Lizard roars at dawn, it marks territory.");
    ElderLine2.SpeakerType = ENarr_DialogueType::TribalElder;
    ElderLine2.Duration = 14.0f;
    ElderLine2.TriggerCondition = ENarr_TriggerCondition::PlayerEnterArea;
    TribalElderLines.Add(ElderLine2);

    // Initialize Scout Warning lines
    FNarr_DialogueLine ScoutLine1;
    ScoutLine1.DialogueText = TEXT("Danger approaches from the eastern cliffs! Three raptors, maybe four, moving in formation. Find high ground now!");
    ScoutLine1.SpeakerType = ENarr_DialogueType::ScoutWarning;
    ScoutLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778886922896_ScoutWarning.mp3");
    ScoutLine1.Duration = 13.0f;
    ScoutLine1.bIsUrgent = true;
    ScoutLine1.TriggerCondition = ENarr_TriggerCondition::DinosaurNearby;
    ScoutWarningLines.Add(ScoutLine1);

    FNarr_DialogueLine ScoutLine2;
    ScoutLine2.DialogueText = TEXT("Pack hunters spotted near the eastern cliffs! They hunt as one mind, one deadly purpose. Prepare to fight for your life!");
    ScoutLine2.SpeakerType = ENarr_DialogueType::ScoutWarning;
    ScoutLine2.Duration = 12.0f;
    ScoutLine2.bIsUrgent = true;
    ScoutLine2.TriggerCondition = ENarr_TriggerCondition::CombatStarted;
    ScoutWarningLines.Add(ScoutLine2);

    // Initialize Ancient Wisdom lines
    FNarr_DialogueLine WisdomLine1;
    WisdomLine1.DialogueText = TEXT("The stone speaks of ancient wisdom. Here, where our ancestors gathered, knowledge flows like water through rock.");
    WisdomLine1.SpeakerType = ENarr_DialogueType::AncientWisdom;
    WisdomLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778886928930_AncientWisdom.mp3");
    WisdomLine1.Duration = 15.0f;
    WisdomLine1.TriggerCondition = ENarr_TriggerCondition::PlayerEnterArea;
    AncientWisdomLines.Add(WisdomLine1);

    // Initialize Combat Narrator lines
    FNarr_DialogueLine CombatLine1;
    CombatLine1.DialogueText = TEXT("Blood on the wind. The hunt begins now. When the earth trembles and shadows move like death itself, only the prepared will see another dawn.");
    CombatLine1.SpeakerType = ENarr_DialogueType::CombatNarrator;
    CombatLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778886934662_CombatNarrator.mp3");
    CombatLine1.Duration = 13.0f;
    CombatLine1.bIsUrgent = true;
    CombatLine1.TriggerCondition = ENarr_TriggerCondition::CombatStarted;
    CombatNarratorLines.Add(CombatLine1);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d dialogue lines across 4 speaker types"), 
        TribalElderLines.Num() + ScoutWarningLines.Num() + AncientWisdomLines.Num() + CombatNarratorLines.Num());
}

void UNarr_DialogueManager::PlayDialogue(const FNarr_DialogueLine& DialogueLine)
{
    if (!bDialogueEnabled || bCurrentlyPlayingDialogue)
    {
        return;
    }

    bCurrentlyPlayingDialogue = true;
    CurrentDialogueEndTime = GetWorld()->GetTimeSeconds() + DialogueLine.Duration;

    // Display dialogue text (in a real implementation, this would trigger UI)
    FString SpeakerName = TEXT("Unknown");
    switch (DialogueLine.SpeakerType)
    {
        case ENarr_DialogueType::TribalElder:
            SpeakerName = TEXT("Tribal Elder");
            break;
        case ENarr_DialogueType::ScoutWarning:
            SpeakerName = TEXT("Scout");
            break;
        case ENarr_DialogueType::AncientWisdom:
            SpeakerName = TEXT("Ancient Voice");
            break;
        case ENarr_DialogueType::CombatNarrator:
            SpeakerName = TEXT("Narrator");
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("[%s]: %s"), *SpeakerName, *DialogueLine.DialogueText);

    // Set timer to clear dialogue state
    FTimerHandle DialogueTimer;
    GetWorld()->GetTimerManager().SetTimer(DialogueTimer, [this]()
    {
        bCurrentlyPlayingDialogue = false;
        CurrentDialogueEndTime = 0.0f;
    }, DialogueLine.Duration, false);
}

void UNarr_DialogueManager::TriggerNarrativeEvent(const FString& EventName, ENarr_TriggerCondition Condition)
{
    if (!CanPlayEvent(EventName))
    {
        return;
    }

    FNarr_NarrativeEvent* Event = RegisteredEvents.Find(EventName);
    if (!Event)
    {
        return;
    }

    // Find dialogue lines matching the condition
    TArray<FNarr_DialogueLine> ValidLines;
    for (const FNarr_DialogueLine& Line : Event->DialogueLines)
    {
        if (Line.TriggerCondition == Condition || Line.TriggerCondition == ENarr_TriggerCondition::None)
        {
            ValidLines.Add(Line);
        }
    }

    if (ValidLines.Num() > 0)
    {
        // Play random valid line
        int32 RandomIndex = FMath::RandRange(0, ValidLines.Num() - 1);
        PlayDialogue(ValidLines[RandomIndex]);

        // Mark as played and set cooldown
        if (Event->bPlayOnce)
        {
            MarkEventPlayed(EventName);
        }
        EventCooldowns.Add(EventName, GetWorld()->GetTimeSeconds() + Event->CooldownTime);
    }
}

void UNarr_DialogueManager::RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event)
{
    RegisteredEvents.Add(Event.EventName, Event);
    UE_LOG(LogTemp, Log, TEXT("Registered narrative event: %s"), *Event.EventName);
}

bool UNarr_DialogueManager::CanPlayEvent(const FString& EventName) const
{
    if (!bDialogueEnabled)
    {
        return false;
    }

    // Check if it's a play-once event that was already played
    if (PlayedOnceEvents.Contains(EventName))
    {
        return false;
    }

    // Check cooldown
    const float* CooldownTime = EventCooldowns.Find(EventName);
    if (CooldownTime && GetWorld()->GetTimeSeconds() < *CooldownTime)
    {
        return false;
    }

    return true;
}

void UNarr_DialogueManager::SetDialogueEnabled(bool bEnabled)
{
    bDialogueEnabled = bEnabled;
    if (!bEnabled)
    {
        StopCurrentDialogue();
    }
}

void UNarr_DialogueManager::PlayRandomDialogue(ENarr_DialogueType SpeakerType)
{
    FNarr_DialogueLine RandomLine = GetRandomDialogue(SpeakerType);
    if (!RandomLine.DialogueText.IsEmpty())
    {
        PlayDialogue(RandomLine);
    }
}

void UNarr_DialogueManager::MarkEventPlayed(const FString& EventName)
{
    PlayedOnceEvents.Add(EventName);
}

void UNarr_DialogueManager::ResetAllEvents()
{
    PlayedOnceEvents.Empty();
    EventCooldowns.Empty();
    UE_LOG(LogTemp, Log, TEXT("Reset all narrative events"));
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    bCurrentlyPlayingDialogue = false;
    CurrentDialogueEndTime = 0.0f;
}

bool UNarr_DialogueManager::IsDialoguePlaying() const
{
    return bCurrentlyPlayingDialogue && GetWorld()->GetTimeSeconds() < CurrentDialogueEndTime;
}

FNarr_DialogueLine UNarr_DialogueManager::GetRandomDialogue(ENarr_DialogueType SpeakerType) const
{
    FNarr_DialogueLine EmptyLine;
    
    switch (SpeakerType)
    {
        case ENarr_DialogueType::TribalElder:
            if (TribalElderLines.Num() > 0)
            {
                return TribalElderLines[FMath::RandRange(0, TribalElderLines.Num() - 1)];
            }
            break;
        case ENarr_DialogueType::ScoutWarning:
            if (ScoutWarningLines.Num() > 0)
            {
                return ScoutWarningLines[FMath::RandRange(0, ScoutWarningLines.Num() - 1)];
            }
            break;
        case ENarr_DialogueType::AncientWisdom:
            if (AncientWisdomLines.Num() > 0)
            {
                return AncientWisdomLines[FMath::RandRange(0, AncientWisdomLines.Num() - 1)];
            }
            break;
        case ENarr_DialogueType::CombatNarrator:
            if (CombatNarratorLines.Num() > 0)
            {
                return CombatNarratorLines[FMath::RandRange(0, CombatNarratorLines.Num() - 1)];
            }
            break;
    }
    
    return EmptyLine;
}