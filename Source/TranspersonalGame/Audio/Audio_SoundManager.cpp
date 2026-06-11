#include "Audio_SoundManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

AAudio_SoundManager::AAudio_SoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create master audio component
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    RootComponent = MasterAudioComponent;

    // Initialize default values
    MasterVolume = 1.0f;
    CurrentBiome = EBiomeType::Forest;
    LastPlayerPosition = FVector::ZeroVector;

    // Setup default sound zones
    FAudio_SoundZone ForestZone;
    ForestZone.ZoneName = TEXT("Forest");
    ForestZone.VolumeMultiplier = 0.7f;
    ForestZone.FadeDistance = 1500.0f;
    SoundZones.Add(ForestZone);

    FAudio_SoundZone CaveZone;
    CaveZone.ZoneName = TEXT("Cave");
    CaveZone.VolumeMultiplier = 0.4f;
    CaveZone.FadeDistance = 800.0f;
    SoundZones.Add(CaveZone);

    FAudio_SoundZone RiverZone;
    RiverZone.ZoneName = TEXT("River");
    RiverZone.VolumeMultiplier = 0.8f;
    RiverZone.FadeDistance = 1200.0f;
    SoundZones.Add(RiverZone);
}

void AAudio_SoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize audio system
    if (MasterAudioComponent)
    {
        MasterAudioComponent->SetVolumeMultiplier(MasterVolume);
        MasterAudioComponent->SetAutoActivate(true);
    }

    // Set initial biome ambient
    SetBiomeAmbient(CurrentBiome);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_SoundManager initialized with %d sound zones"), SoundZones.Num());
}

void AAudio_SoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update ambient audio based on player position
    UpdateAmbientAudio();
}

void AAudio_SoundManager::PlayDinosaurSound(EDinosaurSpecies Species, const FString& SoundType, const FVector& Location)
{
    if (!DinosaurSounds.Contains(Species))
    {
        UE_LOG(LogTemp, Warning, TEXT("No sound set found for dinosaur species"));
        return;
    }

    const FAudio_DinosaurSoundSet& SoundSet = DinosaurSounds[Species];
    USoundCue* SoundToPlay = nullptr;

    if (SoundType == TEXT("Idle"))
    {
        SoundToPlay = SoundSet.IdleSound;
    }
    else if (SoundType == TEXT("Alert"))
    {
        SoundToPlay = SoundSet.AlertSound;
    }
    else if (SoundType == TEXT("Attack"))
    {
        SoundToPlay = SoundSet.AttackSound;
    }
    else if (SoundType == TEXT("Footstep"))
    {
        SoundToPlay = SoundSet.FootstepSound;
    }

    if (SoundToPlay)
    {
        // Calculate attenuation based on distance
        float Attenuation = CalculateDistanceAttenuation(Location, LastPlayerPosition, SoundSet.MaxHearingDistance);
        
        if (Attenuation > 0.1f)
        {
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                SoundToPlay,
                Location,
                Attenuation * MasterVolume
            );
            
            UE_LOG(LogTemp, Log, TEXT("Played %s sound for dinosaur at distance %.2f"), *SoundType, FVector::Dist(Location, LastPlayerPosition));
        }
    }
}

void AAudio_SoundManager::PlayFootstepSound(const FVector& Location, bool bIsPlayer)
{
    if (PlayerFootstepSound)
    {
        float Volume = bIsPlayer ? 0.5f : 0.3f;
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            PlayerFootstepSound,
            Location,
            Volume * MasterVolume
        );
    }
}

void AAudio_SoundManager::PlayCraftingSound(const FVector& Location)
{
    if (CraftingSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            CraftingSound,
            Location,
            0.7f * MasterVolume
        );
    }
}

void AAudio_SoundManager::PlayDangerStinger()
{
    if (DangerStinger)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), DangerStinger, MasterVolume);
        UE_LOG(LogTemp, Log, TEXT("Played danger stinger"));
    }
}

void AAudio_SoundManager::SetBiomeAmbient(EBiomeType BiomeType)
{
    CurrentBiome = BiomeType;
    
    if (BiomeAmbientSounds.Contains(BiomeType))
    {
        USoundCue* AmbientSound = BiomeAmbientSounds[BiomeType];
        if (AmbientSound && MasterAudioComponent)
        {
            MasterAudioComponent->SetSound(AmbientSound);
            MasterAudioComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("Set biome ambient for type %d"), (int32)BiomeType);
        }
    }
}

void AAudio_SoundManager::UpdatePlayerPosition(const FVector& PlayerLocation)
{
    LastPlayerPosition = PlayerLocation;
    CheckSoundZones(PlayerLocation);
}

void AAudio_SoundManager::UpdateAmbientAudio()
{
    // This would typically check for biome transitions and update ambient accordingly
    // For now, we maintain the current biome ambient
}

void AAudio_SoundManager::CheckSoundZones(const FVector& PlayerLocation)
{
    // Check if player has entered any specific sound zones
    for (const FAudio_SoundZone& Zone : SoundZones)
    {
        // In a full implementation, this would check against actual zone geometry
        // For now, we use distance-based approximation
        float DistanceToZoneCenter = FVector::Dist(PlayerLocation, GetActorLocation());
        
        if (DistanceToZoneCenter < Zone.FadeDistance)
        {
            float VolumeModifier = FMath::Clamp(1.0f - (DistanceToZoneCenter / Zone.FadeDistance), 0.0f, 1.0f);
            VolumeModifier *= Zone.VolumeMultiplier;
            
            if (MasterAudioComponent)
            {
                MasterAudioComponent->SetVolumeMultiplier(VolumeModifier * MasterVolume);
            }
        }
    }
}

float AAudio_SoundManager::CalculateDistanceAttenuation(const FVector& SoundLocation, const FVector& ListenerLocation, float MaxDistance)
{
    float Distance = FVector::Dist(SoundLocation, ListenerLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear attenuation for simplicity
    return 1.0f - (Distance / MaxDistance);
}