#include "Narr_DialogueManager.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create audio component for voice playback
    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
    RootComponent = VoiceAudioComponent;

    // Default settings
    DefaultDialogueRange = 1000.0f;
    bAutoPlayVoice = true;
    bIsDialogueActive = false;
    CurrentLineIndex = 0;
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogues();
    
    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->SetVolumeMultiplier(0.8f);
        VoiceAudioComponent->SetPitchMultiplier(1.0f);
    }
}

void ANarr_DialogueManager::InitializeDefaultDialogues()
{
    // Create default hunter dialogue
    FNarr_DialogueSequence HunterGreeting;
    HunterGreeting.SequenceID = TEXT("Hunter_Greeting");
    HunterGreeting.bRepeatable = true;
    HunterGreeting.Priority = 1;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Hunter");
    Line1.DialogueText = FText::FromString(TEXT("The great beasts roam these lands. Stay alert."));
    Line1.DialogueType = ENarr_DialogueType::Warning;
    Line1.Duration = 4.0f;
    HunterGreeting.DialogueLines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Hunter");
    Line2.DialogueText = FText::FromString(TEXT("I have tracked the T-Rex to the eastern cliffs."));
    Line2.DialogueType = ENarr_DialogueType::Information;
    Line2.Duration = 3.5f;
    HunterGreeting.DialogueLines.Add(Line2);

    LoadedDialogues.Add(HunterGreeting.SequenceID, HunterGreeting);

    // Create scout warning dialogue
    FNarr_DialogueSequence ScoutWarning;
    ScoutWarning.SequenceID = TEXT("Scout_Warning");
    ScoutWarning.bRepeatable = false;
    ScoutWarning.Priority = 3;

    FNarr_DialogueLine WarnLine1;
    WarnLine1.SpeakerName = TEXT("Scout");
    WarnLine1.DialogueText = FText::FromString(TEXT("Danger approaches from the ridge!"));
    WarnLine1.DialogueType = ENarr_DialogueType::Warning;
    WarnLine1.Duration = 3.0f;
    ScoutWarning.DialogueLines.Add(WarnLine1);

    FNarr_DialogueLine WarnLine2;
    WarnLine2.SpeakerName = TEXT("Scout");
    WarnLine2.DialogueText = FText::FromString(TEXT("Pack hunters circle like shadows. Seek shelter!"));
    WarnLine2.DialogueType = ENarr_DialogueType::Warning;
    WarnLine2.Duration = 4.0f;
    ScoutWarning.DialogueLines.Add(WarnLine2);

    LoadedDialogues.Add(ScoutWarning.SequenceID, ScoutWarning);

    // Create gatherer trade dialogue
    FNarr_DialogueSequence GathererTrade;
    GathererTrade.SequenceID = TEXT("Gatherer_Trade");
    GathererTrade.bRepeatable = true;
    GathererTrade.Priority = 1;

    FNarr_DialogueLine TradeGreeting;
    TradeGreeting.SpeakerName = TEXT("Gatherer");
    TradeGreeting.DialogueText = FText::FromString(TEXT("Fresh berries and roots. Trade for tools?"));
    TradeGreeting.DialogueType = ENarr_DialogueType::Trade;
    TradeGreeting.Duration = 3.0f;
    GathererTrade.DialogueLines.Add(TradeGreeting);

    LoadedDialogues.Add(GathererTrade.SequenceID, GathererTrade);
}

bool ANarr_DialogueManager::StartDialogue(const FString& SequenceID, APawn* Speaker, APawn* Listener)
{
    if (bIsDialogueActive || !Speaker || !Listener)
    {
        return false;
    }

    if (!LoadedDialogues.Contains(SequenceID))
    {
        if (!LoadDialogueSequence(SequenceID))
        {
            return false;
        }
    }

    // Check distance between speaker and listener
    float Distance = FVector::Dist(Speaker->GetActorLocation(), Listener->GetActorLocation());
    if (Distance > DefaultDialogueRange)
    {
        return false;
    }

    CurrentSequenceID = SequenceID;
    CurrentSpeaker = Speaker;
    CurrentListener = Listener;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;

    OnDialogueStarted(SequenceID);
    
    // Start first line
    NextDialogueLine();

    return true;
}

void ANarr_DialogueManager::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    bIsDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;
    CurrentLineIndex = 0;

    if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
    {
        VoiceAudioComponent->Stop();
    }

    OnDialogueEnded();
}

void ANarr_DialogueManager::NextDialogueLine()
{
    if (!bIsDialogueActive || !LoadedDialogues.Contains(CurrentSequenceID))
    {
        EndDialogue();
        return;
    }

    FNarr_DialogueSequence& CurrentSequence = LoadedDialogues[CurrentSequenceID];
    
    if (CurrentLineIndex >= CurrentSequence.DialogueLines.Num())
    {
        EndDialogue();
        return;
    }

    FNarr_DialogueLine& CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
    
    // Play voice if available and auto-play is enabled
    if (bAutoPlayVoice && CurrentLine.VoiceClip)
    {
        PlayVoiceLine(CurrentLine.VoiceClip);
    }

    OnDialogueLineChanged(CurrentLine);

    // Auto-advance after duration
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        CurrentLineIndex++;
        NextDialogueLine();
    }, CurrentLine.Duration, false);
}

bool ANarr_DialogueManager::LoadDialogueSequence(const FString& SequenceID)
{
    // Try to load from data table if available
    if (DialogueDataTable)
    {
        FNarr_DialogueSequence* FoundSequence = DialogueDataTable->FindRow<FNarr_DialogueSequence>(FName(*SequenceID), TEXT(""));
        if (FoundSequence)
        {
            LoadedDialogues.Add(SequenceID, *FoundSequence);
            return true;
        }
    }

    return false;
}

FNarr_DialogueLine ANarr_DialogueManager::GetCurrentDialogueLine()
{
    if (bIsDialogueActive && LoadedDialogues.Contains(CurrentSequenceID))
    {
        FNarr_DialogueSequence& CurrentSequence = LoadedDialogues[CurrentSequenceID];
        if (CurrentLineIndex < CurrentSequence.DialogueLines.Num())
        {
            return CurrentSequence.DialogueLines[CurrentLineIndex];
        }
    }

    return FNarr_DialogueLine();
}

TArray<FString> ANarr_DialogueManager::GetAvailableDialogues()
{
    TArray<FString> DialogueIDs;
    LoadedDialogues.GetKeys(DialogueIDs);
    return DialogueIDs;
}

void ANarr_DialogueManager::RegisterNPCDialogue(APawn* NPC, const FString& SequenceID)
{
    if (NPC && !SequenceID.IsEmpty())
    {
        NPCDialogueMap.Add(NPC, SequenceID);
    }
}

bool ANarr_DialogueManager::CanStartDialogue(APawn* Speaker, APawn* Listener)
{
    if (bIsDialogueActive || !Speaker || !Listener)
    {
        return false;
    }

    float Distance = FVector::Dist(Speaker->GetActorLocation(), Listener->GetActorLocation());
    return Distance <= DefaultDialogueRange;
}

void ANarr_DialogueManager::PlayVoiceLine(USoundBase* VoiceClip)
{
    if (VoiceAudioComponent && VoiceClip)
    {
        VoiceAudioComponent->SetSound(VoiceClip);
        VoiceAudioComponent->Play();
    }
}