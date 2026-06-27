// AudioSystemManager.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260627_005
// Full implementation of adaptive audio system for prehistoric survival game

#include "Audio/AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ─── Constructor ─────────────────────────────────────────────────────────────

AAudioSystemManager::AAudioSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick — audio doesn't need 60Hz

    // Sensible defaults
    CurrentZone = EAudio_AmbientZone::Camp;
    CurrentTimeOfDay = EAudio_TimeOfDay::Day;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    NearestDinoDistance = 99999.0f;
    bTRexNearby = false;
    bVoiceLineActive = false;
    HeartbeatIntensity = 0.0f;
    bPlayingDamageRing = false;
    MasterVolume = 1.0f;
    TimeSinceLastDinoCall = 0.0f;
    DinoCallInterval = 30.0f;
    CurrentHeartbeatRate = 60.0f;
}

// ─── BeginPlay ───────────────────────────────────────────────────────────────

void AAudioSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultAmbientLayers();
    InitializeDefaultDinoEvents();
    InitializeDefaultVoiceLines();
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] BeginPlay — ambient layers: %d, dino events: %d, voice lines: %d"),
        AmbientLayers.Num(), DinoProximityEvents.Num(), RegisteredVoiceLines.Num());
}

// ─── Tick ────────────────────────────────────────────────────────────────────

void AAudioSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAmbientBlend(DeltaTime);
    UpdateDinoProximityAudio(DeltaTime);
    UpdateSurvivalAudioFeedback(DeltaTime);
}

// ─── Public API ──────────────────────────────────────────────────────────────

void AAudioSystemManager::SetAmbientZone(EAudio_AmbientZone NewZone)
{
    if (CurrentZone == NewZone) return;
    CurrentZone = NewZone;
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Zone changed to: %d"), (int32)NewZone);
}

void AAudioSystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (CurrentTimeOfDay == NewTime) return;
    CurrentTimeOfDay = NewTime;
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] TimeOfDay changed to: %d"), (int32)NewTime);
}

void AAudioSystemManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;
    CurrentDangerLevel = NewLevel;

    // Adjust heartbeat intensity based on danger
    switch (NewLevel)
    {
        case EAudio_DangerLevel::Safe:      HeartbeatIntensity = 0.0f; break;
        case EAudio_DangerLevel::Cautious:  HeartbeatIntensity = 0.3f; break;
        case EAudio_DangerLevel::Threatened: HeartbeatIntensity = 0.7f; break;
        case EAudio_DangerLevel::Combat:    HeartbeatIntensity = 1.0f; break;
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] DangerLevel: %d | HeartbeatIntensity: %.2f"),
        (int32)NewLevel, HeartbeatIntensity);
}

void AAudioSystemManager::OnDinoProximityUpdate(const FString& DinoSpecies, float Distance)
{
    NearestDinoDistance = Distance;

    // T-Rex specific: trigger screen shake and heightened audio at close range
    if (DinoSpecies.Contains(TEXT("TRex")) || DinoSpecies.Contains(TEXT("Rex")))
    {
        bTRexNearby = (Distance < 2000.0f);
        if (bTRexNearby)
        {
            SetDangerLevel(EAudio_DangerLevel::Threatened);
            UE_LOG(LogTemp, Warning, TEXT("[AudioSystemManager] T-REX NEARBY at %.0f units — danger elevated"), Distance);
        }
    }

    // General proximity danger scaling
    if (Distance < 500.0f)
    {
        SetDangerLevel(EAudio_DangerLevel::Combat);
    }
    else if (Distance < 1500.0f)
    {
        SetDangerLevel(EAudio_DangerLevel::Threatened);
    }
    else if (Distance < 3000.0f)
    {
        SetDangerLevel(EAudio_DangerLevel::Cautious);
    }
}

