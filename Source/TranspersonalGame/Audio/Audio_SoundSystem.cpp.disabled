#include "Audio_SoundSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"

AAudio_SoundSystem::AAudio_SoundSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5 seconds

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create audio components
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    MasterAudioComponent->SetupAttachment(RootComponent);
    MasterAudioComponent->bAutoActivate = true;
    MasterAudioComponent->VolumeMultiplier = 1.0f;

    ForestAmbienceComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ForestAmbienceComponent"));
    ForestAmbienceComponent->SetupAttachment(RootComponent);
    ForestAmbienceComponent->bAutoActivate = true;
    ForestAmbienceComponent->VolumeMultiplier = 0.7f;
    ForestAmbienceComponent->bAllowSpatialization = true;

    DangerAmbienceComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DangerAmbienceComponent"));
    DangerAmbienceComponent->SetupAttachment(RootComponent);
    DangerAmbienceComponent->bAutoActivate = false;
    DangerAmbienceComponent->VolumeMultiplier = 0.8f;
    DangerAmbienceComponent->bAllowSpatialization = true;

    WaterAmbienceComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WaterAmbienceComponent"));
    WaterAmbienceComponent->SetupAttachment(RootComponent);
    WaterAmbienceComponent->bAutoActivate = false;
    WaterAmbienceComponent->VolumeMultiplier = 0.6f;
    WaterAmbienceComponent->bAllowSpatialization = true;

    // Initialize default sound zones
    FAudio_SoundZone ForestZone;
    ForestZone.Location = FVector(500, 500, 100);
    ForestZone.Radius = 800.0f;
    ForestZone.BiomeType = EAudio_BiomeAmbience::Forest;
    ForestZone.Volume = 0.7f;
    ForestZone.bIsActive = true;
    SoundZones.Add(ForestZone);

    FAudio_SoundZone DangerZone;
    DangerZone.Location = FVector(-300, -300, 100);
    DangerZone.Radius = 600.0f;
    DangerZone.BiomeType = EAudio_BiomeAmbience::DangerZone;
    DangerZone.Volume = 0.8f;
    DangerZone.bIsActive = true;
    SoundZones.Add(DangerZone);

    FAudio_SoundZone WaterZone;
    WaterZone.Location = FVector(800, -200, 50);
    WaterZone.Radius = 400.0f;
    WaterZone.BiomeType = EAudio_BiomeAmbience::River;
    WaterZone.Volume = 0.6f;
    WaterZone.bIsActive = true;
    SoundZones.Add(WaterZone);

    // Initialize default values
    MasterVolume = 1.0f;
    AmbienceVolume = 0.7f;
    EffectsVolume = 0.8f;
    CurrentIntensity = EAudio_IntensityLevel::Calm;
    TimeOfDay = 12.0f;
    WeatherIntensity = 0.0f;
    bNearDinosaurs = false;
    PlayerFearLevel = 0.0f;
}

void AAudio_SoundSystem::BeginPlay()
{
    Super::BeginPlay();

    // Start periodic updates
    GetWorldTimerManager().SetTimer(AmbienceUpdateTimer, [this]()
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            UpdateAmbienceBasedOnLocation(PlayerPawn->GetActorLocation());
        }
    }, 2.0f, true);

    GetWorldTimerManager().SetTimer(IntensityUpdateTimer, [this]()
    {
        UpdateIntensityBasedOnContext();
    }, 1.0f, true);

    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundSystem: BeginPlay - Audio system initialized"));
}

void AAudio_SoundSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update audio components based on current settings
    if (MasterAudioComponent)
    {
        MasterAudioComponent->SetVolumeMultiplier(MasterVolume);
    }

    if (ForestAmbienceComponent)
    {
        ForestAmbienceComponent->SetVolumeMultiplier(AmbienceVolume * MasterVolume);
    }

    if (DangerAmbienceComponent)
    {
        float DangerVolume = bNearDinosaurs ? AmbienceVolume * 1.2f : 0.0f;
        DangerAmbienceComponent->SetVolumeMultiplier(DangerVolume * MasterVolume);
    }

    if (WaterAmbienceComponent)
    {
        WaterAmbienceComponent->SetVolumeMultiplier(AmbienceVolume * 0.8f * MasterVolume);
    }
}

void AAudio_SoundSystem::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundSystem: Master volume set to %f"), MasterVolume);
}

