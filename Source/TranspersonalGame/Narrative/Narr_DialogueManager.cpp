#include "Narr_DialogueManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize dialogue state
    CurrentState = ENarr_DialogueState::Idle;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
    CurrentStoryBeat = 0;

    // Create audio component
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
    DialogueAudioComponent->bAutoActivate = false;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Initialized with %d dialogue sequences"), DialogueSequences.Num());
}

void ANarr_DialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == ENarr_DialogueState::Playing)
    {
        CurrentLineTimer += DeltaTime;
        ProcessCurrentDialogueLine();
    }
}

void ANarr_DialogueManager::StartDialogueSequence(const FString& SequenceID)
{
    if (!DialogueSequences.Contains(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Sequence '%s' not found"), *SequenceID);
        return;
    }

    const FNarr_DialogueSequence& Sequence = DialogueSequences[SequenceID];
    
    if (!CanPlaySequence(Sequence))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Cannot play sequence '%s' - requirements not met"), *SequenceID);
        return;
    }

    // Stop any current dialogue
    StopCurrentDialogue();

    // Start new sequence
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
    CurrentState = ENarr_DialogueState::Playing;

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Started sequence '%s' with %d lines"), 
           *SequenceID, Sequence.DialogueLines.Num());

    ProcessCurrentDialogueLine();
}

void ANarr_DialogueManager::StopCurrentDialogue()
{
    if (CurrentState != ENarr_DialogueState::Idle)
    {
        StopDialogueAudio();
        CurrentState = ENarr_DialogueState::Idle;
        CurrentSequenceID = TEXT("");
        CurrentLineIndex = 0;
        CurrentLineTimer = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Stopped current dialogue"));
    }
}

void ANarr_DialogueManager::AdvanceDialogue()
{
    if (CurrentState == ENarr_DialogueState::WaitingForResponse || 
        CurrentState == ENarr_DialogueState::Playing)
    {
        OnDialogueLineCompleted();
    }
}

bool ANarr_DialogueManager::IsDialoguePlaying() const
{
    return CurrentState != ENarr_DialogueState::Idle;
}

void ANarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Registered sequence '%s'"), *Sequence.SequenceID);
}

bool ANarr_DialogueManager::HasDialogueSequence(const FString& SequenceID) const
{
    return DialogueSequences.Contains(SequenceID);
}

void ANarr_DialogueManager::TriggerCharacterDialogue(const FString& CharacterName, const FString& Context)
{
    FString SequenceID = FString::Printf(TEXT("%s_%s"), *CharacterName, *Context);
    
    if (HasDialogueSequence(SequenceID))
    {
        StartDialogueSequence(SequenceID);
    }
    else
    {
        // Try generic character dialogue
        FString GenericID = FString::Printf(TEXT("%s_Generic"), *CharacterName);
        if (HasDialogueSequence(GenericID))
        {
            StartDialogueSequence(GenericID);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DialogueManager: No dialogue found for character '%s' in context '%s'"), 
                   *CharacterName, *Context);
        }
    }
}

void ANarr_DialogueManager::SetCharacterVoiceProfile(const FString& CharacterName, USoundCue* VoiceProfile)
{
    if (VoiceProfile)
    {
        CharacterVoiceProfiles.Add(CharacterName, VoiceProfile);
        UE_LOG(LogTemp, Log, TEXT("DialogueManager: Set voice profile for character '%s'"), *CharacterName);
    }
}

void ANarr_DialogueManager::UpdateStoryProgress(int32 NewStoryBeat)
{
    CurrentStoryBeat = NewStoryBeat;
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Updated story progress to beat %d"), NewStoryBeat);
}

TArray<FString> ANarr_DialogueManager::GetAvailableDialogueSequences() const
{
    TArray<FString> AvailableSequences;
    
    for (const auto& Pair : DialogueSequences)
    {
        if (CanPlaySequence(Pair.Value))
        {
            AvailableSequences.Add(Pair.Key);
        }
    }
    
    return AvailableSequences;
}

void ANarr_DialogueManager::PlayDialogueAudio(USoundCue* AudioClip)
{
    if (AudioClip && DialogueAudioComponent)
    {
        DialogueAudioComponent->SetSound(AudioClip);
        DialogueAudioComponent->Play();
    }
}

void ANarr_DialogueManager::StopDialogueAudio()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }
}

