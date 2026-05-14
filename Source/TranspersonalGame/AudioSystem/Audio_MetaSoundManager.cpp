#include "Audio_MetaSoundManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"

void UAudio_MetaSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Initializing MetaSound-based audio system"));
    
    // Initialize default environment settings
    CurrentEnvironment.AmbientVolume = 0.7f;
    CurrentEnvironment.MusicVolume = 0.5f;
    CurrentEnvironment.EffectsVolume = 0.8f;
    CurrentEnvironment.CurrentBiome = EBiomeType::Forest;
    CurrentEnvironment.TimeOfDay = 12.0f;
    CurrentEnvironment.ThreatLevel = 0.0f;
    
    // Load MetaSound assets
    LoadMetaSoundAssets();
    
    // Create audio components when world is available
    if (UWorld* World = GetWorld())
    {
        // Create ambient audio component
        AmbientAudioComponent = UGameplayStatics::SpawnSound2D(World, nullptr, CurrentEnvironment.AmbientVolume, 1.0f, 0.0f, nullptr, false, false);
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->bAutoDestroy = false;
        }
        
        // Create music audio component
        MusicAudioComponent = UGameplayStatics::SpawnSound2D(World, nullptr, CurrentEnvironment.MusicVolume, 1.0f, 0.0f, nullptr, false, false);
        if (MusicAudioComponent)
        {
            MusicAudioComponent->bAutoDestroy = false;
        }
        
        // Create effects audio component
        EffectsAudioComponent = UGameplayStatics::SpawnSound2D(World, nullptr, CurrentEnvironment.EffectsVolume, 1.0f, 0.0f, nullptr, false, false);
        if (EffectsAudioComponent)
        {
            EffectsAudioComponent->bAutoDestroy = false;
        }
        
        // Create narration audio component
        NarrationAudioComponent = UGameplayStatics::SpawnSound2D(World, nullptr, 1.0f, 1.0f, 0.0f, nullptr, false, false);
        if (NarrationAudioComponent)
        {
            NarrationAudioComponent->bAutoDestroy = false;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Audio components created successfully"));
    }
}

void UAudio_MetaSoundManager::Deinitialize()
{
    // Clean up audio components
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
        MusicAudioComponent = nullptr;
    }
    
    if (EffectsAudioComponent && IsValid(EffectsAudioComponent))
    {
        EffectsAudioComponent->Stop();
        EffectsAudioComponent = nullptr;
    }
    
    if (NarrationAudioComponent && IsValid(NarrationAudioComponent))
    {
        NarrationAudioComponent->Stop();
        NarrationAudioComponent = nullptr;
    }
    
    // Clear proximity triggers
    ProximityTriggers.Empty();
    
    // Clear MetaSound assets
    MetaSoundAssets.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Deinitialized"));
    
    Super::Deinitialize();
}

void UAudio_MetaSoundManager::SetEnvironmentSettings(const FAudio_EnvironmentSettings& Settings)
{
    CurrentEnvironment = Settings;
    
    // Update audio component volumes
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->SetVolumeMultiplier(Settings.AmbientVolume);
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->SetVolumeMultiplier(Settings.MusicVolume);
    }
    
    if (EffectsAudioComponent && IsValid(EffectsAudioComponent))
    {
        EffectsAudioComponent->SetVolumeMultiplier(Settings.EffectsVolume);
    }
    
    // Update ambient and music based on new settings
    UpdateAmbientAudio();
    UpdateMusicIntensity();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Environment settings updated - Biome: %d, Time: %.1f, Threat: %.2f"), 
           (int32)Settings.CurrentBiome, Settings.TimeOfDay, Settings.ThreatLevel);
}

void UAudio_MetaSoundManager::UpdateBiomeAudio(EBiomeType NewBiome)
{
    if (CurrentEnvironment.CurrentBiome != NewBiome)
    {
        CurrentEnvironment.CurrentBiome = NewBiome;
        UpdateAmbientAudio();
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Biome changed to %d"), (int32)NewBiome);
    }
}

void UAudio_MetaSoundManager::UpdateTimeOfDay(float Hours)
{
    CurrentEnvironment.TimeOfDay = FMath::Fmod(Hours, 24.0f);
    UpdateAmbientAudio();
    UpdateMusicIntensity();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Time of day updated to %.1f hours"), Hours);
}

void UAudio_MetaSoundManager::SetThreatLevel(float Level)
{
    CurrentEnvironment.ThreatLevel = FMath::Clamp(Level, 0.0f, 1.0f);
    UpdateMusicIntensity();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Threat level set to %.2f"), Level);
}

void UAudio_MetaSoundManager::RegisterProximityTrigger(AActor* TriggerActor, const FAudio_ProximityTrigger& TriggerData)
{
    if (TriggerActor && IsValid(TriggerActor))
    {
        ProximityTriggers.Add(TriggerActor, TriggerData);
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Proximity trigger registered for %s"), *TriggerActor->GetName());
    }
}

