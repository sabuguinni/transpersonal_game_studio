#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UAudio_SoundManager::UAudio_SoundManager()
{
    GlobalReverbAmount = 0.3f;
    GlobalDecayTime = 2.0f;
    CurrentAmbientLoop = nullptr;
}

void UAudio_SoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultVolumes();
    
    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CleanupTimerHandle, this, &UAudio_SoundManager::CleanupFinishedAudioComponents, 5.0f, true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager initialized successfully"));
}

void UAudio_SoundManager::Deinitialize()
{
    // Stop all active audio components
    for (auto& CategoryPair : ActiveAudioComponents)
    {
        for (UAudioComponent* AudioComp : CategoryPair.Value)
        {
            if (IsValid(AudioComp))
            {
                AudioComp->Stop();
            }
        }
    }
    
    ActiveAudioComponents.Empty();
    
    if (IsValid(CurrentAmbientLoop))
    {
        CurrentAmbientLoop->Stop();
        CurrentAmbientLoop = nullptr;
    }
    
    // Clear cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    Super::Deinitialize();
}

void UAudio_SoundManager::InitializeDefaultVolumes()
{
    CategoryVolumes.Add(EAudio_SoundCategory::Ambient, 0.7f);
    CategoryVolumes.Add(EAudio_SoundCategory::Music, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::SFX, 0.8f);
    CategoryVolumes.Add(EAudio_SoundCategory::Voice, 1.0f);
    CategoryVolumes.Add(EAudio_SoundCategory::UI, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Footsteps, 0.6f);
    CategoryVolumes.Add(EAudio_SoundCategory::Combat, 0.9f);
    CategoryVolumes.Add(EAudio_SoundCategory::Environment, 0.5f);
    CategoryVolumes.Add(EAudio_SoundCategory::Dinosaur, 1.0f);
}

UAudioComponent* UAudio_SoundManager::PlaySoundAtLocation(const FAudio_SoundConfig& SoundConfig, const FVector& Location, EAudio_SoundCategory Category)
{
    if (!SoundConfig.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Invalid SoundCue in PlaySoundAtLocation"));
        return nullptr;
    }
    
    UAudioComponent* AudioComponent = CreateAudioComponent(SoundConfig, Location, false);
    if (!AudioComponent)
    {
        return nullptr;
    }
    
    ApplyCategoryVolume(AudioComponent, Category);
    
    // Add to active components list
    if (!ActiveAudioComponents.Contains(Category))
    {
        ActiveAudioComponents.Add(Category, TArray<UAudioComponent*>());
    }
    ActiveAudioComponents[Category].Add(AudioComponent);
    
    AudioComponent->Play();
    return AudioComponent;
}

UAudioComponent* UAudio_SoundManager::PlaySound2D(const FAudio_SoundConfig& SoundConfig, EAudio_SoundCategory Category)
{
    if (!SoundConfig.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Invalid SoundCue in PlaySound2D"));
        return nullptr;
    }
    
    UAudioComponent* AudioComponent = CreateAudioComponent(SoundConfig, FVector::ZeroVector, true);
    if (!AudioComponent)
    {
        return nullptr;
    }
    
    ApplyCategoryVolume(AudioComponent, Category);
    
    // Add to active components list
    if (!ActiveAudioComponents.Contains(Category))
    {
        ActiveAudioComponents.Add(Category, TArray<UAudioComponent*>());
    }
    ActiveAudioComponents[Category].Add(AudioComponent);
    
    AudioComponent->Play();
    return AudioComponent;
}

void UAudio_SoundManager::StopAllSoundsInCategory(EAudio_SoundCategory Category)
{
    if (!ActiveAudioComponents.Contains(Category))
    {
        return;
    }
    
    for (UAudioComponent* AudioComp : ActiveAudioComponents[Category])
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    
    ActiveAudioComponents[Category].Empty();
}

