#include "World_EnvironmentalAudioManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AWorld_EnvironmentalAudioManager::AWorld_EnvironmentalAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create audio components
    BiomeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAudioComponent"));
    RootComponent = BiomeAudioComponent;
    BiomeAudioComponent->bAutoActivate = false;
    BiomeAudioComponent->SetVolumeMultiplier(1.0f);

    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudioComponent"));
    WeatherAudioComponent->SetupAttachment(RootComponent);
    WeatherAudioComponent->bAutoActivate = false;
    WeatherAudioComponent->SetVolumeMultiplier(0.8f);

    DinosaurAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurAudioComponent"));
    DinosaurAudioComponent->SetupAttachment(RootComponent);
    DinosaurAudioComponent->bAutoActivate = false;
    DinosaurAudioComponent->SetVolumeMultiplier(0.6f);

    // Initialize default values
    AudioUpdateInterval = 1.0f;
    MaxAudioDistance = 5000.0f;
    bEnableDynamicWeatherAudio = true;
    bEnableDayNightCycle = true;
    CurrentBiome = EWorld_BiomeAudioType::Forest;
    CurrentWeather = EWorld_WeatherAudioState::Clear;
    bIsDay = true;
    CurrentAudioIntensity = 1.0f;
}

void AWorld_EnvironmentalAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize audio system
    if (BiomeAudioData)
    {
        SetBiomeAudio(CurrentBiome, 0.0f);
        SetWeatherAudio(CurrentWeather, 0.0f);
    }

    UE_LOG(LogTemp, Warning, TEXT("Environmental Audio Manager initialized"));
}

void AWorld_EnvironmentalAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AudioUpdateTimer += DeltaTime;
    
    if (AudioUpdateTimer >= AudioUpdateInterval)
    {
        AudioUpdateTimer = 0.0f;
        UpdateAudioBasedOnPlayerLocation();
        
        if (bEnableDynamicWeatherAudio)
        {
            UpdateWeatherAudio();
        }
        
        if (bEnableDayNightCycle)
        {
            UpdateDayNightAudio();
        }
    }
}

void AWorld_EnvironmentalAudioManager::SetBiomeAudio(EWorld_BiomeAudioType NewBiome, float TransitionTime)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }

    EWorld_BiomeAudioType OldBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    LastBiomeTransitionTime = GetWorld()->GetTimeSeconds();

    FWorld_BiomeAudioData* BiomeData = GetBiomeAudioData(NewBiome);
    if (BiomeData && BiomeAudioComponent)
    {
        USoundCue* TargetSound = bIsDay ? BiomeData->DaySound : BiomeData->NightSound;
        if (!TargetSound)
        {
            TargetSound = BiomeData->AmbientSound;
        }

        if (TargetSound)
        {
            if (TransitionTime > 0.0f && BiomeAudioComponent->IsPlaying())
            {
                // Crossfade implementation would go here
                BiomeAudioComponent->FadeOut(TransitionTime, 0.0f);
                
                FTimerHandle FadeInTimer;
                GetWorld()->GetTimerManager().SetTimer(FadeInTimer, [this, TargetSound, BiomeData]()
                {
                    BiomeAudioComponent->SetSound(TargetSound);
                    BiomeAudioComponent->SetVolumeMultiplier(BiomeData->VolumeMultiplier);
                    BiomeAudioComponent->FadeIn(2.0f, BiomeData->VolumeMultiplier);
                }, TransitionTime, false);
            }
            else
            {
                BiomeAudioComponent->SetSound(TargetSound);
                BiomeAudioComponent->SetVolumeMultiplier(BiomeData->VolumeMultiplier);
                BiomeAudioComponent->Play();
            }
        }
    }

    OnBiomeAudioChanged(NewBiome);
    UE_LOG(LogTemp, Log, TEXT("Biome audio changed from %d to %d"), (int32)OldBiome, (int32)NewBiome);
}