void ANarr_DialogueManager::InitializeDefaultDialogues()
{
    // Elder Kael - Wisdom dialogue
    FNarr_DialogueSequence ElderWisdom;
    ElderWisdom.SequenceID = TEXT("Elder_Kael_Wisdom");
    ElderWisdom.bIsRepeatable = true;
    ElderWisdom.RequiredStoryBeat = 0;

    FNarr_DialogueLine WisdomLine;
    WisdomLine.SpeakerName = TEXT("Elder Kael");
    WisdomLine.DialogueText = TEXT("Listen carefully, young survivor. The elders speak of ancient wisdom passed down through countless seasons.");
    WisdomLine.DisplayDuration = 5.0f;
    WisdomLine.bRequiresPlayerResponse = false;

    ElderWisdom.DialogueLines.Add(WisdomLine);
    RegisterDialogueSequence(ElderWisdom);

    // Scout Vera - Danger warning
    FNarr_DialogueSequence ScoutWarning;
    ScoutWarning.SequenceID = TEXT("Scout_Vera_Warning");
    ScoutWarning.bIsRepeatable = true;
    ScoutWarning.RequiredStoryBeat = 1;

    FNarr_DialogueLine WarningLine;
    WarningLine.SpeakerName = TEXT("Scout Vera");
    WarningLine.DialogueText = TEXT("The pack moves as one through the dense undergrowth. Stay low, stay quiet, and don't run until I give the signal.");
    WarningLine.DisplayDuration = 4.0f;
    WarningLine.bRequiresPlayerResponse = false;

    ScoutWarning.DialogueLines.Add(WarningLine);
    RegisterDialogueSequence(ScoutWarning);

    // Hunt Leader Thane - Battle cry
    FNarr_DialogueSequence HuntBattle;
    HuntBattle.SequenceID = TEXT("Hunt_Leader_Thane_Battle");
    HuntBattle.bIsRepeatable = true;
    HuntBattle.RequiredStoryBeat = 2;

    FNarr_DialogueLine BattleLine;
    BattleLine.SpeakerName = TEXT("Hunt Leader Thane");
    BattleLine.DialogueText = TEXT("The great thunder-foot approaches! All hunters to defensive positions! Show courage, show skill!");
    BattleLine.DisplayDuration = 4.5f;
    BattleLine.bRequiresPlayerResponse = false;

    HuntBattle.DialogueLines.Add(BattleLine);
    RegisterDialogueSequence(HuntBattle);

    // Shaman Aria - Spiritual guidance
    FNarr_DialogueSequence ShamanGuidance;
    ShamanGuidance.SequenceID = TEXT("Shaman_Aria_Guidance");
    ShamanGuidance.bIsRepeatable = true;
    ShamanGuidance.RequiredStoryBeat = 3;

    FNarr_DialogueLine GuidanceLine;
    GuidanceLine.SpeakerName = TEXT("Shaman Aria");
    GuidanceLine.DialogueText = TEXT("The sacred fire burns low tonight, and change comes to our lands. We must choose our path carefully.");
    GuidanceLine.DisplayDuration = 4.0f;
    GuidanceLine.bRequiresPlayerResponse = true;

    ShamanGuidance.DialogueLines.Add(GuidanceLine);
    RegisterDialogueSequence(ShamanGuidance);
}

void ANarr_DialogueManager::ProcessCurrentDialogueLine()
{
    if (!DialogueSequences.Contains(CurrentSequenceID))
    {
        StopCurrentDialogue();
        return;
    }

    const FNarr_DialogueSequence& Sequence = DialogueSequences[CurrentSequenceID];
    
    if (CurrentLineIndex >= Sequence.DialogueLines.Num())
    {
        // Sequence completed
        CurrentState = ENarr_DialogueState::Completed;
        StopCurrentDialogue();
        return;
    }

    const FNarr_DialogueLine& CurrentLine = Sequence.DialogueLines[CurrentLineIndex];

    // Check if we need to wait for player response
    if (CurrentLine.bRequiresPlayerResponse)
    {
        CurrentState = ENarr_DialogueState::WaitingForResponse;
        return;
    }

    // Check if line duration has elapsed
    if (CurrentLineTimer >= CurrentLine.DisplayDuration)
    {
        OnDialogueLineCompleted();
    }
}

bool ANarr_DialogueManager::CanPlaySequence(const FNarr_DialogueSequence& Sequence) const
{
    return CurrentStoryBeat >= Sequence.RequiredStoryBeat;
}

void ANarr_DialogueManager::OnDialogueLineCompleted()
{
    CurrentLineIndex++;
    CurrentLineTimer = 0.0f;
    CurrentState = ENarr_DialogueState::Playing;

    if (DialogueSequences.Contains(CurrentSequenceID))
    {
        const FNarr_DialogueSequence& Sequence = DialogueSequences[CurrentSequenceID];
        
        if (CurrentLineIndex >= Sequence.DialogueLines.Num())
        {
            // Sequence completed
            UE_LOG(LogTemp, Log, TEXT("DialogueManager: Completed sequence '%s'"), *CurrentSequenceID);
            StopCurrentDialogue();
        }
        else
        {
            // Continue to next line
            ProcessCurrentDialogueLine();
        }
    }
}