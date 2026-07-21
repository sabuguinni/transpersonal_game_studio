#include "World_EcosystemAudioManager.h"
#include "World_AudioZoneManager.h"
#include "World_BiomeAudioController.h"
#include "World_EnvironmentalAudio.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AWorld_EcosystemAudioManager::AWorld_EcosystemAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for audio updates

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize audio components
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    MasterAudioComponent->SetupAttachment(RootComponent);
    MasterAudioComponent->bAutoActivate = false;

    BiomeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAudioComponent"));
    BiomeAudioComponent->SetupAttachment(RootComponent);
    BiomeAudioComponent->bAutoActivate = false;

    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudioComponent"));
    WeatherAudioComponent->SetupAttachment(RootComponent);
    WeatherAudioComponent->bAutoActivate = false;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    // Initialize default values
    AudioTransitionSpeed = 2.0f;
    MaxAudioDistance = 5000.0f;
    BiomeBlendRadius = 1000.0f;
    WeatherIntensityMultiplier = 1.0f;
    CurrentBiomeVolume = 1.0f;
    CurrentWeatherIntensity = 0.0f;
    AudioZoneUpdateInterval = 1.0f;
    LastAudioZoneUpdate = 0.0f;

    // Initialize state
    CurrentBiome = EWorld_BiomeType::Forest;
    TargetBiome = EWorld_BiomeType::Forest;
    CurrentWeather = EWorld_WeatherType::Clear;
    BiomeTransitionProgress = 1.0f;
    bIsTransitioning = false;

    // Performance settings
    MaxActiveAudioSources = 32;
    AudioCullDistance = 8000.0f;
    TransitionDuration = 2.0f;
    TransitionTimer = 0.0f;
    AudioUpdateTimer = 0.0f;

    // Initialize biome volume multipliers
    BiomeVolumeMultipliers.Add(EWorld_BiomeType::Forest, 1.0f);
    BiomeVolumeMultipliers.Add(EWorld_BiomeType::Desert, 0.8f);
    BiomeVolumeMultipliers.Add(EWorld_BiomeType::Tundra, 0.6f);
    BiomeVolumeMultipliers.Add(EWorld_BiomeType::Volcanic, 1.2f);
    BiomeVolumeMultipliers.Add(EWorld_BiomeType::Swamp, 0.9f);

    // Initialize weather volume multipliers
    WeatherVolumeMultipliers.Add(EWorld_WeatherType::Clear, 1.0f);
    WeatherVolumeMultipliers.Add(EWorld_WeatherType::Rain, 1.5f);
    WeatherVolumeMultipliers.Add(EWorld_WeatherType::Thunderstorm, 2.0f);
    WeatherVolumeMultipliers.Add(EWorld_WeatherType::Snow, 0.7f);
    WeatherVolumeMultipliers.Add(EWorld_WeatherType::Fog, 0.8f);
}

void AWorld_EcosystemAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEcosystemAudio();
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Initialized with %d audio zones"), ActiveAudioZones.Num());
}

void AWorld_EcosystemAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateEcosystemAudio(DeltaTime);
}