void AWorld_EnvironmentalAudioManager::SetWeatherAudio(EWorld_WeatherAudioState NewWeather, float TransitionTime)
{
    if (CurrentWeather == NewWeather)
    {
        return;
    }

    EWorld_WeatherAudioState OldWeather = CurrentWeather;
    CurrentWeather = NewWeather;
    LastWeatherTransitionTime = GetWorld()->GetTimeSeconds();

    FWorld_WeatherAudioData* WeatherData = GetWeatherAudioData(NewWeather);
    if (WeatherData && WeatherAudioComponent)
    {
        if (WeatherData->WeatherSound)
        {
            if (TransitionTime > 0.0f && WeatherAudioComponent->IsPlaying())
            {
                WeatherAudioComponent->FadeOut(WeatherData->FadeOutTime, 0.0f);
                
                FTimerHandle FadeInTimer;
                GetWorld()->GetTimerManager().SetTimer(FadeInTimer, [this, WeatherData]()
                {
                    WeatherAudioComponent->SetSound(WeatherData->WeatherSound);
                    WeatherAudioComponent->SetVolumeMultiplier(WeatherData->IntensityMultiplier);
                    WeatherAudioComponent->FadeIn(WeatherData->FadeInTime, WeatherData->IntensityMultiplier);
                }, WeatherData->FadeOutTime, false);
            }
            else
            {
                WeatherAudioComponent->SetSound(WeatherData->WeatherSound);
                WeatherAudioComponent->SetVolumeMultiplier(WeatherData->IntensityMultiplier);
                WeatherAudioComponent->Play();
            }
        }
        else if (WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->FadeOut(TransitionTime > 0.0f ? TransitionTime : 2.0f, 0.0f);
        }
    }

    OnWeatherAudioChanged(NewWeather);
    UE_LOG(LogTemp, Log, TEXT("Weather audio changed from %d to %d"), (int32)OldWeather, (int32)NewWeather);
}

void AWorld_EnvironmentalAudioManager::SetDayNightState(bool bNewIsDay, float TransitionTime)
{
    if (bIsDay == bNewIsDay)
    {
        return;
    }

    bIsDay = bNewIsDay;
    
    // Re-apply biome audio with day/night variation
    SetBiomeAudio(CurrentBiome, TransitionTime);
    
    UE_LOG(LogTemp, Log, TEXT("Day/Night state changed to: %s"), bIsDay ? TEXT("Day") : TEXT("Night"));
}

void AWorld_EnvironmentalAudioManager::PlayDinosaurAmbientSound(FVector DinosaurLocation, float Intensity)
{
    if (!DinosaurAudioComponent)
    {
        return;
    }

    float Distance = FVector::Dist(GetActorLocation(), DinosaurLocation);
    if (Distance > MaxAudioDistance)
    {
        return;
    }

    float VolumeMultiplier = CalculateDistanceBasedVolume(DinosaurLocation, GetActorLocation()) * Intensity;
    DinosaurAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    
    // This would play a dinosaur-specific sound based on location and intensity
    UE_LOG(LogTemp, Verbose, TEXT("Playing dinosaur ambient sound at distance %.2f with volume %.2f"), Distance, VolumeMultiplier);
}

void AWorld_EnvironmentalAudioManager::SetMasterAudioVolume(float VolumeMultiplier)
{
    if (BiomeAudioComponent)
    {
        BiomeAudioComponent->SetVolumeMultiplier(BiomeAudioComponent->VolumeMultiplier * VolumeMultiplier);
    }
    
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->SetVolumeMultiplier(WeatherAudioComponent->VolumeMultiplier * VolumeMultiplier);
    }
    
    if (DinosaurAudioComponent)
    {
        DinosaurAudioComponent->SetVolumeMultiplier(DinosaurAudioComponent->VolumeMultiplier * VolumeMultiplier);
    }
}

EWorld_BiomeAudioType AWorld_EnvironmentalAudioManager::DetectBiomeAtLocation(FVector Location)
{
    // Simple biome detection based on location
    // In a real implementation, this would query the biome system
    
    if (Location.Z > 400.0f)
    {
        return EWorld_BiomeAudioType::Mountain;
    }
    else if (Location.Z < 100.0f && FMath::Abs(Location.X) < 1000.0f)
    {
        return EWorld_BiomeAudioType::River;
    }
    else if (Location.X > 2000.0f && Location.Y > 2000.0f)
    {
        return EWorld_BiomeAudioType::Swamp;
    }
    else if (FMath::Abs(Location.X) > 3000.0f || FMath::Abs(Location.Y) > 3000.0f)
    {
        return EWorld_BiomeAudioType::Plains;
    }
    else
    {
        return EWorld_BiomeAudioType::Forest;
    }
}

