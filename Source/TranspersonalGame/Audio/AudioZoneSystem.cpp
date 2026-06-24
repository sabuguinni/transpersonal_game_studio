#include "AudioZoneSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// AAudio_NarrativeZone
// ============================================================

AAudio_NarrativeZone::AAudio_NarrativeZone()
{
    PrimaryActorTick.bCanEverTick = true;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
    AudioComponent->bIsUISound = false;

    ZoneConfig.TriggerRadius = 500.0f;
    ZoneConfig.FadeInDuration = 1.5f;
    ZoneConfig.FadeOutDuration = 2.0f;
    ZoneConfig.BaseVolume = 1.0f;
    ZoneConfig.bLooping = true;
    ZoneConfig.bScaleWithUrgency = false;
}

void AAudio_NarrativeZone::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = EAudio_ZoneState::Inactive;
    CurrentVolume = 0.0f;
}

void AAudio_NarrativeZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    bool bNowInRange = IsPlayerInRange();

    if (bNowInRange && !bPlayerInRange)
    {
        // Player entered range — start audio
        bPlayerInRange = true;
        if (AudioComponent && AudioComponent->Sound)
        {
            AudioComponent->SetVolumeMultiplier(0.0f);
            AudioComponent->Play();
            CurrentState = EAudio_ZoneState::Playing;
        }
        OnNarrativeHookTriggered(ZoneConfig.HookType);
    }
    else if (!bNowInRange && bPlayerInRange)
    {
        // Player left range — fade out
        bPlayerInRange = false;
        StopAudioZone();
    }

    // Fade in/out volume
    if (CurrentState == EAudio_ZoneState::Playing && AudioComponent)
    {
        float TargetVolume = ZoneConfig.BaseVolume;
        if (ZoneConfig.bScaleWithUrgency)
        {
            TargetVolume *= (0.5f + UrgencyScalar * 0.5f);
        }
        CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, 1.0f / FMath::Max(ZoneConfig.FadeInDuration, 0.01f));
        AudioComponent->SetVolumeMultiplier(CurrentVolume);
    }
    else if (CurrentState == EAudio_ZoneState::FadingOut && AudioComponent)
    {
        CurrentVolume = FMath::FInterpTo(CurrentVolume, 0.0f, DeltaTime, 1.0f / FMath::Max(ZoneConfig.FadeOutDuration, 0.01f));
        AudioComponent->SetVolumeMultiplier(CurrentVolume);
        if (CurrentVolume < 0.01f)
        {
            AudioComponent->Stop();
            CurrentState = EAudio_ZoneState::Completed;
        }
    }
}

void AAudio_NarrativeZone::SetUrgencyScalar(float NewUrgency)
{
    UrgencyScalar = FMath::Clamp(NewUrgency, 0.0f, 1.0f);
    OnUrgencyUpdated(UrgencyScalar);
    UpdateVolumeForUrgency();
}

void AAudio_NarrativeZone::TriggerAudioHook(EAudio_NarrativeHook Hook)
{
    ZoneConfig.HookType = Hook;
    if (AudioComponent && AudioComponent->Sound)
    {
        AudioComponent->SetVolumeMultiplier(0.0f);
        AudioComponent->Play();
        CurrentState = EAudio_ZoneState::Playing;
    }
    OnNarrativeHookTriggered(Hook);
}

void AAudio_NarrativeZone::StopAudioZone()
{
    CurrentState = EAudio_ZoneState::FadingOut;
}

bool AAudio_NarrativeZone::IsPlayerInRange() const
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player) return false;
    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Dist <= ZoneConfig.TriggerRadius;
}

void AAudio_NarrativeZone::UpdateVolumeForUrgency()
{
    if (!ZoneConfig.bScaleWithUrgency) return;
    if (!AudioComponent) return;
    float TargetVolume = ZoneConfig.BaseVolume * (0.5f + UrgencyScalar * 0.5f);
    AudioComponent->SetVolumeMultiplier(TargetVolume);
}

// ============================================================
// AAudio_TRexShakeSource
// ============================================================