void UAudio_SoundManager::SetCategoryVolume(EAudio_SoundCategory Category, float Volume)
{
    CategoryVolumes.Add(Category, FMath::Clamp(Volume, 0.0f, 1.0f));
    
    // Update all active audio components in this category
    if (ActiveAudioComponents.Contains(Category))
    {
        for (UAudioComponent* AudioComp : ActiveAudioComponents[Category])
        {
            if (IsValid(AudioComp))
            {
                ApplyCategoryVolume(AudioComp, Category);
            }
        }
    }
}

float UAudio_SoundManager::GetCategoryVolume(EAudio_SoundCategory Category) const
{
    if (CategoryVolumes.Contains(Category))
    {
        return CategoryVolumes[Category];
    }
    return 1.0f;
}

void UAudio_SoundManager::PlayAmbientLoop(const FAudio_SoundConfig& SoundConfig)
{
    // Stop current ambient loop
    if (IsValid(CurrentAmbientLoop))
    {
        CurrentAmbientLoop->Stop();
        CurrentAmbientLoop = nullptr;
    }
    
    if (!SoundConfig.SoundCue.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Invalid SoundCue in PlayAmbientLoop"));
        return;
    }
    
    // Create new ambient loop
    FAudio_SoundConfig LoopConfig = SoundConfig;
    LoopConfig.bLoop = true;
    
    CurrentAmbientLoop = CreateAudioComponent(LoopConfig, FVector::ZeroVector, true);
    if (CurrentAmbientLoop)
    {
        ApplyCategoryVolume(CurrentAmbientLoop, EAudio_SoundCategory::Ambient);
        CurrentAmbientLoop->Play();
    }
}

void UAudio_SoundManager::StopAmbientLoop()
{
    if (IsValid(CurrentAmbientLoop))
    {
        CurrentAmbientLoop->Stop();
        CurrentAmbientLoop = nullptr;
    }
}

void UAudio_SoundManager::CrossfadeAmbient(const FAudio_SoundConfig& NewSoundConfig, float CrossfadeTime)
{
    // For now, simple implementation - just switch
    // TODO: Implement proper crossfading with fade in/out
    StopAmbientLoop();
    PlayAmbientLoop(NewSoundConfig);
}

void UAudio_SoundManager::PlayDinosaurFootstep(EDinosaurSpecies Species, const FVector& Location, float Intensity)
{
    if (!DinosaurFootstepSounds.Contains(Species) || DinosaurFootstepSounds[Species].Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: No footstep sounds for dinosaur species %d"), (int32)Species);
        return;
    }
    
    // Select random footstep sound
    int32 RandomIndex = FMath::RandRange(0, DinosaurFootstepSounds[Species].Num() - 1);
    FAudio_SoundConfig FootstepConfig = DinosaurFootstepSounds[Species][RandomIndex];
    
    // Modify volume based on intensity
    FootstepConfig.Volume *= Intensity;
    
    PlaySoundAtLocation(FootstepConfig, Location, EAudio_SoundCategory::Footsteps);
}

void UAudio_SoundManager::PlayDinosaurVocalization(EDinosaurSpecies Species, const FVector& Location, ECreatureEmotionalState EmotionalState)
{
    if (!DinosaurVocalizationSounds.Contains(Species) || DinosaurVocalizationSounds[Species].Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: No vocalization sounds for dinosaur species %d"), (int32)Species);
        return;
    }
    
    // Select random vocalization sound
    int32 RandomIndex = FMath::RandRange(0, DinosaurVocalizationSounds[Species].Num() - 1);
    FAudio_SoundConfig VocalizationConfig = DinosaurVocalizationSounds[Species][RandomIndex];
    
    // Modify pitch based on emotional state
    switch (EmotionalState)
    {
        case ECreatureEmotionalState::Aggressive:
            VocalizationConfig.Pitch *= 1.2f;
            VocalizationConfig.Volume *= 1.3f;
            break;
        case ECreatureEmotionalState::Fearful:
            VocalizationConfig.Pitch *= 1.5f;
            VocalizationConfig.Volume *= 0.8f;
            break;
        case ECreatureEmotionalState::Calm:
            VocalizationConfig.Pitch *= 0.9f;
            VocalizationConfig.Volume *= 0.7f;
            break;
        default:
            break;
    }
    
    PlaySoundAtLocation(VocalizationConfig, Location, EAudio_SoundCategory::Dinosaur);
}

