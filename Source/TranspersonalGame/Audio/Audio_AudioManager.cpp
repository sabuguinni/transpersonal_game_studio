#include "Audio_AudioManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

UAudio_AudioManager::UAudio_AudioManager()
{
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    SFXVolume = 0.8f;
    AmbienceVolume = 0.6f;
    CurrentMusicIntensity = 0.0f;
    CurrentWeather = EWeatherType::Clear;
    CurrentTimeOfDay = 0.5f;
    LastPlayerLocation = FVector::ZeroVector;

    MusicComponent = nullptr;
    AmbienceComponent = nullptr;
    HeartbeatComponent = nullptr;
    BreathingComponent = nullptr;
}

void UAudio_AudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio Manager initialized"));
    
    // Create audio components
    if (UWorld* World = GetWorld())
    {
        if (AActor* DummyActor = World->SpawnActor<AActor>())
        {
            MusicComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
            AmbienceComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceComponent"));
            HeartbeatComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("HeartbeatComponent"));
            BreathingComponent = DummyActor->CreateDefaultSubobject<UAudioComponent>(TEXT("BreathingComponent"));
            
            if (MusicComponent)
            {
                MusicComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
                MusicComponent->bAutoActivate = false;
            }
            
            if (AmbienceComponent)
            {
                AmbienceComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
                AmbienceComponent->bAutoActivate = false;
            }
            
            if (HeartbeatComponent)
            {
                HeartbeatComponent->SetVolumeMultiplier(SFXVolume * MasterVolume);
                HeartbeatComponent->bAutoActivate = false;
            }
            
            if (BreathingComponent)
            {
                BreathingComponent->SetVolumeMultiplier(SFXVolume * MasterVolume);
                BreathingComponent->bAutoActivate = false;
            }
        }
    }
    
    LoadDinosaurSounds();
}

void UAudio_AudioManager::Deinitialize()
{
    if (MusicComponent && IsValid(MusicComponent))
    {
        MusicComponent->Stop();
    }
    
    if (AmbienceComponent && IsValid(AmbienceComponent))
    {
        AmbienceComponent->Stop();
    }
    
    if (HeartbeatComponent && IsValid(HeartbeatComponent))
    {
        HeartbeatComponent->Stop();
    }
    
    if (BreathingComponent && IsValid(BreathingComponent))
    {
        BreathingComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UAudio_AudioManager::RegisterAudioZone(const FAudio_AudioZoneData& ZoneData)
{
    AudioZones.Add(ZoneData);
    UE_LOG(LogTemp, Warning, TEXT("Audio zone registered at location: %s"), *ZoneData.Location.ToString());
}

void UAudio_AudioManager::UpdatePlayerLocation(const FVector& PlayerLocation)
{
    LastPlayerLocation = PlayerLocation;
    
    // Find closest audio zone
    float ClosestDistance = FLT_MAX;
    FAudio_AudioZoneData* ClosestZone = nullptr;
    
    for (FAudio_AudioZoneData& Zone : AudioZones)
    {
        float Distance = FVector::Dist(PlayerLocation, Zone.Location);
        if (Distance < Zone.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestZone = &Zone;
        }
    }
    
    if (ClosestZone)
    {
        CrossfadeToZone(*ClosestZone);
    }
}

void UAudio_AudioManager::SetMusicIntensity(float Intensity)
{
    CurrentMusicIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (MusicComponent && IsValid(MusicComponent))
    {
        float TargetVolume = MusicVolume * MasterVolume * (0.3f + CurrentMusicIntensity * 0.7f);
        MusicComponent->SetVolumeMultiplier(TargetVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Music intensity set to: %f"), CurrentMusicIntensity);
}

void UAudio_AudioManager::TriggerStingerSound(USoundBase* StingerSound)
{
    if (StingerSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            StingerSound,
            LastPlayerLocation,
            SFXVolume * MasterVolume
        );
    }
}

void UAudio_AudioManager::SetWeatherAudio(EWeatherType WeatherType, float Intensity)
{
    CurrentWeather = WeatherType;
    
    // Adjust ambient audio based on weather
    if (AmbienceComponent && IsValid(AmbienceComponent))
    {
        float WeatherVolume = AmbienceVolume * MasterVolume;
        
        switch (WeatherType)
        {
            case EWeatherType::Rain:
                WeatherVolume *= (0.8f + Intensity * 0.4f);
                break;
            case EWeatherType::Storm:
                WeatherVolume *= (1.0f + Intensity * 0.5f);
                break;
            case EWeatherType::Fog:
                WeatherVolume *= (0.6f + Intensity * 0.2f);
                break;
            default:
                break;
        }
        
        AmbienceComponent->SetVolumeMultiplier(WeatherVolume);
    }
}

void UAudio_AudioManager::SetTimeOfDayAudio(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    
    // Adjust audio based on time of day
    // Dawn/Dusk: More bird activity
    // Night: Reduced ambience, more tension
    float TimeModifier = 1.0f;
    
    if (TimeOfDay < 0.2f || TimeOfDay > 0.8f) // Night
    {
        TimeModifier = 0.7f;
        SetMusicIntensity(CurrentMusicIntensity + 0.2f); // Increase tension at night
    }
    else if (TimeOfDay < 0.3f || TimeOfDay > 0.7f) // Dawn/Dusk
    {
        TimeModifier = 1.2f;
    }
    
    if (AmbienceComponent && IsValid(AmbienceComponent))
    {
        float TimeVolume = AmbienceVolume * MasterVolume * TimeModifier;
        AmbienceComponent->SetVolumeMultiplier(TimeVolume);
    }
}

void UAudio_AudioManager::PlayHeartbeatEffect(float FearLevel)
{
    if (HeartbeatComponent && IsValid(HeartbeatComponent))
    {
        float HeartbeatVolume = SFXVolume * MasterVolume * FMath::Clamp(FearLevel, 0.0f, 1.0f);
        HeartbeatComponent->SetVolumeMultiplier(HeartbeatVolume);
        
        if (FearLevel > 0.1f && !HeartbeatComponent->IsPlaying())
        {
            // Load heartbeat sound if available
            USoundBase* HeartbeatSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Heartbeat_Loop"));
            if (HeartbeatSound)
            {
                HeartbeatComponent->SetSound(HeartbeatSound);
                HeartbeatComponent->Play();
            }
        }
        else if (FearLevel <= 0.1f && HeartbeatComponent->IsPlaying())
        {
            HeartbeatComponent->FadeOut(2.0f, 0.0f);
        }
    }
}

void UAudio_AudioManager::PlayBreathingEffect(float StaminaLevel)
{
    if (BreathingComponent && IsValid(BreathingComponent))
    {
        // Heavy breathing when stamina is low
        float BreathingIntensity = 1.0f - FMath::Clamp(StaminaLevel, 0.0f, 1.0f);
        float BreathingVolume = SFXVolume * MasterVolume * BreathingIntensity;
        
        BreathingComponent->SetVolumeMultiplier(BreathingVolume);
        
        if (BreathingIntensity > 0.3f && !BreathingComponent->IsPlaying())
        {
            USoundBase* BreathingSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/HeavyBreathing_Loop"));
            if (BreathingSound)
            {
                BreathingComponent->SetSound(BreathingSound);
                BreathingComponent->Play();
            }
        }
        else if (BreathingIntensity <= 0.3f && BreathingComponent->IsPlaying())
        {
            BreathingComponent->FadeOut(1.5f, 0.0f);
        }
    }
}

void UAudio_AudioManager::PlayDinosaurAudio(EDinosaurSpecies Species, const FVector& Location, float Distance)
{
    USoundCue** FoundSound = DinosaurSounds.Find(Species);
    if (FoundSound && *FoundSound)
    {
        float DistanceVolume = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.1f, 1.0f);
        float FinalVolume = SFXVolume * MasterVolume * DistanceVolume;
        
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            *FoundSound,
            Location,
            FinalVolume
        );
        
        // Trigger fear response if it's a predator nearby
        if ((Species == EDinosaurSpecies::TRex || Species == EDinosaurSpecies::Velociraptor) && Distance < 1000.0f)
        {
            float FearLevel = 1.0f - (Distance / 1000.0f);
            PlayHeartbeatEffect(FearLevel);
        }
    }
}

void UAudio_AudioManager::UpdateAmbientAudio()
{
    // This method is called periodically to update ambient audio layers
    if (AmbienceComponent && IsValid(AmbienceComponent))
    {
        // Update based on current conditions
        float FinalVolume = AmbienceVolume * MasterVolume;
        
        // Apply weather modifier
        switch (CurrentWeather)
        {
            case EWeatherType::Rain:
                FinalVolume *= 1.2f;
                break;
            case EWeatherType::Storm:
                FinalVolume *= 1.5f;
                break;
            case EWeatherType::Fog:
                FinalVolume *= 0.8f;
                break;
            default:
                break;
        }
        
        // Apply time of day modifier
        if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f)
        {
            FinalVolume *= 0.7f; // Quieter at night
        }
        
        AmbienceComponent->SetVolumeMultiplier(FinalVolume);
    }
}