void UAudio_MetaSoundManager::UnregisterProximityTrigger(AActor* TriggerActor)
{
    if (ProximityTriggers.Remove(TriggerActor) > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Proximity trigger unregistered"));
    }
}

void UAudio_MetaSoundManager::UpdateProximityAudio(const FVector& PlayerLocation)
{
    for (auto& TriggerPair : ProximityTriggers)
    {
        AActor* TriggerActor = TriggerPair.Key;
        FAudio_ProximityTrigger& TriggerData = TriggerPair.Value;
        
        if (!TriggerActor || !IsValid(TriggerActor) || !TriggerData.bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, TriggerActor->GetActorLocation());
        
        if (Distance <= TriggerData.TriggerDistance)
        {
            float Volume = CalculateProximityVolume(Distance, TriggerData);
            PlayEnvironmentalEffect(TriggerData.AudioEventName, TriggerActor->GetActorLocation(), Volume);
        }
    }
}

void UAudio_MetaSoundManager::PlayAdaptiveMusic(const FString& MusicLayer)
{
    UMetaSoundSource* MetaSound = GetMetaSoundAsset(MusicLayer);
    if (MetaSound && MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->SetSound(MetaSound);
        MusicAudioComponent->Play();
        
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Playing adaptive music layer: %s"), *MusicLayer);
    }
}

void UAudio_MetaSoundManager::StopAdaptiveMusic()
{
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        MusicAudioComponent->Stop();
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Adaptive music stopped"));
    }
}

void UAudio_MetaSoundManager::CrossfadeToLayer(const FString& NewLayer, float FadeTime)
{
    // For now, implement as simple transition
    // TODO: Implement proper crossfading with MetaSound parameters
    StopAdaptiveMusic();
    
    // Delay before starting new layer
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, [this, NewLayer]()
        {
            PlayAdaptiveMusic(NewLayer);
        }, FadeTime * 0.5f, false);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Crossfading to layer: %s over %.1fs"), *NewLayer, FadeTime);
}

void UAudio_MetaSoundManager::PlayEnvironmentalEffect(const FString& EffectName, const FVector& Location, float Volume)
{
    UMetaSoundSource* MetaSound = GetMetaSoundAsset(EffectName);
    if (MetaSound && EffectsAudioComponent && IsValid(EffectsAudioComponent))
    {
        EffectsAudioComponent->SetSound(MetaSound);
        EffectsAudioComponent->SetVolumeMultiplier(Volume * CurrentEnvironment.EffectsVolume);
        EffectsAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Playing environmental effect: %s at volume %.2f"), *EffectName, Volume);
    }
}

