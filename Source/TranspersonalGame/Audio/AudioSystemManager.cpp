#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UAudio_SystemManager::UAudio_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    TimeSinceLastTick = 0.0f;
    DangerLevel = 0.0f;
    MusicTransitionSpeed = 1.0f;
    bNightMode = false;

    // === TTS Audio URLs — produced by Agent #15 + Agent #16 (PROD_CYCLE_AUTO_20260624_010) ===
    TrackerElderWarningURL  = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344157638_Tracker_Elder.mp3");
    TribeLeaderKaelURL      = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344176100_Tribe_Leader_Kael.mp3");
    ElderNarratorURL        = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344199704_Elder_Narrator.mp3");
    ScoutMiraURL            = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344208387_Scout_Mira.mp3");
    SurvivalNarratorURL     = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344315523_Survival_Narrator.mp3");
    TrackerElderRaptorURL   = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344344038_Tracker_Elder.mp3");

    // === Freesound References ===
    // Campfire (Position 2) — id:681367 — 22s loop
    CampfireAmbientURL = TEXT("https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3");

    // Screen shake defaults for T-Rex proximity
    TRexShakeConfig.ShakeIntensity = 1.5f;
    TRexShakeConfig.ShakeDuration  = 0.4f;
    TRexShakeConfig.TriggerRadius  = 1200.0f;
}

void UAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultZones();
}

void UAudio_SystemManager::InitializeDefaultZones()
{
    AudioZones.Empty();

    // Zone 1 — T-Rex Valley (matches NarrZone_TRex_Valley from Agent #15)
    {
        FAudio_ZoneConfig Zone;
        Zone.ZoneLabel        = TEXT("TRex_Valley");
        Zone.Location         = FVector(2000.0f, 2500.0f, 200.0f);
        Zone.TriggerRadius    = 900.0f;
        Zone.ZoneType         = EAudio_ZoneType::Danger;
        Zone.NarrativeAudioURL = TrackerElderWarningURL;
        Zone.bOneShot         = false;
        Zone.bHasTriggered    = false;
        AudioZones.Add(Zone);
    }

    // Zone 2 — Raptor River (matches NarrZone_Raptor_River from Agent #15)
    {
        FAudio_ZoneConfig Zone;
        Zone.ZoneLabel        = TEXT("Raptor_River");
        Zone.Location         = FVector(-1500.0f, 3000.0f, 200.0f);
        Zone.TriggerRadius    = 800.0f;
        Zone.ZoneType         = EAudio_ZoneType::Danger;
        Zone.NarrativeAudioURL = TribeLeaderKaelURL;
        Zone.bOneShot         = false;
        Zone.bHasTriggered    = false;
        AudioZones.Add(Zone);
    }

    // Zone 3 — Brachio Plains (matches NarrZone_Brachio_Plains from Agent #15)
    {
        FAudio_ZoneConfig Zone;
        Zone.ZoneLabel        = TEXT("Brachio_Plains");
        Zone.Location         = FVector(2700.0f, 1800.0f, 200.0f);
        Zone.TriggerRadius    = 1000.0f;
        Zone.ZoneType         = EAudio_ZoneType::Ambient;
        Zone.NarrativeAudioURL = SurvivalNarratorURL;
        Zone.bOneShot         = true;
        Zone.bHasTriggered    = false;
        AudioZones.Add(Zone);
    }

    // Zone 4 — Camp Entrance (matches NarrZone_Camp_Entrance from Agent #15)
    {
        FAudio_ZoneConfig Zone;
        Zone.ZoneLabel        = TEXT("Camp_Entrance");
        Zone.Location         = FVector(0.0f, 0.0f, 200.0f);
        Zone.TriggerRadius    = 700.0f;
        Zone.ZoneType         = EAudio_ZoneType::Safe;
        Zone.NarrativeAudioURL = ElderNarratorURL;
        Zone.bOneShot         = true;
        Zone.bHasTriggered    = false;
        AudioZones.Add(Zone);
    }

    // Zone 5 — Raptor Ambush (additional danger zone for Scout Mira escape line)
    {
        FAudio_ZoneConfig Zone;
        Zone.ZoneLabel        = TEXT("Raptor_Ambush_East");
        Zone.Location         = FVector(-800.0f, 4200.0f, 200.0f);
        Zone.TriggerRadius    = 600.0f;
        Zone.ZoneType         = EAudio_ZoneType::Combat;
        Zone.NarrativeAudioURL = ScoutMiraURL;
        Zone.bOneShot         = true;
        Zone.bHasTriggered    = false;
        AudioZones.Add(Zone);
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Initialized %d audio zones."), AudioZones.Num());
}

void UAudio_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastTick += DeltaTime;
    if (TimeSinceLastTick < 0.5f)
    {
        return;
    }
    TimeSinceLastTick = 0.0f;

    // Get player location
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
    TickAudioZones(PlayerLoc);
}

