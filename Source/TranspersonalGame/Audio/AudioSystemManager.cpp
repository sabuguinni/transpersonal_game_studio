#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

AAudioSystemManager::AAudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;

    DangerAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DangerAudioComponent"));
    DangerAudioComponent->SetupAttachment(RootComponent);
    DangerAudioComponent->bAutoActivate = false;

    // Initialize settings
    CurrentBiome = EAudio_BiomeType::Savana;
    bInDangerZone = false;
    BiomeDetectionRadius = 10000.0f;
    AudioUpdateInterval = 2.0f;
    PlayerPawn = nullptr;

    // Initialize biome audio settings
    BiomeAudioSettings.SetNum(5);
    
    // Savana settings
    BiomeAudioSettings[0].BiomeType = EAudio_BiomeType::Savana;
    BiomeAudioSettings[0].AmbientVolume = 0.6f;
    BiomeAudioSettings[0].MusicVolume = 0.4f;
    BiomeAudioSettings[0].CrossfadeTime = 3.0f;

    // Pantano settings
    BiomeAudioSettings[1].BiomeType = EAudio_BiomeType::Pantano;
    BiomeAudioSettings[1].AmbientVolume = 0.8f;
    BiomeAudioSettings[1].MusicVolume = 0.3f;
    BiomeAudioSettings[1].CrossfadeTime = 4.0f;

    // Floresta settings
    BiomeAudioSettings[2].BiomeType = EAudio_BiomeType::Floresta;
    BiomeAudioSettings[2].AmbientVolume = 0.7f;
    BiomeAudioSettings[2].MusicVolume = 0.5f;
    BiomeAudioSettings[2].CrossfadeTime = 2.5f;

    // Deserto settings
    BiomeAudioSettings[3].BiomeType = EAudio_BiomeType::Deserto;
    BiomeAudioSettings[3].AmbientVolume = 0.5f;
    BiomeAudioSettings[3].MusicVolume = 0.6f;
    BiomeAudioSettings[3].CrossfadeTime = 5.0f;

    // Montanha settings
    BiomeAudioSettings[4].BiomeType = EAudio_BiomeType::Montanha;
    BiomeAudioSettings[4].AmbientVolume = 0.9f;
    BiomeAudioSettings[4].MusicVolume = 0.4f;
    BiomeAudioSettings[4].CrossfadeTime = 3.5f;
}

void AAudioSystemManager::BeginPlay()
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

    // Start audio update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AudioUpdateTimer,
            this,
            &AAudioSystemManager::UpdateAudioSystem,
            AudioUpdateInterval,
            true
        );
    }

    // Initialize with current biome
    SetCurrentBiome(CurrentBiome);
}

void AAudioSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update danger state based on nearby dinosaurs
    float DinosaurDistance = CalculateDistanceToNearestDinosaur();
    bool bShouldBeInDanger = DinosaurDistance < DangerSettings.DangerRadius;
    
    if (bShouldBeInDanger != bInDangerZone)
    {
        SetDangerState(bShouldBeInDanger);
    }
}

void AAudioSystemManager::UpdateAudioSystem()
{
    if (!PlayerPawn)
    {
        return;
    }

    // Detect current biome based on player location
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    EAudio_BiomeType DetectedBiome = DetectBiomeAtLocation(PlayerLocation);

    // Change biome if different
    if (DetectedBiome != CurrentBiome)
    {
        SetCurrentBiome(DetectedBiome);
    }

    // Update danger audio
    UpdateDangerAudio();
}

void AAudioSystemManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }

    EAudio_BiomeType OldBiome = CurrentBiome;
    CurrentBiome = NewBiome;

    // Crossfade to new biome audio
    CrossfadeToNewBiome(NewBiome);

    // Trigger Blueprint event
    OnBiomeChanged(OldBiome, NewBiome);
}

void AAudioSystemManager::SetDangerState(bool bDangerous)
{
    if (bDangerous == bInDangerZone)
    {
        return;
    }

    bInDangerZone = bDangerous;

    // Update danger audio
    UpdateDangerAudio();

    // Trigger Blueprint event
    OnDangerStateChanged(bInDangerZone);
}

void AAudioSystemManager::PlayDinosaurRoar(FVector Location, float Intensity)
{
    if (DangerSettings.DinosaurRoarCue.IsValid())
    {
        if (USoundCue* RoarSound = DangerSettings.DinosaurRoarCue.LoadSynchronous())
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                RoarSound,
                Location,
                Intensity * DangerSettings.IntensityMultiplier
            );
        }
    }
}

