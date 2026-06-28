#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"

AAudioSystemManager::AAudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentMusicState = EAudio_GameState::Exploration;
    MusicTransitionBlendTime = 2.0f;
    DinosaurProximityThreshold = 1500.0f;
    TRexRumbleRadius = 3000.0f;
    CampfireAudioRadius = 800.0f;
    TimeSinceLastZoneUpdate = 0.0f;

    // Create music audio component
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->bAutoActivate = false;
    MusicAudioComponent->SetIsReplicated(false);
}

void AAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Register default narration lines from ElevenLabs TTS
    // Survivor_Warning — T-Rex proximity warning
    RegisterNarrationLine(
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782673193481_Survivor_Warning.mp3"),
        TEXT("SURVIVOR_TREX_WARNING")
    );

    // Elder_Survivor — campfire/camp setup instruction
    RegisterNarrationLine(
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782673212760_Elder_Survivor.mp3"),
        TEXT("ELDER_CAMPFIRE_INSTRUCTION")
    );

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initialized with %d narration lines"), NarrationRegistry.Num());
}

void AAudioSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastZoneUpdate += DeltaTime;
    if (TimeSinceLastZoneUpdate >= ZoneUpdateInterval)
    {
        TimeSinceLastZoneUpdate = 0.0f;

        // Get player location for ambient zone updates
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            UpdateAmbientZones(PC->GetPawn()->GetActorLocation());
        }
    }
}

void AAudioSystemManager::SetMusicState(EAudio_GameState NewState)
{
    if (CurrentMusicState == NewState)
    {
        return;
    }

    EAudio_GameState PreviousState = CurrentMusicState;
    CurrentMusicState = NewState;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Music state transition %d -> %d (blend: %.1fs)"),
        (int32)PreviousState, (int32)NewState, MusicTransitionBlendTime);

    // In a full implementation, this would crossfade MetaSound parameters
    // For now, log the transition for Blueprint integration
}

EAudio_GameState AAudioSystemManager::GetCurrentMusicState() const
{
    return CurrentMusicState;
}

void AAudioSystemManager::RegisterAmbientZone(const FAudio_AmbientZone& Zone)
{
    RegisteredAmbientZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered ambient zone '%s' at radius %.0f"),
        *Zone.ZoneName, Zone.ZoneRadius);
}

void AAudioSystemManager::UpdateAmbientZones(const FVector& PlayerLocation)
{
    for (const FAudio_AmbientZone& Zone : RegisteredAmbientZones)
    {
        float Distance = FVector::Dist(PlayerLocation, Zone.ZoneCenter);
        bool bInZone = Distance <= Zone.ZoneRadius;

        if (bInZone && Zone.AmbientSound)
        {
            // Volume falloff based on distance
            float NormalizedDist = FMath::Clamp(Distance / Zone.ZoneRadius, 0.0f, 1.0f);
            float FadedVolume = Zone.Volume * (1.0f - NormalizedDist);
            // Blueprint or MetaSound would handle actual audio playback
        }
    }
}

void AAudioSystemManager::TriggerDinosaurRoar(AActor* DinosaurActor, float Intensity)
{
    if (!DinosaurActor)
    {
        return;
    }

    FVector DinoLocation = DinosaurActor->GetActorLocation();
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Dinosaur roar at (%.0f, %.0f, %.0f) intensity=%.2f"),
        DinoLocation.X, DinoLocation.Y, DinoLocation.Z, Intensity);

    // Transition music to Danger state when a dino roars nearby
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        float DistToPlayer = FVector::Dist(DinoLocation, PC->GetPawn()->GetActorLocation());
        if (DistToPlayer <= DinosaurProximityThreshold)
        {
            SetMusicState(EAudio_GameState::Danger);
        }
    }
}

void AAudioSystemManager::TriggerFootstepImpact(AActor* DinosaurActor, float Weight)
{
    if (!DinosaurActor)
    {
        return;
    }

    // Heavy footstep — trigger screen shake scaled by weight and distance
    float ShakeMagnitude = FMath::Clamp(Weight * 0.5f, 0.1f, 2.0f);
    TriggerTRexScreenShake(ShakeMagnitude, 0.3f);
}

void AAudioSystemManager::SpawnCampfireAudio(const FVector& Location)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Campfire audio spawned at (%.0f, %.0f, %.0f) radius=%.0f"),
        Location.X, Location.Y, Location.Z, CampfireAudioRadius);
    // Freesound asset ID 394952 (crackling campfire) can be imported and assigned here
    // Blueprint will handle actual USoundCue assignment
}

void AAudioSystemManager::TriggerTRexScreenShake(float Magnitude, float Duration)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        return;
    }

    // Scale: T-Rex footstep magnitude 1.0 = noticeable rumble
    // Uses built-in UE5 camera shake system
    PC->ClientStartCameraShake(UCameraShakeBase::StaticClass(), Magnitude);

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Screen shake triggered — magnitude=%.2f duration=%.2f"),
        Magnitude, Duration);
}

void AAudioSystemManager::RegisterNarrationLine(const FString& AudioURL, const FString& LineID)
{
    NarrationRegistry.Add(LineID, AudioURL);
    NarrationAudioURLs.Add(AudioURL);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Narration line registered — ID='%s'"), *LineID);
}

void AAudioSystemManager::PlayNarrationLine(const FString& LineID)
{
    FString* URL = NarrationRegistry.Find(LineID);
    if (URL)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing narration '%s' from URL: %s"),
            *LineID, **URL);
        // In full implementation: load audio from URL and play via AudioComponent
        // For now, Blueprint handles the actual playback via the URL
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Narration line '%s' not found"), *LineID);
    }
}
