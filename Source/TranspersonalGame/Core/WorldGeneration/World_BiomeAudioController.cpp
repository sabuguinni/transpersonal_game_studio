#include "World_BiomeAudioController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AWorld_BiomeAudioController::AWorld_BiomeAudioController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize audio components
    PrimaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAudioComponent"));
    RootComponent = PrimaryAudioComponent;
    PrimaryAudioComponent->bAutoActivate = false;
    
    SecondaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryAudioComponent"));
    SecondaryAudioComponent->SetupAttachment(RootComponent);
    SecondaryAudioComponent->bAutoActivate = false;
    
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    
    // Initialize default values
    TransitionDuration = 2.0f;
    CrossfadeIntensity = 0.8f;
    bUseDistanceBasedTransition = true;
    TransitionRadius = 2000.0f;
    
    MaxConcurrentSounds = 8;
    AudioUpdateFrequency = 0.1f;
    bEnableAudioCulling = true;
    AudioCullingDistance = 10000.0f;
    
    bEnableWeatherAudio = true;
    WeatherAudioIntensityMultiplier = 1.0f;
    
    bEnableTimeBasedAudio = true;
    
    // Initialize state
    CurrentBiome = EWorld_BiomeType::Temperate_Forest;
    TargetBiome = EWorld_BiomeType::Temperate_Forest;
    CurrentWeather = EWorld_WeatherType::Clear;
    
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    CurrentTransitionDuration = TransitionDuration;
    
    LastAudioUpdate = 0.0f;
    ActiveSoundCount = 0;
    CurrentAudioLoad = 0.0f;
    
    LastTimeOfDay = 12.0f;
    bIsDayTime = true;
    bIsTransitioningTimeAudio = false;
}

void AWorld_BiomeAudioController::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize managed audio components
    ManagedAudioComponents.Add(PrimaryAudioComponent);
    ManagedAudioComponents.Add(SecondaryAudioComponent);
    ManagedAudioComponents.Add(AmbientAudioComponent);
    
    // Initialize audio component distances
    AudioComponentDistances.SetNum(ManagedAudioComponents.Num());
    
    // Set initial biome audio
    SetCurrentBiome(CurrentBiome);
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeAudioController initialized with biome: %d"), (int32)CurrentBiome);
}

void AWorld_BiomeAudioController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update audio transition
    if (bIsTransitioning)
    {
        UpdateAudioTransition(DeltaTime);
    }
    
    // Update audio performance optimization
    LastAudioUpdate += DeltaTime;
    if (LastAudioUpdate >= AudioUpdateFrequency)
    {
        ProcessAudioCulling();
        MonitorAudioPerformance();
        LastAudioUpdate = 0.0f;
    }
    
    // Update weather audio intensity
    if (bEnableWeatherAudio)
    {
        UpdateWeatherAudioIntensity();
    }
}

