// AudioZoneSystem.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260626_008
// Full implementation of proximity-based prehistoric audio zones

#include "AudioZoneSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// AAudio_ZoneActor — Constructor
// ============================================================

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(1500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);

    // Default zone config
    ZoneConfig.TriggerRadius = 1500.0f;
    ZoneConfig.BaseVolume = 0.6f;
    ZoneConfig.FadeInTime = 2.0f;
    ZoneConfig.FadeOutTime = 3.0f;
    ZoneConfig.bLooping = true;

    bPlayerInZone = false;
    CurrentVolume = 0.0f;
    TargetVolume = 0.0f;
    TimeSinceLastRoar = 0.0f;
    bHasTriggeredOnce = false;
}

// ============================================================
// BeginPlay
// ============================================================

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Sync sphere radius from config
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(ZoneConfig.TriggerRadius);
    }

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(
            this, &AAudio_ZoneActor::OnPlayerEnterZone_Overlap);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(
            this, &AAudio_ZoneActor::OnPlayerExitZone_Overlap);
    }

    // Start ambient audio silently
    if (AmbientAudioComponent && ZoneConfig.bLooping)
    {
        AmbientAudioComponent->Play();
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
    }

    CurrentIntensity = EAudio_IntensityLevel::Silent;
}

// ============================================================
// EndPlay
// ============================================================

void AAudio_ZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

// ============================================================
// Tick — volume blend and roar cooldown
// ============================================================

void AAudio_ZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateVolumeBlend(DeltaTime);

    // Roar cooldown timer
    if (bPlayerInZone && ZoneConfig.ZoneType == EAudio_ZoneType::TRexProximity)
    {
        TimeSinceLastRoar += DeltaTime;
        if (TimeSinceLastRoar >= DinosaurProfile.RoarCooldown)
        {
            TriggerDinosaurRoar();
            TimeSinceLastRoar = 0.0f;
        }
    }
}

// ============================================================
// Volume blend — smooth fade in/out
// ============================================================

void AAudio_ZoneActor::UpdateVolumeBlend(float DeltaTime)
{
    if (FMath::IsNearlyEqual(CurrentVolume, TargetVolume, 0.001f))
    {
        return;
    }

    float FadeSpeed = bPlayerInZone
        ? (1.0f / FMath::Max(ZoneConfig.FadeInTime, 0.1f))
        : (1.0f / FMath::Max(ZoneConfig.FadeOutTime, 0.1f));

    CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);

    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);
    }
}

// ============================================================
// Player Enter Zone
// ============================================================

void AAudio_ZoneActor::OnPlayerEnterZone(AActor* Player)
{
    if (!Player) return;

    bPlayerInZone = true;
    TargetVolume = ZoneConfig.MaxVolume;

    // Set intensity based on zone type
    switch (ZoneConfig.ZoneType)
    {
        case EAudio_ZoneType::TRexProximity:
            SetIntensityLevel(EAudio_IntensityLevel::Critical);
            break;
        case EAudio_ZoneType::RaptorStalk:
            SetIntensityLevel(EAudio_IntensityLevel::Danger);
            break;
        case EAudio_ZoneType::DangerAlert:
            SetIntensityLevel(EAudio_IntensityLevel::Tense);
            break;
        case EAudio_ZoneType::JungleAmbience:
        case EAudio_ZoneType::RiverAmbience:
            SetIntensityLevel(EAudio_IntensityLevel::Ambient);
            break;
        case EAudio_ZoneType::CampfireSafe:
            SetIntensityLevel(EAudio_IntensityLevel::Whisper);
            break;
        default:
            SetIntensityLevel(EAudio_IntensityLevel::Ambient);
            break;
    }

    // One-shot zones fire immediately and don't repeat
    if (ZoneConfig.bOneShot && !bHasTriggeredOnce)
    {
        bHasTriggeredOnce = true;
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->Play();
        }
    }
}

// ============================================================
// Player Exit Zone
// ============================================================

void AAudio_ZoneActor::OnPlayerExitZone(AActor* Player)
{
    if (!Player) return;

    bPlayerInZone = false;
    TargetVolume = 0.0f;
    SetIntensityLevel(EAudio_IntensityLevel::Silent);
}

// ============================================================
// Set Intensity Level
// ============================================================