void AWorld_EnvironmentalAudioManager::UpdateAudioBasedOnPlayerLocation()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    EWorld_BiomeAudioType DetectedBiome = DetectBiomeAtLocation(PlayerLocation);
    
    if (DetectedBiome != CurrentBiome)
    {
        SetBiomeAudio(DetectedBiome, 3.0f);
    }

    // Update audio intensity based on distance
    float Distance = FVector::Dist(GetActorLocation(), PlayerLocation);
    CurrentAudioIntensity = CalculateDistanceBasedVolume(GetActorLocation(), PlayerLocation);
}

void AWorld_EnvironmentalAudioManager::UpdateBiomeAudio()
{
    // Update biome audio variations and randomizations
    if (BiomeAudioComponent && BiomeAudioComponent->IsPlaying())
    {
        FWorld_BiomeAudioData* BiomeData = GetBiomeAudioData(CurrentBiome);
        if (BiomeData && BiomeData->bUseRandomVariations)
        {
            // Add subtle volume variations for more natural feel
            float VolumeVariation = FMath::RandRange(0.8f, 1.2f);
            BiomeAudioComponent->SetVolumeMultiplier(BiomeData->VolumeMultiplier * VolumeVariation);
        }
    }
}

void AWorld_EnvironmentalAudioManager::UpdateWeatherAudio()
{
    // Simple weather progression system
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastWeatherChange = CurrentTime - LastWeatherTransitionTime;
    
    if (TimeSinceLastWeatherChange > 300.0f) // 5 minutes
    {
        // Randomly change weather occasionally
        if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance per update
        {
            TArray<EWorld_WeatherAudioState> PossibleWeathers = {
                EWorld_WeatherAudioState::Clear,
                EWorld_WeatherAudioState::Cloudy,
                EWorld_WeatherAudioState::Rainy,
                EWorld_WeatherAudioState::Windy
            };
            
            EWorld_WeatherAudioState NewWeather = PossibleWeathers[FMath::RandRange(0, PossibleWeathers.Num() - 1)];
            if (NewWeather != CurrentWeather)
            {
                SetWeatherAudio(NewWeather, 5.0f);
            }
        }
    }
}

void AWorld_EnvironmentalAudioManager::UpdateDayNightAudio()
{
    // Simple day/night cycle based on time
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DayLength = 1200.0f; // 20 minutes day cycle
    float TimeOfDay = FMath::Fmod(CurrentTime, DayLength) / DayLength;
    
    bool ShouldBeDay = (TimeOfDay > 0.25f && TimeOfDay < 0.75f);
    
    if (ShouldBeDay != bIsDay)
    {
        SetDayNightState(ShouldBeDay, 30.0f); // 30 second transition
    }
}

FWorld_BiomeAudioData* AWorld_EnvironmentalAudioManager::GetBiomeAudioData(EWorld_BiomeAudioType BiomeType)
{
    if (!BiomeAudioData)
    {
        return nullptr;
    }

    for (FWorld_BiomeAudioData& Data : BiomeAudioData->BiomeAudioConfigs)
    {
        if (Data.BiomeType == BiomeType)
        {
            return &Data;
        }
    }

    return nullptr;
}

FWorld_WeatherAudioData* AWorld_EnvironmentalAudioManager::GetWeatherAudioData(EWorld_WeatherAudioState WeatherState)
{
    if (!BiomeAudioData)
    {
        return nullptr;
    }

    for (FWorld_WeatherAudioData& Data : BiomeAudioData->WeatherAudioConfigs)
    {
        if (Data.WeatherState == WeatherState)
        {
            return &Data;
        }
    }

    return nullptr;
}

void AWorld_EnvironmentalAudioManager::CrossfadeAudioComponents(UAudioComponent* FromComponent, UAudioComponent* ToComponent, float Duration)
{
    if (!FromComponent || !ToComponent)
    {
        return;
    }

    if (FromComponent->IsPlaying())
    {
        FromComponent->FadeOut(Duration, 0.0f);
    }

    if (ToComponent->GetSound())
    {
        ToComponent->FadeIn(Duration, ToComponent->VolumeMultiplier);
    }
}

float AWorld_EnvironmentalAudioManager::CalculateDistanceBasedVolume(FVector SourceLocation, FVector ListenerLocation)
{
    float Distance = FVector::Dist(SourceLocation, ListenerLocation);
    
    if (Distance >= MaxAudioDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff
    return FMath::Clamp(1.0f - (Distance / MaxAudioDistance), 0.0f, 1.0f);
}