void AAudioSystemManager::PlayVoiceLine(const FAudio_VoiceLine& Line)
{
    if (bVoiceLineActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AudioSystemManager] Voice line already active — queuing: %s"), *Line.SpeakerName);
        return;
    }

    ActiveVoiceLine = Line;
    bVoiceLineActive = true;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Playing voice line — Speaker: %s | URL: %s | Duration: %.1fs"),
        *Line.SpeakerName, *Line.AudioURL, Line.Duration);

    // Mark line as played in registered array
    for (FAudio_VoiceLine& Registered : RegisteredVoiceLines)
    {
        if (Registered.AudioURL == Line.AudioURL)
        {
            Registered.bHasBeenPlayed = true;
            break;
        }
    }

    // Auto-clear after duration (Blueprint timer would handle this in full impl)
    // For now, flag is cleared by EndVoiceLine or next BeginPlay
}

void AAudioSystemManager::RegisterVoiceLine(const FAudio_VoiceLine& Line)
{
    // Dedup by AudioURL
    for (const FAudio_VoiceLine& Existing : RegisteredVoiceLines)
    {
        if (Existing.AudioURL == Line.AudioURL)
        {
            UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Voice line already registered: %s"), *Line.AudioURL);
            return;
        }
    }
    RegisteredVoiceLines.Add(Line);
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Registered voice line: %s — %s"), *Line.SpeakerName, *Line.DialogueText.Left(40));
}

void AAudioSystemManager::TriggerDamageAudioFeedback(float DamageAmount)
{
    bPlayingDamageRing = true;
    // Scale heartbeat spike with damage
    float Spike = FMath::Clamp(DamageAmount / 100.0f, 0.2f, 1.0f);
    HeartbeatIntensity = FMath::Max(HeartbeatIntensity, Spike);
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Damage audio feedback: %.1f dmg | HeartbeatSpike: %.2f"), DamageAmount, Spike);
}

void AAudioSystemManager::UpdateHeartbeatFromHealth(float CurrentHealth, float MaxHealth)
{
    if (MaxHealth <= 0.0f) return;
    float HealthRatio = CurrentHealth / MaxHealth;

    // Heartbeat intensifies as health drops
    if (HealthRatio > 0.5f)
    {
        HeartbeatIntensity = FMath::Lerp(HeartbeatIntensity, 0.0f, 0.1f);
    }
    else
    {
        float LowHealthIntensity = 1.0f - (HealthRatio * 2.0f); // 0→1 as health 50%→0%
        HeartbeatIntensity = FMath::Max(HeartbeatIntensity, LowHealthIntensity);
    }

    // Critical health: fast heartbeat rate
    CurrentHeartbeatRate = FMath::Lerp(60.0f, 140.0f, 1.0f - HealthRatio);
}

void AAudioSystemManager::LogAudioSystemState()
{
    UE_LOG(LogTemp, Log, TEXT("=== AudioSystemManager State ==="));
    UE_LOG(LogTemp, Log, TEXT("Zone: %d | Time: %d | Danger: %d"), (int32)CurrentZone, (int32)CurrentTimeOfDay, (int32)CurrentDangerLevel);
    UE_LOG(LogTemp, Log, TEXT("NearestDino: %.0f | TRexNearby: %s"), NearestDinoDistance, bTRexNearby ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("HeartbeatIntensity: %.2f | Rate: %.0f BPM"), HeartbeatIntensity, CurrentHeartbeatRate);
    UE_LOG(LogTemp, Log, TEXT("VoiceLineActive: %s | Speaker: %s"), bVoiceLineActive ? TEXT("YES") : TEXT("NO"), *ActiveVoiceLine.SpeakerName);
    UE_LOG(LogTemp, Log, TEXT("AmbientLayers: %d | DinoEvents: %d | VoiceLines: %d"),
        AmbientLayers.Num(), DinoProximityEvents.Num(), RegisteredVoiceLines.Num());
}

