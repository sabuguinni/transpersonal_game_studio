#include "Audio_EnvironmentalSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AAudio_EnvironmentalSoundManager::AAudio_EnvironmentalSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create audio components
    BiomeAmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAmbientComponent"));
    RootComponent = BiomeAmbientComponent;
    BiomeAmbientComponent->bAutoActivate = false;
    BiomeAmbientComponent->SetVolumeMultiplier(0.7f);

    WindAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudioComponent"));
    WindAudioComponent->SetupAttachment(RootComponent);
    WindAudioComponent->bAutoActivate = false;
    WindAudioComponent->SetVolumeMultiplier(0.5f);

    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudioComponent"));
    WeatherAudioComponent->SetupAttachment(RootComponent);
    WeatherAudioComponent->bAutoActivate = false;
    WeatherAudioComponent->SetVolumeMultiplier(0.6f);

    DinosaurAmbienceComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurAmbienceComponent"));
    DinosaurAmbienceComponent->SetupAttachment(RootComponent);
    DinosaurAmbienceComponent->bAutoActivate = false;
    DinosaurAmbienceComponent->SetVolumeMultiplier(0.4f);

    // Initialize default values
    CurrentBiome = EBiomeType::Forest;
    CurrentWeather = EWeatherType::Clear;
    AudioFadeDuration = 2.0f;
    EnvironmentalVolumeMultiplier = 1.0f;
    DinosaurAudioMaxDistance = 2000.0f;
    AudioUpdateDistance = 100.0f;
    
    LastPlayerLocation = FVector::ZeroVector;
    CurrentFadeTime = 0.0f;
    TargetVolume = 1.0f;
    bIsFading = false;

    // Set default sound assets to nullptr - will be set in Blueprint or via code
    ForestAmbienceSound = nullptr;
    SwampAmbienceSound = nullptr;
    SavannaAmbienceSound = nullptr;
    DesertAmbienceSound = nullptr;
    MountainAmbienceSound = nullptr;
    WindThroughTreesSound = nullptr;
    HeavyWindSound = nullptr;
    RainSound = nullptr;
    ThunderSound = nullptr;
    DistantDinosaurRoars = nullptr;
    TRexFootstepsSound = nullptr;
}

void AAudio_EnvironmentalSoundManager::BeginPlay()
{
    Super::BeginPlay();

    // Start with forest ambience by default
    SetBiomeAmbience(EBiomeType::Forest);

    // Set up timer for periodic audio updates
    GetWorldTimerManager().SetTimer(AudioUpdateTimer, this, &AAudio_EnvironmentalSoundManager::UpdateDistanceBasedAudio, 1.0f, true);

    UE_LOG(LogTemp, Log, TEXT("Environmental Sound Manager initialized"));
}

void AAudio_EnvironmentalSoundManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up timer
    GetWorldTimerManager().ClearTimer(AudioUpdateTimer);
    
    // Stop all audio
    StopAllEnvironmentalAudio();

    Super::EndPlay(EndPlayReason);
}

void AAudio_EnvironmentalSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle audio fading
    if (bIsFading)
    {
        HandleAudioFade(DeltaTime);
    }

    // Update environmental audio based on player position
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC && PC->GetPawn())
    {
        FVector CurrentPlayerLocation = PC->GetPawn()->GetActorLocation();
        float DistanceMoved = FVector::Dist(CurrentPlayerLocation, LastPlayerLocation);
        
        if (DistanceMoved > AudioUpdateDistance)
        {
            UpdateEnvironmentalAudio(CurrentPlayerLocation);
            LastPlayerLocation = CurrentPlayerLocation;
        }
    }
}

