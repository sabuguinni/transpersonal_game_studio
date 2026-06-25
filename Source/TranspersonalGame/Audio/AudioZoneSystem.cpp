#include "AudioZoneSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

// ============================================================
// AAudio_ZoneActor
// ============================================================

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(1500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    PlayerDistance = 99999.0f;
    CurrentVolume = 0.0f;
    bPlayerInZone = false;
    CurrentTimeOfDay = EAudio_TimeOfDay::Day;
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    if (ZoneConfig.bActive && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
        AmbientAudioComponent->Play();
    }
}

void AAudio_ZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update player distance
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerChar)
    {
        PlayerDistance = FVector::Dist(GetActorLocation(), PlayerChar->GetActorLocation());
    }

    UpdateAudioBlend(DeltaTime);
}

void AAudio_ZoneActor::UpdateAudioBlend(float DeltaTime)
{
    if (!AmbientAudioComponent || !ZoneConfig.bActive)
    {
        return;
    }

    float TargetVolume = 0.0f;
    float BlendRadius = ZoneConfig.BlendRadius;

    if (PlayerDistance < BlendRadius)
    {
        float Alpha = 1.0f - (PlayerDistance / BlendRadius);
        Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

        float TimeVolume = (CurrentTimeOfDay == EAudio_TimeOfDay::Night || CurrentTimeOfDay == EAudio_TimeOfDay::Dawn)
            ? ZoneConfig.NightVolume
            : ZoneConfig.DayVolume;

        TargetVolume = Alpha * TimeVolume * ZoneConfig.MaxVolume;
        bPlayerInZone = true;
    }
    else
    {
        bPlayerInZone = false;
    }

    // Smooth blend
    CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, 2.0f);
    AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);
}

void AAudio_ZoneActor::SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
}

void AAudio_ZoneActor::SetPlayerDistance(float Distance)
{
    PlayerDistance = Distance;
}

float AAudio_ZoneActor::GetBlendedVolume() const
{
    return CurrentVolume;
}

void AAudio_ZoneActor::ActivateZone()
{
    ZoneConfig.bActive = true;
    if (AmbientAudioComponent && !AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }
}

void AAudio_ZoneActor::DeactivateZone()
{
    ZoneConfig.bActive = false;
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->FadeOut(2.0f, 0.0f);
    }
}

// ============================================================
// AAudio_ScreenShakeTrigger
// ============================================================

AAudio_ScreenShakeTrigger::AAudio_ScreenShakeTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetSphereRadius(800.0f);
    DetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = DetectionSphere;

    bPlayerCurrentlyInRange = false;
    LastShakeTime = 0.0f;
    ShakeCooldown = 1.5f;
}

void AAudio_ScreenShakeTrigger::BeginPlay()
{
    Super::BeginPlay();
    DetectionSphere->SetSphereRadius(ShakeConfig.TriggerRadius);
}

void AAudio_ScreenShakeTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckPlayerProximity();
}

void AAudio_ScreenShakeTrigger::CheckPlayerProximity()
{
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerChar)
    {
        return;
    }

    float Dist = FVector::Dist(GetActorLocation(), PlayerChar->GetActorLocation());
    bool bInRange = Dist < ShakeConfig.TriggerRadius;

    if (bInRange && !bPlayerCurrentlyInRange)
    {
        // Player just entered range — trigger shake
        float Now = GetWorld()->GetTimeSeconds();
        if (Now - LastShakeTime > ShakeCooldown)
        {
            TriggerShake(ShakeConfig.ShakeIntensity);
            LastShakeTime = Now;
        }
    }

    bPlayerCurrentlyInRange = bInRange;
}

void AAudio_ScreenShakeTrigger::TriggerShake(float Intensity)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        // Screen shake via client camera shake
        // Parameters: scale, play space
        PC->ClientStartCameraShake(
            nullptr, // Camera shake class — set in Blueprint
            Intensity
        );
    }
}

bool AAudio_ScreenShakeTrigger::IsPlayerInRange() const
{
    return bPlayerCurrentlyInRange;
}

// ============================================================
// UAudio_DayNightManager
// ============================================================

UAudio_DayNightManager::UAudio_DayNightManager()
{
    DayDurationSeconds = 600.0f;
    NightDurationSeconds = 300.0f;
    CurrentTimeNormalized = 0.5f;
    CurrentPhase = EAudio_TimeOfDay::Day;
    ElapsedSeconds = 0.0f;
    TotalCycleDuration = DayDurationSeconds + NightDurationSeconds;
}

void UAudio_DayNightManager::AdvanceTime(float DeltaSeconds)
{
    ElapsedSeconds += DeltaSeconds;
    TotalCycleDuration = DayDurationSeconds + NightDurationSeconds;

    if (ElapsedSeconds >= TotalCycleDuration)
    {
        ElapsedSeconds -= TotalCycleDuration;
    }

    CurrentTimeNormalized = ElapsedSeconds / TotalCycleDuration;

    // Determine phase
    float DayFraction = DayDurationSeconds / TotalCycleDuration;
    float DawnFraction = 0.05f;
    float DuskFraction = DayFraction - 0.05f;

    if (CurrentTimeNormalized < DawnFraction)
    {
        CurrentPhase = EAudio_TimeOfDay::Dawn;
    }
    else if (CurrentTimeNormalized < DuskFraction)
    {
        CurrentPhase = EAudio_TimeOfDay::Day;
    }
    else if (CurrentTimeNormalized < DayFraction)
    {
        CurrentPhase = EAudio_TimeOfDay::Dusk;
    }
    else
    {
        CurrentPhase = EAudio_TimeOfDay::Night;
    }
}

EAudio_TimeOfDay UAudio_DayNightManager::GetCurrentPhase() const
{
    return CurrentPhase;
}

float UAudio_DayNightManager::GetSunPitch() const
{
    // Map normalized time to sun pitch: -90 (midnight) to +90 (noon) to -90 (midnight)
    float Angle = CurrentTimeNormalized * 360.0f - 90.0f;
    return FMath::Sin(FMath::DegreesToRadians(Angle)) * 90.0f;
}

FLinearColor UAudio_DayNightManager::GetAmbientColor() const
{
    switch (CurrentPhase)
    {
    case EAudio_TimeOfDay::Dawn:
        return FLinearColor(1.0f, 0.6f, 0.3f, 1.0f); // Warm orange dawn
    case EAudio_TimeOfDay::Day:
        return FLinearColor(1.0f, 0.95f, 0.85f, 1.0f); // Bright daylight
    case EAudio_TimeOfDay::Dusk:
        return FLinearColor(1.0f, 0.4f, 0.1f, 1.0f); // Deep red dusk
    case EAudio_TimeOfDay::Night:
        return FLinearColor(0.05f, 0.05f, 0.2f, 1.0f); // Dark blue night
    default:
        return FLinearColor::White;
    }
}
