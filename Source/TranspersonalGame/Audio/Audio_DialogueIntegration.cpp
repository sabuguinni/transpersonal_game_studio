#include "Audio_DialogueIntegration.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"
#include "ActiveSound.h"

UAudio_DialogueIntegration::UAudio_DialogueIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

    // Initialize dialogue state
    bIsPlayingDialogue = false;
    CurrentDialogueTime = 0.0f;
    CurrentDialogueDuration = 0.0f;
    DialogueStartTime = 0.0f;
    bDialogueAudioLoaded = false;

    // Initialize audio mixing settings
    DialogueVolumeMultiplier = 1.0f;
    BackgroundMusicDucking = 0.3f;
    EnvironmentSoundDucking = 0.5f;

    // Create audio components
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
    NarrationAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudioComponent"));
}

void UAudio_DialogueIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    SetupVoiceProfiles();
}

void UAudio_DialogueIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsPlayingDialogue)
    {
        UpdateDialoguePlayback(DeltaTime);
    }
}

void UAudio_DialogueIntegration::InitializeAudioComponents()
{
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->bAutoActivate = false;
        DialogueAudioComponent->SetVolumeMultiplier(DialogueVolumeMultiplier);
        DialogueAudioComponent->bAllowSpatialization = true;
        DialogueAudioComponent->bOverrideAttenuation = false;
    }

    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->bAutoActivate = false;
        NarrationAudioComponent->SetVolumeMultiplier(DialogueVolumeMultiplier);
        NarrationAudioComponent->bAllowSpatialization = false; // Narration is typically non-spatial
        NarrationAudioComponent->bOverrideAttenuation = true;
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Audio components initialized"));
}

void UAudio_DialogueIntegration::SetupVoiceProfiles()
{
    // Create voice profiles for narrative characters
    FAudio_VoiceProfile NarrativeGuideProfile;
    NarrativeGuideProfile.CharacterName = TEXT("NarrativeGuide");
    NarrativeGuideProfile.BasePitch = 0.95f;
    NarrativeGuideProfile.BaseVolume = 1.2f;
    NarrativeGuideProfile.bUseDistanceAttenuation = false;
    VoiceProfiles.Add(TEXT("NarrativeGuide"), NarrativeGuideProfile);

    FAudio_VoiceProfile TribalElderProfile;
    TribalElderProfile.CharacterName = TEXT("TribalElder");
    TribalElderProfile.BasePitch = 0.85f;
    TribalElderProfile.BaseVolume = 1.0f;
    TribalElderProfile.bUseDistanceAttenuation = true;
    VoiceProfiles.Add(TEXT("TribalElder"), TribalElderProfile);

    FAudio_VoiceProfile WiseShamanProfile;
    WiseShamanProfile.CharacterName = TEXT("WiseShaman");
    WiseShamanProfile.BasePitch = 0.9f;
    WiseShamanProfile.BaseVolume = 1.1f;
    WiseShamanProfile.bUseDistanceAttenuation = true;
    VoiceProfiles.Add(TEXT("WiseShaman"), WiseShamanProfile);

    FAudio_VoiceProfile PlayerCharacterProfile;
    PlayerCharacterProfile.CharacterName = TEXT("PlayerCharacter");
    PlayerCharacterProfile.BasePitch = 1.0f;
    PlayerCharacterProfile.BaseVolume = 0.9f;
    PlayerCharacterProfile.bUseDistanceAttenuation = false;
    VoiceProfiles.Add(TEXT("PlayerCharacter"), PlayerCharacterProfile);

    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Voice profiles initialized"));
}

bool UAudio_DialogueIntegration::PlayDialogueAudio(const FString& SpeakerName, const FString& DialogueID)
{
    if (!DialogueAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_DialogueIntegration: DialogueAudioComponent is null"));
        return false;
    }

    // Stop any currently playing dialogue
    StopDialogueAudio();

    // Get voice profile for speaker
    FAudio_VoiceProfile* VoiceProfile = VoiceProfiles.Find(SpeakerName);
    if (!VoiceProfile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_DialogueIntegration: No voice profile found for speaker: %s"), *SpeakerName);
        return false;
    }

    // Get dialogue audio data
    FAudio_DialogueAudioData* AudioData = VoiceProfile->DialogueAudioMap.Find(DialogueID);
    if (!AudioData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_DialogueIntegration: No audio data found for dialogue: %s"), *DialogueID);
        return false;
    }

    // Load and play the audio
    USoundWave* SoundWave = AudioData->AudioClip.LoadSynchronous();
    if (!SoundWave)
    {
        UE_LOG(LogTemp, Error, TEXT("Audio_DialogueIntegration: Failed to load sound wave for dialogue: %s"), *DialogueID);
        return false;
    }

    // Configure audio component
    DialogueAudioComponent->SetSound(SoundWave);
    DialogueAudioComponent->SetVolumeMultiplier(AudioData->Volume * VoiceProfile->BaseVolume);
    DialogueAudioComponent->SetPitchMultiplier(VoiceProfile->BasePitch);

    // Set spatial audio properties
    if (AudioData->bIs3D && VoiceProfile->bUseDistanceAttenuation)
    {
        DialogueAudioComponent->bAllowSpatialization = true;
        DialogueAudioComponent->bOverrideAttenuation = false;
    }
    else
    {
        DialogueAudioComponent->bAllowSpatialization = false;
        DialogueAudioComponent->bOverrideAttenuation = true;
    }

    // Start playback
    DialogueAudioComponent->Play();

    // Update state
    bIsPlayingDialogue = true;
    CurrentSpeaker = SpeakerName;
    CurrentDialogueDuration = AudioData->Duration;
    CurrentDialogueTime = 0.0f;
    DialogueStartTime = GetWorld()->GetTimeSeconds();

    // Apply audio ducking
    ApplyAudioDucking();

    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Playing dialogue audio for %s: %s"), *SpeakerName, *DialogueID);
    return true;
}

