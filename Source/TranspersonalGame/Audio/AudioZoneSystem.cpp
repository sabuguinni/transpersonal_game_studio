// AudioZoneSystem.cpp
// Audio Agent #16 — PROD_CYCLE_AUTO_20260623_003
// Proximity-based ambient audio zone system for prehistoric survival game.
// Zones crossfade ambient loops as the player moves through the world.
// Creature vocalisations fire at random intervals when player is inside zone.

#include "AudioZoneSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
AAudio_AmbientZone::AAudio_AmbientZone()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root trigger sphere — radius set from ZoneConfig in BeginPlay
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(800.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    TriggerSphere->SetGenerateOverlapEvents(true);
    RootComponent = TriggerSphere;

    // Audio component — starts silent, volume driven by distance
    AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
    AudioComp->SetupAttachment(RootComponent);
    AudioComp->bAutoActivate = false;
    AudioComp->VolumeMultiplier = 0.0f;

    // Seed default Freesound references for the campfire zone
    FAudio_SoundReference CampfireRef;
    CampfireRef.FreesoundID = 856943;
    CampfireRef.Description = TEXT("Campfire with forest birds — crackling wood, natural ambience");
    CampfireRef.PreviewURL = TEXT("https://cdn.freesound.org/previews/856/856943_12846320-hq.mp3");
    CampfireRef.Duration = 60.0f;
    SoundReferences.Add(CampfireRef);

    FAudio_SoundReference CampfireRef2;
    CampfireRef2.FreesoundID = 729396;
    CampfireRef2.Description = TEXT("Campfire 02 — close perspective crackling loop");
    CampfireRef2.PreviewURL = TEXT("https://cdn.freesound.org/previews/729/729396_12863902-hq.mp3");
    CampfireRef2.Duration = 267.0f;
    SoundReferences.Add(CampfireRef2);
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void AAudio_AmbientZone::BeginPlay()
{
    Super::BeginPlay();

    // Apply zone config radius to sphere
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(ZoneConfig.FadeRadius);
    }

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_AmbientZone::OnSphereBeginOverlap);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_AmbientZone::OnSphereEndOverlap);
    }

    // Assign sound to audio component if set
    if (AudioComp && AmbientSound)
    {
        AudioComp->SetSound(AmbientSound);
    }

    // Schedule first vocalisation
    if (ZoneConfig.bPlayCreatureVocalisations)
    {
        ScheduleNextVocalisation();
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] BeginPlay — Type=%d DangerLevel=%d FadeRadius=%.0f"),
        *GetActorLabel(),
        (int32)ZoneConfig.ZoneType,
        (int32)ZoneConfig.DangerLevel,
        ZoneConfig.FadeRadius);
}

// ─────────────────────────────────────────────────────────────────────────────
// EndPlay
// ─────────────────────────────────────────────────────────────────────────────
void AAudio_AmbientZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AudioComp && AudioComp->IsPlaying())
    {
        AudioComp->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick — volume fade by distance + vocalisation timer
// ─────────────────────────────────────────────────────────────────────────────
void AAudio_AmbientZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bPlayerInZone)
    {
        UpdateVolumeByDistance(DeltaTime);

        // Creature vocalisation timer
        if (ZoneConfig.bPlayCreatureVocalisations && CreatureVocalisationSound)
        {
            VocalisationTimer += DeltaTime;
            if (VocalisationTimer >= NextVocalisationTime)
            {
                UGameplayStatics::PlaySoundAtLocation(this, CreatureVocalisationSound, GetActorLocation());
                UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] — creature vocalisation fired"), *GetActorLabel());
                ScheduleNextVocalisation();
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// OnSphereBeginOverlap
// ─────────────────────────────────────────────────────────────────────────────
void AAudio_AmbientZone::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (OtherActor != PlayerPawn) return;

    bPlayerInZone = true;
    VocalisationTimer = 0.0f;

    if (AudioComp && AmbientSound && !AudioComp->IsPlaying())
    {
        AudioComp->Play();
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] — player ENTERED (DangerLevel=%d)"),
        *GetActorLabel(), (int32)ZoneConfig.DangerLevel);
}

// ─────────────────────────────────────────────────────────────────────────────
// OnSphereEndOverlap
// ─────────────────────────────────────────────────────────────────────────────
void AAudio_AmbientZone::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (OtherActor != PlayerPawn) return;

    bPlayerInZone = false;

    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] — player EXITED"), *GetActorLabel());
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateVolumeByDistance — linear fade between FullVolumeRadius and FadeRadius
// ─────────────────────────────────────────────────────────────────────────────
void AAudio_AmbientZone::UpdateVolumeByDistance(float DeltaTime)
{
    float Dist = GetDistanceToPlayer();
    float TargetVolume = 0.0f;

    if (Dist <= ZoneConfig.FullVolumeRadius)
    {
        TargetVolume = ZoneConfig.VolumeMultiplier;
    }
    else if (Dist < ZoneConfig.FadeRadius)
    {
        float Alpha = 1.0f - ((Dist - ZoneConfig.FullVolumeRadius) /
                              (ZoneConfig.FadeRadius - ZoneConfig.FullVolumeRadius));
        TargetVolume = Alpha * ZoneConfig.VolumeMultiplier;
    }

    // Smooth blend toward target volume
    float BlendSpeed = (ZoneConfig.BlendTime > 0.0f) ? (1.0f / ZoneConfig.BlendTime) : 10.0f;
    CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, BlendSpeed);

    if (AudioComp)
    {
        AudioComp->SetVolumeMultiplier(CurrentVolume);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// GetDistanceToPlayer
// ─────────────────────────────────────────────────────────────────────────────
float AAudio_AmbientZone::GetDistanceToPlayer() const
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return TNumericLimits<float>::Max();

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return TNumericLimits<float>::Max();

    return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
}

// ─────────────────────────────────────────────────────────────────────────────
// SetZoneActive
// ─────────────────────────────────────────────────────────────────────────────
void AAudio_AmbientZone::SetZoneActive(bool bActive)
{
    if (TriggerSphere)
    {
        TriggerSphere->SetGenerateOverlapEvents(bActive);
    }
    if (!bActive && AudioComp && AudioComp->IsPlaying())
    {
        AudioComp->Stop();
        bPlayerInZone = false;
        CurrentVolume = 0.0f;
    }
    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] — SetZoneActive=%d"), *GetActorLabel(), (int32)bActive);
}

// ─────────────────────────────────────────────────────────────────────────────
// GetDangerLevel
// ─────────────────────────────────────────────────────────────────────────────
EAudio_DangerLevel AAudio_AmbientZone::GetDangerLevel() const
{
    return ZoneConfig.DangerLevel;
}

// ─────────────────────────────────────────────────────────────────────────────
// ScheduleNextVocalisation
// ─────────────────────────────────────────────────────────────────────────────
void AAudio_AmbientZone::ScheduleNextVocalisation()
{
    VocalisationTimer = 0.0f;
    NextVocalisationTime = FMath::FRandRange(
        ZoneConfig.VocalisationIntervalMin,
        ZoneConfig.VocalisationIntervalMax);
    UE_LOG(LogTemp, Verbose, TEXT("AudioZone [%s] — next vocalisation in %.1fs"),
        *GetActorLabel(), NextVocalisationTime);
}
