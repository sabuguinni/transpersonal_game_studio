#include "Narr_DialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentState = ENarr_DialogueState::Inactive;
    ActiveSequenceID = TEXT("");
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    bAutoAdvance = true;
    AutoAdvanceDelay = 3.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogues();
    LoadDialogueFromConfig();
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue System initialized with %d sequences"), DialogueSequences.Num());
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentState == ENarr_DialogueState::DisplayingText)
    {
        UpdateDialogueTimer(DeltaTime);
    }
}

void UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    if (!DialogueSequences.Contains(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
        return;
    }
    
    ActiveSequenceID = SequenceID;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    CurrentState = ENarr_DialogueState::DisplayingText;
    
    const FNarr_DialogueSequence& Sequence = DialogueSequences[SequenceID];
    if (Sequence.DialogueLines.Num() > 0)
    {
        const FNarr_DialogueLine& FirstLine = Sequence.DialogueLines[0];
        if (!FirstLine.AudioURL.IsEmpty())
        {
            PlayDialogueAudio(FirstLine.AudioURL);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s - %s"), *FirstLine.SpeakerName, *FirstLine.DialogueText);
    }
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (!CanAdvanceDialogue())
    {
        return;
    }
    
    const FNarr_DialogueSequence& Sequence = DialogueSequences[ActiveSequenceID];
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= Sequence.DialogueLines.Num())
    {
        EndDialogue();
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = Sequence.DialogueLines[CurrentLineIndex];
    DialogueTimer = 0.0f;
    
    if (!CurrentLine.AudioURL.IsEmpty())
    {
        PlayDialogueAudio(CurrentLine.AudioURL);
    }
    
    if (CurrentLine.bIsPlayerChoice)
    {
        CurrentState = ENarr_DialogueState::WaitingForChoice;
    }
    else
    {
        CurrentState = ENarr_DialogueState::DisplayingText;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Advanced dialogue: %s - %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
}

void UNarr_DialogueSystem::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Completed;
    ActiveSequenceID = TEXT("");
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    
    StopDialogueAudio();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue sequence ended"));
    
    // Reset to inactive after a brief delay
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(FTimerHandle(), [this]()
        {
            CurrentState = ENarr_DialogueState::Inactive;
        }, 1.0f, false);
    }
}

bool UNarr_DialogueSystem::IsDialogueActive() const
{
    return CurrentState != ENarr_DialogueState::Inactive && CurrentState != ENarr_DialogueState::Completed;
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentDialogueLine() const
{
    if (!DialogueSequences.Contains(ActiveSequenceID))
    {
        return FNarr_DialogueLine();
    }
    
    const FNarr_DialogueSequence& Sequence = DialogueSequences[ActiveSequenceID];
    if (CurrentLineIndex < 0 || CurrentLineIndex >= Sequence.DialogueLines.Num())
    {
        return FNarr_DialogueLine();
    }
    
    return Sequence.DialogueLines[CurrentLineIndex];
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueSequences.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s with %d lines"), *Sequence.SequenceID, Sequence.DialogueLines.Num());
}

void UNarr_DialogueSystem::LoadDialogueFromConfig()
{
    // TODO: Load dialogue sequences from JSON config file
    // For now, we use the default dialogues initialized in InitializeDefaultDialogues()
    UE_LOG(LogTemp, Log, TEXT("Dialogue config loading - using default dialogues"));
}

void UNarr_DialogueSystem::PlayDialogueAudio(const FString& AudioURL)
{
    // TODO: Integrate with audio system to play dialogue from URL
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *AudioURL);
}

void UNarr_DialogueSystem::StopDialogueAudio()
{
    // TODO: Stop current dialogue audio playback
    UE_LOG(LogTemp, Log, TEXT("Stopping dialogue audio"));
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Tribal Elder Introduction
    FNarr_DialogueSequence ElderIntro;
    ElderIntro.SequenceID = TEXT("elder_intro");
    ElderIntro.bIsRepeatable = false;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Elder");
    Line1.DialogueText = TEXT("Listen well, young hunter. The great predators of this land follow ancient patterns.");
    Line1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778879892767_TribalElder.mp3");
    Line1.DisplayDuration = 5.0f;
    ElderIntro.DialogueLines.Add(Line1);
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Elder");
    Line2.DialogueText = TEXT("When the Thunder Lizard roars at dawn, it marks territory. Learn these signs, or become prey yourself.");
    Line2.DisplayDuration = 4.0f;
    ElderIntro.DialogueLines.Add(Line2);
    
    RegisterDialogueSequence(ElderIntro);
    
    // Scout Warning
    FNarr_DialogueSequence ScoutWarning;
    ScoutWarning.SequenceID = TEXT("scout_warning");
    ScoutWarning.bIsRepeatable = true;
    
    FNarr_DialogueLine Warning1;
    Warning1.SpeakerName = TEXT("Scout");
    Warning1.DialogueText = TEXT("Warning! Raptor pack moving through the eastern valley. Three hunters, coordinated attack pattern.");
    Warning1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778879907731_ScoutWarning.mp3");
    Warning1.DisplayDuration = 4.0f;
    ScoutWarning.DialogueLines.Add(Warning1);
    
    RegisterDialogueSequence(ScoutWarning);
    
    // Night Guard
    FNarr_DialogueSequence NightGuard;
    NightGuard.SequenceID = TEXT("night_guard");
    NightGuard.bIsRepeatable = true;
    
    FNarr_DialogueLine Night1;
    Night1.SpeakerName = TEXT("Night Guard");
    Night1.DialogueText = TEXT("The herds are restless tonight. Something big moves in the darkness beyond our fires.");
    Night1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778879914075_NightGuard.mp3");
    Night1.DisplayDuration = 4.0f;
    NightGuard.DialogueLines.Add(Night1);
    
    RegisterDialogueSequence(NightGuard);
    
    // Game Narrator Introduction
    FNarr_DialogueSequence GameIntro;
    GameIntro.SequenceID = TEXT("game_intro");
    GameIntro.bIsRepeatable = false;
    
    FNarr_DialogueLine Intro1;
    Intro1.SpeakerName = TEXT("Narrator");
    Intro1.DialogueText = TEXT("Welcome to the Cretaceous world, survivor. Here, every sunrise is earned through cunning and courage.");
    Intro1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778879919586_GameNarrator.mp3");
    Intro1.DisplayDuration = 5.0f;
    GameIntro.DialogueLines.Add(Intro1);
    
    RegisterDialogueSequence(GameIntro);
}

void UNarr_DialogueSystem::UpdateDialogueTimer(float DeltaTime)
{
    DialogueTimer += DeltaTime;
    
    if (bAutoAdvance)
    {
        const FNarr_DialogueLine& CurrentLine = GetCurrentDialogueLine();
        float RequiredTime = FMath::Max(CurrentLine.DisplayDuration, AutoAdvanceDelay);
        
        if (DialogueTimer >= RequiredTime)
        {
            AdvanceDialogue();
        }
    }
}

bool UNarr_DialogueSystem::CanAdvanceDialogue() const
{
    return CurrentState == ENarr_DialogueState::DisplayingText || CurrentState == ENarr_DialogueState::WaitingForChoice;
}