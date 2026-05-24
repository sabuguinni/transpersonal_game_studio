#include "Audio_SpatialManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UAudio_SpatialManager::UAudio_SpatialManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check 10 times per second
    
    ZoneTransitionTime = 2.0f;
    ZoneCheckInterval = 1.0f;
    ZoneCheckTimer = 0.0f;
    CurrentZone = EAudio_ZoneType::Forest;
}

void UAudio_SpatialManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetAutoActivate(false);
        AmbientAudioComponent->SetVolumeMultiplier(0.7f);
    }
    
    // Find player actor
    FindPlayerActor();
    
    // Setup default zones
    SetupDefaultZones();
    
    // Initialize audio zones
    InitializeAudioZones();
}

void UAudio_SpatialManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update zone check timer
    ZoneCheckTimer += DeltaTime;
    
    if (ZoneCheckTimer >= ZoneCheckInterval)
    {
        ZoneCheckTimer = 0.0f;
        UpdateCurrentZone();
    }
}

void UAudio_SpatialManager::InitializeAudioZones()
{
    if (AudioZones.Num() == 0)
    {
        SetupDefaultZones();
    }
    
    // Log initialization
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialManager: Initialized %d audio zones"), AudioZones.Num());
    
    // Start with forest ambient
    TransitionToZone(EAudio_ZoneType::Forest);
}

void UAudio_SpatialManager::PlaySpatialSound(USoundCue* SoundCue, FVector Location, EAudio_SoundCategory Category, float VolumeMultiplier)
{
    if (!SoundCue || !GetWorld())
    {
        return;
    }
    
    // Calculate distance-based volume
    float DistanceVolume = CalculateDistanceVolume(Location, 5000.0f);
    float FinalVolume = DistanceVolume * VolumeMultiplier;
    
    if (FinalVolume > 0.01f)
    {
        // Play sound at location with calculated volume
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            SoundCue,
            Location,
            FinalVolume,
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation settings (use default)
            nullptr, // Concurrency settings
            GetOwner() // Owner
        );
        
        UE_LOG(LogTemp, Log, TEXT("Audio_SpatialManager: Played spatial sound at %s with volume %f"), 
               *Location.ToString(), FinalVolume);
    }
}

void UAudio_SpatialManager::UpdateCurrentZone()
{
    if (!PlayerActor)
    {
        FindPlayerActor();
        return;
    }
    
    FVector PlayerLocation = PlayerActor->GetActorLocation();
    EAudio_ZoneType NewZone = GetZoneAtLocation(PlayerLocation);
    
    if (NewZone != CurrentZone)
    {
        TransitionToZone(NewZone);
    }
}

FAudio_SpatialSettings UAudio_SpatialManager::GetCurrentSpatialSettings() const
{
    for (const FAudio_ZoneData& Zone : AudioZones)
    {
        if (Zone.ZoneType == CurrentZone)
        {
            return Zone.SpatialSettings;
        }
    }
    
    // Return default settings if zone not found
    return FAudio_SpatialSettings();
}

void UAudio_SpatialManager::SetZoneAmbientSound(EAudio_ZoneType ZoneType, USoundCue* AmbientSound)
{
    for (FAudio_ZoneData& Zone : AudioZones)
    {
        if (Zone.ZoneType == ZoneType)
        {
            Zone.AmbientSounds.Empty();
            if (AmbientSound)
            {
                Zone.AmbientSounds.Add(AmbientSound);
            }
            break;
        }
    }
}

