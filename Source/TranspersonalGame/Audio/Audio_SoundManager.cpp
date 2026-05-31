#include "Audio_SoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAudio_SoundManager::AAudio_SoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create master audio component
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    RootComponent = MasterAudioComponent;

    // Initialize default settings
    DefaultSettings.Volume = 1.0f;
    DefaultSettings.Pitch = 1.0f;
    DefaultSettings.AttenuationRadius = 1000.0f;
    DefaultSettings.bLooping = false;

    MasterVolume = 1.0f;
}

void AAudio_SoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: BeginPlay - Audio system initialized"));
}

void AAudio_SoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Clean up finished audio components every few seconds
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    
    if (CleanupTimer >= 2.0f)
    {
        CleanupFinishedAudioComponents();
        CleanupTimer = 0.0f;
    }
}

void AAudio_SoundManager::PlayBiomeAmbient(EAudio_BiomeType BiomeType, const FAudio_SoundSettings& Settings)
{
    if (!BiomeAmbientSounds.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: No ambient sound configured for biome type"));
        return;
    }

    TSoftObjectPtr<USoundCue> SoundCuePtr = BiomeAmbientSounds[BiomeType];
    if (!SoundCuePtr.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Invalid sound cue for biome"));
        return;
    }

    USoundCue* SoundCue = SoundCuePtr.Get();
    if (SoundCue)
    {
        FAudio_SoundSettings LoopingSettings = Settings;
        LoopingSettings.bLooping = true;
        
        UAudioComponent* AudioComp = CreateAudioComponent(SoundCue, GetActorLocation(), LoopingSettings);
        if (AudioComp)
        {
            ActiveAudioComponents.Add(AudioComp);
            UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing biome ambient sound"));
        }
    }
}

void AAudio_SoundManager::PlayDinosaurSound(EAudio_DinosaurType DinosaurType, FVector Location, const FAudio_SoundSettings& Settings)
{
    if (!DinosaurSounds.Contains(DinosaurType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: No sound configured for dinosaur type"));
        return;
    }

    TSoftObjectPtr<USoundCue> SoundCuePtr = DinosaurSounds[DinosaurType];
    if (!SoundCuePtr.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundManager: Invalid sound cue for dinosaur"));
        return;
    }

    USoundCue* SoundCue = SoundCuePtr.Get();
    if (SoundCue)
    {
        UAudioComponent* AudioComp = CreateAudioComponent(SoundCue, Location, Settings);
        if (AudioComp)
        {
            ActiveAudioComponents.Add(AudioComp);
            UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing dinosaur sound at location"));
        }
    }
}

void AAudio_SoundManager::StopAllAmbientSounds()
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && AudioComp->IsValidLowLevel())
        {
            AudioComp->Stop();
        }
    }
    
    ActiveAudioComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Stopped all ambient sounds"));
}

void AAudio_SoundManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    
    // Update all active audio components
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && AudioComp->IsValidLowLevel())
        {
            AudioComp->SetVolumeMultiplier(AudioComp->VolumeMultiplier * MasterVolume);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Master volume set to %f"), MasterVolume);
}

void AAudio_SoundManager::PlayFootstepSound(FVector Location, float Intensity)
{
    // Create footstep sound settings based on intensity
    FAudio_SoundSettings FootstepSettings;
    FootstepSettings.Volume = FMath::Clamp(Intensity, 0.1f, 1.0f);
    FootstepSettings.Pitch = FMath::RandRange(0.8f, 1.2f);
    FootstepSettings.AttenuationRadius = 500.0f;
    FootstepSettings.bLooping = false;

    // For now, use a placeholder - in production this would reference actual footstep sound cues
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing footstep sound at intensity %f"), Intensity);
}

void AAudio_SoundManager::PlayEnvironmentalSound(const FString& SoundName, FVector Location, const FAudio_SoundSettings& Settings)
{
    // This would load and play environmental sounds by name
    // For now, just log the request
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager: Playing environmental sound: %s"), *SoundName);
}

void AAudio_SoundManager::CleanupFinishedAudioComponents()
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (!AudioComp || !AudioComp->IsValidLowLevel() || !AudioComp->IsPlaying())
        {
            if (AudioComp && AudioComp->IsValidLowLevel())
            {
                AudioComp->DestroyComponent();
            }
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

UAudioComponent* AAudio_SoundManager::CreateAudioComponent(USoundCue* SoundCue, FVector Location, const FAudio_SoundSettings& Settings)
{
    if (!SoundCue)
    {
        return nullptr;
    }

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        SoundCue,
        Location,
        FRotator::ZeroRotator,
        Settings.Volume * MasterVolume,
        Settings.Pitch,
        0.0f,
        nullptr,
        nullptr,
        true
    );

    if (AudioComp)
    {
        AudioComp->bAutoDestroy = !Settings.bLooping;
        if (Settings.bLooping)
        {
            AudioComp->SetUISound(false);
        }
    }

    return AudioComp;
}