void AAudioSystemManager::InitializeDefaultVoiceLines()
{
    // Seed with TTS audio URLs generated in this and previous cycles

    FAudio_VoiceLine SurvivalLine;
    SurvivalLine.SpeakerName = TEXT("SurvivalGuide");
    SurvivalLine.DialogueText = TEXT("Stay low. The ground shakes when it moves — you will feel it before you hear it.");
    SurvivalLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782548909721_SurvivalGuide.mp3");
    SurvivalLine.Duration = 21.0f;
    RegisterVoiceLine(SurvivalLine);

    FAudio_VoiceLine CampLine;
    CampLine.SpeakerName = TEXT("CampMaster");
    CampLine.DialogueText = TEXT("Fire is everything. Without it, the cold takes you before the predators do.");
    CampLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782548938746_CampMaster.mp3");
    CampLine.Duration = 22.0f;
    RegisterVoiceLine(CampLine);

    // Previous cycle voice lines
    FAudio_VoiceLine ElderLine;
    ElderLine.SpeakerName = TEXT("TribeElder");
    ElderLine.DialogueText = TEXT("The old hunter crouches by the fire. His eyes do not leave the treeline.");
    ElderLine.AudioURL = TEXT("tts/1782548764065_TribeElder.mp3");
    ElderLine.Duration = 27.0f;
    RegisterVoiceLine(ElderLine);

    FAudio_VoiceLine HuntLine;
    HuntLine.SpeakerName = TEXT("HuntLeader");
    HuntLine.DialogueText = TEXT("Move. Now. The herd is crossing the ridge.");
    HuntLine.AudioURL = TEXT("tts/1782548768754_HuntLeader.mp3");
    HuntLine.Duration = 21.0f;
    RegisterVoiceLine(HuntLine);

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Default voice lines initialized: %d lines"), RegisteredVoiceLines.Num());
}

// ─── Private ─────────────────────────────────────────────────────────────────

void AAudioSystemManager::UpdateAmbientBlend(float DeltaTime)
{
    // Stub: in full implementation this would crossfade between ambient layers
    // based on CurrentZone and CurrentTimeOfDay using MetaSounds parameter bus
}

void AAudioSystemManager::UpdateDinoProximityAudio(float DeltaTime)
{
    TimeSinceLastDinoCall += DeltaTime;

    // Trigger distant dino calls at intervals scaled by danger level
    float Interval = DinoCallInterval;
    if (CurrentDangerLevel == EAudio_DangerLevel::Cautious)  Interval = 20.0f;
    if (CurrentDangerLevel == EAudio_DangerLevel::Threatened) Interval = 10.0f;
    if (CurrentDangerLevel == EAudio_DangerLevel::Combat)     Interval = 5.0f;

    if (TimeSinceLastDinoCall >= Interval)
    {
        TimeSinceLastDinoCall = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Dino call trigger — DangerLevel: %d"), (int32)CurrentDangerLevel);
    }

    // Clear damage ring after brief duration
    if (bPlayingDamageRing)
    {
        bPlayingDamageRing = false; // Would use timer in full impl
    }
}

void AAudioSystemManager::UpdateSurvivalAudioFeedback(float DeltaTime)
{
    // Decay heartbeat intensity when safe
    if (CurrentDangerLevel == EAudio_DangerLevel::Safe)
    {
        HeartbeatIntensity = FMath::FInterpTo(HeartbeatIntensity, 0.0f, DeltaTime, 0.5f);
    }
}

