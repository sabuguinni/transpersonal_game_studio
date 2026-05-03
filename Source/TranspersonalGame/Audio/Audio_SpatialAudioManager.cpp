#include "Audio_SpatialAudioManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

AAudio_SpatialAudioManager::AAudio_SpatialAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize audio components
    ForestAmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ForestAmbientComponent"));
    PlainsAmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PlainsAmbientComponent"));
    RiverAmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RiverAmbientComponent"));
    DangerAmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DangerAmbientComponent"));

    // Set root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Attach audio components
    if (ForestAmbientComponent)
    {
        ForestAmbientComponent->SetupAttachment(RootComponent);
        ForestAmbientComponent->bAutoActivate = false;
    }

    if (PlainsAmbientComponent)
    {
        PlainsAmbientComponent->SetupAttachment(RootComponent);
        PlainsAmbientComponent->bAutoActivate = false;
    }

    if (RiverAmbientComponent)
    {
        RiverAmbientComponent->SetupAttachment(RootComponent);
        RiverAmbientComponent->bAutoActivate = false;
    }

    if (DangerAmbientComponent)
    {
        DangerAmbientComponent->SetupAttachment(RootComponent);
        DangerAmbientComponent->bAutoActivate = false;
    }

    // Initialize default values
    MasterVolume = 1.0f;
    AmbientVolume = 0.8f;
    EffectsVolume = 1.0f;
    VoiceVolume = 0.9f;
    
    CurrentZone = EAudio_ZoneType::Forest;
    PreviousZone = EAudio_ZoneType::Forest;
    ZoneTransitionTime = 0.0f;
    ZoneTransitionDuration = 2.0f;

    PlayerPawn = nullptr;
}

void AAudio_SpatialAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Get player pawn reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }

    // Initialize audio zones and setup default sounds
    InitializeAudioZones();
    SetupDefaultAmbientSounds();

    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: BeginPlay completed"));
}

void AAudio_SpatialAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update audio mixing based on player position and game state
    UpdateAudioMixing(DeltaTime);
    
    // Update emitter volumes based on distance
    UpdateEmitterVolumes();
    
    // Update player proximity audio
    UpdatePlayerProximityAudio();
    
    // Handle zone transitions
    UpdateZoneTransitions(DeltaTime);
}

void AAudio_SpatialAudioManager::InitializeAudioZones()
{
    // Clear existing zones
    AudioZones.Empty();

    // Forest zone
    FAudio_ZoneSettings ForestZone;
    ForestZone.ZoneType = EAudio_ZoneType::Forest;
    ForestZone.BaseVolume = 0.7f;
    ForestZone.FadeDistance = 800.0f;
    ForestZone.MaxDistance = 1500.0f;
    AudioZones.Add(ForestZone);

    // Plains zone
    FAudio_ZoneSettings PlainsZone;
    PlainsZone.ZoneType = EAudio_ZoneType::Plains;
    PlainsZone.BaseVolume = 0.6f;
    PlainsZone.FadeDistance = 1200.0f;
    PlainsZone.MaxDistance = 2000.0f;
    AudioZones.Add(PlainsZone);

    // River zone
    FAudio_ZoneSettings RiverZone;
    RiverZone.ZoneType = EAudio_ZoneType::River;
    RiverZone.BaseVolume = 0.8f;
    RiverZone.FadeDistance = 600.0f;
    RiverZone.MaxDistance = 1000.0f;
    AudioZones.Add(RiverZone);

    // Danger zone
    FAudio_ZoneSettings DangerZone;
    DangerZone.ZoneType = EAudio_ZoneType::Danger;
    DangerZone.BaseVolume = 0.9f;
    DangerZone.FadeDistance = 500.0f;
    DangerZone.MaxDistance = 800.0f;
    AudioZones.Add(DangerZone);

    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Initialized %d audio zones"), AudioZones.Num());
}

void AAudio_SpatialAudioManager::SetupDefaultAmbientSounds()
{
    // Setup forest ambient sounds
    if (ForestAmbientComponent)
    {
        ForestAmbientComponent->SetVolumeMultiplier(AmbientVolume * 0.7f);
        ForestAmbientComponent->bOverrideAttenuation = true;
    }

    // Setup plains ambient sounds
    if (PlainsAmbientComponent)
    {
        PlainsAmbientComponent->SetVolumeMultiplier(AmbientVolume * 0.6f);
        PlainsAmbientComponent->bOverrideAttenuation = true;
    }

    // Setup river ambient sounds
    if (RiverAmbientComponent)
    {
        RiverAmbientComponent->SetVolumeMultiplier(AmbientVolume * 0.8f);
        RiverAmbientComponent->bOverrideAttenuation = true;
    }

    // Setup danger ambient sounds
    if (DangerAmbientComponent)
    {
        DangerAmbientComponent->SetVolumeMultiplier(AmbientVolume * 0.9f);
        DangerAmbientComponent->bOverrideAttenuation = true;
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Default ambient sounds configured"));
}

void AAudio_SpatialAudioManager::RegisterAudioZone(EAudio_ZoneType ZoneType, FVector Location, float Radius)
{
    FAudio_ZoneSettings NewZone;
    NewZone.ZoneType = ZoneType;
    NewZone.BaseVolume = 0.7f;
    NewZone.FadeDistance = Radius * 0.8f;
    NewZone.MaxDistance = Radius;
    NewZone.bIs3D = true;
    NewZone.bAutoActivate = true;

    AudioZones.Add(NewZone);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Registered audio zone type %d at location %s"), 
           (int32)ZoneType, *Location.ToString());
}

