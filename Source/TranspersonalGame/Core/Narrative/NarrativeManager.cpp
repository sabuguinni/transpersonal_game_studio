#include "NarrativeManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UNarrativeManager::UNarrativeManager()
{
    bDialogueActive = false;
    CurrentDialogueIndex = 0;
    VoiceAudioComponent = nullptr;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default story content
    InitializeDefaultStoryBeats();
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized"));
}

void UNarrativeManager::Deinitialize()
{
    EndDialogue();
    
    if (VoiceAudioComponent && IsValid(VoiceAudioComponent))
    {
        VoiceAudioComponent->Stop();
        VoiceAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UNarrativeManager::StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener)
{
    if (bDialogueActive)
    {
        EndDialogue();
    }
    
    if (!QuestDialogues.Contains(DialogueID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue ID not found: %s"), *DialogueID);
        return;
    }
    
    bDialogueActive = true;
    CurrentDialogueID = DialogueID;
    CurrentDialogueIndex = 0;
    CurrentSpeaker = Speaker;
    CurrentListener = Listener;
    
    // Start first dialogue line
    const FNarr_QuestDialogue& QuestDialogue = QuestDialogues[DialogueID];
    if (QuestDialogue.DialogueLines.Num() > 0)
    {
        ProcessDialogueLine(QuestDialogue.DialogueLines[0]);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s"), *DialogueID);
}

void UNarrativeManager::EndDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }
    
    bDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentDialogueIndex = 0;
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;
    
    if (VoiceAudioComponent && IsValid(VoiceAudioComponent))
    {
        VoiceAudioComponent->Stop();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ended dialogue"));
}

void UNarrativeManager::AdvanceDialogue()
{
    if (!bDialogueActive || !QuestDialogues.Contains(CurrentDialogueID))
    {
        return;
    }
    
    const FNarr_QuestDialogue& QuestDialogue = QuestDialogues[CurrentDialogueID];
    CurrentDialogueIndex++;
    
    if (CurrentDialogueIndex >= QuestDialogue.DialogueLines.Num())
    {
        EndDialogue();
        return;
    }
    
    ProcessDialogueLine(QuestDialogue.DialogueLines[CurrentDialogueIndex]);
}

void UNarrativeManager::TriggerStoryBeat(const FString& BeatID)
{
    if (!StoryBeats.Contains(BeatID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Story beat not found: %s"), *BeatID);
        return;
    }
    
    if (CompletedStoryBeats.Contains(BeatID))
    {
        UE_LOG(LogTemp, Log, TEXT("Story beat already completed: %s"), *BeatID);
        return;
    }
    
    const FNarr_StoryBeat& StoryBeat = StoryBeats[BeatID];
    
    // Check trigger conditions
    if (!CheckConditions(StoryBeat.TriggerConditions))
    {
        UE_LOG(LogTemp, Log, TEXT("Story beat conditions not met: %s"), *BeatID);
        return;
    }
    
    // Mark as completed
    CompletedStoryBeats.Add(BeatID);
    
    // Apply completion flags
    ApplyCompletionFlags(StoryBeat.CompletionFlags);
    
    UE_LOG(LogTemp, Log, TEXT("Triggered story beat: %s"), *BeatID);
}

void UNarrativeManager::SetStoryFlag(const FString& FlagName, bool bValue)
{
    StoryFlags.Add(FlagName, bValue);
    UE_LOG(LogTemp, Log, TEXT("Set story flag %s to %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool UNarrativeManager::GetStoryFlag(const FString& FlagName) const
{
    if (const bool* FlagValue = StoryFlags.Find(FlagName))
    {
        return *FlagValue;
    }
    return false;
}

void UNarrativeManager::RegisterQuestDialogue(const FString& QuestID, const FNarr_QuestDialogue& QuestDialogue)
{
    QuestDialogues.Add(QuestID, QuestDialogue);
    UE_LOG(LogTemp, Log, TEXT("Registered quest dialogue: %s"), *QuestID);
}

void UNarrativeManager::PlayQuestDialogue(const FString& QuestID)
{
    if (CurrentSpeaker.IsValid() && CurrentListener.IsValid())
    {
        StartDialogue(QuestID, CurrentSpeaker.Get(), CurrentListener.Get());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play quest dialogue - no valid speaker/listener"));
    }
}

void UNarrativeManager::PlayCharacterVoiceLine(const FString& CharacterName, const FString& LineID)
{
    if (CharacterVoices.Contains(CharacterName))
    {
        TSoftObjectPtr<USoundCue> VoiceCue = CharacterVoices[CharacterName];
        if (VoiceCue.IsValid())
        {
            UGameplayStatics::PlaySound2D(GetWorld(), VoiceCue.Get());
            UE_LOG(LogTemp, Log, TEXT("Playing voice line for %s: %s"), *CharacterName, *LineID);
        }
    }
}

void UNarrativeManager::RegisterCharacterVoice(const FString& CharacterName, USoundCue* VoiceCue)
{
    if (VoiceCue)
    {
        CharacterVoices.Add(CharacterName, VoiceCue);
        UE_LOG(LogTemp, Log, TEXT("Registered voice for character: %s"), *CharacterName);
    }
}

void UNarrativeManager::ProcessDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    // Check if line conditions are met
    if (!CheckConditions(DialogueLine.RequiredFlags))
    {
        AdvanceDialogue(); // Skip this line
        return;
    }
    
    // Play voice line if available
    if (DialogueLine.VoiceLine.IsValid())
    {
        if (!VoiceAudioComponent)
        {
            VoiceAudioComponent = UGameplayStatics::CreateSound2D(GetWorld(), DialogueLine.VoiceLine.Get());
        }
        
        if (VoiceAudioComponent)
        {
            VoiceAudioComponent->Play();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Processing dialogue line: %s - %s"), 
           *DialogueLine.SpeakerName, 
           *DialogueLine.DialogueText.ToString());
}

bool UNarrativeManager::CheckConditions(const TArray<FString>& Conditions) const
{
    for (const FString& Condition : Conditions)
    {
        if (!GetStoryFlag(Condition))
        {
            return false;
        }
    }
    return true;
}

void UNarrativeManager::ApplyCompletionFlags(const TArray<FString>& Flags)
{
    for (const FString& Flag : Flags)
    {
        SetStoryFlag(Flag, true);
    }
}

void UNarrativeManager::InitializeDefaultStoryBeats()
{
    // First encounter with dinosaurs
    FNarr_StoryBeat FirstEncounter;
    FirstEncounter.BeatID = TEXT("first_dinosaur_encounter");
    FirstEncounter.NarrativeText = FText::FromString(TEXT("The ground trembles as massive footsteps approach. Your first glimpse of the ancient giants that rule this world."));
    FirstEncounter.TriggerConditions.Add(TEXT("player_spawned"));
    FirstEncounter.CompletionFlags.Add(TEXT("seen_dinosaur"));
    FirstEncounter.Priority = 10;
    StoryBeats.Add(FirstEncounter.BeatID, FirstEncounter);
    
    // Learning to hunt
    FNarr_StoryBeat LearnHunting;
    LearnHunting.BeatID = TEXT("learn_hunting");
    LearnHunting.NarrativeText = FText::FromString(TEXT("Survival demands adaptation. The weak become prey, but the clever become predators."));
    LearnHunting.TriggerConditions.Add(TEXT("seen_dinosaur"));
    LearnHunting.CompletionFlags.Add(TEXT("hunting_unlocked"));
    LearnHunting.Priority = 20;
    StoryBeats.Add(LearnHunting.BeatID, LearnHunting);
    
    // Tribal contact
    FNarr_StoryBeat TribalContact;
    TribalContact.BeatID = TEXT("tribal_contact");
    TribalContact.NarrativeText = FText::FromString(TEXT("Smoke rises in the distance. Others have survived in this harsh world. Perhaps alliance is possible."));
    TribalContact.TriggerConditions.Add(TEXT("hunting_unlocked"));
    TribalContact.CompletionFlags.Add(TEXT("met_tribe"));
    TribalContact.Priority = 30;
    StoryBeats.Add(TribalContact.BeatID, TribalContact);
}

void UNarrativeManager::InitializeDefaultDialogues()
{
    // Tribal leader introduction
    FNarr_QuestDialogue TribalIntro;
    TribalIntro.QuestID = TEXT("tribal_leader_intro");
    TribalIntro.bIsMainQuest = true;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Leader");
    Line1.DialogueText = FText::FromString(TEXT("You survive alone in the wilderness. Impressive. Few last more than a day without the protection of the tribe."));
    Line1.Duration = 5.0f;
    TribalIntro.DialogueLines.Add(Line1);
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Leader");
    Line2.DialogueText = FText::FromString(TEXT("The great beasts grow bolder each season. We need hunters who understand their ways. Will you stand with us?"));
    Line2.Duration = 5.0f;
    TribalIntro.DialogueLines.Add(Line2);
    
    QuestDialogues.Add(TribalIntro.QuestID, TribalIntro);
    
    // Wounded warrior dialogue
    FNarr_QuestDialogue WoundedWarrior;
    WoundedWarrior.QuestID = TEXT("wounded_warrior_warning");
    WoundedWarrior.bIsMainQuest = false;
    
    FNarr_DialogueLine WarriorLine;
    WarriorLine.SpeakerName = TEXT("Wounded Warrior");
    WarriorLine.DialogueText = FText::FromString(TEXT("The Carnotaurus... it's not like the others. It hunts alone, thinks like us. Stay away from the canyon - it knows we come for water there."));
    WarriorLine.Duration = 6.0f;
    WarriorLine.RequiredFlags.Add(TEXT("met_tribe"));
    WoundedWarrior.DialogueLines.Add(WarriorLine);
    
    QuestDialogues.Add(WoundedWarrior.QuestID, WoundedWarrior);
}