
#include "AudioFeedbackSystem.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UAudioFeedbackSystem::UAudioFeedbackSystem()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Default shake config for T-Rex
    TRexShakeConfig.ShakeScale = 2.5f;
    TRexShakeConfig.ShakeDuration = 0.6f;
    TRexShakeConfig.ShakeRadius = 2000.0f;
    TRexShakeConfig.bFalloffByDistance = true;

    // Default damage flash
    DamageFlashConfig.FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.6f);
    DamageFlashConfig.FlashDuration = 0.25f;
    DamageFlashConfig.FadeOutTime = 0.5f;

    // Default footstep config
    FootstepConfig.DustParticleScale = 1.0f;
    FootstepConfig.FootstepVolumeMultiplier = 1.0f;
    FootstepConfig.bSpawnDustOnSoftGround = true;
    FootstepConfig.bSpawnDustOnHardGround = false;

    // State
    CurrentTimePhase = EAudio_TimeOfDayPhase::Day;
    bIsInDanger = false;
    CurrentDangerIntensity = 0.0f;
    DamageFlashAlpha = 0.0f;
    DamageFlashTimer = 0.0f;
    bFlashActive = false;

    TRexShakeTriggerDistance = 1500.0f;
    NightAmbientVolumeMultiplier = 1.4f;
    DayAmbientVolumeMultiplier = 0.8f;
}

void UAudioFeedbackSystem::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: Initialized on %s"), *GetOwner()->GetName());
}

void UAudioFeedbackSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateDamageFlash(DeltaTime);
}

// ─────────────────────────────────────────────────────────────
//  Screen Shake
// ─────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::TriggerTRexShake(float DistanceToTRex)
{
    if (!GetWorld()) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Scale shake by distance — closer = stronger
    float DistanceFactor = FMath::Clamp(1.0f - (DistanceToTRex / TRexShakeTriggerDistance), 0.0f, 1.0f);
    float FinalScale = TRexShakeConfig.ShakeScale * DistanceFactor;

    if (FinalScale > 0.05f)
    {
        // Use legacy camera shake via console command as fallback
        FString ShakeCmd = FString::Printf(TEXT("ce TRexShake %.2f"), FinalScale);
        UGameplayStatics::PlayWorldCameraShake(
            GetWorld(),
            nullptr, // ShakeClass — set in Blueprint
            GetOwner()->GetActorLocation(),
            0.0f,
            TRexShakeConfig.ShakeRadius,
            FinalScale,
            false
        );
        UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: TRex shake triggered — dist=%.0f scale=%.2f"), DistanceToTRex, FinalScale);
    }
}

void UAudioFeedbackSystem::TriggerDamageShake(float DamageAmount)
{
    if (!GetWorld()) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    float ShakeScale = FMath::Clamp(DamageAmount / 100.0f, 0.2f, 1.5f);

    UGameplayStatics::PlayWorldCameraShake(
        GetWorld(),
        nullptr,
        GetOwner()->GetActorLocation(),
        0.0f,
        500.0f,
        ShakeScale,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: Damage shake — damage=%.1f scale=%.2f"), DamageAmount, ShakeScale);
}

void UAudioFeedbackSystem::TriggerCustomShake(FAudio_ScreenShakeConfig Config)
{
    if (!GetWorld()) return;

    UGameplayStatics::PlayWorldCameraShake(
        GetWorld(),
        nullptr,
        GetOwner()->GetActorLocation(),
        0.0f,
        Config.ShakeRadius,
        Config.ShakeScale,
        Config.bFalloffByDistance
    );
}

// ─────────────────────────────────────────────────────────────
//  Damage Flash
// ─────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::TriggerDamageFlash(float DamageNormalized)
{
    DamageFlashAlpha = FMath::Clamp(DamageNormalized, 0.1f, 1.0f);
    DamageFlashTimer = DamageFlashConfig.FlashDuration + DamageFlashConfig.FadeOutTime;
    bFlashActive = true;
    UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: Damage flash triggered — alpha=%.2f"), DamageFlashAlpha);
}

void UAudioFeedbackSystem::TriggerDeathFlash()
{
    DamageFlashAlpha = 1.0f;
    DamageFlashTimer = 2.0f; // Longer death flash
    bFlashActive = true;
    UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: Death flash triggered"));
}

void UAudioFeedbackSystem::UpdateDamageFlash(float DeltaTime)
{
    if (!bFlashActive) return;

    DamageFlashTimer -= DeltaTime;

    if (DamageFlashTimer <= DamageFlashConfig.FadeOutTime)
    {
        // Fade out phase
        float FadeAlpha = DamageFlashTimer / FMath::Max(DamageFlashConfig.FadeOutTime, 0.01f);
        DamageFlashAlpha = FMath::Lerp(0.0f, DamageFlashAlpha, FadeAlpha);
    }

    if (DamageFlashTimer <= 0.0f)
    {
        DamageFlashAlpha = 0.0f;
        bFlashActive = false;
    }
}

// ─────────────────────────────────────────────────────────────
//  Footstep Dust
// ─────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::SpawnFootstepDust(FVector FootLocation, bool bIsHeavyCreature)
{
    if (!GetWorld()) return;

    // Log footstep event — actual Niagara spawn handled by VFX Agent (#17)
    float Scale = bIsHeavyCreature ? FootstepConfig.DustParticleScale * 2.0f : FootstepConfig.DustParticleScale;
    UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: Footstep dust at (%.0f, %.0f, %.0f) scale=%.2f heavy=%d"),
        FootLocation.X, FootLocation.Y, FootLocation.Z, Scale, bIsHeavyCreature ? 1 : 0);
}