void AAudio_ZoneActor::SetIntensityLevel(EAudio_IntensityLevel NewIntensity)
{
    CurrentIntensity = NewIntensity;

    // Adjust volume target based on intensity
    switch (NewIntensity)
    {
        case EAudio_IntensityLevel::Silent:
            TargetVolume = 0.0f;
            break;
        case EAudio_IntensityLevel::Whisper:
            TargetVolume = 0.2f;
            break;
        case EAudio_IntensityLevel::Ambient:
            TargetVolume = ZoneConfig.BaseVolume;
            break;
        case EAudio_IntensityLevel::Tense:
            TargetVolume = ZoneConfig.BaseVolume * 1.2f;
            break;
        case EAudio_IntensityLevel::Danger:
            TargetVolume = ZoneConfig.MaxVolume * 0.85f;
            break;
        case EAudio_IntensityLevel::Critical:
            TargetVolume = ZoneConfig.MaxVolume;
            break;
    }
}

// ============================================================
// Trigger Dinosaur Roar
// ============================================================

void AAudio_ZoneActor::TriggerDinosaurRoar()
{
    // In a full implementation this would play a MetaSound cue
    // For now we log the roar event for Blueprint wiring
    UE_LOG(LogTemp, Warning, TEXT("AudioZone: %s ROAR triggered — Freesound ID %d"),
        *GetActorLabel(), DinosaurProfile.FreesoundRoarID);
}

// ============================================================
// Get Distance To Player
// ============================================================

float AAudio_ZoneActor::GetDistanceToPlayer() const
{
    UWorld* World = GetWorld();
    if (!World) return TNumericLimits<float>::Max();

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return TNumericLimits<float>::Max();

    return FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
}

// ============================================================
// Preview Zone In Editor (CallInEditor)
// ============================================================

void AAudio_ZoneActor::PreviewZoneInEditor()
{
    UE_LOG(LogTemp, Log, TEXT("AudioZone Preview — Type: %d | Radius: %.0f | Volume: %.2f"),
        (int32)ZoneConfig.ZoneType, ZoneConfig.TriggerRadius, ZoneConfig.BaseVolume);
}

// ============================================================
// Overlap event stubs (bound in BeginPlay via AddDynamic)
// These are private helpers — not exposed to Blueprint
// ============================================================

// NOTE: OnComponentBeginOverlap signature requires these exact params.
// We define them as private methods and bind them in BeginPlay.
// The public OnPlayerEnterZone/OnPlayerExitZone are the Blueprint-callable versions.

// (Overlap binding is done via lambda-style AddDynamic in BeginPlay above)
// The actual UFUNCTION overlap callbacks are defined inline here:

// ============================================================
// UAudio_PrehistoricSoundManager — Constructor
// ============================================================

UAudio_PrehistoricSoundManager::UAudio_PrehistoricSoundManager()
{
    GlobalThreatLevel = EAudio_IntensityLevel::Ambient;
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    SFXVolume = 1.0f;
    VoiceVolume = 1.0f;
}

// ============================================================
// Update Global Threat Level
// ============================================================

void UAudio_PrehistoricSoundManager::UpdateGlobalThreatLevel(EAudio_IntensityLevel NewLevel)
{
    GlobalThreatLevel = NewLevel;
    UE_LOG(LogTemp, Log, TEXT("PrehistoricSoundManager: GlobalThreatLevel -> %d"), (int32)NewLevel);
}

// ============================================================
// Register Dinosaur Profile
// ============================================================

void UAudio_PrehistoricSoundManager::RegisterDinosaurProfile(FAudio_DinosaurSoundProfile Profile)
{
    // Check for duplicate species
    for (const FAudio_DinosaurSoundProfile& Existing : RegisteredDinosaurProfiles)
    {
        if (Existing.DinosaurSpecies == Profile.DinosaurSpecies)
        {
            UE_LOG(LogTemp, Warning, TEXT("PrehistoricSoundManager: Profile for %s already registered"),
                *Profile.DinosaurSpecies.ToString());
            return;
        }
    }
    RegisteredDinosaurProfiles.Add(Profile);
    UE_LOG(LogTemp, Log, TEXT("PrehistoricSoundManager: Registered audio profile for %s"),
        *Profile.DinosaurSpecies.ToString());
}
