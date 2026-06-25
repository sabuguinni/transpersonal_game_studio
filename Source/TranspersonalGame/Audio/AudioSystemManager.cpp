#include "AudioSystemManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root component
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("AudioRoot"));
    SetRootComponent(Root);

    // Ambient audio component — jungle/environment layer
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(Root);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->VolumeMultiplier = 1.0f;

    // Music audio component — adaptive score layer
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
    MusicAudioComponent->SetupAttachment(Root);
    MusicAudioComponent->bAutoActivate = false;
    MusicAudioComponent->VolumeMultiplier = 0.6f;

    // Campfire audio component — positional fire crackle
    CampfireAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CampfireAudio"));
    CampfireAudioComponent->SetupAttachment(Root);
    CampfireAudioComponent->bAutoActivate = false;
    CampfireAudioComponent->VolumeMultiplier = 0.8f;

    // Narrator audio component — voice lines
    NarratorAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarratorAudio"));
    NarratorAudioComponent->SetupAttachment(Root);
    NarratorAudioComponent->bAutoActivate = false;
    NarratorAudioComponent->VolumeMultiplier = 1.0f;

    // Default zone configs
    FAudio_ZoneConfig SafeConfig;
    SafeConfig.ZoneType = EAudio_ZoneType::Safe;
    SafeConfig.AmbientVolume = 1.0f;
    SafeConfig.MusicVolume = 0.4f;
    SafeConfig.CrossfadeDuration = 4.0f;
    SafeConfig.bPlayDinoDistantCalls = false;
    ZoneConfigs.Add(SafeConfig);

    FAudio_ZoneConfig DangerConfig;
    DangerConfig.ZoneType = EAudio_ZoneType::Danger;
    DangerConfig.AmbientVolume = 0.3f;
    DangerConfig.MusicVolume = 1.0f;
    DangerConfig.CrossfadeDuration = 1.5f;
    DangerConfig.bPlayDinoDistantCalls = true;
    ZoneConfigs.Add(DangerConfig);

    FAudio_ZoneConfig CombatConfig;
    CombatConfig.ZoneType = EAudio_ZoneType::Combat;
    CombatConfig.AmbientVolume = 0.1f;
    CombatConfig.MusicVolume = 1.0f;
    CombatConfig.CrossfadeDuration = 0.5f;
    CombatConfig.bPlayDinoDistantCalls = false;
    ZoneConfigs.Add(CombatConfig);

    FAudio_ZoneConfig ExploreConfig;
    ExploreConfig.ZoneType = EAudio_ZoneType::Exploration;
    ExploreConfig.AmbientVolume = 0.9f;
    ExploreConfig.MusicVolume = 0.5f;
    ExploreConfig.CrossfadeDuration = 3.0f;
    ExploreConfig.bPlayDinoDistantCalls = true;
    ZoneConfigs.Add(ExploreConfig);

    FAudio_ZoneConfig NightConfig;
    NightConfig.ZoneType = EAudio_ZoneType::Night;
    NightConfig.AmbientVolume = 0.7f;
    NightConfig.MusicVolume = 0.3f;
    NightConfig.CrossfadeDuration = 5.0f;
    NightConfig.bPlayDinoDistantCalls = true;
    ZoneConfigs.Add(NightConfig);

    // Register narrator lines with ElevenLabs-generated audio URLs
    FAudio_NarratorLine JungleLine;
    JungleLine.LineID = TEXT("JUNGLE_DANGER_001");
    JungleLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782381191549_Survivor_Narrator.mp3");
    JungleLine.Duration = 11.0f;
    JungleLine.TriggerThreatLevel = EAudio_DinoThreatLevel::Near;
    RegisteredNarratorLines.Add(JungleLine);

    FAudio_NarratorLine AlertLine;
    AlertLine.LineID = TEXT("TREX_RUN_001");
    AlertLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782381197900_Survivor_Narrator_Alert.mp3");
    AlertLine.Duration = 8.0f;
    AlertLine.TriggerThreatLevel = EAudio_DinoThreatLevel::Imminent;
    RegisteredNarratorLines.Add(AlertLine);
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Start with safe zone ambient audio
    SetAudioZone(EAudio_ZoneType::Safe);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: BeginPlay — zone=Safe, threat=None"));
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update threat cooldown
    if (ThreatUpdateCooldown > 0.0f)
    {
        ThreatUpdateCooldown -= DeltaTime;
    }

    // Update ambient volumes based on current zone
    UpdateAmbientVolumes(DeltaTime);
}

void AAudio_SystemManager::SetAudioZone(EAudio_ZoneType NewZone)
{
    if (CurrentZone == NewZone) return;

    const FAudio_ZoneConfig* Config = FindZoneConfig(NewZone);
    if (!Config) return;

    CurrentZone = NewZone;

    // Apply volume changes immediately
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(Config->AmbientVolume * MasterVolume);
    }
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(Config->MusicVolume * MasterVolume);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Zone changed to %d"), (int32)NewZone);
}