void AAudio_SpatialAudioManager::UnregisterAudioZone(EAudio_ZoneType ZoneType)
{
    AudioZones.RemoveAll([ZoneType](const FAudio_ZoneSettings& Zone)
    {
        return Zone.ZoneType == ZoneType;
    });

    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Unregistered audio zone type %d"), (int32)ZoneType);
}

void AAudio_SpatialAudioManager::UpdateAudioMixing(float DeltaTime)
{
    if (!PlayerPawn)
    {
        return;
    }

    // Determine current player zone
    EAudio_ZoneType NewZone = GetCurrentPlayerZone();
    
    if (NewZone != CurrentZone)
    {
        OnPlayerExitZone(CurrentZone);
        OnPlayerEnterZone(NewZone);
        PreviousZone = CurrentZone;
        CurrentZone = NewZone;
        ZoneTransitionTime = 0.0f;
    }

    // Update component volumes based on current zone
    float ForestVolume = (CurrentZone == EAudio_ZoneType::Forest) ? AmbientVolume : AmbientVolume * 0.3f;
    float PlainsVolume = (CurrentZone == EAudio_ZoneType::Plains) ? AmbientVolume : AmbientVolume * 0.3f;
    float RiverVolume = (CurrentZone == EAudio_ZoneType::River) ? AmbientVolume : AmbientVolume * 0.3f;

    if (ForestAmbientComponent)
    {
        ForestAmbientComponent->SetVolumeMultiplier(ForestVolume * MasterVolume);
    }
    
    if (PlainsAmbientComponent)
    {
        PlainsAmbientComponent->SetVolumeMultiplier(PlainsVolume * MasterVolume);
    }
    
    if (RiverAmbientComponent)
    {
        RiverAmbientComponent->SetVolumeMultiplier(RiverVolume * MasterVolume);
    }
}

void AAudio_SpatialAudioManager::AddAudioEmitter(FVector Location, EAudio_Priority Priority, const FString& AudioURL)
{
    FAudio_EmitterData NewEmitter;
    NewEmitter.Location = Location;
    NewEmitter.Priority = Priority;
    NewEmitter.AudioURL = AudioURL;
    NewEmitter.bIsActive = true;
    NewEmitter.Volume = 1.0f;
    NewEmitter.Pitch = 1.0f;

    AudioEmitters.Add(NewEmitter);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Added audio emitter at %s with URL: %s"), 
           *Location.ToString(), *AudioURL);
}

void AAudio_SpatialAudioManager::RemoveAudioEmitter(FVector Location)
{
    AudioEmitters.RemoveAll([Location](const FAudio_EmitterData& Emitter)
    {
        return FVector::Dist(Emitter.Location, Location) < 100.0f;
    });

    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Removed audio emitter at %s"), *Location.ToString());
}

void AAudio_SpatialAudioManager::UpdateEmitterVolumes()
{
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (FAudio_EmitterData& Emitter : AudioEmitters)
    {
        if (!Emitter.bIsActive)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Emitter.Location);
        float Attenuation = CalculateDistanceAttenuation(Emitter.Location, 1000.0f);
        
        // Adjust volume based on priority and distance
        float PriorityMultiplier = 1.0f;
        switch (Emitter.Priority)
        {
            case EAudio_Priority::Emergency:
                PriorityMultiplier = 1.5f;
                break;
            case EAudio_Priority::Critical:
                PriorityMultiplier = 1.3f;
                break;
            case EAudio_Priority::Gameplay:
                PriorityMultiplier = 1.1f;
                break;
            case EAudio_Priority::Ambient:
                PriorityMultiplier = 0.8f;
                break;
            case EAudio_Priority::Background:
                PriorityMultiplier = 0.6f;
                break;
        }

        Emitter.Volume = Attenuation * PriorityMultiplier * EffectsVolume * MasterVolume;
    }
}

