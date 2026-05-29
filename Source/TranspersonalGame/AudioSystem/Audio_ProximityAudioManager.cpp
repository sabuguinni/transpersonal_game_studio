#include "Audio_ProximityAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "../TranspersonalCharacter.h"

UAudio_ProximityAudioManager::UAudio_ProximityAudioManager()
{
    LastPlayerLocation = FVector::ZeroVector;
    CurrentAudioMood = EAudioMood::Neutral;
    GlobalVolumeMultiplier = 1.0f;
    ProximityUpdateInterval = 0.1f;
    MaxPooledComponents = 20;
}

void UAudio_ProximityAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("UAudio_ProximityAudioManager: Initializing proximity audio system"));
    
    // Initialize audio component pool
    PooledAudioComponents.Reserve(MaxPooledComponents);
    
    // Set up proximity update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProximityUpdateTimer,
            [this]()
            {
                if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
                {
                    UpdatePlayerProximity(Player->GetActorLocation());
                }
            },
            ProximityUpdateInterval,
            true
        );
        
        // Set up dinosaur audio update timer
        World->GetTimerManager().SetTimer(
            DinosaurAudioTimer,
            this,
            &UAudio_ProximityAudioManager::UpdateDinosaurProximity,
            0.5f, // Update dinosaur audio every 0.5 seconds
            true
        );
    }
}

void UAudio_ProximityAudioManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("UAudio_ProximityAudioManager: Deinitializing proximity audio system"));
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProximityUpdateTimer);
        World->GetTimerManager().ClearTimer(DinosaurAudioTimer);
    }
    
    // Stop all audio
    StopAllProximityAudio();
    
    // Clean up pooled components
    for (UAudioComponent* Component : PooledAudioComponents)
    {
        if (IsValid(Component))
        {
            Component->Stop();
            Component->DestroyComponent();
        }
    }
    PooledAudioComponents.Empty();
    
    Super::Deinitialize();
}

void UAudio_ProximityAudioManager::RegisterAudioZone(const FAudio_ProximityAudioZone& AudioZone)
{
    if (!AudioZone.bIsActive)
    {
        return;
    }
    
    // Check if zone already exists at this location
    for (const FAudio_ProximityAudioZone& ExistingZone : RegisteredAudioZones)
    {
        if (FVector::Dist(ExistingZone.ZoneCenter, AudioZone.ZoneCenter) < 100.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Audio zone already exists near location: %s"), *AudioZone.ZoneCenter.ToString());
            return;
        }
    }
    
    RegisteredAudioZones.Add(AudioZone);
    
    // Create audio component for this zone
    UAudioComponent* AudioComp = CreateAudioComponentForZone(AudioZone);
    if (AudioComp)
    {
        ActiveAudioComponents.Add(AudioComp);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered audio zone at %s with %d total zones"), 
           *AudioZone.ZoneCenter.ToString(), RegisteredAudioZones.Num());
}

void UAudio_ProximityAudioManager::UnregisterAudioZone(FVector ZoneCenter, float Tolerance)
{
    for (int32 i = RegisteredAudioZones.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(RegisteredAudioZones[i].ZoneCenter, ZoneCenter) <= Tolerance)
        {
            RegisteredAudioZones.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Unregistered audio zone at %s"), *ZoneCenter.ToString());
            break;
        }
    }
    
    // Clean up associated audio components
    CleanupInactiveAudioComponents();
}

void UAudio_ProximityAudioManager::UpdatePlayerProximity(FVector PlayerLocation)
{
    if (FVector::Dist(PlayerLocation, LastPlayerLocation) < 50.0f)
    {
        return; // Player hasn't moved significantly
    }
    
    LastPlayerLocation = PlayerLocation;
    UpdateAudioZoneVolumes(PlayerLocation);
}

