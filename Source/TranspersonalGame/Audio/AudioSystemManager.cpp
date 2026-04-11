#include "AudioSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"

UAudioSystemManager::UAudioSystemManager()
{
    CurrentConsciousnessLevel = 0.0f;
    CurrentMusicIntensity = 0.5f;
    CurrentBiome = TEXT("Forest");
    CurrentMusicState = TEXT("Exploration");
    WorldContext = nullptr;
    MusicComponent = nullptr;
    AmbienceComponent = nullptr;
}

void UAudioSystemManager::InitializeAudioSystem(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Cannot initialize without valid world"));
        return;
    }

    WorldContext = World;

    // Create main audio components
    if (!MusicComponent)
    {
        MusicComponent = NewObject<UAudioComponent>(this);
        if (MusicComponent)
        {
            MusicComponent->bAutoActivate = false;
            MusicComponent->SetVolumeMultiplier(0.7f);
        }
    }

    if (!AmbienceComponent)
    {
        AmbienceComponent = NewObject<UAudioComponent>(this);
        if (AmbienceComponent)
        {
            AmbienceComponent->bAutoActivate = false;
            AmbienceComponent->SetVolumeMultiplier(0.5f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Audio system initialized successfully"));
}

void UAudioSystemManager::UpdateAudioState(float DeltaTime)
{
    if (!WorldContext)
    {
        return;
    }

    // Process active audio fades
    ProcessAudioFades(DeltaTime);

    // Update consciousness-based audio filters
    UpdateConsciousnessAudioFilters();

    // Update spatial audio parameters
    CalculateSpatialAudioParameters();
}

void UAudioSystemManager::SetConsciousnessLevel(float Level)
{
    CurrentConsciousnessLevel = FMath::Clamp(Level, 0.0f, 1.0f);

    // Trigger consciousness-based audio changes
    FString ConsciousnessState;
    if (CurrentConsciousnessLevel < 0.3f)
    {
        ConsciousnessState = TEXT("Mundane");
    }
    else if (CurrentConsciousnessLevel < 0.7f)
    {
        ConsciousnessState = TEXT("Awakening");
    }
    else
    {
        ConsciousnessState = TEXT("Transcendent");
    }

    // Apply consciousness-based audio processing
    if (USoundBase** ConsciousnessAudio = ConsciousnessAudioStates.Find(ConsciousnessState))
    {
        if (*ConsciousnessAudio && AmbienceComponent)
        {
            AmbienceComponent->SetSound(*ConsciousnessAudio);
            if (!AmbienceComponent->IsPlaying())
            {
                AmbienceComponent->Play();
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Consciousness level set to %f, state: %s"), 
           CurrentConsciousnessLevel, *ConsciousnessState);
}

void UAudioSystemManager::TriggerTranscendenceAudio()
{
    // Special audio event for transcendence moments
    if (USoundBase** TranscendenceSound = ConsciousnessAudioStates.Find(TEXT("Transcendence")))
    {
        if (*TranscendenceSound && WorldContext)
        {
            UGameplayStatics::PlaySound2D(WorldContext, *TranscendenceSound, 0.8f);
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transcendence audio triggered"));
        }
    }
}

void UAudioSystemManager::TransitionToMusicState(const FString& StateName, float FadeTime)
{
    if (StateName == CurrentMusicState)
    {
        return; // Already in this state
    }

    USoundBase** NewMusicSound = MusicStates.Find(StateName);
    if (!NewMusicSound || !*NewMusicSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Music state '%s' not found"), *StateName);
        return;
    }

    // Fade out current music
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        FAudioFade FadeOut;
        FadeOut.Component = MusicComponent;
        FadeOut.StartVolume = MusicComponent->VolumeMultiplier;
        FadeOut.TargetVolume = 0.0f;
        FadeOut.FadeTime = FadeTime * 0.5f; // Fade out faster
        FadeOut.ElapsedTime = 0.0f;
        ActiveFades.Add(FadeOut);
    }

    // Start new music after fade
    CurrentMusicState = StateName;
    if (MusicComponent)
    {
        MusicComponent->SetSound(*NewMusicSound);
        MusicComponent->Play();

        // Fade in new music
        FAudioFade FadeIn;
        FadeIn.Component = MusicComponent;
        FadeIn.StartVolume = 0.0f;
        FadeIn.TargetVolume = 0.7f * CurrentMusicIntensity;
        FadeIn.FadeTime = FadeTime;
        FadeIn.ElapsedTime = 0.0f;
        ActiveFades.Add(FadeIn);
        
        MusicComponent->SetVolumeMultiplier(0.0f); // Start silent
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Transitioning to music state '%s'"), *StateName);
}

void UAudioSystemManager::SetMusicIntensity(float Intensity)
{
    CurrentMusicIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (MusicComponent)
    {
        float TargetVolume = 0.7f * CurrentMusicIntensity;
        MusicComponent->SetVolumeMultiplier(TargetVolume);
    }
}

void UAudioSystemManager::UpdateEnvironmentalAudio(const FVector& PlayerLocation)
{
    // Update environmental audio based on player location
    // This would integrate with the world generation system to detect biomes
    
    // For now, simple distance-based environmental audio
    if (AmbienceComponent && WorldContext)
    {
        // Calculate environmental factors
        float EnvironmentalIntensity = 1.0f; // Base intensity
        
        // Adjust ambience volume based on environmental factors
        AmbienceComponent->SetVolumeMultiplier(0.5f * EnvironmentalIntensity);
    }
}

void UAudioSystemManager::SetBiomeAudio(const FString& BiomeName)
{
    if (BiomeName == CurrentBiome)
    {
        return; // Already in this biome
    }

    USoundBase** BiomeAmbience = BiomeAmbiences.Find(BiomeName);
    if (!BiomeAmbience || !*BiomeAmbience)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Biome ambience '%s' not found"), *BiomeName);
        return;
    }

    CurrentBiome = BiomeName;
    
    if (AmbienceComponent)
    {
        AmbienceComponent->SetSound(*BiomeAmbience);
        if (!AmbienceComponent->IsPlaying())
        {
            AmbienceComponent->Play();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Set biome audio to '%s'"), *BiomeName);
}

void UAudioSystemManager::RegisterSpatialAudioSource(AActor* SourceActor, USoundBase* Sound)
{
    if (!SourceActor || !Sound)
    {
        return;
    }

    // Create audio component for spatial source
    UAudioComponent* SpatialComponent = NewObject<UAudioComponent>(SourceActor);
    if (SpatialComponent)
    {
        SpatialComponent->SetSound(Sound);
        SpatialComponent->bAutoActivate = true;
        SpatialComponent->AttachToComponent(SourceActor->GetRootComponent(), 
                                          FAttachmentTransformRules::KeepWorldTransform);
        
        ActiveAudioComponents.Add(SpatialComponent);
        SpatialComponent->Play();
    }
}

void UAudioSystemManager::UpdateSpatialAudioAttenuation()
{
    // Update attenuation settings for all spatial audio sources
    for (UAudioComponent* Component : ActiveAudioComponents)
    {
        if (Component && Component->IsValidLowLevel())
        {
            // Apply consciousness-based audio filtering to spatial sources
            float ConsciousnessModifier = 1.0f + (CurrentConsciousnessLevel * 0.5f);
            Component->SetVolumeMultiplier(Component->VolumeMultiplier * ConsciousnessModifier);
        }
    }
}

void UAudioSystemManager::ProcessAudioFades(float DeltaTime)
{
    for (int32 i = ActiveFades.Num() - 1; i >= 0; i--)
    {
        FAudioFade& Fade = ActiveFades[i];
        
        if (!Fade.Component || !Fade.Component->IsValidLowLevel())
        {
            ActiveFades.RemoveAt(i);
            continue;
        }

        Fade.ElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(Fade.ElapsedTime / Fade.FadeTime, 0.0f, 1.0f);
        float CurrentVolume = FMath::Lerp(Fade.StartVolume, Fade.TargetVolume, Alpha);
        
        Fade.Component->SetVolumeMultiplier(CurrentVolume);

        if (Alpha >= 1.0f)
        {
            // Fade complete
            if (Fade.TargetVolume <= 0.0f)
            {
                Fade.Component->Stop();
            }
            ActiveFades.RemoveAt(i);
        }
    }
}

void UAudioSystemManager::UpdateConsciousnessAudioFilters()
{
    // Apply consciousness-based audio processing
    // Higher consciousness levels could add reverb, delay, or frequency modulation
    
    if (MusicComponent)
    {
        // Example: Add subtle pitch modulation based on consciousness level
        float PitchModulation = 1.0f + (CurrentConsciousnessLevel * 0.1f);
        MusicComponent->SetPitchMultiplier(PitchModulation);
    }
}

void UAudioSystemManager::CalculateSpatialAudioParameters()
{
    // Calculate and apply spatial audio parameters
    // This would integrate with the player's position and consciousness state
    
    if (!WorldContext)
    {
        return;
    }

    // Get player location for spatial calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContext, 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        UpdateEnvironmentalAudio(PlayerLocation);
    }
}