void UAudioFeedbackSystem::SpawnTRexFootstepDust(FVector FootLocation, float TRexMassKg)
{
    if (!GetWorld()) return;

    // T-Rex footstep — massive dust cloud + ground crack
    float MassFactor = FMath::Clamp(TRexMassKg / 8000.0f, 1.0f, 4.0f);
    float DustScale = FootstepConfig.DustParticleScale * MassFactor;

    UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: T-Rex footstep dust at (%.0f, %.0f, %.0f) mass=%.0fkg scale=%.2f"),
        FootLocation.X, FootLocation.Y, FootLocation.Z, TRexMassKg, DustScale);

    // Trigger screen shake based on distance to player
    AActor* Owner = GetOwner();
    if (Owner)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            float Dist = FVector::Dist(FootLocation, PC->GetPawn()->GetActorLocation());
            TriggerTRexShake(Dist);
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  Day/Night Audio Transitions
// ─────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::OnTimeOfDayChanged(EAudio_TimeOfDayPhase NewPhase)
{
    if (CurrentTimePhase == NewPhase) return;

    EAudio_TimeOfDayPhase OldPhase = CurrentTimePhase;
    CurrentTimePhase = NewPhase;

    const TArray<FString> PhaseNames = { TEXT("Dawn"), TEXT("Day"), TEXT("Dusk"), TEXT("Night") };
    UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: Time phase changed %s → %s"),
        *PhaseNames[(int32)OldPhase],
        *PhaseNames[(int32)NewPhase]);

    // Dispatch phase-specific audio events
    switch (NewPhase)
    {
    case EAudio_TimeOfDayPhase::Dawn:
        // Birds start, insects fade, light wind
        UpdateAmbientMix(0.1f);
        break;
    case EAudio_TimeOfDayPhase::Day:
        // Full bird chorus, insects, wind
        UpdateAmbientMix(0.0f);
        break;
    case EAudio_TimeOfDayPhase::Dusk:
        // Birds fade, crickets start, tension rises
        UpdateAmbientMix(0.6f);
        break;
    case EAudio_TimeOfDayPhase::Night:
        // Full night ambience — crickets, distant roars, danger
        UpdateAmbientMix(1.0f);
        break;
    }
}

void UAudioFeedbackSystem::UpdateAmbientMix(float DayNightAlpha)
{
    // DayNightAlpha: 0.0 = full day, 1.0 = full night
    float CurrentVolume = FMath::Lerp(DayAmbientVolumeMultiplier, NightAmbientVolumeMultiplier, DayNightAlpha);
    UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: Ambient mix updated — DayNightAlpha=%.2f volume=%.2f"),
        DayNightAlpha, CurrentVolume);
}

// ─────────────────────────────────────────────────────────────
//  Feedback Event Dispatcher
// ─────────────────────────────────────────────────────────────

void UAudioFeedbackSystem::DispatchFeedbackEvent(EAudio_FeedbackEvent Event, float Intensity)
{
    switch (Event)
    {
    case EAudio_FeedbackEvent::PlayerDamaged:
        TriggerDamageFlash(Intensity);
        TriggerDamageShake(Intensity * 50.0f);
        bIsInDanger = true;
        CurrentDangerIntensity = Intensity;
        break;

    case EAudio_FeedbackEvent::TRexNearby:
        bIsInDanger = true;
        CurrentDangerIntensity = FMath::Max(CurrentDangerIntensity, Intensity);
        break;

    case EAudio_FeedbackEvent::RaptorAttack:
        TriggerDamageShake(Intensity * 30.0f);
        bIsInDanger = true;
        break;

    case EAudio_FeedbackEvent::PlayerDeath:
        TriggerDeathFlash();
        bIsInDanger = false;
        CurrentDangerIntensity = 0.0f;
        break;

    case EAudio_FeedbackEvent::CampfireLit:
        // Reduce danger intensity — fire provides safety
        CurrentDangerIntensity = FMath::Max(0.0f, CurrentDangerIntensity - 0.3f);
        if (CurrentDangerIntensity <= 0.0f) bIsInDanger = false;
        break;

    case EAudio_FeedbackEvent::DinoKilled:
        CurrentDangerIntensity = FMath::Max(0.0f, CurrentDangerIntensity - 0.5f);
        if (CurrentDangerIntensity <= 0.0f) bIsInDanger = false;
        break;

    default:
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioFeedbackSystem: Event dispatched — event=%d intensity=%.2f danger=%d"),
        (int32)Event, Intensity, bIsInDanger ? 1 : 0);
}
