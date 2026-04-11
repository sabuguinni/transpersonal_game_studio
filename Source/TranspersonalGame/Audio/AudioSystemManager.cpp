#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundMix.h"
#include "AudioDevice.h"

UAudioSystemManager::UAudioSystemManager()
{
    // Initialize default values
    CurrentMusicState = "Silence";
    CurrentConsciousnessLevel = 0.0f;
    MasterVolumeLevel = 1.0f;
    MusicTransitionTimer = 0.0f;
    ConsciousnessFilterIntensity = 0.0f;
    bIsTransitioning = false;
    PendingMusicState = "";
    PendingFadeTime = 2.0f;
}

void UAudioSystemManager::InitializeAudioSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Initializing Audio System..."));

    // Create audio components for different audio layers
    if (!MusicAudioComponent)
    {
        MusicAudioComponent = NewObject<UAudioComponent>(this);
        if (MusicAudioComponent)
        {
            MusicAudioComponent->SetVolumeMultiplier(0.7f); // Music slightly lower than SFX
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Music AudioComponent created"));
        }
    }

    if (!AmbienceAudioComponent)
    {
        AmbienceAudioComponent = NewObject<UAudioComponent>(this);
        if (AmbienceAudioComponent)
        {
            AmbienceAudioComponent->SetVolumeMultiplier(0.5f); // Ambience in background
            AmbienceAudioComponent->bAutoActivate = true;
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Ambience AudioComponent created"));
        }
    }

    if (!NarrationAudioComponent)
    {
        NarrationAudioComponent = NewObject<UAudioComponent>(this);
        if (NarrationAudioComponent)
        {
            NarrationAudioComponent->SetVolumeMultiplier(0.9f); // Narration prominent
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Narration AudioComponent created"));
        }
    }

    // Initialize default audio states
    CurrentMusicState = "Awakening";
    CurrentConsciousnessLevel = 0.1f; // Start slightly awakened

    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Audio System initialization complete"));
}

void UAudioSystemManager::UpdateAudioState(float DeltaTime)
{
    // Update adaptive music based on consciousness level
    UpdateAdaptiveMusic(DeltaTime);
    
    // Process consciousness-based audio effects
    ProcessConsciousnessAudio();
    
    // Handle any pending audio transitions
    HandleAudioTransitions();
}

void UAudioSystemManager::SetMasterVolume(float Volume)
{
    MasterVolumeLevel = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Apply to all audio components
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(0.7f * MasterVolumeLevel);
    }
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(0.5f * MasterVolumeLevel);
    }
    
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->SetVolumeMultiplier(0.9f * MasterVolumeLevel);
    }

    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Master volume set to %f"), MasterVolumeLevel);
}

void UAudioSystemManager::TransitionToMusicState(const FString& NewState, float FadeTime)
{
    if (NewState == CurrentMusicState)
    {
        return; // Already in this state
    }

    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Transitioning from %s to %s (fade: %fs)"), 
           *CurrentMusicState, *NewState, FadeTime);

    PendingMusicState = NewState;
    PendingFadeTime = FadeTime;
    bIsTransitioning = true;
    MusicTransitionTimer = 0.0f;

    // Start fade out of current music
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(FadeTime * 0.5f, 0.0f);
    }
}

void UAudioSystemManager::SetConsciousnessLevel(float Level)
{
    float PreviousLevel = CurrentConsciousnessLevel;
    CurrentConsciousnessLevel = FMath::Clamp(Level, 0.0f, 1.0f);

    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Consciousness level changed from %f to %f"), 
           PreviousLevel, CurrentConsciousnessLevel);

    // Trigger music state changes based on consciousness thresholds
    if (CurrentConsciousnessLevel < 0.3f && PreviousLevel >= 0.3f)
    {
        TransitionToMusicState("Dormant", 3.0f);
    }
    else if (CurrentConsciousnessLevel >= 0.3f && CurrentConsciousnessLevel < 0.7f)
    {
        TransitionToMusicState("Awakening", 2.5f);
    }
    else if (CurrentConsciousnessLevel >= 0.7f && PreviousLevel < 0.7f)
    {
        TransitionToMusicState("Enlightened", 4.0f);
    }
}

void UAudioSystemManager::TriggerSpiritualMoment(const FString& MomentType)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Triggering spiritual moment: %s"), *MomentType);

    // Play specific audio cues for different spiritual moments
    if (MomentType == "VisionQuest")
    {
        ApplyConsciousnessFilter(0.8f);
        TransitionToMusicState("Transcendent", 1.5f);
    }
    else if (MomentType == "AncientWisdom")
    {
        TriggerAncientWisdomWhisper(FVector::ZeroVector);
    }
    else if (MomentType == "SoulConnection")
    {
        ApplyConsciousnessFilter(0.6f);
    }
}

