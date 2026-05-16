#include "DialogueSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ADialogueSystem::ADialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component for dialogue playback
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;
    
    // Initialize state
    bIsPlayingDialogue = false;
    DialogueTimer = 0.0f;
    
    // Set default audio properties
    AudioComponent->bAutoActivate = false;
    AudioComponent->SetVolumeMultiplier(1.0f);
}

void ADialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize character voice profiles with default prehistoric characters
    FNarr_CharacterVoiceProfile TribalElder;
    TribalElder.CharacterName = TEXT("TribalElder");
    TribalElder.VoiceDescription = TEXT("Wise elder with deep, weathered voice");
    TribalElder.PitchModifier = 0.9f;
    TribalElder.VolumeModifier = 1.1f;
    CharacterVoices.Add(TEXT("TribalElder"), TribalElder);
    
    FNarr_CharacterVoiceProfile ScoutWarrior;
    ScoutWarrior.CharacterName = TEXT("ScoutWarrior");
    ScoutWarrior.VoiceDescription = TEXT("Alert scout with urgent tone");
    ScoutWarrior.PitchModifier = 1.1f;
    ScoutWarrior.VolumeModifier = 1.0f;
    CharacterVoices.Add(TEXT("ScoutWarrior"), ScoutWarrior);
    
    FNarr_CharacterVoiceProfile ExperiencedHunter;
    ExperiencedHunter.CharacterName = TEXT("ExperiencedHunter");
    ExperiencedHunter.VoiceDescription = TEXT("Seasoned hunter with calm authority");
    ExperiencedHunter.PitchModifier = 1.0f;
    ExperiencedHunter.VolumeModifier = 1.0f;
    CharacterVoices.Add(TEXT("ExperiencedHunter"), ExperiencedHunter);
    
    FNarr_CharacterVoiceProfile NightWatchman;
    NightWatchman.CharacterName = TEXT("NightWatchman");
    NightWatchman.VoiceDescription = TEXT("Vigilant guard with hushed warnings");
    NightWatchman.PitchModifier = 0.95f;
    NightWatchman.VolumeModifier = 0.9f;
    CharacterVoices.Add(TEXT("NightWatchman"), NightWatchman);
}

void ADialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsPlayingDialogue)
    {
        DialogueTimer += DeltaTime;
        
        // Check if dialogue duration has elapsed
        if (DialogueTimer >= CurrentDialogue.Duration)
        {
            OnDialogueFinished();
        }
    }
}

bool ADialogueSystem::PlayDialogue(const FString& DialogueID)
{
    if (bIsPlayingDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already playing, stopping current dialogue"));
        StopDialogue();
    }
    
    FNarr_DialogueEntry* DialogueEntry = FindDialogueEntry(DialogueID);
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue ID not found: %s"), *DialogueID);
        return false;
    }
    
    CurrentDialogue = *DialogueEntry;
    bIsPlayingDialogue = true;
    DialogueTimer = 0.0f;
    
    // Apply voice profile if available
    ApplyVoiceProfile(CurrentDialogue.SpeakerName);
    
    // Play audio if available
    if (CurrentDialogue.VoiceAudio.IsValid())
    {
        USoundWave* SoundWave = CurrentDialogue.VoiceAudio.LoadSynchronous();
        if (SoundWave && AudioComponent)
        {
            AudioComponent->SetSound(SoundWave);
            AudioComponent->Play();
        }
    }
    
    // Trigger blueprint event
    OnDialogueStarted(CurrentDialogue);
    
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: %s - %s"), *CurrentDialogue.SpeakerName, *CurrentDialogue.DialogueText);
    return true;
}

void ADialogueSystem::StopDialogue()
{
    if (bIsPlayingDialogue)
    {
        bIsPlayingDialogue = false;
        DialogueTimer = 0.0f;
        
        if (AudioComponent && AudioComponent->IsPlaying())
        {
            AudioComponent->Stop();
        }
        
        OnDialogueCompleted(CurrentDialogue);
        UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
    }
}

bool ADialogueSystem::IsDialogueActive() const
{
    return bIsPlayingDialogue;
}

FNarr_DialogueEntry ADialogueSystem::GetCurrentDialogue() const
{
    return CurrentDialogue;
}

TArray<FString> ADialogueSystem::GetAvailableDialogueOptions() const
{
    if (bIsPlayingDialogue)
    {
        return CurrentDialogue.NextDialogueOptions;
    }
    return TArray<FString>();
}

void ADialogueSystem::RegisterCharacterVoice(const FString& CharacterName, const FNarr_CharacterVoiceProfile& VoiceProfile)
{
    CharacterVoices.Add(CharacterName, VoiceProfile);
    UE_LOG(LogTemp, Log, TEXT("Registered voice profile for character: %s"), *CharacterName);
}

bool ADialogueSystem::PlayCharacterLine(const FString& CharacterName, const FString& LineText, USoundWave* AudioClip)
{
    // Create temporary dialogue entry
    FNarr_DialogueEntry TempDialogue;
    TempDialogue.DialogueID = FString::Printf(TEXT("temp_%s_%d"), *CharacterName, FMath::Rand());
    TempDialogue.SpeakerName = CharacterName;
    TempDialogue.DialogueText = LineText;
    TempDialogue.Duration = AudioClip ? AudioClip->GetDuration() : 5.0f;
    
    if (AudioClip)
    {
        TempDialogue.VoiceAudio = AudioClip;
    }
    
    CurrentDialogue = TempDialogue;
    bIsPlayingDialogue = true;
    DialogueTimer = 0.0f;
    
    ApplyVoiceProfile(CharacterName);
    
    if (AudioClip && AudioComponent)
    {
        AudioComponent->SetSound(AudioClip);
        AudioComponent->Play();
    }
    
    OnDialogueStarted(CurrentDialogue);
    UE_LOG(LogTemp, Log, TEXT("Playing character line: %s - %s"), *CharacterName, *LineText);
    return true;
}

void ADialogueSystem::TriggerNarrativeEvent(const FString& EventID)
{
    OnNarrativeEventTriggered(EventID);
    UE_LOG(LogTemp, Log, TEXT("Narrative event triggered: %s"), *EventID);
}

void ADialogueSystem::OnDialogueFinished()
{
    if (bIsPlayingDialogue)
    {
        bIsPlayingDialogue = false;
        DialogueTimer = 0.0f;
        
        OnDialogueCompleted(CurrentDialogue);
        UE_LOG(LogTemp, Log, TEXT("Dialogue finished: %s"), *CurrentDialogue.DialogueID);
    }
}

FNarr_DialogueEntry* ADialogueSystem::FindDialogueEntry(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue data table assigned"));
        return nullptr;
    }
    
    // Try to find dialogue entry in data table
    FNarr_DialogueEntry* FoundEntry = DialogueDataTable->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT("FindDialogueEntry"));
    return FoundEntry;
}

void ADialogueSystem::ApplyVoiceProfile(const FString& CharacterName)
{
    if (CharacterVoices.Contains(CharacterName) && AudioComponent)
    {
        const FNarr_CharacterVoiceProfile& Profile = CharacterVoices[CharacterName];
        AudioComponent->SetPitchMultiplier(Profile.PitchModifier);
        AudioComponent->SetVolumeMultiplier(Profile.VolumeModifier);
        
        UE_LOG(LogTemp, Log, TEXT("Applied voice profile for %s: Pitch=%.2f, Volume=%.2f"), 
               *CharacterName, Profile.PitchModifier, Profile.VolumeModifier);
    }
}