void AAudioSystemManager::PlayFootstepAudio(FVector Location, bool bHeavyFootsteps)
{
    if (DangerSettings.FootstepsCue.IsValid())
    {
        if (USoundCue* FootstepsSound = DangerSettings.FootstepsCue.LoadSynchronous())
        {
            float VolumeMultiplier = bHeavyFootsteps ? 1.5f : 1.0f;
            UGameplayStatics::PlaySoundAtLocation(
                this,
                FootstepsSound,
                Location,
                VolumeMultiplier
            );
        }
    }
}

EAudio_BiomeType AAudioSystemManager::DetectBiomeAtLocation(FVector Location)
{
    // Biome detection based on world coordinates
    // Savana: center (0,0)
    // Pantano: SW (-50000,-45000)
    // Floresta: NW (-45000,40000)
    // Deserto: E (55000,0)
    // Montanha: NE (40000,50000)

    float X = Location.X;
    float Y = Location.Y;

    // Calculate distances to biome centers
    float DistToSavana = FVector::Dist2D(Location, FVector(0, 0, 0));
    float DistToPantano = FVector::Dist2D(Location, FVector(-50000, -45000, 0));
    float DistToFloresta = FVector::Dist2D(Location, FVector(-45000, 40000, 0));
    float DistToDeserto = FVector::Dist2D(Location, FVector(55000, 0, 0));
    float DistToMontanha = FVector::Dist2D(Location, FVector(40000, 50000, 0));

    // Find closest biome
    float MinDistance = DistToSavana;
    EAudio_BiomeType ClosestBiome = EAudio_BiomeType::Savana;

    if (DistToPantano < MinDistance)
    {
        MinDistance = DistToPantano;
        ClosestBiome = EAudio_BiomeType::Pantano;
    }
    if (DistToFloresta < MinDistance)
    {
        MinDistance = DistToFloresta;
        ClosestBiome = EAudio_BiomeType::Floresta;
    }
    if (DistToDeserto < MinDistance)
    {
        MinDistance = DistToDeserto;
        ClosestBiome = EAudio_BiomeType::Deserto;
    }
    if (DistToMontanha < MinDistance)
    {
        MinDistance = DistToMontanha;
        ClosestBiome = EAudio_BiomeType::Montanha;
    }

    return ClosestBiome;
}

void AAudioSystemManager::CrossfadeToNewBiome(EAudio_BiomeType NewBiome)
{
    // Find biome settings
    FAudio_BiomeAudioData* BiomeData = nullptr;
    for (FAudio_BiomeAudioData& Data : BiomeAudioSettings)
    {
        if (Data.BiomeType == NewBiome)
        {
            BiomeData = &Data;
            break;
        }
    }

    if (!BiomeData)
    {
        return;
    }

    // Stop current audio
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->FadeOut(BiomeData->CrossfadeTime, 0.0f);
    }
    if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->FadeOut(BiomeData->CrossfadeTime, 0.0f);
    }

    // Start new biome audio
    if (BiomeData->AmbientSoundCue.IsValid())
    {
        if (USoundCue* AmbientSound = BiomeData->AmbientSoundCue.LoadSynchronous())
        {
            AmbientAudioComponent->SetSound(AmbientSound);
            AmbientAudioComponent->FadeIn(BiomeData->CrossfadeTime, BiomeData->AmbientVolume);
        }
    }

    if (BiomeData->MusicCue.IsValid())
    {
        if (USoundCue* MusicSound = BiomeData->MusicCue.LoadSynchronous())
        {
            MusicAudioComponent->SetSound(MusicSound);
            MusicAudioComponent->FadeIn(BiomeData->CrossfadeTime, BiomeData->MusicVolume);
        }
    }
}

void AAudioSystemManager::UpdateDangerAudio()
{
    if (bInDangerZone)
    {
        // Start tension music if not already playing
        if (DangerSettings.TensionMusicCue.IsValid() && !DangerAudioComponent->IsPlaying())
        {
            if (USoundCue* TensionSound = DangerSettings.TensionMusicCue.LoadSynchronous())
            {
                DangerAudioComponent->SetSound(TensionSound);
                DangerAudioComponent->FadeIn(1.0f, 0.8f);
            }
        }
    }
    else
    {
        // Stop tension music
        if (DangerAudioComponent->IsPlaying())
        {
            DangerAudioComponent->FadeOut(2.0f, 0.0f);
        }
    }
}

float AAudioSystemManager::CalculateDistanceToNearestDinosaur()
{
    if (!PlayerPawn)
    {
        return 99999.0f;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return 99999.0f;
    }

    float MinDistance = 99999.0f;
    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Find all actors with "Dinosaur" or "TRex" or "Raptor" in their name
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }

        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Dinosaur")) || 
            ActorName.Contains(TEXT("TRex")) || 
            ActorName.Contains(TEXT("Raptor")) ||
            ActorName.Contains(TEXT("Brachiosaurus")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
            }
        }
    }

    return MinDistance;
}