void AAudio_SpatialAudioManager::UpdatePlayerProximityAudio()
{
    if (!PlayerPawn)
    {
        return;
    }

    // This would integrate with dinosaur AI system to trigger proximity audio
    // For now, we'll just log the player's current zone
    static float LogTimer = 0.0f;
    LogTimer += GetWorld()->GetDeltaSeconds();
    
    if (LogTimer >= 5.0f) // Log every 5 seconds
    {
        UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Player in zone %d, %d active emitters"), 
               (int32)CurrentZone, AudioEmitters.Num());
        LogTimer = 0.0f;
    }
}

float AAudio_SpatialAudioManager::CalculateDistanceAttenuation(FVector EmitterLocation, float MaxDistance)
{
    if (!PlayerPawn)
    {
        return 0.0f;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, EmitterLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff for now - could be made more sophisticated
    return FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);
}

EAudio_ZoneType AAudio_SpatialAudioManager::GetCurrentPlayerZone()
{
    if (!PlayerPawn)
    {
        return EAudio_ZoneType::Forest;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Simple zone detection based on player position
    // This would be more sophisticated in a real implementation
    if (PlayerLocation.X < -200.0f && PlayerLocation.Y < -200.0f)
    {
        return EAudio_ZoneType::Forest;
    }
    else if (PlayerLocation.X > 600.0f && FMath::Abs(PlayerLocation.Y) < 400.0f)
    {
        return EAudio_ZoneType::River;
    }
    else if (PlayerLocation.Y > 600.0f)
    {
        return EAudio_ZoneType::Plains;
    }
    
    return EAudio_ZoneType::Forest; // Default
}

void AAudio_SpatialAudioManager::OnPlayerEnterZone(EAudio_ZoneType NewZone)
{
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Player entered zone %d"), (int32)NewZone);
    
    // Activate appropriate ambient component
    switch (NewZone)
    {
        case EAudio_ZoneType::Forest:
            if (ForestAmbientComponent && !ForestAmbientComponent->IsPlaying())
            {
                ForestAmbientComponent->Play();
            }
            break;
        case EAudio_ZoneType::Plains:
            if (PlainsAmbientComponent && !PlainsAmbientComponent->IsPlaying())
            {
                PlainsAmbientComponent->Play();
            }
            break;
        case EAudio_ZoneType::River:
            if (RiverAmbientComponent && !RiverAmbientComponent->IsPlaying())
            {
                RiverAmbientComponent->Play();
            }
            break;
        case EAudio_ZoneType::Danger:
            if (DangerAmbientComponent && !DangerAmbientComponent->IsPlaying())
            {
                DangerAmbientComponent->Play();
            }
            break;
    }
}

void AAudio_SpatialAudioManager::OnPlayerExitZone(EAudio_ZoneType OldZone)
{
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Player exited zone %d"), (int32)OldZone);
    
    // Don't immediately stop ambient sounds - let them fade out naturally
}

void AAudio_SpatialAudioManager::PlayAudioFromURL(const FString& AudioURL, FVector Location, float Volume)
{
    // This would integrate with a URL-based audio loading system
    // For now, we'll add it as an emitter
    AddAudioEmitter(Location, EAudio_Priority::Gameplay, AudioURL);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Playing audio from URL %s at location %s"), 
           *AudioURL, *Location.ToString());
}

void AAudio_SpatialAudioManager::StopAudioFromURL(const FString& AudioURL)
{
    // Remove emitters with matching URL
    AudioEmitters.RemoveAll([AudioURL](const FAudio_EmitterData& Emitter)
    {
        return Emitter.AudioURL == AudioURL;
    });
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Stopped audio from URL %s"), *AudioURL);
}

void AAudio_SpatialAudioManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Master volume set to %f"), MasterVolume);
}

void AAudio_SpatialAudioManager::SetAmbientVolume(float NewVolume)
{
    AmbientVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Ambient volume set to %f"), AmbientVolume);
}

void AAudio_SpatialAudioManager::SetEffectsVolume(float NewVolume)
{
    EffectsVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Effects volume set to %f"), EffectsVolume);
}

void AAudio_SpatialAudioManager::SetVoiceVolume(float NewVolume)
{
    VoiceVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Voice volume set to %f"), VoiceVolume);
}

void AAudio_SpatialAudioManager::UpdateZoneTransitions(float DeltaTime)
{
    if (CurrentZone != PreviousZone)
    {
        ZoneTransitionTime += DeltaTime;
        
        if (ZoneTransitionTime >= ZoneTransitionDuration)
        {
            // Transition complete
            PreviousZone = CurrentZone;
            ZoneTransitionTime = 0.0f;
        }
        else
        {
            // Smooth transition between zones
            float TransitionAlpha = ZoneTransitionTime / ZoneTransitionDuration;
            // Apply transition effects here
        }
    }
}