void UAudio_SoundManager::PlayScreenShakeAudio(const FVector& SourceLocation, float Intensity, float Duration)
{
    // Create a low-frequency rumble sound for screen shake
    // This would typically use a specific rumble sound cue
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing screen shake audio at intensity %f for %f seconds"), Intensity, Duration);
    
    // TODO: Implement actual rumble sound playback
    // For now, just log the event
}

void UAudio_SoundManager::UpdateAudioOcclusion()
{
    // TODO: Implement audio occlusion based on geometry
    // This would trace from audio sources to the listener and apply occlusion filters
}

void UAudio_SoundManager::SetEnvironmentalReverb(float ReverbAmount, float DecayTime)
{
    GlobalReverbAmount = FMath::Clamp(ReverbAmount, 0.0f, 1.0f);
    GlobalDecayTime = FMath::Clamp(DecayTime, 0.1f, 10.0f);
    
    // TODO: Apply reverb settings to audio components
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Set environmental reverb - Amount: %f, Decay: %f"), GlobalReverbAmount, GlobalDecayTime);
}

UAudioComponent* UAudio_SoundManager::CreateAudioComponent(const FAudio_SoundConfig& SoundConfig, const FVector& Location, bool bIs2D)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComponent = NewObject<UAudioComponent>(World);
    if (!AudioComponent)
    {
        return nullptr;
    }
    
    // Load the sound cue
    USoundCue* SoundCue = SoundConfig.SoundCue.LoadSynchronous();
    if (!SoundCue)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Failed to load SoundCue"));
        return nullptr;
    }
    
    AudioComponent->SetSound(SoundCue);
    AudioComponent->SetVolumeMultiplier(SoundConfig.Volume);
    AudioComponent->SetPitchMultiplier(SoundConfig.Pitch);
    AudioComponent->bAutoDestroy = true;
    
    if (bIs2D)
    {
        AudioComponent->bIsUISound = true;
        AudioComponent->bAllowSpatialization = false;
    }
    else
    {
        AudioComponent->SetWorldLocation(Location);
        AudioComponent->bAllowSpatialization = true;
        AudioComponent->AttenuationSettings.DistanceAlgorithm = ESoundDistanceModel::Linear;
        AudioComponent->AttenuationSettings.MaxDistance = SoundConfig.MaxDistance;
    }
    
    if (SoundConfig.bLoop)
    {
        AudioComponent->bAutoDestroy = false; // Don't auto-destroy looping sounds
    }
    
    return AudioComponent;
}

void UAudio_SoundManager::ApplyCategoryVolume(UAudioComponent* AudioComponent, EAudio_SoundCategory Category)
{
    if (!IsValid(AudioComponent))
    {
        return;
    }
    
    float CategoryVolume = GetCategoryVolume(Category);
    float CurrentVolume = AudioComponent->VolumeMultiplier;
    AudioComponent->SetVolumeMultiplier(CurrentVolume * CategoryVolume);
}

void UAudio_SoundManager::CleanupFinishedAudioComponents()
{
    for (auto& CategoryPair : ActiveAudioComponents)
    {
        TArray<UAudioComponent*>& AudioComponents = CategoryPair.Value;
        
        // Remove finished or invalid audio components
        AudioComponents.RemoveAll([](UAudioComponent* AudioComp)
        {
            return !IsValid(AudioComp) || !AudioComp->IsPlaying();
        });
    }
}