void UAudio_ProximityAudioManager::RegisterDinosaurActor(AActor* DinosaurActor, EDinosaurSpecies Species)
{
    if (!IsValid(DinosaurActor))
    {
        return;
    }
    
    FAudio_DinosaurAudioProfile AudioProfile;
    AudioProfile.Species = Species;
    
    // Set species-specific audio parameters
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            AudioProfile.FootstepInterval = 3.0f;
            AudioProfile.ProximityThreatDistance = 3000.0f;
            AudioProfile.VolumeMultiplier = 1.5f;
            break;
            
        case EDinosaurSpecies::Raptor:
            AudioProfile.FootstepInterval = 1.0f;
            AudioProfile.ProximityThreatDistance = 1500.0f;
            AudioProfile.VolumeMultiplier = 0.8f;
            break;
            
        case EDinosaurSpecies::Brachiosaurus:
            AudioProfile.FootstepInterval = 4.0f;
            AudioProfile.ProximityThreatDistance = 2500.0f;
            AudioProfile.VolumeMultiplier = 1.2f;
            break;
            
        default:
            AudioProfile.FootstepInterval = 2.0f;
            AudioProfile.ProximityThreatDistance = 2000.0f;
            AudioProfile.VolumeMultiplier = 1.0f;
            break;
    }
    
    DinosaurAudioMap.Add(DinosaurActor, AudioProfile);
    
    UE_LOG(LogTemp, Log, TEXT("Registered dinosaur audio for %s species"), 
           *UEnum::GetValueAsString(Species));
}

void UAudio_ProximityAudioManager::UpdateDinosaurProximity()
{
    if (!IsValid(GetWorld()))
    {
        return;
    }
    
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!IsValid(Player))
    {
        return;
    }
    
    FVector PlayerLocation = Player->GetActorLocation();
    
    // Clean up invalid dinosaur references
    TArray<AActor*> InvalidActors;
    for (auto& DinosaurPair : DinosaurAudioMap)
    {
        if (!IsValid(DinosaurPair.Key))
        {
            InvalidActors.Add(DinosaurPair.Key);
        }
        else
        {
            ProcessDinosaurProximityAudio(DinosaurPair.Key, DinosaurPair.Value);
        }
    }
    
    // Remove invalid actors
    for (AActor* InvalidActor : InvalidActors)
    {
        DinosaurAudioMap.Remove(InvalidActor);
    }
}

void UAudio_ProximityAudioManager::SetGlobalAudioMood(EAudioMood NewMood)
{
    if (CurrentAudioMood == NewMood)
    {
        return;
    }
    
    EAudioMood PreviousMood = CurrentAudioMood;
    CurrentAudioMood = NewMood;
    
    // Adjust global volume based on mood
    switch (NewMood)
    {
        case EAudioMood::Tense:
            GlobalVolumeMultiplier = 1.2f;
            break;
        case EAudioMood::Calm:
            GlobalVolumeMultiplier = 0.8f;
            break;
        case EAudioMood::Dangerous:
            GlobalVolumeMultiplier = 1.5f;
            break;
        default:
            GlobalVolumeMultiplier = 1.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio mood changed from %s to %s"), 
           *UEnum::GetValueAsString(PreviousMood), 
           *UEnum::GetValueAsString(NewMood));
}

void UAudio_ProximityAudioManager::PlayNarrativeAudio(const FString& AudioURL, FVector WorldLocation)
{
    if (AudioURL.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayNarrativeAudio: Empty audio URL provided"));
        return;
    }
    
    // For now, log the narrative audio request
    // In a full implementation, this would stream audio from the URL
    UE_LOG(LogTemp, Log, TEXT("Playing narrative audio from URL: %s at location: %s"), 
           *AudioURL, *WorldLocation.ToString());
    
    // TODO: Implement actual audio streaming from URL
    // This would require additional audio streaming components
}

void UAudio_ProximityAudioManager::StopAllProximityAudio()
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    
    ActiveAudioComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("Stopped all proximity audio"));
}

