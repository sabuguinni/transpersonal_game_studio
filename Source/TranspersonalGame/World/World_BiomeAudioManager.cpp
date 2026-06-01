#include "World_BiomeAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"

UWorld_BiomeAudioManager::UWorld_BiomeAudioManager()
{
    CurrentBiome = EBiomeType::Savanna;
    CurrentWeather = EWeatherType::Clear;
    CurrentTimeOfDay = 0.5f; // Noon
    bDynamicAudioEnabled = true;
    bNightAudioEnabled = true;
    AmbientAudioComponent = nullptr;
    WeatherAudioComponent = nullptr;
    RandomSoundComponent = nullptr;
}

void UWorld_BiomeAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Initializing biome audio system"));
    
    // Initialize audio data
    InitializeBiomeAudioData();
    InitializeWeatherAudioData();
    
    // Create audio components
    if (UWorld* World = GetWorld())
    {
        // Create ambient audio component
        AmbientAudioComponent = NewObject<UAudioComponent>(this, TEXT("AmbientAudio"));
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->bAutoActivate = false;
            AmbientAudioComponent->SetVolumeMultiplier(0.5f);
        }
        
        // Create weather audio component
        WeatherAudioComponent = NewObject<UAudioComponent>(this, TEXT("WeatherAudio"));
        if (WeatherAudioComponent)
        {
            WeatherAudioComponent->bAutoActivate = false;
            WeatherAudioComponent->SetVolumeMultiplier(0.7f);
        }
        
        // Create random sound component
        RandomSoundComponent = NewObject<UAudioComponent>(this, TEXT("RandomSound"));
        if (RandomSoundComponent)
        {
            RandomSoundComponent->bAutoActivate = false;
            RandomSoundComponent->SetVolumeMultiplier(0.6f);
        }
        
        // Start random sound timer
        StartRandomSoundTimer();
        
        UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Audio components created successfully"));
    }
}

void UWorld_BiomeAudioManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Deinitializing"));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RandomSoundTimer);
        World->GetTimerManager().ClearTimer(WeatherUpdateTimer);
    }
    
    // Stop audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
    }
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->Stop();
    }
    if (RandomSoundComponent)
    {
        RandomSoundComponent->Stop();
    }
    
    Super::Deinitialize();
}

void UWorld_BiomeAudioManager::UpdatePlayerBiome(EBiomeType NewBiome, const FVector& PlayerLocation)
{
    if (!bDynamicAudioEnabled)
    {
        return;
    }
    
    if (NewBiome != CurrentBiome)
    {
        UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Player entered biome %d"), (int32)NewBiome);
        
        EBiomeType OldBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        
        // Transition biome audio
        TransitionBiomeAudio(OldBiome, NewBiome);
    }
}

void UWorld_BiomeAudioManager::SetWeatherState(EWeatherType WeatherType, float Intensity)
{
    CurrentWeather = WeatherType;
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Weather changed to %d, intensity %f"), (int32)WeatherType, Intensity);
    
    UpdateWeatherAudio();
}

void UWorld_BiomeAudioManager::PlayRandomBiomeSound(EBiomeType BiomeType, const FVector& Location)
{
    if (!bDynamicAudioEnabled || !RandomSoundComponent)
    {
        return;
    }
    
    if (BiomeAudioMap.Contains(BiomeType))
    {
        const FWorld_BiomeAudioData& AudioData = BiomeAudioMap[BiomeType];
        
        if (AudioData.RandomSounds.Num() > 0)
        {
            // Pick random sound
            int32 RandomIndex = FMath::RandRange(0, AudioData.RandomSounds.Num() - 1);
            USoundWave* RandomSound = AudioData.RandomSounds[RandomIndex];
            
            if (RandomSound)
            {
                RandomSoundComponent->SetSound(RandomSound);
                RandomSoundComponent->SetWorldLocation(Location);
                RandomSoundComponent->Play();
                
                UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Playing random sound for biome %d at location %s"), 
                       (int32)BiomeType, *Location.ToString());
            }
        }
    }
}

void UWorld_BiomeAudioManager::SetGlobalAudioVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(Volume * 0.5f);
    }
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->SetVolumeMultiplier(Volume * 0.7f);
    }
    if (RandomSoundComponent)
    {
        RandomSoundComponent->SetVolumeMultiplier(Volume * 0.6f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Global audio volume set to %f"), Volume);
}

void UWorld_BiomeAudioManager::EnableDynamicAudio(bool bEnabled)
{
    bDynamicAudioEnabled = bEnabled;
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Dynamic audio %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
    
    if (!bEnabled)
    {
        // Stop all audio when disabled
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->Stop();
        }
        if (WeatherAudioComponent)
        {
            WeatherAudioComponent->Stop();
        }
        if (RandomSoundComponent)
        {
            RandomSoundComponent->Stop();
        }
    }
}

