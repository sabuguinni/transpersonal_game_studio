#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

AAudioManager::AAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5 seconds

    // Create master audio component
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    RootComponent = MasterAudioComponent;

    // Initialize default values
    MasterVolume = 1.0f;
    AmbientVolume = 0.7f;
    SFXVolume = 0.8f;
    VoiceVolume = 0.9f;
    bUseMetaSounds = true;
    
    LastZoneUpdateTime = 0.0f;
    CurrentActiveZone = nullptr;
}

void AAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioZones();
    SetupDinosaurSounds();
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager initialized with %d ambient zones"), AmbientZones.Num());
}

void AAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update ambient audio based on player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        UpdateAmbientZone(PlayerPawn->GetActorLocation());
    }
}

void AAudioManager::InitializeAudioZones()
{
    AmbientZones.Empty();
    
    // Forest zone
    FAudio_SoundZone ForestZone;
    ForestZone.Location = FVector(1000, 1000, 200);
    ForestZone.Radius = 1500.0f;
    ForestZone.Volume = 0.6f;
    ForestZone.bIsActive = true;
    AmbientZones.Add(ForestZone);
    
    // River zone
    FAudio_SoundZone RiverZone;
    RiverZone.Location = FVector(-500, 2000, 100);
    RiverZone.Radius = 800.0f;
    RiverZone.Volume = 0.8f;
    RiverZone.bIsActive = true;
    AmbientZones.Add(RiverZone);
    
    // Camp zone
    FAudio_SoundZone CampZone;
    CampZone.Location = FVector(0, 0, 150);
    CampZone.Radius = 500.0f;
    CampZone.Volume = 0.5f;
    CampZone.bIsActive = true;
    AmbientZones.Add(CampZone);
    
    // Danger zone (T-Rex territory)
    FAudio_SoundZone DangerZone;
    DangerZone.Location = FVector(2000, -1000, 200);
    DangerZone.Radius = 1200.0f;
    DangerZone.Volume = 0.4f;
    DangerZone.bIsActive = true;
    AmbientZones.Add(DangerZone);
}

void AAudioManager::SetupDinosaurSounds()
{
    // Initialize T-Rex sounds
    FAudio_DinosaurSounds TRexSounds;
    TRexSounds.RoarVolume = 1.0f;
    TRexSounds.FootstepVolume = 0.9f;
    TRexSounds.MaxHearingDistance = 3000.0f;
    DinosaurSoundMap.Add(EDinosaurSpecies::TRex, TRexSounds);
    
    // Initialize Raptor sounds
    FAudio_DinosaurSounds RaptorSounds;
    RaptorSounds.RoarVolume = 0.7f;
    RaptorSounds.FootstepVolume = 0.4f;
    RaptorSounds.MaxHearingDistance = 1500.0f;
    DinosaurSoundMap.Add(EDinosaurSpecies::Raptor, RaptorSounds);
    
    // Initialize Brachiosaurus sounds
    FAudio_DinosaurSounds BrachioSounds;
    BrachioSounds.RoarVolume = 0.8f;
    BrachioSounds.FootstepVolume = 1.0f;
    BrachioSounds.MaxHearingDistance = 2500.0f;
    DinosaurSoundMap.Add(EDinosaurSpecies::Brachiosaurus, BrachioSounds);
    
    // Initialize Triceratops sounds
    FAudio_DinosaurSounds TriceratopsSounds;
    TriceratopsSounds.RoarVolume = 0.6f;
    TriceratopsSounds.FootstepVolume = 0.7f;
    TriceratopsSounds.MaxHearingDistance = 1800.0f;
    DinosaurSoundMap.Add(EDinosaurSpecies::Triceratops, TriceratopsSounds);
}