void UAudio_ProximityAudioManager::TriggerDinosaurFootstep(AActor* DinosaurActor, float IntensityMultiplier)
{
    if (!IsValid(DinosaurActor))
    {
        return;
    }
    
    FAudio_DinosaurAudioProfile* AudioProfile = DinosaurAudioMap.Find(DinosaurActor);
    if (!AudioProfile)
    {
        return;
    }
    
    // Calculate screen shake intensity based on dinosaur size and proximity
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (IsValid(Player))
    {
        float Distance = FVector::Dist(Player->GetActorLocation(), DinosaurActor->GetActorLocation());
        float ShakeIntensity = FMath::Clamp(IntensityMultiplier * AudioProfile->VolumeMultiplier * (2000.0f / FMath::Max(Distance, 100.0f)), 0.0f, 2.0f);
        
        if (ShakeIntensity > 0.1f)
        {
            // TODO: Trigger camera shake
            UE_LOG(LogTemp, Log, TEXT("Dinosaur footstep shake intensity: %f"), ShakeIntensity);
        }
    }
}

void UAudio_ProximityAudioManager::PlayDinosaurThreatSound(EDinosaurSpecies Species, FVector Location)
{
    // TODO: Load and play species-specific threat sounds
    UE_LOG(LogTemp, Log, TEXT("Playing %s threat sound at %s"), 
           *UEnum::GetValueAsString(Species), *Location.ToString());
}

void UAudio_ProximityAudioManager::DebugPrintAudioZones()
{
    UE_LOG(LogTemp, Log, TEXT("=== Audio Zone Debug Info ==="));
    UE_LOG(LogTemp, Log, TEXT("Total registered zones: %d"), RegisteredAudioZones.Num());
    UE_LOG(LogTemp, Log, TEXT("Active audio components: %d"), ActiveAudioComponents.Num());
    UE_LOG(LogTemp, Log, TEXT("Current audio mood: %s"), *UEnum::GetValueAsString(CurrentAudioMood));
    UE_LOG(LogTemp, Log, TEXT("Global volume multiplier: %f"), GlobalVolumeMultiplier);
    
    for (int32 i = 0; i < RegisteredAudioZones.Num(); i++)
    {
        const FAudio_ProximityAudioZone& Zone = RegisteredAudioZones[i];
        UE_LOG(LogTemp, Log, TEXT("Zone %d: Center=%s, InnerRadius=%f, OuterRadius=%f, Biome=%s"), 
               i, *Zone.ZoneCenter.ToString(), Zone.InnerRadius, Zone.OuterRadius, 
               *UEnum::GetValueAsString(Zone.BiomeType));
    }
}

int32 UAudio_ProximityAudioManager::GetActiveAudioZoneCount() const
{
    return RegisteredAudioZones.Num();
}

void UAudio_ProximityAudioManager::UpdateAudioZoneVolumes(FVector PlayerLocation)
{
    for (int32 i = 0; i < RegisteredAudioZones.Num(); i++)
    {
        const FAudio_ProximityAudioZone& Zone = RegisteredAudioZones[i];
        if (!Zone.bIsActive)
        {
            continue;
        }
        
        float ProximityVolume = CalculateProximityVolume(PlayerLocation, Zone);
        
        // Update corresponding audio component volume
        if (i < ActiveAudioComponents.Num() && IsValid(ActiveAudioComponents[i]))
        {
            float FinalVolume = ProximityVolume * Zone.BaseVolume * GlobalVolumeMultiplier;
            ActiveAudioComponents[i]->SetVolumeMultiplier(FinalVolume);
        }
    }
}

void UAudio_ProximityAudioManager::CleanupInactiveAudioComponents()
{
    for (int32 i = ActiveAudioComponents.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveAudioComponents[i]))
        {
            ActiveAudioComponents.RemoveAt(i);
        }
    }
}

