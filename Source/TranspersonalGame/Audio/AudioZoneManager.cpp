// AudioZoneManager.cpp — Agent #16 Audio Agent
// Ambient audio zone management for prehistoric survival game.
// Zones correspond to narrative trigger zones placed by Agent #15.
//
// Sound references (Freesound.org — free license):
//   Camp fire:       ID #729395 — Campfire 01 (108s loop, outdoor)
//   Camp fire alt:   ID #729396 — Campfire 02 (267s loop)
//   Dino roars:      ID #586545 — Dinosaur Roars Pack 2 (39s)
//   Dino growls:     ID #586547 — Dinosaur Growls Pack 2 (34s)
//   Croc/dino bellow:ID #811310 — crocodile_dinosaur_bellowing (44s)
//   T-Rex recreation:ID #837048 — berserker_hercules_roar (16s, bassy)

#include "AudioZoneManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// UAudio_ZoneComponent
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetSphereRadius(500.0f);
    SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void UAudio_ZoneComponent::OnPlayerEnterZone(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsPlayerControlled())
    {
        bPlayerInZone = true;
        UE_LOG(LogTemp, Log, TEXT("AudioZone: Player ENTERED zone type=%d"),
            static_cast<int32>(ZoneConfig.ZoneType));
    }
}

void UAudio_ZoneComponent::OnPlayerExitZone(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsPlayerControlled())
    {
        bPlayerInZone = false;
        UE_LOG(LogTemp, Log, TEXT("AudioZone: Player EXITED zone type=%d"),
            static_cast<int32>(ZoneConfig.ZoneType));
    }
}

float UAudio_ZoneComponent::GetCurrentVolume() const
{
    if (!bPlayerInZone) return 0.0f;
    return ZoneConfig.MaxVolume;
}

// ============================================================
// AAudio_ZoneManager
// ============================================================

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s — audio doesn't need per-frame

    // Default day/night state — start at midday
    DayNightState.CurrentHour = 12.0f;
    DayNightState.DayNightCycleDuration = 1200.0f;
    DayNightState.DawnStartHour = 5.5f;
    DayNightState.DuskStartHour = 18.0f;
    DayNightState.NightStartHour = 20.0f;
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: BeginPlay — ActiveZone=%d DayHour=%.1f"),
        static_cast<int32>(ActiveZoneType), DayNightState.CurrentHour);
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateDayNightCycle(DeltaTime);
}

void AAudio_ZoneManager::SetActiveZone(EAudio_ZoneType NewZone)
{
    if (ActiveZoneType == NewZone) return;

    PreviousZoneType = ActiveZoneType;
    ActiveZoneType = NewZone;
    ZoneTransitionTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Zone transition %d -> %d"),
        static_cast<int32>(PreviousZoneType), static_cast<int32>(ActiveZoneType));
}

void AAudio_ZoneManager::UpdateDayNightCycle(float DeltaTime)
{
    if (DayNightState.DayNightCycleDuration <= 0.0f) return;

    // Advance time — 24 game hours per cycle duration
    const float HoursPerSecond = 24.0f / DayNightState.DayNightCycleDuration;
    DayNightState.CurrentHour += HoursPerSecond * DeltaTime;

    // Wrap at 24h
    if (DayNightState.CurrentHour >= 24.0f)
    {
        DayNightState.CurrentHour -= 24.0f;
    }
}

float AAudio_ZoneManager::GetDayNightBlend() const
{
    const float Hour = DayNightState.CurrentHour;

    // Night: 20:00 - 05:30
    if (Hour >= DayNightState.NightStartHour || Hour < DayNightState.DawnStartHour)
    {
        return 1.0f; // Full night
    }

    // Dawn transition: 05:30 - 07:00
    if (Hour >= DayNightState.DawnStartHour && Hour < 7.0f)
    {
        return 1.0f - ((Hour - DayNightState.DawnStartHour) / (7.0f - DayNightState.DawnStartHour));
    }

    // Full day: 07:00 - 18:00
    if (Hour >= 7.0f && Hour < DayNightState.DuskStartHour)
    {
        return 0.0f; // Full day
    }

    // Dusk transition: 18:00 - 20:00
    if (Hour >= DayNightState.DuskStartHour && Hour < DayNightState.NightStartHour)
    {
        return (Hour - DayNightState.DuskStartHour) / (DayNightState.NightStartHour - DayNightState.DuskStartHour);
    }

    return 0.0f;
}

bool AAudio_ZoneManager::IsDawnOrDusk() const
{
    const float Hour = DayNightState.CurrentHour;
    const bool bDawn = (Hour >= DayNightState.DawnStartHour && Hour < 7.0f);
    const bool bDusk = (Hour >= DayNightState.DuskStartHour && Hour < DayNightState.NightStartHour);
    return bDawn || bDusk;
}

void AAudio_ZoneManager::SetPredatorProximity(float Factor)
{
    PredatorProximityFactor = FMath::Clamp(Factor, 0.0f, 1.0f);

    // When predator is very close, force predator zone audio
    if (PredatorProximityFactor > 0.8f && ActiveZoneType != EAudio_ZoneType::Combat)
    {
        SetActiveZone(EAudio_ZoneType::Predator);
    }
    else if (PredatorProximityFactor < 0.1f && ActiveZoneType == EAudio_ZoneType::Predator)
    {
        SetActiveZone(EAudio_ZoneType::Wind); // Return to ambient
    }
}

FString AAudio_ZoneManager::GetCurrentAudioState() const
{
    const float DayNightBlend = GetDayNightBlend();
    const bool bTransition = IsDawnOrDusk();

    return FString::Printf(
        TEXT("Zone=%d Hour=%.1f DayNight=%.2f Predator=%.2f DawnDusk=%s"),
        static_cast<int32>(ActiveZoneType),
        DayNightState.CurrentHour,
        DayNightBlend,
        PredatorProximityFactor,
        bTransition ? TEXT("YES") : TEXT("NO")
    );
}

void AAudio_ZoneManager::LogAudioState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AUDIO STATE === %s"), *GetCurrentAudioState());
}