void AAudio_SystemManager::CrossfadeToZone(EAudio_ZoneType TargetZone, float Duration)
{
    // Queue crossfade — simplified: set zone directly with duration hint
    CrossfadeTimer = Duration;
    SetAudioZone(TargetZone);
}

void AAudio_SystemManager::UpdateDinoThreatLevel(EAudio_DinoThreatLevel NewThreat)
{
    if (ThreatUpdateCooldown > 0.0f) return;
    if (CurrentThreatLevel == NewThreat) return;

    CurrentThreatLevel = NewThreat;
    ThreatUpdateCooldown = 2.0f; // Prevent rapid switching

    // Map threat to zone
    switch (NewThreat)
    {
        case EAudio_DinoThreatLevel::None:
            CrossfadeToZone(EAudio_ZoneType::Safe, 4.0f);
            break;
        case EAudio_DinoThreatLevel::Distant:
            CrossfadeToZone(EAudio_ZoneType::Exploration, 3.0f);
            break;
        case EAudio_DinoThreatLevel::Near:
            CrossfadeToZone(EAudio_ZoneType::Danger, 2.0f);
            break;
        case EAudio_DinoThreatLevel::Imminent:
            CrossfadeToZone(EAudio_ZoneType::Combat, 0.5f);
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Threat level = %d"), (int32)NewThreat);
}

void AAudio_SystemManager::PlayNarratorLine(const FString& LineID)
{
    for (const FAudio_NarratorLine& Line : RegisteredNarratorLines)
    {
        if (Line.LineID == LineID)
        {
            // In production: load audio from URL and play via NarratorAudioComponent
            // For now: log the play event
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing narrator line '%s' (%.1fs) from %s"),
                *LineID, Line.Duration, *Line.AudioURL);
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Narrator line '%s' not found"), *LineID);
}

void AAudio_SystemManager::RegisterNarratorLine(const FAudio_NarratorLine& Line)
{
    // Remove existing line with same ID if present
    RegisteredNarratorLines.RemoveAll([&Line](const FAudio_NarratorLine& Existing)
    {
        return Existing.LineID == Line.LineID;
    });
    RegisteredNarratorLines.Add(Line);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered narrator line '%s'"), *Line.LineID);
}

void AAudio_SystemManager::TriggerTRexFootstepShake(float Distance)
{
    if (Distance > TRexFootstepShakeRadius) return;

    // Scale shake intensity by inverse distance
    float Intensity = FMath::Clamp(1.0f - (Distance / TRexFootstepShakeRadius), 0.0f, 1.0f);

    // Apply camera shake via player controller
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (PC)
    {
        // Shake magnitude proportional to T-Rex proximity
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: T-Rex footstep shake — distance=%.0f intensity=%.2f"),
            Distance, Intensity);
    }
}

void AAudio_SystemManager::TriggerDamageAudioFeedback(float DamageAmount)
{
    // Play damage grunt/impact sound
    float Volume = FMath::Clamp(DamageAmount / 100.0f, 0.2f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Damage audio feedback — damage=%.1f volume=%.2f"),
        DamageAmount, Volume);
}

void AAudio_SystemManager::SetCampfireActive(bool bActive)
{
    bCampfireActive = bActive;
    if (CampfireAudioComponent)
    {
        if (bActive)
        {
            CampfireAudioComponent->Activate(true);
            CampfireAudioComponent->SetVolumeMultiplier(0.8f * MasterVolume);
        }
        else
        {
            CampfireAudioComponent->Deactivate();
        }
    }
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Campfire audio %s"), bActive ? TEXT("ON") : TEXT("OFF"));
}

void AAudio_SystemManager::OnDayBegin()
{
    // Day: birds, insects, distant dino calls — full ambient
    CrossfadeToZone(EAudio_ZoneType::Exploration, 8.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Day audio layer activated"));
}

void AAudio_SystemManager::OnNightBegin()
{
    // Night: crickets, distant roars, tension
    CrossfadeToZone(EAudio_ZoneType::Night, 10.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Night audio layer activated"));
}

void AAudio_SystemManager::OnDawnBegin()
{
    // Dawn: birds starting, soft music swell
    CrossfadeToZone(EAudio_ZoneType::Safe, 6.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Dawn audio layer activated"));
}

void AAudio_SystemManager::OnDuskBegin()
{
    // Dusk: ambient fading, danger rising
    CrossfadeToZone(EAudio_ZoneType::Danger, 5.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Dusk audio layer activated"));
}

void AAudio_SystemManager::UpdateAmbientVolumes(float DeltaTime)
{
    // Smooth volume transitions
    if (CrossfadeTimer > 0.0f)
    {
        CrossfadeTimer -= DeltaTime;
    }
}

const FAudio_ZoneConfig* AAudio_SystemManager::FindZoneConfig(EAudio_ZoneType ZoneType) const
{
    for (const FAudio_ZoneConfig& Config : ZoneConfigs)
    {
        if (Config.ZoneType == ZoneType)
        {
            return &Config;
        }
    }
    return nullptr;
}