UAudioComponent* UAudio_ProximityAudioManager::CreateAudioComponentForZone(const FAudio_ProximityAudioZone& AudioZone)
{
    if (!IsValid(GetWorld()))
    {
        return nullptr;
    }
    
    // Try to get a pooled component first
    UAudioComponent* AudioComp = nullptr;
    for (UAudioComponent* PooledComp : PooledAudioComponents)
    {
        if (IsValid(PooledComp) && !PooledComp->IsPlaying())
        {
            AudioComp = PooledComp;
            break;
        }
    }
    
    // Create new component if none available
    if (!AudioComp)
    {
        AudioComp = NewObject<UAudioComponent>(GetWorld());
        if (PooledAudioComponents.Num() < MaxPooledComponents)
        {
            PooledAudioComponents.Add(AudioComp);
        }
    }
    
    if (AudioComp)
    {
        // Configure audio component
        AudioComp->SetWorldLocation(AudioZone.ZoneCenter);
        AudioComp->SetVolumeMultiplier(0.0f); // Start silent, proximity will adjust
        AudioComp->bAutoActivate = false;
        
        // Set sound asset if available
        if (AudioZone.AmbientSound.IsValid())
        {
            AudioComp->SetSound(AudioZone.AmbientSound.Get());
        }
        
        UE_LOG(LogTemp, Log, TEXT("Created audio component for zone at %s"), *AudioZone.ZoneCenter.ToString());
    }
    
    return AudioComp;
}

float UAudio_ProximityAudioManager::CalculateProximityVolume(FVector PlayerLocation, const FAudio_ProximityAudioZone& AudioZone)
{
    float Distance = FVector::Dist(PlayerLocation, AudioZone.ZoneCenter);
    
    if (Distance <= AudioZone.InnerRadius)
    {
        return 1.0f; // Full volume
    }
    else if (Distance >= AudioZone.OuterRadius)
    {
        return 0.0f; // Silent
    }
    else
    {
        // Linear interpolation between inner and outer radius
        float Alpha = (AudioZone.OuterRadius - Distance) / (AudioZone.OuterRadius - AudioZone.InnerRadius);
        return FMath::Clamp(Alpha, 0.0f, 1.0f);
    }
}

void UAudio_ProximityAudioManager::ProcessDinosaurProximityAudio(AActor* DinosaurActor, const FAudio_DinosaurAudioProfile& AudioProfile)
{
    if (!IsValid(DinosaurActor) || !IsValid(GetWorld()))
    {
        return;
    }
    
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!IsValid(Player))
    {
        return;
    }
    
    FVector PlayerLocation = Player->GetActorLocation();
    FVector DinosaurLocation = DinosaurActor->GetActorLocation();
    
    float ThreatLevel = CalculateDinosaurThreatLevel(PlayerLocation, DinosaurLocation, AudioProfile.ProximityThreatDistance);
    
    if (ThreatLevel > 0.1f)
    {
        // Trigger appropriate audio based on threat level
        if (ThreatLevel > 0.8f)
        {
            // High threat - play aggressive sounds
            PlayDinosaurThreatSound(AudioProfile.Species, DinosaurLocation);
        }
        else if (ThreatLevel > 0.4f)
        {
            // Medium threat - play movement sounds
            // TODO: Play movement audio
        }
        
        // Update audio mood based on highest threat level
        if (ThreatLevel > 0.8f && CurrentAudioMood != EAudioMood::Dangerous)
        {
            SetGlobalAudioMood(EAudioMood::Dangerous);
        }
        else if (ThreatLevel > 0.4f && CurrentAudioMood == EAudioMood::Neutral)
        {
            SetGlobalAudioMood(EAudioMood::Tense);
        }
    }
}

float UAudio_ProximityAudioManager::CalculateDinosaurThreatLevel(FVector PlayerLocation, FVector DinosaurLocation, float ThreatDistance)
{
    float Distance = FVector::Dist(PlayerLocation, DinosaurLocation);
    
    if (Distance >= ThreatDistance)
    {
        return 0.0f;
    }
    
    float ThreatLevel = 1.0f - (Distance / ThreatDistance);
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}