void AAudioManager::PlayDinosaurSound(EDinosaurSpecies Species, const FString& SoundType, const FVector& Location)
{
    if (!DinosaurSoundMap.Contains(Species))
    {
        UE_LOG(LogTemp, Warning, TEXT("No sound data for dinosaur species"));
        return;
    }
    
    const FAudio_DinosaurSounds& SoundData = DinosaurSoundMap[Species];
    USoundCue* SoundToPlay = nullptr;
    float Volume = SFXVolume;
    
    if (SoundType == "Roar")
    {
        SoundToPlay = SoundData.RoarSound.LoadSynchronous();
        Volume *= SoundData.RoarVolume;
    }
    else if (SoundType == "Footstep")
    {
        SoundToPlay = SoundData.FootstepSound.LoadSynchronous();
        Volume *= SoundData.FootstepVolume;
    }
    else if (SoundType == "Breathing")
    {
        SoundToPlay = SoundData.BreathingSound.LoadSynchronous();
        Volume *= 0.6f;
    }
    
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            SoundToPlay,
            Location,
            Volume * MasterVolume,
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation override
            nullptr, // Concurrency settings
            nullptr  // Owner
        );
        
        UE_LOG(LogTemp, Log, TEXT("Played %s sound for dinosaur at location %s"), 
               *SoundType, *Location.ToString());
    }
}

void AAudioManager::UpdateAmbientZone(const FVector& PlayerLocation)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastZoneUpdateTime < 1.0f) // Update once per second
    {
        return;
    }
    
    LastZoneUpdateTime = CurrentTime;
    
    FAudio_SoundZone* ClosestZone = GetClosestAmbientZone(PlayerLocation);
    
    if (ClosestZone && ClosestZone != CurrentActiveZone)
    {
        // Stop current ambient sound
        if (CurrentActiveZone)
        {
            StopAllAmbientSounds();
        }
        
        // Start new ambient sound
        if (ClosestZone->AmbientSound.LoadSynchronous())
        {
            UAudioComponent* NewAmbientComponent = UGameplayStatics::SpawnSoundAtLocation(
                GetWorld(),
                ClosestZone->AmbientSound.LoadSynchronous(),
                ClosestZone->Location,
                FRotator::ZeroRotator,
                ClosestZone->Volume * AmbientVolume * MasterVolume,
                1.0f, // Pitch
                0.0f, // Start time
                nullptr, // Attenuation
                nullptr, // Concurrency
                true // Auto destroy
            );
            
            if (NewAmbientComponent)
            {
                ActiveAudioComponents.Add(NewAmbientComponent);
                CurrentActiveZone = ClosestZone;
                UE_LOG(LogTemp, Log, TEXT("Switched to ambient zone at %s"), 
                       *ClosestZone->Location.ToString());
            }
        }
    }
}

FAudio_SoundZone* AAudioManager::GetClosestAmbientZone(const FVector& Location)
{
    FAudio_SoundZone* ClosestZone = nullptr;
    float ClosestDistance = FLT_MAX;
    
    for (FAudio_SoundZone& Zone : AmbientZones)
    {
        if (!Zone.bIsActive) continue;
        
        float Distance = FVector::Dist(Location, Zone.Location);
        if (Distance <= Zone.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestZone = &Zone;
        }
    }
    
    return ClosestZone;
}

void AAudioManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    // Update all active audio components
    for (UAudioComponent* Component : ActiveAudioComponents)
    {
        if (IsValid(Component))
        {
            Component->SetVolumeMultiplier(MasterVolume);
        }
    }
}

void AAudioManager::PlaySurvivalNarration(const FString& NarrationKey)
{
    // This would load and play narration audio based on the key
    // For now, just log the request
    UE_LOG(LogTemp, Log, TEXT("Playing survival narration: %s"), *NarrationKey);
    
    // In a full implementation, this would:
    // 1. Look up the narration file by key
    // 2. Load the audio asset
    // 3. Play it through a dedicated narration audio component
    // 4. Handle subtitles if needed
}

void AAudioManager::StopAllAmbientSounds()
{
    for (UAudioComponent* Component : ActiveAudioComponents)
    {
        if (IsValid(Component))
        {
            Component->Stop();
        }
    }
    
    ActiveAudioComponents.Empty();
    CurrentActiveZone = nullptr;
}