void AWorld_EcosystemAudioManager::InitializeEcosystemAudio()
{
    // Load audio cues
    LoadBiomeAudioCues();
    LoadWeatherAudioCues();
    
    // Configure audio components
    UpdateAudioComponentSettings();
    
    // Start with current biome audio
    SetCurrentBiome(CurrentBiome);
    SetCurrentWeather(CurrentWeather, 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Audio system initialized"));
}

void AWorld_EcosystemAudioManager::UpdateEcosystemAudio(float DeltaTime)
{
    AudioUpdateTimer += DeltaTime;
    
    // Update biome audio
    UpdateBiomeAudio(DeltaTime);
    
    // Update weather audio
    UpdateWeatherAudio(DeltaTime);
    
    // Update ambient audio
    UpdateAmbientAudio(DeltaTime);
    
    // Update audio zones periodically
    if (AudioUpdateTimer >= AudioZoneUpdateInterval)
    {
        UpdateAudioZones();
        AudioUpdateTimer = 0.0f;
    }
    
    // Performance optimization
    if (AudioUpdateTimer >= 2.0f) // Every 2 seconds
    {
        OptimizeAudioPerformance();
    }
}

void AWorld_EcosystemAudioManager::ShutdownEcosystemAudio()
{
    // Stop all audio components
    if (MasterAudioComponent && MasterAudioComponent->IsPlaying())
    {
        MasterAudioComponent->Stop();
    }
    
    if (BiomeAudioComponent && BiomeAudioComponent->IsPlaying())
    {
        BiomeAudioComponent->Stop();
    }
    
    if (WeatherAudioComponent && WeatherAudioComponent->IsPlaying())
    {
        WeatherAudioComponent->Stop();
    }
    
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    
    // Clear audio zones
    ActiveAudioZones.Empty();
    ActiveAudioComponents.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Audio system shutdown"));
}

void AWorld_EcosystemAudioManager::SetCurrentBiome(EWorld_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        TransitionToBiome(NewBiome, AudioTransitionSpeed);
    }
}

void AWorld_EcosystemAudioManager::TransitionToBiome(EWorld_BiomeType TargetBiomeType, float TransitionDurationOverride)
{
    if (TargetBiome != TargetBiomeType)
    {
        TargetBiome = TargetBiomeType;
        bIsTransitioning = true;
        BiomeTransitionProgress = 0.0f;
        TransitionDuration = TransitionDurationOverride;
        TransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Starting biome transition from %d to %d"), 
               (int32)CurrentBiome, (int32)TargetBiome);
    }
}

void AWorld_EcosystemAudioManager::UpdateBiomeAudio(float DeltaTime)
{
    if (bIsTransitioning)
    {
        ProcessBiomeTransition(DeltaTime);
    }
    
    // Update biome audio volume based on current settings
    if (BiomeAudioComponent)
    {
        float TargetVolume = GetBiomeAudioVolume(CurrentBiome);
        float CurrentVolume = BiomeAudioComponent->GetVolumeMultiplier();
        float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, AudioTransitionSpeed);
        
        BiomeAudioComponent->SetVolumeMultiplier(NewVolume);
        CurrentBiomeVolume = NewVolume;
    }
}

float AWorld_EcosystemAudioManager::GetBiomeAudioVolume(EWorld_BiomeType BiomeType) const
{
    const float* VolumeMultiplier = BiomeVolumeMultipliers.Find(BiomeType);
    return VolumeMultiplier ? *VolumeMultiplier : 1.0f;
}