void UAudio_SystemManager::TickAudioZones(const FVector& PlayerLocation)
{
    float MaxDangerContribution = 0.0f;

    for (FAudio_ZoneConfig& Zone : AudioZones)
    {
        float Dist = FVector::Dist(PlayerLocation, Zone.Location);

        if (Dist <= Zone.TriggerRadius)
        {
            // Trigger narrative audio if not yet triggered (or repeatable)
            if (!Zone.bHasTriggered || !Zone.bOneShot)
            {
                TriggerZone(Zone);
                Zone.bHasTriggered = true;
            }

            // Contribute to danger level
            float Contribution = 0.0f;
            switch (Zone.ZoneType)
            {
                case EAudio_ZoneType::Combat:  Contribution = 1.0f; break;
                case EAudio_ZoneType::Danger:  Contribution = 0.7f; break;
                case EAudio_ZoneType::Ambient: Contribution = 0.2f; break;
                case EAudio_ZoneType::Safe:    Contribution = 0.0f; break;
                default: break;
            }
            MaxDangerContribution = FMath::Max(MaxDangerContribution, Contribution);
        }
    }

    // Smoothly update danger level
    DangerLevel = FMath::FInterpTo(DangerLevel, MaxDangerContribution, 0.5f, MusicTransitionSpeed);
}

void UAudio_SystemManager::TriggerZone(const FAudio_ZoneConfig& Zone)
{
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Zone triggered: %s | Type: %d | Audio: %s"),
        *Zone.ZoneLabel,
        (int32)Zone.ZoneType,
        *Zone.NarrativeAudioURL);

    // In a full implementation, this would:
    // 1. Load the audio URL into a USoundWave via HTTP streaming
    // 2. Play it via UAudioComponent attached to the player
    // 3. Blend adaptive music layer based on ZoneType
    // For now, we log and let Blueprint handle playback via the URL
}

void UAudio_SystemManager::UpdateDangerLevel(float NewDanger)
{
    DangerLevel = FMath::Clamp(NewDanger, 0.0f, 1.0f);
}

void UAudio_SystemManager::TriggerScreenShake(const FVector& SourceLocation, const FVector& PlayerLocation)
{
    float Dist = FVector::Dist(SourceLocation, PlayerLocation);
    if (Dist > TRexShakeConfig.TriggerRadius) return;

    // Intensity scales with proximity
    float NormalizedDist = 1.0f - (Dist / TRexShakeConfig.TriggerRadius);
    float FinalIntensity = TRexShakeConfig.ShakeIntensity * NormalizedDist;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Screen shake triggered — intensity: %.2f, dist: %.0f"),
        FinalIntensity, Dist);

    // Blueprint should call ClientPlayCameraShake on the PlayerController
    // with intensity = FinalIntensity and duration = TRexShakeConfig.ShakeDuration
}

void UAudio_SystemManager::TriggerDamageFlash()
{
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Damage flash triggered — red screen overlay cue sent."));
    // Blueprint handles the actual PostProcess material parameter change
    // (RedFlash scalar parameter on the player camera's post-process volume)
}

float UAudio_SystemManager::GetCurrentDangerLevel() const
{
    return DangerLevel;
}

bool UAudio_SystemManager::IsPlayerInDangerZone(const FVector& PlayerLocation) const
{
    for (const FAudio_ZoneConfig& Zone : AudioZones)
    {
        if (Zone.ZoneType == EAudio_ZoneType::Danger || Zone.ZoneType == EAudio_ZoneType::Combat)
        {
            if (FVector::Dist(PlayerLocation, Zone.Location) <= Zone.TriggerRadius)
            {
                return true;
            }
        }
    }
    return false;
}

void UAudio_SystemManager::SetNightMode(bool bIsNight)
{
    bNightMode = bIsNight;
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Night mode: %s — switching ambient soundscape layer."),
        bIsNight ? TEXT("ON") : TEXT("OFF"));
    // Night mode: suppress bird calls, amplify insect chorus, add distant predator rumbles
    // Day mode: full jungle ambience, distant herbivore calls, wind through canopy
}