void UAudioSystemManager::PlaySoundAtLocation(USoundBase* Sound, FVector Location, float Volume)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Attempted to play null sound"));
        return;
    }

    UWorld* World = GetWorld();
    if (World)
    {
        UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, Volume * MasterVolumeLevel);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Played sound at location (%s)"), *Location.ToString());
    }
}

void UAudioSystemManager::SetEnvironmentAudioProfile(const FString& ProfileName)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Setting environment audio profile: %s"), *ProfileName);

    // Look for the environment profile in our asset map
    if (EnvironmentProfiles.Contains(ProfileName))
    {
        USoundBase* ProfileSound = EnvironmentProfiles[ProfileName];
        if (ProfileSound && AmbienceAudioComponent)
        {
            AmbienceAudioComponent->SetSound(ProfileSound);
            AmbienceAudioComponent->Play();
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Environment profile %s activated"), *ProfileName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Environment profile %s not found"), *ProfileName);
    }
}

void UAudioSystemManager::ApplyConsciousnessFilter(float Intensity)
{
    ConsciousnessFilterIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Applying consciousness filter with intensity %f"), 
           ConsciousnessFilterIntensity);

    // Apply ethereal audio processing effects
    // This would typically involve audio DSP effects, reverb, filtering
    // For now, we'll adjust volume and apply some basic effects
    
    float FilteredVolume = FMath::Lerp(1.0f, 0.7f, ConsciousnessFilterIntensity);
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(FilteredVolume * 0.7f * MasterVolumeLevel);
    }
}

void UAudioSystemManager::PlayNarrationVoice(const FString& NarrationKey, bool bInterruptCurrent)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Playing narration voice: %s"), *NarrationKey);

    if (bInterruptCurrent && NarrationAudioComponent && NarrationAudioComponent->IsPlaying())
    {
        NarrationAudioComponent->Stop();
    }

    if (NarrationVoices.Contains(NarrationKey))
    {
        USoundBase* NarrationSound = NarrationVoices[NarrationKey];
        if (NarrationSound && NarrationAudioComponent)
        {
            NarrationAudioComponent->SetSound(NarrationSound);
            NarrationAudioComponent->Play();
            UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Narration %s started"), *NarrationKey);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Narration key %s not found"), *NarrationKey);
    }
}

void UAudioSystemManager::TriggerAncientWisdomWhisper(FVector Location)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Triggering ancient wisdom whisper"));

    if (SpiritualWhisperSound)
    {
        PlaySoundAtLocation(SpiritualWhisperSound, Location, 0.8f);
        
        // Apply temporary consciousness filter for ethereal effect
        float PreviousIntensity = ConsciousnessFilterIntensity;
        ApplyConsciousnessFilter(0.9f);
        
        // TODO: Set timer to restore previous filter intensity after whisper ends
    }
}

void UAudioSystemManager::UpdateAdaptiveMusic(float DeltaTime)
{
    // Adaptive music logic based on consciousness level and game state
    // Music becomes more ethereal and complex as consciousness increases
    
    if (MusicAudioComponent)
    {
        // Adjust music parameters based on consciousness level
        float ConsciousnessVolume = FMath::Lerp(0.3f, 0.8f, CurrentConsciousnessLevel);
        float BaseVolume = 0.7f * MasterVolumeLevel * ConsciousnessVolume;
        
        MusicAudioComponent->SetVolumeMultiplier(BaseVolume);
    }
}

void UAudioSystemManager::ProcessConsciousnessAudio()
{
    // Process consciousness-based audio effects
    // Higher consciousness = more ethereal, spiritual audio processing
    
    if (ConsciousnessFilterIntensity > 0.0f)
    {
        // Gradually reduce filter intensity over time for natural decay
        ConsciousnessFilterIntensity = FMath::Max(0.0f, ConsciousnessFilterIntensity - 0.1f * GetWorld()->GetDeltaSeconds());
    }
}

void UAudioSystemManager::HandleAudioTransitions()
{
    if (!bIsTransitioning)
    {
        return;
    }

    MusicTransitionTimer += GetWorld()->GetDeltaSeconds();
    
    // Check if we're halfway through transition (time to start new music)
    if (MusicTransitionTimer >= PendingFadeTime * 0.5f && !PendingMusicState.IsEmpty())
    {
        CurrentMusicState = PendingMusicState;
        
        // Look for the new music state in our asset map
        if (MusicStates.Contains(CurrentMusicState))
        {
            USoundBase* NewMusic = MusicStates[CurrentMusicState];
            if (NewMusic && MusicAudioComponent)
            {
                MusicAudioComponent->SetSound(NewMusic);
                MusicAudioComponent->FadeIn(PendingFadeTime * 0.5f, 0.7f * MasterVolumeLevel);
                UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Started new music state: %s"), *CurrentMusicState);
            }
        }
        
        PendingMusicState = "";
    }
    
    // Check if transition is complete
    if (MusicTransitionTimer >= PendingFadeTime)
    {
        bIsTransitioning = false;
        MusicTransitionTimer = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Music transition complete"));
    }
}