void AWorld_EcosystemAudioManager::SetCurrentWeather(EWorld_WeatherType NewWeather, float Intensity)
{
    if (CurrentWeather != NewWeather || FMath::Abs(CurrentWeatherIntensity - Intensity) > 0.1f)
    {
        CurrentWeather = NewWeather;
        CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        
        // Update weather audio cue if available
        if (WeatherAudioComponent)
        {
            const TSoftObjectPtr<USoundCue>* WeatherCue = WeatherAudioCues.Find(NewWeather);
            if (WeatherCue && WeatherCue->IsValid())
            {
                USoundCue* SoundCue = WeatherCue->LoadSynchronous();
                if (SoundCue)
                {
                    WeatherAudioComponent->SetSound(SoundCue);
                    if (!WeatherAudioComponent->IsPlaying() && Intensity > 0.0f)
                    {
                        WeatherAudioComponent->Play();
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Weather changed to %d with intensity %.2f"), 
               (int32)NewWeather, Intensity);
    }
}

void AWorld_EcosystemAudioManager::UpdateWeatherAudio(float DeltaTime)
{
    if (WeatherAudioComponent)
    {
        float TargetVolume = GetWeatherAudioVolume(CurrentWeather) * CurrentWeatherIntensity * WeatherIntensityMultiplier;
        float CurrentVolume = WeatherAudioComponent->GetVolumeMultiplier();
        float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, AudioTransitionSpeed);
        
        WeatherAudioComponent->SetVolumeMultiplier(NewVolume);
        
        // Stop weather audio if intensity is very low
        if (NewVolume < 0.01f && WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->Stop();
        }
        else if (NewVolume >= 0.01f && !WeatherAudioComponent->IsPlaying())
        {
            WeatherAudioComponent->Play();
        }
    }
}

float AWorld_EcosystemAudioManager::GetWeatherAudioVolume(EWorld_WeatherType WeatherType) const
{
    const float* VolumeMultiplier = WeatherVolumeMultipliers.Find(WeatherType);
    return VolumeMultiplier ? *VolumeMultiplier : 1.0f;
}

void AWorld_EcosystemAudioManager::RegisterAudioZone(AWorld_AudioZoneManager* AudioZone)
{
    if (AudioZone && !ActiveAudioZones.Contains(AudioZone))
    {
        ActiveAudioZones.Add(AudioZone);
        UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Registered audio zone %s"), *AudioZone->GetName());
    }
}

void AWorld_EcosystemAudioManager::UnregisterAudioZone(AWorld_AudioZoneManager* AudioZone)
{
    if (AudioZone)
    {
        ActiveAudioZones.Remove(AudioZone);
        UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Unregistered audio zone %s"), *AudioZone->GetName());
    }
}

void AWorld_EcosystemAudioManager::UpdateAudioZones()
{
    // Get player location for distance calculations
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    LastPlayerLocation = PlayerLocation;
    
    // Update each audio zone
    for (int32 i = ActiveAudioZones.Num() - 1; i >= 0; i--)
    {
        AWorld_AudioZoneManager* AudioZone = ActiveAudioZones[i];
        if (!IsValid(AudioZone))
        {
            ActiveAudioZones.RemoveAt(i);
            continue;
        }
        
        // Calculate distance to audio zone
        float Distance = FVector::Dist(PlayerLocation, AudioZone->GetActorLocation());
        
        // Update audio zone based on distance
        if (Distance <= MaxAudioDistance)
        {
            float DistanceAttenuation = CalculateDistanceAttenuation(AudioZone->GetActorLocation(), PlayerLocation);
            // Audio zone would update its volume based on distance
        }
    }
}

void AWorld_EcosystemAudioManager::PlayEnvironmentalSound(USoundCue* SoundCue, FVector Location, float Volume)
{
    if (SoundCue && GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundCue, Location, Volume);
        UE_LOG(LogTemp, Log, TEXT("EcosystemAudioManager: Playing environmental sound at location %s"), *Location.ToString());
    }
}

void AWorld_EcosystemAudioManager::StopEnvironmentalSound(USoundCue* SoundCue)
{
    // Implementation would track and stop specific environmental sounds
    UE_LOG(LogTemp, Log, TEXT("EcosystemAudioManager: Stopping environmental sound"));
}

void AWorld_EcosystemAudioManager::UpdateAmbientAudio(float DeltaTime)
{
    if (AmbientAudioComponent)
    {
        // Combine biome and weather influences for ambient audio
        float BiomeInfluence = CurrentBiomeVolume * 0.6f;
        float WeatherInfluence = CurrentWeatherIntensity * 0.4f;
        float TargetVolume = FMath::Clamp(BiomeInfluence + WeatherInfluence, 0.0f, 1.0f);
        
        float CurrentVolume = AmbientAudioComponent->GetVolumeMultiplier();
        float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, AudioTransitionSpeed * 0.5f);
        
        AmbientAudioComponent->SetVolumeMultiplier(NewVolume);
    }
}

