#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UAudio_ZoneComponent
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAudio_ZoneComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UAudio_ZoneComponent::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    CurrentThreatLevel = NewLevel;
}

EAudio_ThreatLevel UAudio_ZoneComponent::GetThreatLevel() const
{
    return CurrentThreatLevel;
}

float UAudio_ZoneComponent::GetBlendedVolume(float TimeOfDay) const
{
    float Base = ZoneConfig.BaseVolume;
    bool bIsNight = (TimeOfDay < 0.25f || TimeOfDay > 0.75f);
    if (bIsNight)
    {
        Base *= ZoneConfig.NightVolumeMultiplier;
    }

    // Threat level modifies volume
    switch (CurrentThreatLevel)
    {
    case EAudio_ThreatLevel::Calm:
        return Base;
    case EAudio_ThreatLevel::Uneasy:
        return Base * 0.7f;
    case EAudio_ThreatLevel::Danger:
        return Base * 0.4f;
    case EAudio_ThreatLevel::Combat:
        return Base * 0.2f;
    default:
        return Base;
    }
}

// ============================================================
// AAudio_AmbientZoneActor
// ============================================================

AAudio_AmbientZoneActor::AAudio_AmbientZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f;  // Check every 0.5s for performance

    AudioZoneComp = CreateDefaultSubobject<UAudio_ZoneComponent>(TEXT("AudioZoneComp"));
    AmbientAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComp"));

    if (AmbientAudioComp)
    {
        AmbientAudioComp->SetupAttachment(GetRootComponent());
        AmbientAudioComp->bAutoActivate = false;
    }
}

void AAudio_AmbientZoneActor::BeginPlay()
{
    Super::BeginPlay();
    bIsActive = false;
}

void AAudio_AmbientZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    bool bInRange = IsPlayerInRange();
    if (bInRange && !bIsActive)
    {
        ActivateZone();
    }
    else if (!bInRange && bIsActive)
    {
        DeactivateZone();
    }
}

void AAudio_AmbientZoneActor::ActivateZone()
{
    bIsActive = true;
    if (AmbientAudioComp && !AmbientAudioComp->IsPlaying())
    {
        AmbientAudioComp->Play();
    }
}

void AAudio_AmbientZoneActor::DeactivateZone()
{
    bIsActive = false;
    if (AmbientAudioComp && AmbientAudioComp->IsPlaying())
    {
        AmbientAudioComp->FadeOut(2.0f, 0.0f);
    }
}

bool AAudio_AmbientZoneActor::IsPlayerInRange() const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return false;

    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= ActivationRadius;
}

// ============================================================
// AAudio_SystemManager
// ============================================================

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;  // Tick every second for performance
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    CurrentThreatBlend = 0.0f;
    TargetThreatBlend = 0.0f;
    RecentImpacts.Empty();
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateThreatBlend(DeltaTime);
    AuditDinosaurProximity();
}

void AAudio_SystemManager::RegisterImpactEvent(const FAudio_ImpactEvent& Event)
{
    RecentImpacts.Add(Event);

    // Keep only last 10 impacts
    if (RecentImpacts.Num() > 10)
    {
        RecentImpacts.RemoveAt(0);
    }

    // Large impacts escalate threat level
    if (Event.Magnitude >= 5.0f)
    {
        SetGlobalThreatLevel(EAudio_ThreatLevel::Danger);
    }
    else if (Event.Magnitude >= 2.0f && GlobalThreatLevel == EAudio_ThreatLevel::Calm)
    {
        SetGlobalThreatLevel(EAudio_ThreatLevel::Uneasy);
    }
}

void AAudio_SystemManager::SetGlobalThreatLevel(EAudio_ThreatLevel NewLevel)
{
    GlobalThreatLevel = NewLevel;
    switch (NewLevel)
    {
    case EAudio_ThreatLevel::Calm:    TargetThreatBlend = 0.0f; break;
    case EAudio_ThreatLevel::Uneasy:  TargetThreatBlend = 0.33f; break;
    case EAudio_ThreatLevel::Danger:  TargetThreatBlend = 0.66f; break;
    case EAudio_ThreatLevel::Combat:  TargetThreatBlend = 1.0f; break;
    default: TargetThreatBlend = 0.0f; break;
    }
}

EAudio_ThreatLevel AAudio_SystemManager::GetGlobalThreatLevel() const
{
    return GlobalThreatLevel;
}

void AAudio_SystemManager::UpdateTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
}

bool AAudio_SystemManager::IsNightTime() const
{
    return (TimeOfDay < 0.25f || TimeOfDay > 0.75f);
}

void AAudio_SystemManager::TriggerDinosaurProximityAlert(FVector DinosaurLocation, float DinosaurSize)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    float Distance = FVector::Dist(DinosaurLocation, PlayerPawn->GetActorLocation());

    if (Distance <= DinosaurProximityRadius)
    {
        // Large dinosaur (T-Rex scale) = immediate danger
        if (DinosaurSize >= 3.0f)
        {
            SetGlobalThreatLevel(EAudio_ThreatLevel::Danger);
        }
        else if (DinosaurSize >= 1.5f && GlobalThreatLevel == EAudio_ThreatLevel::Calm)
        {
            SetGlobalThreatLevel(EAudio_ThreatLevel::Uneasy);
        }
    }
}

void AAudio_SystemManager::UpdateThreatBlend(float DeltaTime)
{
    if (!FMath::IsNearlyEqual(CurrentThreatBlend, TargetThreatBlend, 0.01f))
    {
        CurrentThreatBlend = FMath::FInterpTo(
            CurrentThreatBlend, TargetThreatBlend, DeltaTime, ThreatTransitionSpeed
        );
    }
}

void AAudio_SystemManager::AuditDinosaurProximity()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    // Scan for actors tagged as dinosaurs
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Dinosaur"), AllActors);

    bool bDinoNearby = false;
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        float Dist = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
        if (Dist <= DinosaurProximityRadius)
        {
            bDinoNearby = true;
            break;
        }
    }

    // Auto-calm if no dinos nearby and not in combat
    if (!bDinoNearby && GlobalThreatLevel == EAudio_ThreatLevel::Uneasy)
    {
        SetGlobalThreatLevel(EAudio_ThreatLevel::Calm);
    }
}