void UAudio_AudioManager::CrossfadeToZone(const FAudio_AudioZoneData& NewZone)
{
    if (AmbienceComponent && IsValid(AmbienceComponent))
    {
        // Fade out current ambience
        if (AmbienceComponent->IsPlaying())
        {
            AmbienceComponent->FadeOut(2.0f, 0.0f);
        }
        
        // Load and play new zone ambience
        if (NewZone.AmbientLayers.Num() > 0 && NewZone.AmbientLayers[0].SoundAsset)
        {
            AmbienceComponent->SetSound(NewZone.AmbientLayers[0].SoundAsset);
            AmbienceComponent->FadeIn(2.0f, NewZone.AmbientLayers[0].Volume * AmbienceVolume * MasterVolume);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crossfaded to audio zone: %s"), *UEnum::GetValueAsString(NewZone.BiomeType));
}

void UAudio_AudioManager::LoadDinosaurSounds()
{
    // Load dinosaur sound cues
    DinosaurSounds.Empty();
    
    // T-Rex sounds
    if (USoundCue* TRexSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/TRex_Roar_Cue")))
    {
        DinosaurSounds.Add(EDinosaurSpecies::TRex, TRexSound);
    }
    
    // Velociraptor sounds
    if (USoundCue* RaptorSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/Raptor_Screech_Cue")))
    {
        DinosaurSounds.Add(EDinosaurSpecies::Velociraptor, RaptorSound);
    }
    
    // Triceratops sounds
    if (USoundCue* TriceratopsSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/Triceratops_Grunt_Cue")))
    {
        DinosaurSounds.Add(EDinosaurSpecies::Triceratops, TriceratopsSound);
    }
    
    // Brachiosaurus sounds
    if (USoundCue* BrachiosaurusSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/Brachiosaurus_Call_Cue")))
    {
        DinosaurSounds.Add(EDinosaurSpecies::Brachiosaurus, BrachiosaurusSound);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Loaded %d dinosaur sound cues"), DinosaurSounds.Num());
}