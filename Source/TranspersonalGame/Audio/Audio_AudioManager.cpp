#include "Audio_AudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "../TranspersonalCharacter.h"

AAudio_AudioManager::AAudio_AudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create audio components
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    MasterAudioComponent->SetupAttachment(RootComponent);
    
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    
    SFXAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SFXAudioComponent"));
    SFXAudioComponent->SetupAttachment(RootComponent);
    
    // Initialize default values
    MasterVolume = 1.0f;
    AmbientVolume = 0.6f;
    MusicVolume = 0.4f;
    SFXVolume = 0.8f;
    
    CurrentBiome = EBiomeType::Forest;
    CurrentDangerLevel = 0.0f;
    bIsNightTime = false;
    
    AudioUpdateInterval = 0.1f;
    LastAudioUpdate = 0.0f;
    CurrentPoolIndex = 0;
    
    PlayerCharacter = nullptr;
}

void AAudio_AudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player character
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerCharacter = Cast<ATranspersonalCharacter>(PC->GetPawn());
        }
    }
    
    // Initialize audio component pool for performance
    AudioComponentPool.Reserve(20);
    for (int32 i = 0; i < 20; i++)
    {
        UAudioComponent* PooledAudio = CreateDefaultSubobject<UAudioComponent>(*FString::Printf(TEXT("PooledAudio_%d"), i));
        PooledAudio->SetupAttachment(RootComponent);
        PooledAudio->SetAutoActivate(false);
        AudioComponentPool.Add(PooledAudio);
    }
    
    // Setup initial biome audio
    UpdateBiomeAudio(CurrentBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio Manager initialized with %d pooled components"), AudioComponentPool.Num());
}

void AAudio_AudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastAudioUpdate += DeltaTime;
    
    if (LastAudioUpdate >= AudioUpdateInterval)
    {
        UpdateAmbientAudio();
        UpdateMusicLayers();
        ProcessAudioCulling();
        LastAudioUpdate = 0.0f;
    }
}

void AAudio_AudioManager::PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float VolumeMultiplier)
{
    if (!SoundCue || !GetWorld())
    {
        return;
    }
    
    // Use pooled audio component for performance
    if (AudioComponentPool.IsValidIndex(CurrentPoolIndex))
    {
        UAudioComponent* AudioComp = AudioComponentPool[CurrentPoolIndex];
        AudioComp->SetWorldLocation(Location);
        AudioComp->SetSound(SoundCue);
        AudioComp->SetVolumeMultiplier(VolumeMultiplier * MasterVolume);
        AudioComp->Play();
        
        CurrentPoolIndex = (CurrentPoolIndex + 1) % AudioComponentPool.Num();
    }
    else
    {
        // Fallback to gameplay statics
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundCue, Location, VolumeMultiplier * MasterVolume);
    }
}

void AAudio_AudioManager::PlayFootstepSound(FVector Location, ECreatureSize CreatureSize)
{
    // Different footstep sounds based on creature size
    float VolumeMultiplier = 0.5f;
    float PitchMultiplier = 1.0f;
    
    switch (CreatureSize)
    {
        case ECreatureSize::Tiny:
            VolumeMultiplier = 0.2f;
            PitchMultiplier = 1.5f;
            break;
        case ECreatureSize::Small:
            VolumeMultiplier = 0.4f;
            PitchMultiplier = 1.2f;
            break;
        case ECreatureSize::Medium:
            VolumeMultiplier = 0.6f;
            PitchMultiplier = 1.0f;
            break;
        case ECreatureSize::Large:
            VolumeMultiplier = 0.8f;
            PitchMultiplier = 0.8f;
            break;
        case ECreatureSize::Massive:
            VolumeMultiplier = 1.0f;
            PitchMultiplier = 0.6f;
            break;
    }
    
    // Use pooled audio component
    if (AudioComponentPool.IsValidIndex(CurrentPoolIndex))
    {
        UAudioComponent* AudioComp = AudioComponentPool[CurrentPoolIndex];
        AudioComp->SetWorldLocation(Location);
        AudioComp->SetVolumeMultiplier(VolumeMultiplier * SFXVolume * MasterVolume);
        AudioComp->SetPitchMultiplier(PitchMultiplier);
        
        // TODO: Set appropriate footstep sound based on terrain and creature size
        // AudioComp->SetSound(FootstepSoundCue);
        // AudioComp->Play();
        
        CurrentPoolIndex = (CurrentPoolIndex + 1) % AudioComponentPool.Num();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Footstep sound played at %s for size %d"), *Location.ToString(), (int32)CreatureSize);
}

void AAudio_AudioManager::PlayDinosaurRoar(FVector Location, EDinosaurSpecies Species, float IntensityLevel)
{
    float VolumeMultiplier = FMath::Clamp(IntensityLevel, 0.3f, 1.0f);
    float AttenuationRadius = 2000.0f;
    
    // Adjust audio parameters based on species
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            VolumeMultiplier *= 1.0f;
            AttenuationRadius = 3000.0f;
            break;
        case EDinosaurSpecies::Velociraptor:
            VolumeMultiplier *= 0.6f;
            AttenuationRadius = 1000.0f;
            break;
        case EDinosaurSpecies::Triceratops:
            VolumeMultiplier *= 0.8f;
            AttenuationRadius = 2500.0f;
            break;
        case EDinosaurSpecies::Brachiosaurus:
            VolumeMultiplier *= 0.9f;
            AttenuationRadius = 4000.0f;
            break;
        default:
            VolumeMultiplier *= 0.7f;
            AttenuationRadius = 1500.0f;
            break;
    }
    
    // Use pooled audio component with custom attenuation
    if (AudioComponentPool.IsValidIndex(CurrentPoolIndex))
    {
        UAudioComponent* AudioComp = AudioComponentPool[CurrentPoolIndex];
        AudioComp->SetWorldLocation(Location);
        AudioComp->SetVolumeMultiplier(VolumeMultiplier * SFXVolume * MasterVolume);
        
        // TODO: Set species-specific roar sound
        // AudioComp->SetSound(GetRoarSoundForSpecies(Species));
        // AudioComp->Play();
        
        CurrentPoolIndex = (CurrentPoolIndex + 1) % AudioComponentPool.Num();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur roar played: Species %d, Intensity %.2f at %s"), 
           (int32)Species, IntensityLevel, *Location.ToString());
}