void AAudio_EnvironmentalSoundManager::SetBiomeAmbience(EBiomeType NewBiome)
{
    if (NewBiome == CurrentBiome)
    {
        return; // No change needed
    }

    CurrentBiome = NewBiome;
    USoundCue* NewAmbienceSound = nullptr;

    // Select appropriate ambience sound
    switch (NewBiome)
    {
        case EBiomeType::Forest:
            NewAmbienceSound = ForestAmbienceSound;
            break;
        case EBiomeType::Swamp:
            NewAmbienceSound = SwampAmbienceSound;
            break;
        case EBiomeType::Savanna:
            NewAmbienceSound = SavannaAmbienceSound;
            break;
        case EBiomeType::Desert:
            NewAmbienceSound = DesertAmbienceSound;
            break;
        case EBiomeType::Mountain:
            NewAmbienceSound = MountainAmbienceSound;
            break;
        default:
            NewAmbienceSound = ForestAmbienceSound;
            break;
    }

    // Crossfade to new ambience
    if (NewAmbienceSound && BiomeAmbientComponent)
    {
        if (BiomeAmbientComponent->IsPlaying())
        {
            // Start fade out current sound
            bIsFading = true;
            CurrentFadeTime = 0.0f;
            TargetVolume = 0.0f;
        }
        else
        {
            // No current sound, start new one immediately
            BiomeAmbientComponent->SetSound(NewAmbienceSound);
            BiomeAmbientComponent->Play();
            BiomeAmbientComponent->SetVolumeMultiplier(0.7f * EnvironmentalVolumeMultiplier);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Biome ambience changed to: %d"), (int32)NewBiome);
}

void AAudio_EnvironmentalSoundManager::SetWeatherAudio(EWeatherType NewWeather, float Intensity)
{
    CurrentWeather = NewWeather;
    
    if (!WeatherAudioComponent)
    {
        return;
    }

    // Stop current weather audio
    if (WeatherAudioComponent->IsPlaying())
    {
        WeatherAudioComponent->Stop();
    }

    USoundCue* WeatherSound = nullptr;
    float WeatherVolume = Intensity * EnvironmentalVolumeMultiplier;

    switch (NewWeather)
    {
        case EWeatherType::Rain:
            WeatherSound = RainSound;
            WeatherVolume *= 0.6f;
            break;
        case EWeatherType::Storm:
            WeatherSound = ThunderSound;
            WeatherVolume *= 0.8f;
            break;
        case EWeatherType::Windy:
            WeatherSound = HeavyWindSound;
            WeatherVolume *= 0.5f;
            break;
        case EWeatherType::Clear:
        default:
            // No weather audio for clear weather
            break;
    }

    if (WeatherSound)
    {
        WeatherAudioComponent->SetSound(WeatherSound);
        WeatherAudioComponent->SetVolumeMultiplier(WeatherVolume);
        WeatherAudioComponent->Play();
    }

    // Also update wind audio
    if (WindAudioComponent && WindThroughTreesSound)
    {
        if (NewWeather == EWeatherType::Windy || NewWeather == EWeatherType::Storm)
        {
            if (!WindAudioComponent->IsPlaying())
            {
                WindAudioComponent->SetSound(WindThroughTreesSound);
                WindAudioComponent->SetVolumeMultiplier(0.5f * Intensity * EnvironmentalVolumeMultiplier);
                WindAudioComponent->Play();
            }
        }
        else
        {
            if (WindAudioComponent->IsPlaying())
            {
                WindAudioComponent->Stop();
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Weather audio set to: %d with intensity: %f"), (int32)NewWeather, Intensity);
}

void AAudio_EnvironmentalSoundManager::PlayDistantDinosaurRoar(FVector Location, float VolumeMultiplier)
{
    if (!DistantDinosaurRoars)
    {
        return;
    }

    // Calculate distance from player
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, Location);

    // Only play if within audible range
    if (Distance <= DinosaurAudioMaxDistance)
    {
        float DistanceVolume = CalculateVolumeByDistance(Distance, DinosaurAudioMaxDistance);
        float FinalVolume = DistanceVolume * VolumeMultiplier * EnvironmentalVolumeMultiplier;

        // Play at world location
        UGameplayStatics::PlaySoundAtLocation(this, DistantDinosaurRoars, Location, FinalVolume);
        
        UE_LOG(LogTemp, Log, TEXT("Distant dinosaur roar played at distance: %f, volume: %f"), Distance, FinalVolume);
    }
}

void AAudio_EnvironmentalSoundManager::PlayTRexFootsteps(FVector TRexLocation, float Distance)
{
    if (!TRexFootstepsSound)
    {
        return;
    }

    // T-Rex footsteps should be audible from much further away
    float TRexMaxDistance = DinosaurAudioMaxDistance * 2.0f;
    
    if (Distance <= TRexMaxDistance)
    {
        float DistanceVolume = CalculateVolumeByDistance(Distance, TRexMaxDistance);
        float FinalVolume = DistanceVolume * 0.8f * EnvironmentalVolumeMultiplier;

        // Play earthquake footsteps
        UGameplayStatics::PlaySoundAtLocation(this, TRexFootstepsSound, TRexLocation, FinalVolume);
        
        UE_LOG(LogTemp, Log, TEXT("T-Rex footsteps played at distance: %f, volume: %f"), Distance, FinalVolume);
    }
}

void AAudio_EnvironmentalSoundManager::UpdateEnvironmentalAudio(FVector PlayerLocation)
{
    // Determine current biome based on player location
    EBiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);
    
    // Update biome ambience if changed
    if (NewBiome != CurrentBiome)
    {
        SetBiomeAmbience(NewBiome);
    }

    // Update distance-based audio
    UpdateDistanceBasedAudio();
}

void AAudio_EnvironmentalSoundManager::StopAllEnvironmentalAudio()
{
    if (BiomeAmbientComponent && BiomeAmbientComponent->IsPlaying())
    {
        BiomeAmbientComponent->Stop();
    }
    
    if (WindAudioComponent && WindAudioComponent->IsPlaying())
    {
        WindAudioComponent->Stop();
    }
    
    if (WeatherAudioComponent && WeatherAudioComponent->IsPlaying())
    {
        WeatherAudioComponent->Stop();
    }
    
    if (DinosaurAmbienceComponent && DinosaurAmbienceComponent->IsPlaying())
    {
        DinosaurAmbienceComponent->Stop();
    }

    UE_LOG(LogTemp, Log, TEXT("All environmental audio stopped"));
}

EBiomeType AAudio_EnvironmentalSoundManager::GetBiomeAtLocation(FVector WorldLocation)
{
    // Simple biome determination based on world coordinates
    // This should eventually interface with the actual biome system
    
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;
    
    // Basic biome mapping (placeholder logic)
    if (X < -2000.0f && Y < -2000.0f)
    {
        return EBiomeType::Swamp; // Southwest
    }
    else if (X < -2000.0f && Y > 2000.0f)
    {
        return EBiomeType::Forest; // Northwest
    }
    else if (X > 2000.0f)
    {
        if (Y > 0.0f)
        {
            return EBiomeType::Mountain; // Northeast
        }
        else
        {
            return EBiomeType::Desert; // Southeast
        }
    }
    else
    {
        return EBiomeType::Savanna; // Center
    }
}

void AAudio_EnvironmentalSoundManager::HandleAudioFade(float DeltaTime)
{
    if (!bIsFading)
    {
        return;
    }

    CurrentFadeTime += DeltaTime;
    float FadeProgress = FMath::Clamp(CurrentFadeTime / AudioFadeDuration, 0.0f, 1.0f);
    
    if (TargetVolume == 0.0f)
    {
        // Fading out
        float CurrentVolume = FMath::Lerp(0.7f, 0.0f, FadeProgress);
        BiomeAmbientComponent->SetVolumeMultiplier(CurrentVolume * EnvironmentalVolumeMultiplier);
        
        if (FadeProgress >= 1.0f)
        {
            // Fade out complete, stop current sound and start new one
            BiomeAmbientComponent->Stop();
            
            // Start new sound based on current biome
            USoundCue* NewSound = nullptr;
            switch (CurrentBiome)
            {
                case EBiomeType::Forest: NewSound = ForestAmbienceSound; break;
                case EBiomeType::Swamp: NewSound = SwampAmbienceSound; break;
                case EBiomeType::Savanna: NewSound = SavannaAmbienceSound; break;
                case EBiomeType::Desert: NewSound = DesertAmbienceSound; break;
                case EBiomeType::Mountain: NewSound = MountainAmbienceSound; break;
            }
            
            if (NewSound)
            {
                BiomeAmbientComponent->SetSound(NewSound);
                BiomeAmbientComponent->Play();
                
                // Start fade in
                TargetVolume = 0.7f;
                CurrentFadeTime = 0.0f;
            }
            else
            {
                bIsFading = false;
            }
        }
    }
    else
    {
        // Fading in
        float CurrentVolume = FMath::Lerp(0.0f, 0.7f, FadeProgress);
        BiomeAmbientComponent->SetVolumeMultiplier(CurrentVolume * EnvironmentalVolumeMultiplier);
        
        if (FadeProgress >= 1.0f)
        {
            bIsFading = false;
        }
    }
}

void AAudio_EnvironmentalSoundManager::UpdateDistanceBasedAudio()
{
    // This function would update audio based on nearby dinosaurs and other distance-based effects
    // For now, it's a placeholder for future implementation
    
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    // TODO: Query nearby dinosaurs and update dinosaur ambience component
    // TODO: Update volume based on distance to various sound sources
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Distance-based audio updated for player at: %s"), *PlayerLocation.ToString());
}

float AAudio_EnvironmentalSoundManager::CalculateVolumeByDistance(float Distance, float MaxDistance)
{
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff for now, could be improved with curves
    return 1.0f - (Distance / MaxDistance);
}