#include "AudioZoneSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"

// ============================================================
// Audio Agent #16 — AudioZoneSystem.cpp
// Freesound references catalogued:
//   Campfire:      ID 729396 "Campfire 02" (267s loop, 44100hz)
//   Campfire alt:  ID 729395 "Campfire 01" (109s loop)
//   Dino Roar:     ID 841837 "berserker_breath_roar" (18.7s, tagged dino/dinosaur)
//   Dino Growl:    ID 456854 "Dragon Growl 04 (Far)" (12.2s, tagged dinosaur/distant)
//   Dino Growl 2:  ID 334473 "Dragon Growl 03 (Not So Far, In The Forest)" (9.4s)
// ============================================================

// ─────────────────────────────────────────────────────────────
// AAudio_ZoneActor
// ─────────────────────────────────────────────────────────────

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(1000.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
    AudioComponent->VolumeMultiplier = 0.0f;

    // Default freesound reference: campfire loop
    FreesoundReference.FreesoundID = 729396;
    FreesoundReference.SoundName = TEXT("Campfire 02");
    FreesoundReference.PreviewURL = TEXT("https://cdn.freesound.org/previews/729/729396_12863902-hq.mp3");
    FreesoundReference.DurationSeconds = 267.613f;
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Apply config to sphere
    TriggerSphere->SetSphereRadius(ZoneConfig.TriggerRadius);

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneActor::OnPlayerEnter);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneActor::OnPlayerExit);

    // Initialise random trigger timer for one-shot zones
    if (ZoneConfig.bRandomizeInterval)
    {
        RandomTriggerTimer = FMath::RandRange(ZoneConfig.RandomIntervalMin, ZoneConfig.RandomIntervalMax);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] type=%d radius=%.0f ready"),
        *GetActorLabel(), (int32)ZoneConfig.ZoneType, ZoneConfig.TriggerRadius);
}

void AAudio_ZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smooth fade in/out
    if (bPlayerInside && CurrentVolume < ZoneConfig.MaxVolume)
    {
        float FadeSpeed = (ZoneConfig.FadeInTime > 0.0f)
            ? (ZoneConfig.MaxVolume / ZoneConfig.FadeInTime) * DeltaTime
            : ZoneConfig.MaxVolume;
        CurrentVolume = FMath::Min(CurrentVolume + FadeSpeed, ZoneConfig.MaxVolume);
        AudioComponent->SetVolumeMultiplier(CurrentVolume);
    }
    else if (!bPlayerInside && CurrentVolume > 0.0f)
    {
        float FadeSpeed = (ZoneConfig.FadeOutTime > 0.0f)
            ? (ZoneConfig.MaxVolume / ZoneConfig.FadeOutTime) * DeltaTime
            : ZoneConfig.MaxVolume;
        CurrentVolume = FMath::Max(CurrentVolume - FadeSpeed, 0.0f);
        AudioComponent->SetVolumeMultiplier(CurrentVolume);
        if (CurrentVolume <= 0.0f)
        {
            AudioComponent->Stop();
        }
    }

    // Random one-shot trigger (e.g. distant dino calls)
    if (ZoneConfig.bRandomizeInterval && bPlayerInside)
    {
        RandomTriggerTimer -= DeltaTime;
        if (RandomTriggerTimer <= 0.0f)
        {
            // Fire one-shot — audio designer wires this to a MetaSound cue
            UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] random trigger fired"), *GetActorLabel());
            RandomTriggerTimer = FMath::RandRange(ZoneConfig.RandomIntervalMin, ZoneConfig.RandomIntervalMax);
        }
    }
}

void AAudio_ZoneActor::FadeInAudio()
{
    if (!AudioComponent->IsPlaying())
    {
        AudioComponent->Play();
    }
    bPlayerInside = true;
    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] FadeIn started"), *GetActorLabel());
}

void AAudio_ZoneActor::FadeOutAudio()
{
    bPlayerInside = false;
    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] FadeOut started"), *GetActorLabel());
}

bool AAudio_ZoneActor::IsPlayerInside() const
{
    return bPlayerInside;
}

EAudio_ZoneType AAudio_ZoneActor::GetZoneType() const
{
    return ZoneConfig.ZoneType;
}

void AAudio_ZoneActor::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor == nullptr) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar == nullptr) return;

    // Only respond to locally controlled player
    if (!PlayerChar->IsLocallyControlled()) return;

    FadeInAudio();
    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] player entered — type=%d"),
        *GetActorLabel(), (int32)ZoneConfig.ZoneType);
}

void AAudio_ZoneActor::OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == nullptr) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar == nullptr) return;

    if (!PlayerChar->IsLocallyControlled()) return;

    FadeOutAudio();
    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] player exited"), *GetActorLabel());
}

// ─────────────────────────────────────────────────────────────
// UAudio_ZoneManager
// ─────────────────────────────────────────────────────────────

UAudio_ZoneManager::UAudio_ZoneManager()
{
    GlobalDangerLevel = 0.0f;
}

void UAudio_ZoneManager::RegisterZone(AAudio_ZoneActor* Zone)
{
    if (Zone == nullptr) return;
    if (RegisteredZones.Contains(Zone)) return;
    RegisteredZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: registered zone [%s] total=%d"),
        *Zone->GetActorLabel(), RegisteredZones.Num());
}

void UAudio_ZoneManager::UnregisterZone(AAudio_ZoneActor* Zone)
{
    if (Zone == nullptr) return;
    RegisteredZones.Remove(Zone);
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: unregistered zone [%s] total=%d"),
        *Zone->GetActorLabel(), RegisteredZones.Num());
}

int32 UAudio_ZoneManager::GetActiveZoneCount() const
{
    int32 Count = 0;
    for (const AAudio_ZoneActor* Zone : RegisteredZones)
    {
        if (Zone != nullptr && Zone->IsPlayerInside())
        {
            Count++;
        }
    }
    return Count;
}

TArray<AAudio_ZoneActor*> UAudio_ZoneManager::GetZonesOfType(EAudio_ZoneType ZoneType) const
{
    TArray<AAudio_ZoneActor*> Result;
    for (AAudio_ZoneActor* Zone : RegisteredZones)
    {
        if (Zone != nullptr && Zone->GetZoneType() == ZoneType)
        {
            Result.Add(Zone);
        }
    }
    return Result;
}

void UAudio_ZoneManager::SetDangerLevel(float NewLevel)
{
    GlobalDangerLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: danger level set to %.2f"), GlobalDangerLevel);
}