void UWorld_BiomeAudioManager::CreateAudioZone(const FVector& Center, float Radius, EBiomeType BiomeType)
{
    AudioZoneCenters.Add(Center);
    AudioZoneRadii.Add(Radius);
    AudioZoneBiomes.Add(BiomeType);
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Created audio zone for biome %d at %s, radius %f"), 
           (int32)BiomeType, *Center.ToString(), Radius);
}

void UWorld_BiomeAudioManager::RemoveAudioZone(const FVector& Center)
{
    for (int32 i = AudioZoneCenters.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(AudioZoneCenters[i], Center) < 100.0f) // 1m tolerance
        {
            AudioZoneCenters.RemoveAt(i);
            AudioZoneRadii.RemoveAt(i);
            AudioZoneBiomes.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Removed audio zone at %s"), *Center.ToString());
            break;
        }
    }
}

EBiomeType UWorld_BiomeAudioManager::GetBiomeAtLocation(const FVector& Location)
{
    EBiomeType NearestBiome = EBiomeType::Savanna;
    float MinDistance = CalculateDistanceToNearestZone(Location, NearestBiome);
    
    // If no zones nearby, determine biome by world coordinates
    if (MinDistance > 10000.0f) // 100m
    {
        // Default biome determination based on coordinates
        if (Location.X > 30000.0f && Location.Y > 30000.0f)
        {
            return EBiomeType::Mountain;
        }
        else if (Location.X > 30000.0f)
        {
            return EBiomeType::Desert;
        }
        else if (Location.Y > 20000.0f)
        {
            return EBiomeType::Forest;
        }
        else if (Location.X < -30000.0f)
        {
            return EBiomeType::Swamp;
        }
        else
        {
            return EBiomeType::Savanna;
        }
    }
    
    return NearestBiome;
}

void UWorld_BiomeAudioManager::UpdateTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    
    ApplyTimeOfDayModifiers();
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Time of day updated to %f"), CurrentTimeOfDay);
}

void UWorld_BiomeAudioManager::SetNightAudioEnabled(bool bEnabled)
{
    bNightAudioEnabled = bEnabled;
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Night audio %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
    
    ApplyTimeOfDayModifiers();
}

void UWorld_BiomeAudioManager::InitializeBiomeAudioData()
{
    // Initialize default biome audio data
    // Note: In a real implementation, these would load actual sound assets
    
    FWorld_BiomeAudioData SavannaAudio;
    SavannaAudio.AmbientVolume = 0.4f;
    SavannaAudio.RandomSoundChance = 0.3f;
    SavannaAudio.MinRandomInterval = 15.0f;
    SavannaAudio.MaxRandomInterval = 45.0f;
    BiomeAudioMap.Add(EBiomeType::Savanna, SavannaAudio);
    
    FWorld_BiomeAudioData ForestAudio;
    ForestAudio.AmbientVolume = 0.6f;
    ForestAudio.RandomSoundChance = 0.5f;
    ForestAudio.MinRandomInterval = 8.0f;
    ForestAudio.MaxRandomInterval = 25.0f;
    BiomeAudioMap.Add(EBiomeType::Forest, ForestAudio);
    
    FWorld_BiomeAudioData DesertAudio;
    DesertAudio.AmbientVolume = 0.3f;
    DesertAudio.RandomSoundChance = 0.2f;
    DesertAudio.MinRandomInterval = 20.0f;
    DesertAudio.MaxRandomInterval = 60.0f;
    BiomeAudioMap.Add(EBiomeType::Desert, DesertAudio);
    
    FWorld_BiomeAudioData SwampAudio;
    SwampAudio.AmbientVolume = 0.7f;
    SwampAudio.RandomSoundChance = 0.6f;
    SwampAudio.MinRandomInterval = 5.0f;
    SwampAudio.MaxRandomInterval = 20.0f;
    BiomeAudioMap.Add(EBiomeType::Swamp, SwampAudio);
    
    FWorld_BiomeAudioData MountainAudio;
    MountainAudio.AmbientVolume = 0.5f;
    MountainAudio.RandomSoundChance = 0.4f;
    MountainAudio.MinRandomInterval = 12.0f;
    MountainAudio.MaxRandomInterval = 35.0f;
    BiomeAudioMap.Add(EBiomeType::Mountain, MountainAudio);
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Initialized audio data for %d biomes"), BiomeAudioMap.Num());
}

void UWorld_BiomeAudioManager::InitializeWeatherAudioData()
{
    // Initialize weather audio with default values
    WeatherAudio.WeatherVolume = 0.7f;
    WeatherAudio.ThunderChance = 0.1f;
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Initialized weather audio data"));
}

void UWorld_BiomeAudioManager::StartRandomSoundTimer()
{
    if (UWorld* World = GetWorld())
    {
        float RandomInterval = FMath::RandRange(10.0f, 30.0f);
        World->GetTimerManager().SetTimer(RandomSoundTimer, this, &UWorld_BiomeAudioManager::PlayRandomSound, RandomInterval, true);
        
        UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Started random sound timer with %f second interval"), RandomInterval);
    }
}