float UAudio_SpatialManager::CalculateDistanceVolume(FVector SoundLocation, float MaxDistance) const
{
    if (!PlayerActor)
    {
        return 1.0f;
    }
    
    float Distance = FVector::Dist(PlayerActor->GetActorLocation(), SoundLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear falloff
    return 1.0f - (Distance / MaxDistance);
}

bool UAudio_SpatialManager::IsPlayerInZone(EAudio_ZoneType ZoneType) const
{
    return CurrentZone == ZoneType;
}

EAudio_ZoneType UAudio_SpatialManager::GetZoneAtLocation(FVector Location) const
{
    // Check each zone to see if location is within its radius
    for (const FAudio_ZoneData& Zone : AudioZones)
    {
        float Distance = FVector::Dist(Location, Zone.Center);
        if (Distance <= Zone.Radius)
        {
            return Zone.ZoneType;
        }
    }
    
    // Default to forest if no zone found
    return EAudio_ZoneType::Forest;
}

void UAudio_SpatialManager::TransitionToZone(EAudio_ZoneType NewZone)
{
    if (NewZone == CurrentZone)
    {
        return;
    }
    
    EAudio_ZoneType PreviousZone = CurrentZone;
    CurrentZone = NewZone;
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialManager: Transitioning from zone %d to zone %d"), 
           (int32)PreviousZone, (int32)CurrentZone);
    
    // Find new zone data
    for (const FAudio_ZoneData& Zone : AudioZones)
    {
        if (Zone.ZoneType == CurrentZone && Zone.AmbientSounds.Num() > 0)
        {
            // Load and play new ambient sound
            if (USoundCue* NewAmbient = Zone.AmbientSounds[0].LoadSynchronous())
            {
                if (AmbientAudioComponent)
                {
                    AmbientAudioComponent->Stop();
                    AmbientAudioComponent->SetSound(NewAmbient);
                    AmbientAudioComponent->SetVolumeMultiplier(Zone.SpatialSettings.VolumeMultiplier);
                    AmbientAudioComponent->Play();
                }
            }
            break;
        }
    }
}

void UAudio_SpatialManager::SetupDefaultZones()
{
    AudioZones.Empty();
    
    // Forest Zone (center of map)
    FAudio_ZoneData ForestZone;
    ForestZone.ZoneType = EAudio_ZoneType::Forest;
    ForestZone.Center = FVector(0.0f, 0.0f, 0.0f);
    ForestZone.Radius = 3000.0f;
    ForestZone.SpatialSettings.VolumeMultiplier = 0.8f;
    ForestZone.SpatialSettings.ReverbWetness = 0.4f;
    AudioZones.Add(ForestZone);
    
    // Plains Zone (east)
    FAudio_ZoneData PlainsZone;
    PlainsZone.ZoneType = EAudio_ZoneType::Plains;
    PlainsZone.Center = FVector(5000.0f, 0.0f, 0.0f);
    PlainsZone.Radius = 4000.0f;
    PlainsZone.SpatialSettings.VolumeMultiplier = 1.0f;
    PlainsZone.SpatialSettings.ReverbWetness = 0.1f;
    AudioZones.Add(PlainsZone);
    
    // Swamp Zone (south)
    FAudio_ZoneData SwampZone;
    SwampZone.ZoneType = EAudio_ZoneType::Swamp;
    SwampZone.Center = FVector(0.0f, -5000.0f, 0.0f);
    SwampZone.Radius = 3500.0f;
    SwampZone.SpatialSettings.VolumeMultiplier = 0.9f;
    SwampZone.SpatialSettings.ReverbWetness = 0.6f;
    AudioZones.Add(SwampZone);
    
    // River Zone (north)
    FAudio_ZoneData RiverZone;
    RiverZone.ZoneType = EAudio_ZoneType::River;
    RiverZone.Center = FVector(0.0f, 5000.0f, 0.0f);
    RiverZone.Radius = 2000.0f;
    RiverZone.SpatialSettings.VolumeMultiplier = 0.7f;
    RiverZone.SpatialSettings.ReverbWetness = 0.3f;
    AudioZones.Add(RiverZone);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialManager: Setup %d default audio zones"), AudioZones.Num());
}

void UAudio_SpatialManager::FindPlayerActor()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerActor = PC->GetPawn();
            if (PlayerActor)
            {
                UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialManager: Found player actor: %s"), 
                       *PlayerActor->GetName());
            }
        }
    }
}