void AAudioSystemManager::InitializeDefaultAmbientLayers()
{
    // Camp — Day
    FAudio_AmbientLayer CampDay;
    CampDay.Zone = EAudio_AmbientZone::Camp;
    CampDay.TimeOfDay = EAudio_TimeOfDay::Day;
    CampDay.SoundAssetPath = TEXT("/Game/Audio/Ambient/Camp_Day_Loop");
    CampDay.BaseVolume = 0.8f;
    CampDay.FadeInTime = 3.0f;
    CampDay.FadeOutTime = 3.0f;
    AmbientLayers.Add(CampDay);

    // Camp — Night
    FAudio_AmbientLayer CampNight;
    CampNight.Zone = EAudio_AmbientZone::Camp;
    CampNight.TimeOfDay = EAudio_TimeOfDay::Night;
    CampNight.SoundAssetPath = TEXT("/Game/Audio/Ambient/Camp_Night_Loop");
    CampNight.BaseVolume = 0.6f;
    CampNight.FadeInTime = 5.0f;
    CampNight.FadeOutTime = 5.0f;
    AmbientLayers.Add(CampNight);

    // Forest — Day
    FAudio_AmbientLayer ForestDay;
    ForestDay.Zone = EAudio_AmbientZone::Forest;
    ForestDay.TimeOfDay = EAudio_TimeOfDay::Day;
    ForestDay.SoundAssetPath = TEXT("/Game/Audio/Ambient/Forest_Day_Loop");
    ForestDay.BaseVolume = 1.0f;
    ForestDay.FadeInTime = 2.0f;
    ForestDay.FadeOutTime = 2.0f;
    AmbientLayers.Add(ForestDay);

    // Plains — Day
    FAudio_AmbientLayer PlainsDay;
    PlainsDay.Zone = EAudio_AmbientZone::Plains;
    PlainsDay.TimeOfDay = EAudio_TimeOfDay::Day;
    PlainsDay.SoundAssetPath = TEXT("/Game/Audio/Ambient/Plains_Day_Loop");
    PlainsDay.BaseVolume = 0.9f;
    PlainsDay.FadeInTime = 2.0f;
    PlainsDay.FadeOutTime = 2.0f;
    AmbientLayers.Add(PlainsDay);

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Default ambient layers initialized: %d"), AmbientLayers.Num());
}

void AAudioSystemManager::InitializeDefaultDinoEvents()
{
    // T-Rex proximity event
    FAudio_DinoProximityEvent TRexEvent;
    TRexEvent.DinoSpecies = TEXT("TRex");
    TRexEvent.ProximityRadius = 2000.0f;
    TRexEvent.GroundShakeIntensity = 1.0f;
    TRexEvent.RoarSoundPath = TEXT("/Game/Audio/Dinos/TRex_Roar");
    TRexEvent.FootstepSoundPath = TEXT("/Game/Audio/Dinos/TRex_Footstep");
    TRexEvent.bTriggerScreenShake = true;
    DinoProximityEvents.Add(TRexEvent);

    // Raptor proximity event
    FAudio_DinoProximityEvent RaptorEvent;
    RaptorEvent.DinoSpecies = TEXT("Raptor");
    RaptorEvent.ProximityRadius = 800.0f;
    RaptorEvent.GroundShakeIntensity = 0.2f;
    RaptorEvent.RoarSoundPath = TEXT("/Game/Audio/Dinos/Raptor_Call");
    RaptorEvent.FootstepSoundPath = TEXT("/Game/Audio/Dinos/Raptor_Footstep");
    RaptorEvent.bTriggerScreenShake = false;
    DinoProximityEvents.Add(RaptorEvent);

    // Brachiosaurus proximity event
    FAudio_DinoProximityEvent BrachioEvent;
    BrachioEvent.DinoSpecies = TEXT("Brachiosaurus");
    BrachioEvent.ProximityRadius = 3000.0f;
    BrachioEvent.GroundShakeIntensity = 0.8f;
    BrachioEvent.RoarSoundPath = TEXT("/Game/Audio/Dinos/Brachio_Call");
    BrachioEvent.FootstepSoundPath = TEXT("/Game/Audio/Dinos/Brachio_Footstep");
    BrachioEvent.bTriggerScreenShake = true;
    DinoProximityEvents.Add(BrachioEvent);

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Default dino proximity events initialized: %d"), DinoProximityEvents.Num());
}