void UAudio_DialogueIntegration::StopDialogueAudio()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }

    if (NarrationAudioComponent && NarrationAudioComponent->IsPlaying())
    {
        NarrationAudioComponent->Stop();
    }

    bIsPlayingDialogue = false;
    CurrentSpeaker.Empty();
    CurrentDialogueTime = 0.0f;
    CurrentDialogueDuration = 0.0f;

    // Restore background audio
    RemoveAudioDucking();

    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Dialogue audio stopped"));
}

void UAudio_DialogueIntegration::PauseDialogueAudio()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->SetPaused(true);
    }

    if (NarrationAudioComponent && NarrationAudioComponent->IsPlaying())
    {
        NarrationAudioComponent->SetPaused(true);
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Dialogue audio paused"));
}

void UAudio_DialogueIntegration::ResumeDialogueAudio()
{
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->SetPaused(false);
    }

    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->SetPaused(false);
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Dialogue audio resumed"));
}

void UAudio_DialogueIntegration::RegisterVoiceProfile(const FString& CharacterName, const FAudio_VoiceProfile& VoiceProfile)
{
    VoiceProfiles.Add(CharacterName, VoiceProfile);
    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Registered voice profile for %s"), *CharacterName);
}

bool UAudio_DialogueIntegration::GetVoiceProfile(const FString& CharacterName, FAudio_VoiceProfile& OutVoiceProfile)
{
    FAudio_VoiceProfile* Profile = VoiceProfiles.Find(CharacterName);
    if (Profile)
    {
        OutVoiceProfile = *Profile;
        return true;
    }
    return false;
}

void UAudio_DialogueIntegration::AddDialogueAudioToProfile(const FString& CharacterName, const FString& DialogueID, const FAudio_DialogueAudioData& AudioData)
{
    FAudio_VoiceProfile* Profile = VoiceProfiles.Find(CharacterName);
    if (Profile)
    {
        Profile->DialogueAudioMap.Add(DialogueID, AudioData);
        UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Added dialogue audio %s to profile %s"), *DialogueID, *CharacterName);
    }
}

void UAudio_DialogueIntegration::SetDialogueVolume(float Volume)
{
    DialogueVolumeMultiplier = FMath::Clamp(Volume, 0.0f, 2.0f);
    
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->SetVolumeMultiplier(DialogueVolumeMultiplier);
    }
    
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->SetVolumeMultiplier(DialogueVolumeMultiplier);
    }
}

void UAudio_DialogueIntegration::DuckBackgroundAudio(float DuckingAmount)
{
    BackgroundMusicDucking = FMath::Clamp(DuckingAmount, 0.0f, 1.0f);
    ApplyAudioDucking();
}

void UAudio_DialogueIntegration::RestoreBackgroundAudio()
{
    RemoveAudioDucking();
}

void UAudio_DialogueIntegration::OnDialogueStarted(const FString& SpeakerName, const FString& DialogueText)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Dialogue started - Speaker: %s"), *SpeakerName);
    
    // This would typically trigger audio playback based on the dialogue system
    // For now, we log the event
    CurrentSpeaker = SpeakerName;
}

void UAudio_DialogueIntegration::OnDialogueFinished()
{
    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Dialogue finished"));
    StopDialogueAudio();
}

void UAudio_DialogueIntegration::OnDialogueNodeChanged(const FString& NewSpeaker, const FString& NewDialogueID)
{
    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Dialogue node changed - Speaker: %s, ID: %s"), *NewSpeaker, *NewDialogueID);
    
    // Play the new dialogue audio
    PlayDialogueAudio(NewSpeaker, NewDialogueID);
}

void UAudio_DialogueIntegration::UpdateSpatialAudioPosition(const FVector& NewPosition)
{
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->SetWorldLocation(NewPosition);
    }
}

void UAudio_DialogueIntegration::SetSpatialAudioAttenuation(float MaxDistance, float FalloffDistance)
{
    // This would configure attenuation settings for spatial dialogue audio
    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Set spatial audio attenuation - Max: %f, Falloff: %f"), MaxDistance, FalloffDistance);
}

void UAudio_DialogueIntegration::UpdateDialoguePlayback(float DeltaTime)
{
    CurrentDialogueTime += DeltaTime;
    
    // Check if dialogue has finished
    if (CurrentDialogueTime >= CurrentDialogueDuration)
    {
        OnDialogueFinished();
    }
}

bool UAudio_DialogueIntegration::LoadDialogueAudio(const FString& AudioPath, USoundWave*& OutSoundWave)
{
    OutSoundWave = LoadObject<USoundWave>(nullptr, *AudioPath);
    return OutSoundWave != nullptr;
}

void UAudio_DialogueIntegration::ApplyAudioDucking()
{
    // This would integrate with the main audio system to duck background music and environment sounds
    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Applying audio ducking - Music: %f, Environment: %f"), BackgroundMusicDucking, EnvironmentSoundDucking);
}

void UAudio_DialogueIntegration::RemoveAudioDucking()
{
    // This would restore normal volume levels for background audio
    UE_LOG(LogTemp, Log, TEXT("Audio_DialogueIntegration: Removing audio ducking"));
}