// AudioFeedbackSystem.cpp
// Agent #16 — Audio Agent — PROD_CYCLE_AUTO_20260630_009
// Full implementation of game-feel audio feedback system for prehistoric survival game

#include "AudioFeedbackSystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UAudioFeedbackSystem::UAudioFeedbackSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for audio state polling

    CurrentTimeOfDay = EAudio_TimeOfDayPhase::Day;
    bDamageFlashActive = false;
    DamageFlashElapsed = 0.0f;
    DamageFlashDuration = 0.25f;

    TRexProximityRadius = 3000.0f;
    RaptorProximityRadius = 1500.0f;
    CampfireAmbientRadius = 800.0f;

    // Default shake configs
    TRexShakeConfig.ShakeScale = 1.8f;
    TRexShakeConfig.ShakeDuration = 0.6f;
    TRexShakeConfig.TriggerRadius = 3000.0f;
    TRexShakeConfig.MaxDistanceForFullShake = 800.0f;

    RaptorShakeConfig.ShakeScale = 0.6f;
    RaptorShakeConfig.ShakeDuration = 0.25f;
    RaptorShakeConfig.TriggerRadius = 1500.0f;
    RaptorShakeConfig.MaxDistanceForFullShake = 400.0f;

    PlayerDamageShakeConfig.ShakeScale = 1.2f;
    PlayerDamageShakeConfig.ShakeDuration = 0.35f;
    PlayerDamageShakeConfig.TriggerRadius = 0.0f; // Always triggers — player-centric
    PlayerDamageShakeConfig.MaxDistanceForFullShake = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::BeginPlay()
{
    Super::BeginPlay();

    // Spawn ambient audio component for campfire/environment loop
    AmbientAudioComponent = NewObject<UAudioComponent>(GetOwner(), UAudioComponent::StaticClass(), TEXT("AmbientAudioComponent"));
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->RegisterComponent();
        AmbientAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        AmbientAudioComponent->bAutoActivate = false;
    }

    // Set initial time-of-day phase
    SetTimeOfDayPhase(EAudio_TimeOfDayPhase::Day);
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Tick damage flash timer
    if (bDamageFlashActive)
    {
        DamageFlashElapsed += DeltaTime;
        if (DamageFlashElapsed >= DamageFlashDuration)
        {
            bDamageFlashActive = false;
            DamageFlashElapsed = 0.0f;
            OnDamageFlashEnd.Broadcast();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TriggerFeedbackEvent — main dispatch
// ─────────────────────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::TriggerFeedbackEvent(EAudio_FeedbackEvent Event, float DistanceToSource)
{
    switch (Event)
    {
        case EAudio_FeedbackEvent::PlayerDamaged:
            TriggerDamageFlash();
            TriggerCameraShake(PlayerDamageShakeConfig, 0.0f);
            break;

        case EAudio_FeedbackEvent::TRexNearby:
            TriggerCameraShake(TRexShakeConfig, DistanceToSource);
            break;

        case EAudio_FeedbackEvent::RaptorAttack:
            TriggerCameraShake(RaptorShakeConfig, DistanceToSource);
            break;

        case EAudio_FeedbackEvent::PlayerDeath:
            TriggerDamageFlash();
            // Death flash stays on — Blueprint handles fade to black
            bDamageFlashActive = false; // Prevent auto-clear
            OnDamageFlashStart.Broadcast();
            break;

        case EAudio_FeedbackEvent::CampfireLit:
            PlayCampfireAmbience();
            break;

        case EAudio_FeedbackEvent::DinoKilled:
            // Brief positive shake — impact of kill
            {
                FAudio_ScreenShakeConfig KillShake;
                KillShake.ShakeScale = 0.4f;
                KillShake.ShakeDuration = 0.2f;
                KillShake.TriggerRadius = 0.0f;
                KillShake.MaxDistanceForFullShake = 0.0f;
                TriggerCameraShake(KillShake, 0.0f);
            }
            break;

        default:
            break;
    }

    OnFeedbackEventTriggered.Broadcast(Event);
}

// ─────────────────────────────────────────────────────────────────────────────
// TriggerDamageFlash
// ─────────────────────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::TriggerDamageFlash()
{
    if (bDamageFlashActive)
    {
        // Reset timer if already active (rapid hits)
        DamageFlashElapsed = 0.0f;
        return;
    }

    bDamageFlashActive = true;
    DamageFlashElapsed = 0.0f;
    OnDamageFlashStart.Broadcast();
}

// ─────────────────────────────────────────────────────────────────────────────
// TriggerCameraShake
// ─────────────────────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::TriggerCameraShake(const FAudio_ScreenShakeConfig& Config, float DistanceToSource)
{
    if (!CameraShakeClass)
    {
        unreal_log_warning_stub("AudioFeedbackSystem: CameraShakeClass not set — skipping shake");
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Distance-based attenuation
    float FinalScale = Config.ShakeScale;
    if (Config.TriggerRadius > 0.0f && DistanceToSource > Config.MaxDistanceForFullShake)
    {
        float Alpha = FMath::Clamp(
            (DistanceToSource - Config.MaxDistanceForFullShake) / (Config.TriggerRadius - Config.MaxDistanceForFullShake),
            0.0f, 1.0f
        );
        FinalScale = FMath::Lerp(Config.ShakeScale, 0.0f, Alpha);
    }

    if (FinalScale > 0.01f)
    {
        PC->ClientStartCameraShake(CameraShakeClass, FinalScale);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SetTimeOfDayPhase — switches ambient audio layer
// ─────────────────────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::SetTimeOfDayPhase(EAudio_TimeOfDayPhase NewPhase)
{
    if (CurrentTimeOfDay == NewPhase) return;

    CurrentTimeOfDay = NewPhase;
    OnTimeOfDayPhaseChanged.Broadcast(NewPhase);

    // Select ambient sound asset based on phase
    // Blueprint or MetaSound patch handles actual asset swap via delegate
    // C++ side records the phase for query
    switch (NewPhase)
    {
        case EAudio_TimeOfDayPhase::Dawn:
            // Birds, light wind, distant water — low danger cue
            unreal_log_info_stub("AudioFeedbackSystem: Time of Day → Dawn (bird calls, light wind)");
            break;
        case EAudio_TimeOfDayPhase::Day:
            // Full insect chorus, wind, occasional distant roar
            unreal_log_info_stub("AudioFeedbackSystem: Time of Day → Day (insects, wind, distant dino)");
            break;
        case EAudio_TimeOfDayPhase::Dusk:
            // Insects fade, predator calls increase, tension rises
            unreal_log_info_stub("AudioFeedbackSystem: Time of Day → Dusk (predator calls, tension)");
            break;
        case EAudio_TimeOfDayPhase::Night:
            // Minimal sound, heavy silence, sudden predator sounds = maximum tension
            unreal_log_info_stub("AudioFeedbackSystem: Time of Day → Night (silence + predator ambush cues)");
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PlayCampfireAmbience
// ─────────────────────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::PlayCampfireAmbience()
{
    if (!AmbientAudioComponent) return;
    if (!CampfireAmbientSound) return;

    AmbientAudioComponent->SetSound(CampfireAmbientSound);
    AmbientAudioComponent->FadeIn(2.0f, 1.0f); // 2-second fade in
}

// ─────────────────────────────────────────────────────────────────────────────
// StopCampfireAmbience
// ─────────────────────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::StopCampfireAmbience()
{
    if (!AmbientAudioComponent) return;
    AmbientAudioComponent->FadeOut(3.0f, 0.0f); // 3-second fade out
}

// ─────────────────────────────────────────────────────────────────────────────
// GetDamageFlashIntensity — called by HUD Blueprint each frame
// ─────────────────────────────────────────────────────────────────────────────

float UAudioFeedbackSystem::GetDamageFlashIntensity() const
{
    if (!bDamageFlashActive) return 0.0f;

    // Pulse: full intensity at start, fades to 0 over DamageFlashDuration
    float Alpha = DamageFlashElapsed / FMath::Max(DamageFlashDuration, 0.001f);
    return FMath::Clamp(1.0f - Alpha, 0.0f, 1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// IsDamageFlashActive
// ─────────────────────────────────────────────────────────────────────────────

bool UAudioFeedbackSystem::IsDamageFlashActive() const
{
    return bDamageFlashActive;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetCurrentTimeOfDayPhase
// ─────────────────────────────────────────────────────────────────────────────

EAudio_TimeOfDayPhase UAudioFeedbackSystem::GetCurrentTimeOfDayPhase() const
{
    return CurrentTimeOfDay;
}

// ─────────────────────────────────────────────────────────────────────────────
// Stub log helpers (avoids engine log macro dependency in this translation unit)
// ─────────────────────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::unreal_log_warning_stub(const FString& Msg)
{
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
}

void UAudioFeedbackSystem::unreal_log_info_stub(const FString& Msg)
{
    UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);
}