void UWorld_BiomeAudioManager::PlayRandomSound()
{
    if (!bDynamicAudioEnabled)
    {
        return;
    }
    
    // Play random sound for current biome at player location
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Add some randomness to the sound location
        FVector SoundLocation = PlayerLocation + FVector(
            FMath::RandRange(-2000.0f, 2000.0f),
            FMath::RandRange(-2000.0f, 2000.0f),
            FMath::RandRange(-500.0f, 500.0f)
        );
        
        PlayRandomBiomeSound(CurrentBiome, SoundLocation);
    }
}

void UWorld_BiomeAudioManager::UpdateWeatherAudio()
{
    if (!WeatherAudioComponent || !bDynamicAudioEnabled)
    {
        return;
    }
    
    switch (CurrentWeather)
    {
        case EWeatherType::Clear:
            WeatherAudioComponent->Stop();
            break;
            
        case EWeatherType::Rain:
            if (WeatherAudio.RainSound)
            {
                WeatherAudioComponent->SetSound(WeatherAudio.RainSound);
                WeatherAudioComponent->SetVolumeMultiplier(WeatherAudio.WeatherVolume);
                WeatherAudioComponent->Play();
            }
            break;
            
        case EWeatherType::Storm:
            if (WeatherAudio.RainSound)
            {
                WeatherAudioComponent->SetSound(WeatherAudio.RainSound);
                WeatherAudioComponent->SetVolumeMultiplier(WeatherAudio.WeatherVolume * 1.5f);
                WeatherAudioComponent->Play();
                
                // Chance for thunder
                if (FMath::RandRange(0.0f, 1.0f) < WeatherAudio.ThunderChance && WeatherAudio.ThunderSound)
                {
                    // Play thunder sound after a short delay
                    FTimerHandle ThunderTimer;
                    GetWorld()->GetTimerManager().SetTimer(ThunderTimer, [this]()
                    {
                        if (RandomSoundComponent && WeatherAudio.ThunderSound)
                        {
                            RandomSoundComponent->SetSound(WeatherAudio.ThunderSound);
                            RandomSoundComponent->Play();
                        }
                    }, FMath::RandRange(1.0f, 5.0f), false);
                }
            }
            break;
            
        default:
            break;
    }
}

void UWorld_BiomeAudioManager::TransitionBiomeAudio(EBiomeType FromBiome, EBiomeType ToBiome)
{
    if (!AmbientAudioComponent || !bDynamicAudioEnabled)
    {
        return;
    }
    
    // Stop current ambient audio
    AmbientAudioComponent->Stop();
    
    // Start new biome ambient audio
    if (BiomeAudioMap.Contains(ToBiome))
    {
        const FWorld_BiomeAudioData& AudioData = BiomeAudioMap[ToBiome];
        
        if (AudioData.AmbientLoop)
        {
            AmbientAudioComponent->SetSound(AudioData.AmbientLoop);
            AmbientAudioComponent->SetVolumeMultiplier(AudioData.AmbientVolume);
            AmbientAudioComponent->Play();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Transitioned audio from biome %d to %d"), (int32)FromBiome, (int32)ToBiome);
}

void UWorld_BiomeAudioManager::ApplyTimeOfDayModifiers()
{
    if (!bNightAudioEnabled || !bDynamicAudioEnabled)
    {
        return;
    }
    
    // Calculate night factor (0.0 = day, 1.0 = night)
    float NightFactor = 0.0f;
    if (CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f) // Night time
    {
        if (CurrentTimeOfDay < 0.25f)
        {
            NightFactor = (0.25f - CurrentTimeOfDay) / 0.25f;
        }
        else
        {
            NightFactor = (CurrentTimeOfDay - 0.75f) / 0.25f;
        }
    }
    
    // Apply night audio modifiers
    if (AmbientAudioComponent)
    {
        float DayVolume = BiomeAudioMap.Contains(CurrentBiome) ? BiomeAudioMap[CurrentBiome].AmbientVolume : 0.5f;
        float NightVolume = DayVolume * 0.7f; // Quieter at night
        float FinalVolume = FMath::Lerp(DayVolume, NightVolume, NightFactor);
        
        AmbientAudioComponent->SetVolumeMultiplier(FinalVolume);
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioManager: Applied time of day modifiers, night factor %f"), NightFactor);
}

float UWorld_BiomeAudioManager::CalculateDistanceToNearestZone(const FVector& Location, EBiomeType& OutNearestBiome)
{
    float MinDistance = FLT_MAX;
    OutNearestBiome = EBiomeType::Savanna;
    
    for (int32 i = 0; i < AudioZoneCenters.Num(); i++)
    {
        float Distance = FVector::Dist(Location, AudioZoneCenters[i]);
        
        if (Distance < AudioZoneRadii[i] && Distance < MinDistance)
        {
            MinDistance = Distance;
            OutNearestBiome = AudioZoneBiomes[i];
        }
    }
    
    return MinDistance;
}