AAudio_TRexShakeSource::AAudio_TRexShakeSource()
{
    PrimaryActorTick.bCanEverTick = true;

    RumbleAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RumbleAudioComponent"));
    RumbleAudioComponent->SetupAttachment(RootComponent);
    RumbleAudioComponent->bAutoActivate = false;

    ShakeConfig.TriggerRadius = 1200.0f;
    ShakeConfig.MaxShakeIntensity = 1.0f;
    ShakeConfig.ShakeFrequency = 2.5f;
    ShakeConfig.ShakeDuration = 0.3f;
}

void AAudio_TRexShakeSource::BeginPlay()
{
    Super::BeginPlay();
}

void AAudio_TRexShakeSource::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float Proximity = GetProximityToPlayer();
    if (Proximity <= 0.0f) return;

    // Scale rumble volume with proximity
    if (RumbleAudioComponent && RumbleAudioComponent->Sound)
    {
        if (!RumbleAudioComponent->IsPlaying())
        {
            RumbleAudioComponent->Play();
        }
        RumbleAudioComponent->SetVolumeMultiplier(Proximity);
    }

    // Trigger screen shake at intervals scaled by proximity
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DynamicCooldown = FMath::Lerp(0.5f, ShakeCooldown, Proximity);
    if (CurrentTime - LastShakeTime >= DynamicCooldown)
    {
        LastShakeTime = CurrentTime;
        OnTRexShakeTriggered(Proximity * ShakeConfig.MaxShakeIntensity);
    }
}

float AAudio_TRexShakeSource::GetProximityToPlayer() const
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player) return 0.0f;
    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    if (Dist >= ShakeConfig.TriggerRadius) return 0.0f;
    return 1.0f - (Dist / ShakeConfig.TriggerRadius);
}

// ============================================================
// AAudio_DayNightAmbience
// ============================================================

AAudio_DayNightAmbience::AAudio_DayNightAmbience()
{
    PrimaryActorTick.bCanEverTick = true;

    DayAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DayAudioComponent"));
    DayAudioComponent->SetupAttachment(RootComponent);
    DayAudioComponent->bAutoActivate = false;

    NightAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NightAudioComponent"));
    NightAudioComponent->SetupAttachment(RootComponent);
    NightAudioComponent->bAutoActivate = false;

    TimeOfDay = 0.5f;
    bIsCurrentlyDay = true;
}

void AAudio_DayNightAmbience::BeginPlay()
{
    Super::BeginPlay();
    BlendAmbienceVolumes();
}

void AAudio_DayNightAmbience::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    BlendAmbienceVolumes();
}

void AAudio_DayNightAmbience::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);

    // 0.25..0.75 = day, outside = night
    bool bNewIsDay = (TimeOfDay >= 0.25f && TimeOfDay <= 0.75f);
    if (bNewIsDay != bIsCurrentlyDay)
    {
        bIsCurrentlyDay = bNewIsDay;
        OnDayNightTransition(bIsCurrentlyDay);
    }
}

void AAudio_DayNightAmbience::BlendAmbienceVolumes()
{
    // Day volume: peaks at noon (0.5), zero at midnight (0.0/1.0)
    float DayBlend = 0.0f;
    if (TimeOfDay >= 0.25f && TimeOfDay <= 0.75f)
    {
        // Smooth bell curve peaking at 0.5
        float t = (TimeOfDay - 0.25f) / 0.5f; // 0..1
        DayBlend = FMath::Sin(t * PI);
    }
    float NightBlend = 1.0f - DayBlend;

    if (DayAudioComponent)
    {
        if (!DayAudioComponent->IsPlaying() && DayBlend > 0.01f && DayAudioComponent->Sound)
            DayAudioComponent->Play();
        DayAudioComponent->SetVolumeMultiplier(DayAmbienceVolume * DayBlend);
    }

    if (NightAudioComponent)
    {
        if (!NightAudioComponent->IsPlaying() && NightBlend > 0.01f && NightAudioComponent->Sound)
            NightAudioComponent->Play();
        NightAudioComponent->SetVolumeMultiplier(NightAmbienceVolume * NightBlend);
    }
}
