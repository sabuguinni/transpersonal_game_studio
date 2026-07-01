#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAudio_SystemManager::UAudio_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // update every 0.5s — not every frame
}

void UAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultNarratorLines();
    RecalculateMusicIntensity();
    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Initialized — Zone: %d, Danger: %d"),
        (int32)CurrentEnvironmentState.CurrentZone,
        (int32)CurrentEnvironmentState.DangerLevel);
}

void UAudio_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Tick narrator cooldown
    if (NarratorCooldownTimer > 0.0f)
    {
        NarratorCooldownTimer -= DeltaTime;
    }

    // Auto-detect night from time of day
    const float Hour = CurrentEnvironmentState.TimeOfDay;
    const bool bShouldBeNight = (Hour >= NIGHT_START_HOUR || Hour < NIGHT_END_HOUR);
    if (bShouldBeNight != CurrentEnvironmentState.bIsNight)
    {
        CurrentEnvironmentState.bIsNight = bShouldBeNight;
        if (bShouldBeNight)
        {
            // Night falling — trigger narrator if not on cooldown
            TriggerNarratorLine(EAudio_NarratorTrigger::NightFalling);
            // Escalate danger at night
            if (CurrentEnvironmentState.DangerLevel == EAudio_DangerLevel::Safe)
            {
                UpdateDangerLevel(EAudio_DangerLevel::Cautious);
            }
        }
    }

    // Auto-escalate danger based on dinosaur proximity
    if (CurrentEnvironmentState.NearestDinosaurDistance < TREX_DANGER_RADIUS)
    {
        const float Proximity = 1.0f - (CurrentEnvironmentState.NearestDinosaurDistance / TREX_DANGER_RADIUS);
        if (Proximity > 0.8f && CurrentEnvironmentState.DangerLevel < EAudio_DangerLevel::Critical)
        {
            UpdateDangerLevel(EAudio_DangerLevel::Critical);
            TriggerNarratorLine(EAudio_NarratorTrigger::DinosaurNearby);
        }
        else if (Proximity > 0.5f && CurrentEnvironmentState.DangerLevel < EAudio_DangerLevel::Tense)
        {
            UpdateDangerLevel(EAudio_DangerLevel::Tense);
        }
        else if (Proximity > 0.2f && CurrentEnvironmentState.DangerLevel < EAudio_DangerLevel::Cautious)
        {
            UpdateDangerLevel(EAudio_DangerLevel::Cautious);
        }
    }
}

void UAudio_SystemManager::UpdateEnvironmentZone(EAudio_EnvironmentZone NewZone)
{
    if (NewZone == CurrentEnvironmentState.CurrentZone) return;

    CurrentEnvironmentState.CurrentZone = NewZone;
    RecalculateMusicIntensity();
    OnZoneChanged.Broadcast(NewZone);

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Zone changed to: %d"), (int32)NewZone);
}

void UAudio_SystemManager::UpdateDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (NewLevel == CurrentEnvironmentState.DangerLevel) return;

    CurrentEnvironmentState.DangerLevel = NewLevel;
    RecalculateMusicIntensity();
    OnDangerLevelChanged.Broadcast(NewLevel);

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Danger level changed to: %d"), (int32)NewLevel);
}

void UAudio_SystemManager::UpdateNearestDinosaurDistance(float Distance)
{
    CurrentEnvironmentState.NearestDinosaurDistance = FMath::Max(0.0f, Distance);
    // Danger recalculation happens in Tick
}

void UAudio_SystemManager::TriggerNarratorLine(EAudio_NarratorTrigger Trigger)
{
    // Cooldown guard — narrator lines don't stack
    if (NarratorCooldownTimer > 0.0f) return;

    for (FAudio_NarratorLine& Line : NarratorLines)
    {
        if (Line.TriggerType == Trigger)
        {
            // One-shot lines check
            if (Line.bHasPlayed && Line.CooldownSeconds <= 0.0f) return;

            Line.bHasPlayed = true;
            NarratorCooldownTimer = Line.CooldownSeconds;

            OnNarratorTriggered.Broadcast(Trigger);

            UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Narrator triggered: %d — %s"),
                (int32)Trigger, *Line.TranscriptText);
            return;
        }
    }
}

void UAudio_SystemManager::SetTimeOfDay(float TimeHours)
{
    CurrentEnvironmentState.TimeOfDay = FMath::Clamp(TimeHours, 0.0f, 24.0f);
}

float UAudio_SystemManager::GetMusicDangerIntensity() const
{
    return MusicDangerIntensity;
}

