#include "Audio_NarrativeAudioManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

AAudio_NarrativeAudioManager::AAudio_NarrativeAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component for narrative playback
    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudioComponent"));
    RootComponent = NarrativeAudioComponent;

    // Initialize default values
    MasterNarrativeVolume = 0.8f;
    VoicelineAttenuation = 0.5f;
    MaxAudibleDistance = 2000.0f;
    bIsPlayingVoiceLine = false;
    CurrentVoiceLineTimer = 0.0f;
    CurrentCharacterSpeaking = TEXT("");

    // Setup audio component properties
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->bAutoActivate = false;
        NarrativeAudioComponent->VolumeMultiplier = MasterNarrativeVolume;
        NarrativeAudioComponent->AttenuationSettings = nullptr; // Will be set dynamically
    }

    // Initialize character voices with generated samples
    FAudio_CharacterVoiceData ThaneVoice;
    ThaneVoice.CharacterName = TEXT("Thane");
    ThaneVoice.VoiceURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1780053363055_NarratorSurvival.mp3");
    ThaneVoice.Duration = 15.0f;
    ThaneVoice.PersonalityTrait = EAudio_PersonalityTrait::Wise;
    CharacterVoices.Add(TEXT("Thane"), ThaneVoice);

    FAudio_CharacterVoiceData DangerVoice;
    DangerVoice.CharacterName = TEXT("DangerAlert");
    DangerVoice.VoiceURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1780053367668_DangerAlert.mp3");
    DangerVoice.Duration = 11.0f;
    DangerVoice.PersonalityTrait = EAudio_PersonalityTrait::Aggressive;
    CharacterVoices.Add(TEXT("DangerAlert"), DangerVoice);
}

void AAudio_NarrativeAudioManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Audio_NarrativeAudioManager initialized with %d character voices"), CharacterVoices.Num());

    // Setup default narrative events
    FAudio_NarrativeEvent WelcomeEvent;
    WelcomeEvent.EventName = TEXT("PlayerEnterWorld");
    WelcomeEvent.TriggerRadius = 1000.0f;
    WelcomeEvent.bIsOneShot = true;
    
    if (CharacterVoices.Contains(TEXT("Thane")))
    {
        WelcomeEvent.VoiceLines.Add(CharacterVoices[TEXT("Thane")]);
    }
    
    NarrativeEvents.Add(WelcomeEvent);

    FAudio_NarrativeEvent DangerEvent;
    DangerEvent.EventName = TEXT("TRexNearby");
    DangerEvent.TriggerRadius = 1500.0f;
    DangerEvent.bIsOneShot = false;
    
    if (CharacterVoices.Contains(TEXT("DangerAlert")))
    {
        DangerEvent.VoiceLines.Add(CharacterVoices[TEXT("DangerAlert")]);
    }
    
    NarrativeEvents.Add(DangerEvent);
}

void AAudio_NarrativeAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateVoiceLinePlayback(DeltaTime);
}

void AAudio_NarrativeAudioManager::UpdateVoiceLinePlayback(float DeltaTime)
{
    if (bIsPlayingVoiceLine && CurrentVoiceLineTimer > 0.0f)
    {
        CurrentVoiceLineTimer -= DeltaTime;
        
        if (CurrentVoiceLineTimer <= 0.0f)
        {
            OnVoiceLineComplete();
        }
    }
}

void AAudio_NarrativeAudioManager::OnVoiceLineComplete()
{
    bIsPlayingVoiceLine = false;
    CurrentVoiceLineTimer = 0.0f;
    CurrentCharacterSpeaking = TEXT("");
    
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Voice line playback completed"));
}

