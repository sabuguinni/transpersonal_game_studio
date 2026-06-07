#include "Audio_PrehistoricAudioManager.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AAudio_PrehistoricAudioManager::AAudio_PrehistoricAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create master audio component
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    MasterAudioComponent->SetupAttachment(RootComponent);

    // Initialize default values
    CurrentThreatLevel = 0.0f;
    bThreatMusicPlaying = false;
    PlayerPawn = nullptr;

    // Initialize default ambient zones
    FAudio_AmbientZone ForestZone;
    ForestZone.BiomeType = EAudio_BiomeType::Forest;
    ForestZone.Volume = 0.7f;
    ForestZone.FadeDistance = 1500.0f;
    AmbientZones.Add(ForestZone);

    FAudio_AmbientZone CanyonZone;
    CanyonZone.BiomeType = EAudio_BiomeType::Canyon;
    CanyonZone.Volume = 0.5f;
    CanyonZone.FadeDistance = 2500.0f;
    AmbientZones.Add(CanyonZone);
}

void AAudio_PrehistoricAudioManager::BeginPlay()
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

    // Start ambient audio processing
    ProcessAmbientZones();
}

void AAudio_PrehistoricAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update spatial audio based on player position
    UpdateSpatialAudio();

    // Update threat level based on nearby dinosaurs
    if (PlayerPawn)
    {
        // Simple threat detection - in real implementation would check for nearby dinosaurs
        float DistanceToNearestThreat = 5000.0f; // Placeholder
        
        if (DistanceToNearestThreat < 1000.0f)
        {
            float NewThreatLevel = FMath::Clamp(1.0f - (DistanceToNearestThreat / 1000.0f), 0.0f, 1.0f);
            
            if (NewThreatLevel > 0.3f && !bThreatMusicPlaying)
            {
                PlayThreatMusic(NewThreatLevel);
            }
            else if (NewThreatLevel < 0.1f && bThreatMusicPlaying)
            {
                StopThreatMusic();
            }
            
            CurrentThreatLevel = NewThreatLevel;
        }
    }
}

void AAudio_PrehistoricAudioManager::PlayDinosaurRoar(FVector Location, float Volume)
{
    if (TRexRoarSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            TRexRoarSound,
            Location,
            Volume,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            true
        );

        UE_LOG(LogTemp, Log, TEXT("AudioManager: Played dinosaur roar at location %s"), *Location.ToString());
    }
}

void AAudio_PrehistoricAudioManager::PlayFootstepSound(FVector Location, bool bIsHeavy)
{
    USoundCue* SoundToPlay = bIsHeavy ? HeavyFootstepSound : PlayerFootstepSound;
    
    if (SoundToPlay)
    {
        float Volume = bIsHeavy ? 1.0f : 0.6f;
        
        UGameplayStatics::PlaySoundAtLocation(
            this,
            SoundToPlay,
            Location,
            Volume,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            true
        );
    }
}

void AAudio_PrehistoricAudioManager::SetAmbientZone(EAudio_BiomeType BiomeType)
{
    for (const FAudio_AmbientZone& Zone : AmbientZones)
    {
        if (Zone.BiomeType == BiomeType && Zone.AmbientSound)
        {
            if (MasterAudioComponent)
            {
                MasterAudioComponent->SetSound(Zone.AmbientSound);
                MasterAudioComponent->SetVolumeMultiplier(Zone.Volume);
                MasterAudioComponent->Play();
                
                UE_LOG(LogTemp, Log, TEXT("AudioManager: Switched to %d biome ambient"), (int32)BiomeType);
            }
            break;
        }
    }
}

void AAudio_PrehistoricAudioManager::PlayThreatMusic(float Intensity)
{
    if (!bThreatMusicPlaying)
    {
        bThreatMusicPlaying = true;
        CurrentThreatLevel = Intensity;
        
        // In a real implementation, would play dynamic threat music
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: THREAT MUSIC STARTED - Intensity: %f"), Intensity);
    }
}

void AAudio_PrehistoricAudioManager::StopThreatMusic()
{
    if (bThreatMusicPlaying)
    {
        bThreatMusicPlaying = false;
        CurrentThreatLevel = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Threat music stopped"));
    }
}

void AAudio_PrehistoricAudioManager::UpdateSpatialAudio()
{
    if (!PlayerPawn || !MasterAudioComponent)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector AudioManagerLocation = GetActorLocation();
    
    float DistanceToPlayer = FVector::Dist(PlayerLocation, AudioManagerLocation);
    
    // Adjust volume based on distance
    float VolumeMultiplier = FMath::Clamp(1.0f - (DistanceToPlayer / 3000.0f), 0.0f, 1.0f);
    MasterAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
}

void AAudio_PrehistoricAudioManager::ProcessAmbientZones()
{
    // Set default forest ambience
    SetAmbientZone(EAudio_BiomeType::Forest);
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Ambient zones initialized"));
}