bool UAudio_SystemManager::IsNight() const
{
    return CurrentEnvironmentState.bIsNight;
}

void UAudio_SystemManager::RegisterNarratorLine(EAudio_NarratorTrigger Trigger, const FString& AudioURL, const FString& Transcript, float Cooldown)
{
    // Replace existing entry for same trigger
    for (FAudio_NarratorLine& Line : NarratorLines)
    {
        if (Line.TriggerType == Trigger)
        {
            Line.AudioURL = AudioURL;
            Line.TranscriptText = Transcript;
            Line.CooldownSeconds = Cooldown;
            Line.bHasPlayed = false;
            return;
        }
    }

    FAudio_NarratorLine NewLine;
    NewLine.TriggerType = Trigger;
    NewLine.AudioURL = AudioURL;
    NewLine.TranscriptText = Transcript;
    NewLine.CooldownSeconds = Cooldown;
    NarratorLines.Add(NewLine);
}

void UAudio_SystemManager::RegisterSound(FName SoundID, float Volume, float MaxDistance, bool bLoop)
{
    for (FAudio_SoundEntry& Entry : SoundLibrary)
    {
        if (Entry.SoundID == SoundID)
        {
            Entry.Volume = Volume;
            Entry.MaxAudibleDistance = MaxDistance;
            Entry.bLooping = bLoop;
            return;
        }
    }

    FAudio_SoundEntry NewEntry;
    NewEntry.SoundID = SoundID;
    NewEntry.Volume = Volume;
    NewEntry.MaxAudibleDistance = MaxDistance;
    NewEntry.bLooping = bLoop;
    SoundLibrary.Add(NewEntry);
}

void UAudio_SystemManager::RecalculateMusicIntensity()
{
    float ZoneModifier = 0.0f;
    switch (CurrentEnvironmentState.CurrentZone)
    {
        case EAudio_EnvironmentZone::CampFireSafe:       ZoneModifier = 0.0f; break;
        case EAudio_EnvironmentZone::OpenPlains:         ZoneModifier = 0.1f; break;
        case EAudio_EnvironmentZone::RiverBank:          ZoneModifier = 0.15f; break;
        case EAudio_EnvironmentZone::DenseForest:        ZoneModifier = 0.3f; break;
        case EAudio_EnvironmentZone::Cave:               ZoneModifier = 0.4f; break;
        case EAudio_EnvironmentZone::DinosaurTerritory:  ZoneModifier = 0.6f; break;
        case EAudio_EnvironmentZone::NightDanger:        ZoneModifier = 0.7f; break;
        default:                                          ZoneModifier = 0.1f; break;
    }

    float DangerModifier = 0.0f;
    switch (CurrentEnvironmentState.DangerLevel)
    {
        case EAudio_DangerLevel::Safe:     DangerModifier = 0.0f; break;
        case EAudio_DangerLevel::Cautious: DangerModifier = 0.25f; break;
        case EAudio_DangerLevel::Tense:    DangerModifier = 0.55f; break;
        case EAudio_DangerLevel::Critical: DangerModifier = 1.0f; break;
        default:                            DangerModifier = 0.0f; break;
    }

    float NightModifier = CurrentEnvironmentState.bIsNight ? 0.15f : 0.0f;

    MusicDangerIntensity = FMath::Clamp(ZoneModifier + DangerModifier + NightModifier, 0.0f, 1.0f);
}

void UAudio_SystemManager::InitializeDefaultNarratorLines()
{
    // Danger narrator line — URL from TTS generation this cycle
    RegisterNarratorLine(
        EAudio_NarratorTrigger::DinosaurNearby,
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782907215913_Narrator_Danger.mp3"),
        TEXT("Danger. Something large is moving through the trees. Stay low. Do not run."),
        90.0f
    );

    // Night falling narrator line — URL from TTS generation this cycle
    RegisterNarratorLine(
        EAudio_NarratorTrigger::NightFalling,
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782907236487_Narrator_NightFalling.mp3"),
        TEXT("Night is falling. The predators wake when the sun dies. Find shelter before the darkness takes the sky."),
        300.0f
    );

    // Campfire sound registration
    RegisterSound(FName("Campfire_Loop"), 0.8f, 800.0f, true);
    RegisterSound(FName("Wind_Plains"), 0.4f, 9999.0f, true);
    RegisterSound(FName("TRex_Footstep"), 1.0f, 1500.0f, false);
    RegisterSound(FName("Insects_Night"), 0.5f, 9999.0f, true);
    RegisterSound(FName("River_Ambient"), 0.6f, 600.0f, true);

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Default narrator lines and sound library initialized"));
}