void AWorld_EcosystemAudioManager::OptimizeAudioPerformance()
{
    CullDistantAudioSources();
    
    // Limit active audio sources
    if (ActiveAudioComponents.Num() > MaxActiveAudioSources)
    {
        // Sort by distance and cull farthest
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PlayerController && PlayerController->GetPawn())
        {
            FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
            
            ActiveAudioComponents.Sort([PlayerLocation](const UAudioComponent& A, const UAudioComponent& B) {
                float DistA = FVector::DistSquared(A.GetComponentLocation(), PlayerLocation);
                float DistB = FVector::DistSquared(B.GetComponentLocation(), PlayerLocation);
                return DistA < DistB;
            });
            
            // Stop farthest audio sources
            for (int32 i = MaxActiveAudioSources; i < ActiveAudioComponents.Num(); i++)
            {
                if (ActiveAudioComponents[i] && ActiveAudioComponents[i]->IsPlaying())
                {
                    ActiveAudioComponents[i]->Stop();
                }
            }
        }
    }
}

int32 AWorld_EcosystemAudioManager::GetActiveAudioSourceCount() const
{
    int32 Count = 0;
    
    if (MasterAudioComponent && MasterAudioComponent->IsPlaying()) Count++;
    if (BiomeAudioComponent && BiomeAudioComponent->IsPlaying()) Count++;
    if (WeatherAudioComponent && WeatherAudioComponent->IsPlaying()) Count++;
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying()) Count++;
    
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            Count++;
        }
    }
    
    return Count;
}

void AWorld_EcosystemAudioManager::CullDistantAudioSources()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioComp = ActiveAudioComponents[i];
        if (!IsValid(AudioComp))
        {
            ActiveAudioComponents.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(AudioComp->GetComponentLocation(), PlayerLocation);
        if (Distance > AudioCullDistance && AudioComp->IsPlaying())
        {
            AudioComp->Stop();
        }
    }
}

void AWorld_EcosystemAudioManager::LoadBiomeAudioCues()
{
    // Load biome-specific audio cues
    // This would typically load from a data table or asset registry
    UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Loading biome audio cues"));
}

void AWorld_EcosystemAudioManager::LoadWeatherAudioCues()
{
    // Load weather-specific audio cues
    // This would typically load from a data table or asset registry
    UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Loading weather audio cues"));
}

void AWorld_EcosystemAudioManager::UpdateAudioComponentSettings()
{
    // Configure audio component settings for optimal performance
    if (MasterAudioComponent)
    {
        MasterAudioComponent->bAutoActivate = true;
        MasterAudioComponent->VolumeMultiplier = 1.0f;
    }
    
    if (BiomeAudioComponent)
    {
        BiomeAudioComponent->bAutoActivate = false;
        BiomeAudioComponent->VolumeMultiplier = 0.0f;
    }
    
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->bAutoActivate = false;
        WeatherAudioComponent->VolumeMultiplier = 0.0f;
    }
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = true;
        AmbientAudioComponent->VolumeMultiplier = 0.5f;
    }
}

float AWorld_EcosystemAudioManager::CalculateDistanceAttenuation(FVector SourceLocation, FVector ListenerLocation) const
{
    float Distance = FVector::Dist(SourceLocation, ListenerLocation);
    if (Distance <= 0.0f)
    {
        return 1.0f;
    }
    
    // Linear attenuation with max distance
    float Attenuation = 1.0f - (Distance / MaxAudioDistance);
    return FMath::Clamp(Attenuation, 0.0f, 1.0f);
}

void AWorld_EcosystemAudioManager::ProcessBiomeTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    TransitionTimer += DeltaTime;
    BiomeTransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
    
    if (BiomeTransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentBiome = TargetBiome;
        bIsTransitioning = false;
        BiomeTransitionProgress = 1.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("EcosystemAudioManager: Biome transition to %d completed"), (int32)CurrentBiome);
    }
}

void AWorld_EcosystemAudioManager::ValidateAudioComponents()
{
    // Validate that all audio components are properly configured
    if (!MasterAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("EcosystemAudioManager: MasterAudioComponent is null"));
    }
    
    if (!BiomeAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("EcosystemAudioManager: BiomeAudioComponent is null"));
    }
    
    if (!WeatherAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("EcosystemAudioManager: WeatherAudioComponent is null"));
    }
    
    if (!AmbientAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("EcosystemAudioManager: AmbientAudioComponent is null"));
    }
}