void UAudio_MetaSoundManager::StartWeatherAudio(EWeatherType WeatherType)
{
    FString WeatherSoundName;
    
    switch (WeatherType)
    {
        case EWeatherType::Clear:
            WeatherSoundName = TEXT("Weather_Clear");
            break;
        case EWeatherType::Rain:
            WeatherSoundName = TEXT("Weather_Rain");
            break;
        case EWeatherType::Storm:
            WeatherSoundName = TEXT("Weather_Storm");
            break;
        case EWeatherType::Fog:
            WeatherSoundName = TEXT("Weather_Fog");
            break;
        default:
            WeatherSoundName = TEXT("Weather_Clear");
            break;
    }
    
    PlayEnvironmentalEffect(WeatherSoundName, FVector::ZeroVector, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Started weather audio: %s"), *WeatherSoundName);
}

void UAudio_MetaSoundManager::StopWeatherAudio()
{
    if (EffectsAudioComponent && IsValid(EffectsAudioComponent))
    {
        EffectsAudioComponent->Stop();
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Weather audio stopped"));
    }
}

void UAudio_MetaSoundManager::PlayNarration(const FString& NarrationKey, bool bInterruptCurrent)
{
    if (NarrationAudioComponent && IsValid(NarrationAudioComponent))
    {
        if (bInterruptCurrent || !NarrationAudioComponent->IsPlaying())
        {
            UMetaSoundSource* NarrationSound = GetMetaSoundAsset(NarrationKey);
            if (NarrationSound)
            {
                NarrationAudioComponent->SetSound(NarrationSound);
                NarrationAudioComponent->Play();
                
                UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Playing narration: %s"), *NarrationKey);
            }
        }
    }
}

void UAudio_MetaSoundManager::StopNarration()
{
    if (NarrationAudioComponent && IsValid(NarrationAudioComponent))
    {
        NarrationAudioComponent->Stop();
        UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: Narration stopped"));
    }
}

bool UAudio_MetaSoundManager::IsNarrationPlaying() const
{
    return NarrationAudioComponent && IsValid(NarrationAudioComponent) && NarrationAudioComponent->IsPlaying();
}

void UAudio_MetaSoundManager::LoadMetaSoundAssets()
{
    // Load MetaSound assets from content directory
    // For now, create placeholder entries
    MetaSoundAssets.Add(TEXT("Forest_Ambient"), nullptr);
    MetaSoundAssets.Add(TEXT("Swamp_Ambient"), nullptr);
    MetaSoundAssets.Add(TEXT("Savanna_Ambient"), nullptr);
    MetaSoundAssets.Add(TEXT("Desert_Ambient"), nullptr);
    MetaSoundAssets.Add(TEXT("Mountain_Ambient"), nullptr);
    
    MetaSoundAssets.Add(TEXT("Music_Peaceful"), nullptr);
    MetaSoundAssets.Add(TEXT("Music_Tense"), nullptr);
    MetaSoundAssets.Add(TEXT("Music_Combat"), nullptr);
    
    MetaSoundAssets.Add(TEXT("TRex_Footsteps"), nullptr);
    MetaSoundAssets.Add(TEXT("Raptor_Calls"), nullptr);
    MetaSoundAssets.Add(TEXT("Thunder_Lizard_Roar"), nullptr);
    
    MetaSoundAssets.Add(TEXT("Weather_Clear"), nullptr);
    MetaSoundAssets.Add(TEXT("Weather_Rain"), nullptr);
    MetaSoundAssets.Add(TEXT("Weather_Storm"), nullptr);
    MetaSoundAssets.Add(TEXT("Weather_Fog"), nullptr);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundManager: MetaSound asset placeholders loaded"));
}

void UAudio_MetaSoundManager::UpdateAmbientAudio()
{
    FString AmbientSoundName;
    
    // Select ambient sound based on biome and time of day
    switch (CurrentEnvironment.CurrentBiome)
    {
        case EBiomeType::Forest:
            AmbientSoundName = TEXT("Forest_Ambient");
            break;
        case EBiomeType::Swamp:
            AmbientSoundName = TEXT("Swamp_Ambient");
            break;
        case EBiomeType::Savanna:
            AmbientSoundName = TEXT("Savanna_Ambient");
            break;
        case EBiomeType::Desert:
            AmbientSoundName = TEXT("Desert_Ambient");
            break;
        case EBiomeType::Mountain:
            AmbientSoundName = TEXT("Mountain_Ambient");
            break;
        default:
            AmbientSoundName = TEXT("Forest_Ambient");
            break;
    }
    
    // Modify volume based on time of day
    float TimeVolume = 1.0f;
    if (CurrentEnvironment.TimeOfDay < 6.0f || CurrentEnvironment.TimeOfDay > 20.0f)
    {
        TimeVolume = 0.7f; // Quieter at night
    }
    
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        UMetaSoundSource* AmbientSound = GetMetaSoundAsset(AmbientSoundName);
        if (AmbientSound)
        {
            AmbientAudioComponent->SetSound(AmbientSound);
            AmbientAudioComponent->SetVolumeMultiplier(CurrentEnvironment.AmbientVolume * TimeVolume);
            
            if (!AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->Play();
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Updated ambient audio: %s (Volume: %.2f)"), *AmbientSoundName, TimeVolume);
}

void UAudio_MetaSoundManager::UpdateMusicIntensity()
{
    FString MusicLayer;
    
    // Select music based on threat level
    if (CurrentEnvironment.ThreatLevel > 0.7f)
    {
        MusicLayer = TEXT("Music_Combat");
    }
    else if (CurrentEnvironment.ThreatLevel > 0.3f)
    {
        MusicLayer = TEXT("Music_Tense");
    }
    else
    {
        MusicLayer = TEXT("Music_Peaceful");
    }
    
    // Adjust volume based on time of day
    float TimeMultiplier = 1.0f;
    if (CurrentEnvironment.TimeOfDay < 6.0f || CurrentEnvironment.TimeOfDay > 20.0f)
    {
        TimeMultiplier = 0.8f; // Slightly quieter music at night
    }
    
    if (MusicAudioComponent && IsValid(MusicAudioComponent))
    {
        UMetaSoundSource* MusicSound = GetMetaSoundAsset(MusicLayer);
        if (MusicSound)
        {
            MusicAudioComponent->SetSound(MusicSound);
            MusicAudioComponent->SetVolumeMultiplier(CurrentEnvironment.MusicVolume * TimeMultiplier);
            
            if (!MusicAudioComponent->IsPlaying())
            {
                MusicAudioComponent->Play();
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio_MetaSoundManager: Updated music intensity: %s"), *MusicLayer);
}

float UAudio_MetaSoundManager::CalculateProximityVolume(float Distance, const FAudio_ProximityTrigger& Trigger) const
{
    if (Distance <= Trigger.TriggerDistance)
    {
        return 1.0f;
    }
    else if (Distance >= Trigger.MaxDistance)
    {
        return 0.0f;
    }
    else
    {
        // Linear falloff between trigger and max distance
        float FalloffRange = Trigger.MaxDistance - Trigger.TriggerDistance;
        float FalloffDistance = Distance - Trigger.TriggerDistance;
        return 1.0f - (FalloffDistance / FalloffRange);
    }
}

UMetaSoundSource* UAudio_MetaSoundManager::GetMetaSoundAsset(const FString& AssetName) const
{
    if (const UMetaSoundSource* const* FoundAsset = MetaSoundAssets.Find(AssetName))
    {
        return *FoundAsset;
    }
    
    return nullptr;
}