void AAudio_SoundSystem::SetIntensityLevel(EAudio_IntensityLevel NewIntensity)
{
    CurrentIntensity = NewIntensity;

    // Adjust volumes based on intensity
    switch (CurrentIntensity)
    {
        case EAudio_IntensityLevel::Calm:
            AmbienceVolume = 0.7f;
            EffectsVolume = 0.6f;
            break;
        case EAudio_IntensityLevel::Tense:
            AmbienceVolume = 0.8f;
            EffectsVolume = 0.7f;
            break;
        case EAudio_IntensityLevel::Dangerous:
            AmbienceVolume = 0.9f;
            EffectsVolume = 0.9f;
            break;
        case EAudio_IntensityLevel::Critical:
            AmbienceVolume = 1.0f;
            EffectsVolume = 1.0f;
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundSystem: Intensity level changed to %d"), (int32)CurrentIntensity);
}

void AAudio_SoundSystem::UpdatePlayerLocation(FVector PlayerLocation)
{
    UpdateAmbienceBasedOnLocation(PlayerLocation);
}

void AAudio_SoundSystem::TriggerDangerAudio(bool bDangerActive)
{
    bNearDinosaurs = bDangerActive;

    if (bDangerActive)
    {
        if (DangerAmbienceComponent && !DangerAmbienceComponent->IsPlaying())
        {
            DangerAmbienceComponent->Activate();
            DangerAmbienceComponent->Play();
        }
        SetIntensityLevel(EAudio_IntensityLevel::Dangerous);
    }
    else
    {
        if (DangerAmbienceComponent && DangerAmbienceComponent->IsPlaying())
        {
            DangerAmbienceComponent->Stop();
        }
        SetIntensityLevel(EAudio_IntensityLevel::Calm);
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundSystem: Danger audio %s"), bDangerActive ? TEXT("activated") : TEXT("deactivated"));
}

void AAudio_SoundSystem::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Fmod(NewTimeOfDay, 24.0f);

    // Adjust ambience based on time of day
    if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f)
    {
        // Day time - more active ambience
        AmbienceVolume = FMath::Lerp(0.5f, 0.8f, CurrentIntensity == EAudio_IntensityLevel::Calm ? 0.7f : 1.0f);
    }
    else
    {
        // Night time - quieter, more tense
        AmbienceVolume = FMath::Lerp(0.3f, 0.6f, CurrentIntensity == EAudio_IntensityLevel::Calm ? 0.5f : 1.0f);
        if (CurrentIntensity == EAudio_IntensityLevel::Calm)
        {
            SetIntensityLevel(EAudio_IntensityLevel::Tense);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_SoundSystem: Time of day set to %f"), TimeOfDay);
}

void AAudio_SoundSystem::PlayFootstepSound(FVector Location, bool bIsHeavy)
{
    // Spawn temporary audio component for footstep
    if (UAudioComponent* FootstepComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(), 
        nullptr, // Will need to load actual sound asset
        Location,
        FRotator::ZeroRotator,
        bIsHeavy ? 0.8f : 0.4f))
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundSystem: Footstep sound played at location %s (Heavy: %s)"), 
               *Location.ToString(), bIsHeavy ? TEXT("true") : TEXT("false"));
    }
}

void AAudio_SoundSystem::PlayDinosaurRoar(FVector Location, float Intensity)
{
    // Spawn temporary audio component for roar
    if (UAudioComponent* RoarComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        nullptr, // Will need to load actual sound asset
        Location,
        FRotator::ZeroRotator,
        FMath::Clamp(Intensity, 0.1f, 2.0f)))
    {
        // Trigger danger response
        TriggerDangerAudio(true);
        
        // Auto-disable danger after roar fades
        GetWorldTimerManager().SetTimer(FTimerHandle(), [this]()
        {
            TriggerDangerAudio(false);
        }, 5.0f, false);

        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundSystem: Dinosaur roar played at location %s with intensity %f"), 
               *Location.ToString(), Intensity);
    }
}

void AAudio_SoundSystem::UpdateAmbienceBasedOnLocation(FVector PlayerLocation)
{
    FAudio_SoundZone* NearestZone = GetNearestSoundZone(PlayerLocation);
    
    if (NearestZone)
    {
        // Activate appropriate ambience based on zone type
        switch (NearestZone->BiomeType)
        {
            case EAudio_BiomeAmbience::Forest:
                if (ForestAmbienceComponent && !ForestAmbienceComponent->IsPlaying())
                {
                    ForestAmbienceComponent->Activate();
                    ForestAmbienceComponent->Play();
                }
                break;
            case EAudio_BiomeAmbience::River:
                if (WaterAmbienceComponent && !WaterAmbienceComponent->IsPlaying())
                {
                    WaterAmbienceComponent->Activate();
                    WaterAmbienceComponent->Play();
                }
                break;
            case EAudio_BiomeAmbience::DangerZone:
                TriggerDangerAudio(true);
                break;
        }

        UE_LOG(LogTemp, Warning, TEXT("Audio_SoundSystem: Player in %s zone"), 
               *UEnum::GetValueAsString(NearestZone->BiomeType));
    }
}

void AAudio_SoundSystem::UpdateIntensityBasedOnContext()
{
    // Check for nearby dinosaurs or other threats
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    bool bThreatNearby = false;
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor != PlayerPawn && Actor->GetName().Contains(TEXT("Dinosaur")))
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                if (Distance < 1000.0f) // Within 10 meters
                {
                    bThreatNearby = true;
                    break;
                }
            }
        }
    }

    if (bThreatNearby && CurrentIntensity == EAudio_IntensityLevel::Calm)
    {
        SetIntensityLevel(EAudio_IntensityLevel::Tense);
    }
    else if (!bThreatNearby && CurrentIntensity > EAudio_IntensityLevel::Calm)
    {
        SetIntensityLevel(EAudio_IntensityLevel::Calm);
    }
}

void AAudio_SoundSystem::FadeAudioComponent(UAudioComponent* Component, float TargetVolume, float FadeTime)
{
    if (Component)
    {
        // Simple fade implementation - in production would use more sophisticated fading
        Component->SetVolumeMultiplier(TargetVolume);
    }
}

FAudio_SoundZone* AAudio_SoundSystem::GetNearestSoundZone(FVector Location)
{
    FAudio_SoundZone* NearestZone = nullptr;
    float NearestDistance = FLT_MAX;

    for (FAudio_SoundZone& Zone : SoundZones)
    {
        if (Zone.bIsActive)
        {
            float Distance = FVector::Dist(Location, Zone.Location);
            if (Distance <= Zone.Radius && Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestZone = &Zone;
            }
        }
    }

    return NearestZone;
}