void AAudio_AudioManager::UpdateBiomeAudio(EBiomeType NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }
    
    CurrentBiome = NewBiome;
    
    // Find biome audio settings
    FAudio_BiomeAudioSettings* BiomeSettings = nullptr;
    for (FAudio_BiomeAudioSettings& Settings : BiomeAudioSettings)
    {
        if (Settings.BiomeType == NewBiome)
        {
            BiomeSettings = &Settings;
            break;
        }
    }
    
    if (BiomeSettings && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetSound(BiomeSettings->AmbientSound.SoundCue);
        AmbientAudioComponent->SetVolumeMultiplier(BiomeSettings->AmbientSound.VolumeMultiplier * AmbientVolume * MasterVolume);
        AmbientAudioComponent->Play();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Biome audio updated to: %d"), (int32)NewBiome);
}

void AAudio_AudioManager::UpdateDangerLevel(float DangerLevel)
{
    CurrentDangerLevel = FMath::Clamp(DangerLevel, 0.0f, 1.0f);
    
    // Adjust music intensity based on danger level
    if (MusicAudioComponent)
    {
        float MusicIntensity = FMath::Lerp(0.3f, 1.0f, CurrentDangerLevel);
        MusicAudioComponent->SetVolumeMultiplier(MusicIntensity * MusicVolume * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Danger level updated to: %.2f"), CurrentDangerLevel);
}

void AAudio_AudioManager::SetTimeOfDay(bool bNightTime)
{
    if (bIsNightTime == bNightTime)
    {
        return;
    }
    
    bIsNightTime = bNightTime;
    
    // Adjust ambient audio for day/night cycle
    if (AmbientAudioComponent)
    {
        float NightVolumeModifier = bIsNightTime ? 1.2f : 1.0f;
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * NightVolumeModifier * MasterVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Time of day updated: %s"), bIsNightTime ? TEXT("Night") : TEXT("Day"));
}

void AAudio_AudioManager::PlayNarrationLine(const FString& NarrationText, float DelaySeconds)
{
    // TODO: Implement text-to-speech integration or play pre-recorded narration
    UE_LOG(LogTemp, Warning, TEXT("Narration: %s (Delay: %.1fs)"), *NarrationText, DelaySeconds);
}

void AAudio_AudioManager::TriggerMetaSoundEvent(const FString& EventName, float Parameter1, float Parameter2)
{
    // TODO: Implement MetaSound event triggering
    UE_LOG(LogTemp, Log, TEXT("MetaSound Event: %s (P1: %.2f, P2: %.2f)"), *EventName, Parameter1, Parameter2);
}

void AAudio_AudioManager::UpdateAmbientAudio()
{
    // Update ambient audio based on player location and game state
    if (!PlayerCharacter)
    {
        return;
    }
    
    // TODO: Implement biome detection based on player location
    // TODO: Adjust ambient audio based on nearby threats
}

void AAudio_AudioManager::UpdateMusicLayers()
{
    // Update dynamic music layers based on game state
    if (!MusicAudioComponent)
    {
        return;
    }
    
    // TODO: Implement adaptive music system
    // TODO: Layer different music tracks based on danger, biome, time of day
}

void AAudio_AudioManager::ProcessAudioCulling()
{
    // Cull distant audio sources for performance
    if (!PlayerCharacter)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Stop audio components that are too far from player
    for (UAudioComponent* AudioComp : AudioComponentPool)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            float Distance = FVector::Dist(AudioComp->GetComponentLocation(), PlayerLocation);
            if (Distance > 5000.0f) // 50 meter culling distance
            {
                AudioComp->Stop();
            }
        }
    }
}