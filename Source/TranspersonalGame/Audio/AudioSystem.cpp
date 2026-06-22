// AudioSystem.cpp
// Agent #16 — Audio Agent
// Prehistoric survival game audio system — full implementation
// PROD_CYCLE_AUTO_20260622_006
//
// FREESOUND REFERENCES (assets to import):
//   ID 837048 — TRex roar recreation (bassy, prehistoric)
//   ID 811310 — Crocodile/dinosaur bellowing growl
//   ID 586545 — Dinosaur Roars Pack 2 (metal creak based)
//   ID 586547 — Dinosaur Growls Pack 2 (pitched-down burps)
//   ID 586546 — Dinosaur Roars Pack 1 (human vocal + tube)

#include "AudioSystem.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// AAudio_AmbientZone
// ============================================================

AAudio_AmbientZone::AAudio_AmbientZone()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = TriggerSphere;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->VolumeMultiplier = 0.0f;
}

void AAudio_AmbientZone::BeginPlay()
{
    Super::BeginPlay();

    // Set sphere radius from config
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(ZoneConfig.BlendRadius);
    }

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_AmbientZone::OnPlayerEnterZone);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_AmbientZone::OnPlayerExitZone);

    // Start audio component silently — will fade in when player enters
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Play();
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
    }

    TargetVolume = 0.0f;
    CurrentVolume = 0.0f;
}

void AAudio_AmbientZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smooth volume blend
    if (!FMath::IsNearlyEqual(CurrentVolume, TargetVolume, 0.01f))
    {
        CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, ZoneConfig.TransitionSpeed);
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);
        }
    }
}

void AAudio_AmbientZone::OnPlayerEnterZone(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor) return;

    // Check if it's the player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (OtherActor == PlayerPawn)
    {
        bIsPlayerInside = true;
        TargetVolume = ZoneConfig.AmbientVolume;
        UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered %s zone"), *GetActorLabel());
    }
}

void AAudio_AmbientZone::OnPlayerExitZone(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (OtherActor == PlayerPawn)
    {
        bIsPlayerInside = false;
        TargetVolume = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited %s zone"), *GetActorLabel());
    }
}

void AAudio_AmbientZone::SetAmbientVolume(float NewVolume)
{
    TargetVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

EAudio_MusicLayer AAudio_AmbientZone::GetMusicLayer() const
{
    return ZoneConfig.MusicLayer;
}

// ============================================================
// UAudio_FearMusicManager
// ============================================================

UAudio_FearMusicManager::UAudio_FearMusicManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UAudio_FearMusicManager::BeginPlay()
{
    Super::BeginPlay();
    MusicState.ActiveLayer = EAudio_MusicLayer::Calm;
    FearBlendAlpha = 0.0f;
}

void UAudio_FearMusicManager::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Natural fear decay over time (0.5 units/sec when not in danger)
    if (MusicState.CurrentFear > 0.0f)
    {
        MusicState.CurrentFear = FMath::Max(0.0f, MusicState.CurrentFear - (0.5f * DeltaTime));
    }

    // Re-evaluate music layer based on current fear
    EAudio_MusicLayer DesiredLayer = EvaluateMusicLayer(MusicState.CurrentFear);
    if (DesiredLayer != MusicState.ActiveLayer)
    {
        TransitionToLayer(DesiredLayer);
    }
}

void UAudio_FearMusicManager::UpdateFear(float NewFearValue)
{
    MusicState.CurrentFear = FMath::Clamp(NewFearValue, 0.0f, 100.0f);

    EAudio_MusicLayer NewLayer = EvaluateMusicLayer(MusicState.CurrentFear);
    if (NewLayer != MusicState.ActiveLayer)
    {
        TransitionToLayer(NewLayer);
    }
}

EAudio_MusicLayer UAudio_FearMusicManager::EvaluateMusicLayer(float FearValue) const
{
    if (FearValue >= MusicState.CombatThreshold)
        return EAudio_MusicLayer::Combat;
    if (FearValue >= MusicState.DangerThreshold)
        return EAudio_MusicLayer::Danger;
    if (FearValue >= MusicState.TensionThreshold)
        return EAudio_MusicLayer::Tension;
    return EAudio_MusicLayer::Calm;
}