void AWorld_BiomeAudioController::SetCurrentBiome(EWorld_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome && !bIsTransitioning)
    {
        return;
    }
    
    CurrentBiome = NewBiome;
    
    // Find and play biome ambient sound
    if (BiomeAmbientSounds.Contains(NewBiome))
    {
        USoundCue* AmbientSound = BiomeAmbientSounds[NewBiome];
        if (AmbientSound && PrimaryAudioComponent)
        {
            PrimaryAudioComponent->SetSound(AmbientSound);
            PrimaryAudioComponent->Play();
            ActiveSoundCount++;
        }
    }
    
    // Play biome creature sounds
    if (BiomeCreatureSounds.Contains(NewBiome))
    {
        USoundCue* CreatureSound = BiomeCreatureSounds[NewBiome];
        if (CreatureSound && SecondaryAudioComponent)
        {
            SecondaryAudioComponent->SetSound(CreatureSound);
            SecondaryAudioComponent->Play();
            ActiveSoundCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set current biome to: %d"), (int32)NewBiome);
}

void AWorld_BiomeAudioController::TransitionToBiome(EWorld_BiomeType TargetBiome, float CustomTransitionTime)
{
    if (TargetBiome == CurrentBiome)
    {
        return;
    }
    
    this->TargetBiome = TargetBiome;
    bIsTransitioning = true;
    TransitionProgress = 0.0f;
    
    CurrentTransitionDuration = (CustomTransitionTime > 0.0f) ? CustomTransitionTime : TransitionDuration;
    
    // Prepare secondary audio component for crossfade
    if (BiomeAmbientSounds.Contains(TargetBiome))
    {
        USoundCue* TargetSound = BiomeAmbientSounds[TargetBiome];
        if (TargetSound && SecondaryAudioComponent)
        {
            SecondaryAudioComponent->SetSound(TargetSound);
            SecondaryAudioComponent->SetVolumeMultiplier(0.0f);
            SecondaryAudioComponent->Play();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Starting transition from biome %d to %d"), (int32)CurrentBiome, (int32)TargetBiome);
}

void AWorld_BiomeAudioController::SetWeatherAudio(EWorld_WeatherType WeatherType, float Intensity)
{
    CurrentWeather = WeatherType;
    WeatherAudioIntensityMultiplier = FMath::Clamp(Intensity, 0.0f, 2.0f);
    
    if (!bEnableWeatherAudio)
    {
        return;
    }
    
    // Find and play weather sound
    if (WeatherSounds.Contains(WeatherType))
    {
        USoundCue* WeatherSound = WeatherSounds[WeatherType];
        if (WeatherSound && AmbientAudioComponent)
        {
            AmbientAudioComponent->SetSound(WeatherSound);
            AmbientAudioComponent->SetVolumeMultiplier(WeatherAudioIntensityMultiplier);
            AmbientAudioComponent->Play();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set weather audio: %d with intensity: %f"), (int32)WeatherType, Intensity);
}

void AWorld_BiomeAudioController::UpdateTimeBasedAudio(float TimeOfDay)
{
    if (!bEnableTimeBasedAudio)
    {
        return;
    }
    
    bool bNewIsDayTime = (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f);
    
    if (bNewIsDayTime != bIsDayTime)
    {
        bIsDayTime = bNewIsDayTime;
        bIsTransitioningTimeAudio = true;
        
        HandleTimeOfDayTransition(TimeOfDay);
    }
    
    LastTimeOfDay = TimeOfDay;
}

void AWorld_BiomeAudioController::OptimizeAudioPerformance()
{
    // Cull distant audio sources
    CullDistantAudioSources();
    
    // Optimize audio components
    OptimizeAudioComponents();
    
    // Update performance metrics
    MonitorAudioPerformance();
    
    UE_LOG(LogTemp, Log, TEXT("Audio performance optimized. Active sounds: %d, Load: %f"), ActiveSoundCount, CurrentAudioLoad);
}

void AWorld_BiomeAudioController::SetMasterVolume(float Volume)
{
    float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    for (UAudioComponent* AudioComp : ManagedAudioComponents)
    {
        if (AudioComp)
        {
            AudioComp->SetVolumeMultiplier(ClampedVolume);
        }
    }
}

void AWorld_BiomeAudioController::PauseAllBiomeAudio()
{
    for (UAudioComponent* AudioComp : ManagedAudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            AudioComp->SetPaused(true);
        }
    }
}

void AWorld_BiomeAudioController::ResumeAllBiomeAudio()
{
    for (UAudioComponent* AudioComp : ManagedAudioComponents)
    {
        if (AudioComp && AudioComp->IsPaused())
        {
            AudioComp->SetPaused(false);
        }
    }
}

int32 AWorld_BiomeAudioController::GetActiveSoundCount() const
{
    int32 Count = 0;
    for (const UAudioComponent* AudioComp : ManagedAudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            Count++;
        }
    }
    return Count;
}

float AWorld_BiomeAudioController::GetCurrentAudioLoad() const
{
    return CurrentAudioLoad;
}

void AWorld_BiomeAudioController::UpdateAudioTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    TransitionProgress += DeltaTime / CurrentTransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        
        // Swap audio components
        if (PrimaryAudioComponent && SecondaryAudioComponent)
        {
            PrimaryAudioComponent->Stop();
            
            // Move secondary to primary
            USoundCue* NewSound = SecondaryAudioComponent->GetSound();
            if (NewSound)
            {
                PrimaryAudioComponent->SetSound(NewSound);
                PrimaryAudioComponent->SetVolumeMultiplier(1.0f);
                PrimaryAudioComponent->Play();
            }
            
            SecondaryAudioComponent->Stop();
        }
        
        CurrentBiome = TargetBiome;
        UE_LOG(LogTemp, Log, TEXT("Biome transition completed to: %d"), (int32)CurrentBiome);
    }
    else
    {
        // Update crossfade volumes
        float PrimaryVolume = FMath::Lerp(1.0f, 0.0f, TransitionProgress * CrossfadeIntensity);
        float SecondaryVolume = FMath::Lerp(0.0f, 1.0f, TransitionProgress * CrossfadeIntensity);
        
        if (PrimaryAudioComponent)
        {
            PrimaryAudioComponent->SetVolumeMultiplier(PrimaryVolume);
        }
        
        if (SecondaryAudioComponent)
        {
            SecondaryAudioComponent->SetVolumeMultiplier(SecondaryVolume);
        }
    }
}

void AWorld_BiomeAudioController::ProcessAudioCulling()
{
    if (!bEnableAudioCulling)
    {
        return;
    }
    
    // Get player location for distance calculations
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    FVector AudioLocation = GetActorLocation();
    float DistanceToPlayer = FVector::Dist(PlayerLocation, AudioLocation);
    
    // Cull audio if too far from player
    if (DistanceToPlayer > AudioCullingDistance)
    {
        for (UAudioComponent* AudioComp : ManagedAudioComponents)
        {
            if (AudioComp && AudioComp->IsPlaying())
            {
                AudioComp->Stop();
            }
        }
        ActiveSoundCount = 0;
    }
    else
    {
        // Adjust volume based on distance
        float VolumeMultiplier = FMath::Clamp(1.0f - (DistanceToPlayer / AudioCullingDistance), 0.1f, 1.0f);
        
        for (UAudioComponent* AudioComp : ManagedAudioComponents)
        {
            if (AudioComp)
            {
                AudioComp->SetVolumeMultiplier(VolumeMultiplier);
            }
        }
    }
}

void AWorld_BiomeAudioController::UpdateWeatherAudioIntensity()
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        // Modulate weather audio intensity based on current weather conditions
        float IntensityModulation = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f) * 0.1f + 0.9f;
        float FinalIntensity = WeatherAudioIntensityMultiplier * IntensityModulation;
        
        AmbientAudioComponent->SetVolumeMultiplier(FinalIntensity);
    }
}

