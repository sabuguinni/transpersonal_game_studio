#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue System Initialized"));
    
    // Initialize default dialogue database
    InitializeDefaultDialogues();
    LoadDialogueDatabase();
}

void UNarr_DialogueSystem::RegisterDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    DialogueDatabase.Add(DialogueLine);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue line: %s"), *DialogueLine.DialogueID);
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& DialogueSequence)
{
    DialogueSequences.Add(DialogueSequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s"), *DialogueSequence.SequenceID);
}

FNarr_DialogueLine UNarr_DialogueSystem::GetRandomDialogue(ENarr_DialogueType DialogueType, ENarr_CharacterType CharacterType)
{
    TArray<FNarr_DialogueLine> MatchingDialogues;
    
    for (const FNarr_DialogueLine& Dialogue : DialogueDatabase)
    {
        if (Dialogue.DialogueType == DialogueType && Dialogue.SpeakerType == CharacterType)
        {
            if (CheckDialogueConditions(Dialogue.TriggerConditions))
            {
                MatchingDialogues.Add(Dialogue);
            }
        }
    }
    
    if (MatchingDialogues.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, MatchingDialogues.Num() - 1);
        return MatchingDialogues[RandomIndex];
    }
    
    // Return empty dialogue if none found
    return FNarr_DialogueLine();
}

TArray<FNarr_DialogueLine> UNarr_DialogueSystem::GetDialogueSequence(const FString& SequenceID)
{
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            return Sequence.DialogueLines;
        }
    }
    
    return TArray<FNarr_DialogueLine>();
}

void UNarr_DialogueSystem::PlayDialogue(const FNarr_DialogueLine& DialogueLine)
{
    if (DialogueLine.DialogueText.IsEmpty())
    {
        return;
    }
    
    // Log dialogue for debugging
    UE_LOG(LogTemp, Warning, TEXT("Playing Dialogue: %s"), *DialogueLine.DialogueText.ToString());
    
    // Track played dialogues
    if (PlayedDialogues.Contains(DialogueLine.DialogueID))
    {
        PlayedDialogues[DialogueLine.DialogueID]++;
    }
    else
    {
        PlayedDialogues.Add(DialogueLine.DialogueID, 1);
    }
    
    // TODO: Integrate with audio system when available
    // For now, just log the audio URL
    if (!DialogueLine.AudioURL.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Audio URL: %s"), *DialogueLine.AudioURL);
    }
}

void UNarr_DialogueSystem::LoadDialogueDatabase()
{
    UE_LOG(LogTemp, Warning, TEXT("Loading dialogue database - %d dialogues registered"), DialogueDatabase.Num());
}

int32 UNarr_DialogueSystem::GetTotalDialogueCount() const
{
    return DialogueDatabase.Num();
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Initialize with the generated TTS dialogues
    FNarr_DialogueLine ThunderLizardDialogue;
    ThunderLizardDialogue.DialogueID = "elder_thunder_lizard_warning";
    ThunderLizardDialogue.SpeakerType = ENarr_CharacterType::TribalElder;
    ThunderLizardDialogue.DialogueType = ENarr_DialogueType::Warning;
    ThunderLizardDialogue.DialogueText = FText::FromString("The great Thunder Lizard has returned to the valley! Its footsteps shake the earth and its roar echoes through the canyons. The tribe must prepare for the ancient ritual of the Hunt. Only the bravest warriors dare face the king of all beasts.");
    ThunderLizardDialogue.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778904906665_TribalElder.mp3";
    ThunderLizardDialogue.Duration = 17.0f;
    RegisterDialogueLine(ThunderLizardDialogue);
    
    FNarr_DialogueLine RaptorWarning;
    RaptorWarning.DialogueID = "warrior_raptor_warning";
    RaptorWarning.SpeakerType = ENarr_CharacterType::TribalWarrior;
    RaptorWarning.DialogueType = ENarr_DialogueType::Warning;
    RaptorWarning.DialogueText = FText::FromString("The pack hunters circle in the darkness beyond the firelight. Their eyes gleam like stars in the night, watching, waiting for weakness. Stay close to the flames, young one, for the Raptors fear only fire and the courage of united hunters.");
    RaptorWarning.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778904912638_TribalWarrior.mp3";
    RaptorWarning.Duration = 16.0f;
    RegisterDialogueLine(RaptorWarning);
    
    FNarr_DialogueLine BrachiosaurusInfo;
    BrachiosaurusInfo.DialogueID = "scout_brachiosaurus_info";
    BrachiosaurusInfo.SpeakerType = ENarr_CharacterType::TribalScout;
    BrachiosaurusInfo.DialogueType = ENarr_DialogueType::Information;
    BrachiosaurusInfo.DialogueText = FText::FromString("The gentle giants graze in the eastern meadows, their long necks reaching toward the sky like living trees. The Brachiosaurus brings no threat to those who respect their domain, but beware - where they flee, greater dangers follow.");
    BrachiosaurusInfo.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778904918646_TribalScout.mp3";
    BrachiosaurusInfo.Duration = 16.0f;
    RegisterDialogueLine(BrachiosaurusInfo);
    
    FNarr_DialogueLine BloodScent;
    BloodScent.DialogueID = "tracker_blood_warning";
    BloodScent.SpeakerType = ENarr_CharacterType::TribalTracker;
    BloodScent.DialogueType = ENarr_DialogueType::Warning;
    BloodScent.DialogueText = FText::FromString("Blood on the wind... the scent carries far across the savanna. Something has fallen to predators in the northern hunting grounds. We must move carefully - the feast will draw every carnivore for miles around.");
    BloodScent.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778904924389_TribalTracker.mp3";
    BloodScent.Duration = 14.0f;
    RegisterDialogueLine(BloodScent);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default dialogues"), DialogueDatabase.Num());
}

bool UNarr_DialogueSystem::CheckDialogueConditions(const TArray<FString>& Conditions)
{
    // For now, return true for all conditions
    // TODO: Implement condition checking system
    return true;
}