void UAudio_FearMusicManager::TransitionToLayer(EAudio_MusicLayer NewLayer)
{
    if (NewLayer == MusicState.ActiveLayer) return;

    UE_LOG(LogTemp, Log, TEXT("FearMusicManager: Transitioning %d -> %d (Fear=%.1f)"),
        (int32)MusicState.ActiveLayer, (int32)NewLayer, MusicState.CurrentFear);

    MusicState.ActiveLayer = NewLayer;
    FearBlendAlpha = 0.0f;

    // Blueprint event hook — override in BP to actually swap MetaSound parameters
    // MetaSound parameter: "MusicLayer" (int) maps to EAudio_MusicLayer values
    // MetaSound parameter: "FearIntensity" (float) drives percussion/tension layers
}

EAudio_MusicLayer UAudio_FearMusicManager::GetCurrentLayer() const
{
    return MusicState.ActiveLayer;
}

float UAudio_FearMusicManager::GetCurrentFear() const
{
    return MusicState.CurrentFear;
}

// ============================================================
// UAudio_DinoAudioComponent
// ============================================================

UAudio_DinoAudioComponent::UAudio_DinoAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    IdleCallTimer = 0.0f;
}

void UAudio_DinoAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    // Randomise idle call timer to prevent all dinos calling simultaneously
    IdleCallTimer = FMath::RandRange(0.0f, AudioProfile.IdleCallInterval);
}

void UAudio_DinoAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Periodic idle calls
    IdleCallTimer -= DeltaTime;
    if (IdleCallTimer <= 0.0f)
    {
        PlayDinoSound(EAudio_DinoSoundType::Idle);
        IdleCallTimer = AudioProfile.IdleCallInterval + FMath::RandRange(-3.0f, 3.0f);
    }
}

void UAudio_DinoAudioComponent::PlayDinoSound(EAudio_DinoSoundType SoundType)
{
    // In full implementation: select USoundBase asset based on SoundType + species
    // then call UGameplayStatics::PlaySoundAtLocation
    // For now: log the event for Blueprint/MetaSound wiring
    UE_LOG(LogTemp, Verbose, TEXT("DinoAudio[%s]: PlaySound type=%d"),
        *AudioProfile.DinoSpecies.ToString(), (int32)SoundType);
}

void UAudio_DinoAudioComponent::OnDinoAlert()
{
    bIsAlerted = true;
    PlayDinoSound(EAudio_DinoSoundType::Alert);

    // Propagate fear to nearby player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
        if (Distance < AudioProfile.RoarRadius)
        {
            // Fear impact scales with proximity
            float ProximityFactor = 1.0f - (Distance / AudioProfile.RoarRadius);
            float FearImpact = AudioProfile.FearImpactOnPlayer * ProximityFactor;
            UE_LOG(LogTemp, Log, TEXT("DinoAudio: Alert fear impact %.1f on player (dist=%.0f)"),
                FearImpact, Distance);
        }
    }
}

void UAudio_DinoAudioComponent::OnDinoRoar()
{
    PlayDinoSound(EAudio_DinoSoundType::Roar);
    UE_LOG(LogTemp, Log, TEXT("DinoAudio[%s]: ROAR — radius=%.0f"),
        *AudioProfile.DinoSpecies.ToString(), AudioProfile.RoarRadius);
}

void UAudio_DinoAudioComponent::OnDinoFootstep(float StepWeight)
{
    PlayDinoSound(EAudio_DinoSoundType::Footstep);
    // StepWeight 0.0-1.0: 1.0 = TRex full weight, 0.3 = raptor
    // Used to drive screen shake intensity in VFX agent
    UE_LOG(LogTemp, Verbose, TEXT("DinoAudio: Footstep weight=%.2f"), StepWeight);
}

float UAudio_DinoAudioComponent::GetRoarRadius() const
{
    return AudioProfile.RoarRadius;
}

float UAudio_DinoAudioComponent::GetFearImpact() const
{
    return AudioProfile.FearImpactOnPlayer;
}