void AAudio_NarrativeAudioManager::PlayCharacterVoiceLine(const FString& CharacterName, const FString& VoiceURL)
{
    if (!CanPlayVoiceLine(CharacterName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play voice line for character: %s"), *CharacterName);
        return;
    }

    if (CharacterVoices.Contains(CharacterName))
    {
        const FAudio_CharacterVoiceData& VoiceData = CharacterVoices[CharacterName];
        
        // Stop current voice line if playing
        if (bIsPlayingVoiceLine)
        {
            StopCurrentVoiceLine();
        }
        
        // Start new voice line
        bIsPlayingVoiceLine = true;
        CurrentVoiceLineTimer = VoiceData.Duration;
        CurrentCharacterSpeaking = CharacterName;
        
        UE_LOG(LogTemp, Warning, TEXT("Playing voice line for %s: %s (Duration: %.1fs)"), 
               *CharacterName, *VoiceData.VoiceURL, VoiceData.Duration);
        
        // In a real implementation, we would load and play the audio from URL
        // For now, we simulate playback with the timer system
        if (NarrativeAudioComponent)
        {
            NarrativeAudioComponent->SetVolumeMultiplier(MasterNarrativeVolume);
            // Note: Actual audio loading from URL would require custom implementation
        }
    }
}

void AAudio_NarrativeAudioManager::TriggerNarrativeEvent(const FString& EventName, FVector PlayerLocation)
{
    for (const FAudio_NarrativeEvent& Event : NarrativeEvents)
    {
        if (Event.EventName == EventName)
        {
            float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerLocation);
            
            if (DistanceToPlayer <= Event.TriggerRadius)
            {
                if (Event.VoiceLines.Num() > 0)
                {
                    // Play random voice line from event
                    int32 RandomIndex = FMath::RandRange(0, Event.VoiceLines.Num() - 1);
                    const FAudio_CharacterVoiceData& VoiceLine = Event.VoiceLines[RandomIndex];
                    
                    PlayCharacterVoiceLine(VoiceLine.CharacterName, VoiceLine.VoiceURL);
                    
                    UE_LOG(LogTemp, Warning, TEXT("Triggered narrative event: %s"), *EventName);
                }
            }
            break;
        }
    }
}

void AAudio_NarrativeAudioManager::RegisterCharacterVoice(const FString& CharacterName, const FString& VoiceURL, float Duration, EAudio_PersonalityTrait Trait)
{
    FAudio_CharacterVoiceData NewVoiceData;
    NewVoiceData.CharacterName = CharacterName;
    NewVoiceData.VoiceURL = VoiceURL;
    NewVoiceData.Duration = Duration;
    NewVoiceData.PersonalityTrait = Trait;
    
    CharacterVoices.Add(CharacterName, NewVoiceData);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered voice for character: %s"), *CharacterName);
}

bool AAudio_NarrativeAudioManager::IsVoiceLineActive() const
{
    return bIsPlayingVoiceLine;
}

void AAudio_NarrativeAudioManager::StopCurrentVoiceLine()
{
    if (bIsPlayingVoiceLine)
    {
        OnVoiceLineComplete();
    }
}

void AAudio_NarrativeAudioManager::SetNarrativeVolume(float Volume)
{
    MasterNarrativeVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(MasterNarrativeVolume);
    }
}

FAudio_CharacterVoiceData AAudio_NarrativeAudioManager::GetCharacterVoiceData(const FString& CharacterName) const
{
    if (CharacterVoices.Contains(CharacterName))
    {
        return CharacterVoices[CharacterName];
    }
    
    return FAudio_CharacterVoiceData();
}

TArray<FString> AAudio_NarrativeAudioManager::GetAvailableCharacterVoices() const
{
    TArray<FString> VoiceNames;
    CharacterVoices.GetKeys(VoiceNames);
    return VoiceNames;
}

bool AAudio_NarrativeAudioManager::CanPlayVoiceLine(const FString& CharacterName) const
{
    // Don't interrupt if same character is already speaking
    if (bIsPlayingVoiceLine && CurrentCharacterSpeaking == CharacterName)
    {
        return false;
    }
    
    // Check if character voice exists
    return CharacterVoices.Contains(CharacterName);
}