void AWorld_BiomeAudioController::HandleTimeOfDayTransition(float TimeOfDay)
{
    USoundCue* TargetSound = nullptr;
    
    if (bIsDayTime)
    {
        TargetSound = DayAmbientSound;
    }
    else
    {
        TargetSound = NightAmbientSound;
    }
    
    // Handle dawn/dusk transition
    if ((TimeOfDay >= 5.0f && TimeOfDay <= 7.0f) || (TimeOfDay >= 17.0f && TimeOfDay <= 19.0f))
    {
        TargetSound = DawnDuskSound;
    }
    
    if (TargetSound && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetSound(TargetSound);
        if (!AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->Play();
        }
    }
}

void AWorld_BiomeAudioController::MonitorAudioPerformance()
{
    ActiveSoundCount = GetActiveSoundCount();
    
    // Calculate audio load as percentage of max concurrent sounds
    CurrentAudioLoad = static_cast<float>(ActiveSoundCount) / static_cast<float>(MaxConcurrentSounds);
    
    // Performance warning if load is high
    if (CurrentAudioLoad > 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("High audio load detected: %f (%d/%d sounds)"), 
               CurrentAudioLoad, ActiveSoundCount, MaxConcurrentSounds);
    }
}

void AWorld_BiomeAudioController::CullDistantAudioSources()
{
    // Implementation for culling distant audio sources
    ProcessAudioCulling();
}

void AWorld_BiomeAudioController::OptimizeAudioComponents()
{
    // Stop inactive audio components to free resources
    for (UAudioComponent* AudioComp : ManagedAudioComponents)
    {
        if (AudioComp && !AudioComp->IsPlaying() && AudioComp->IsActive())
        {
            AudioComp->Deactivate();
        }
    }
    
    // Reactivate needed components
    if (PrimaryAudioComponent && !PrimaryAudioComponent->IsActive() && BiomeAmbientSounds.Contains(CurrentBiome))
    {
        PrimaryAudioComponent->Activate();
    }
}