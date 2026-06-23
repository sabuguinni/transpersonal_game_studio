// AudioZoneManager.cpp
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260622_013
// Prehistoric survival game — ambient audio zone system
// NO spiritual/mystical content. Realistic survival soundscape only.

#include "AudioZoneManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─── Constructor ──────────────────────────────────────────────────────────

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick — audio doesn't need 60Hz

    // Zone trigger sphere
    ZoneTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneTrigger"));
    ZoneTrigger->SetSphereRadius(800.0f);
    ZoneTrigger->SetCollisionProfileName(TEXT("Trigger"));
    ZoneTrigger->SetGenerateOverlapEvents(true);
    RootComponent = ZoneTrigger;

    // Ambient audio component (asset assigned in editor or via BP)
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f); // start silent, fade in on player enter
}

// ─── BeginPlay ────────────────────────────────────────────────────────────

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    ZoneTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerEnterZone);
    ZoneTrigger->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerExitZone);

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager [%s] BeginPlay — ZoneType=%d"),
           *GetActorLabel(), (int32)ZoneConfig.ZoneType);
}

// ─── EndPlay ──────────────────────────────────────────────────────────────

void AAudio_ZoneManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

// ─── Tick ─────────────────────────────────────────────────────────────────

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bPlayerInZone) return;

    // Periodic danger check
    DangerCheckTimer += DeltaTime;
    if (DangerCheckTimer >= DangerCheckInterval)
    {
        DangerCheckTimer = 0.0f;
        CheckNearbyPredators();
    }

    // Smooth ambient volume blend toward target
    if (AmbientAudioComponent)
    {
        float TargetVol = GetAmbientVolumeForTime();
        BlendAmbientVolume(TargetVol, DeltaTime);
    }
}

// ─── Overlap Handlers ─────────────────────────────────────────────────────

void AAudio_ZoneManager::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp,
                                            AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp,
                                            int32 OtherBodyIndex,
                                            bool bFromSweep,
                                            const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    // Only respond to the local player character
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || PC->GetCharacter() != PlayerChar) return;

    bPlayerInZone = true;

    if (AmbientAudioComponent && !AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] — Player entered, ambient started"), *GetActorLabel());
    }
}

void AAudio_ZoneManager::OnPlayerExitZone(UPrimitiveComponent* OverlappedComp,
                                           AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp,
                                           int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || PC->GetCharacter() != PlayerChar) return;

    bPlayerInZone = false;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;

    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->FadeOut(2.0f, 0.0f);
        UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] — Player exited, ambient fading out"), *GetActorLabel());
    }
}

// ─── SetDangerLevel ───────────────────────────────────────────────────────

void AAudio_ZoneManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (NewLevel == CurrentDangerLevel) return;

    EAudio_DangerLevel OldLevel = CurrentDangerLevel;
    CurrentDangerLevel = NewLevel;

    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s] — Danger: %d -> %d"),
           *GetActorLabel(), (int32)OldLevel, (int32)NewLevel);

    // Adjust ambient volume based on danger
    if (AmbientAudioComponent)
    {
        switch (NewLevel)
        {
        case EAudio_DangerLevel::Safe:
            AmbientAudioComponent->SetVolumeMultiplier(ZoneConfig.AmbientVolume);
            break;
        case EAudio_DangerLevel::Caution:
            // Duck ambient, let danger sting through
            AmbientAudioComponent->SetVolumeMultiplier(ZoneConfig.AmbientVolume * 0.5f);
            break;
        case EAudio_DangerLevel::Imminent:
        case EAudio_DangerLevel::Combat:
            AmbientAudioComponent->SetVolumeMultiplier(0.1f);
            break;
        }
    }
}

// ─── UpdateTimeOfDay ──────────────────────────────────────────────────────

void AAudio_ZoneManager::UpdateTimeOfDay(float NormalizedTime)
{
    CurrentTimeOfDay = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    UE_LOG(LogTemp, Verbose, TEXT("AudioZone [%s] — TimeOfDay=%.2f"), *GetActorLabel(), CurrentTimeOfDay);
}

// ─── GetAmbientVolumeForTime ──────────────────────────────────────────────

float AAudio_ZoneManager::GetAmbientVolumeForTime() const
{
    // Sine curve: noon (0.5) = day volume, midnight (0.0/1.0) = night volume
    // Night insects and predator calls are louder
    float DayBlend = FMath::Sin(CurrentTimeOfDay * PI); // 0 at midnight, 1 at noon
    return FMath::Lerp(NightAmbientVolume, DayAmbientVolume, DayBlend);
}

// ─── TriggerTRexStompShake ────────────────────────────────────────────────

void AAudio_ZoneManager::TriggerTRexStompShake(float Distance)
{
    if (Distance > FootstepConfig.ScreenShakeRadiusTRex) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    // Scale shake intensity by proximity (closer = stronger)
    float ProximityFactor = 1.0f - (Distance / FootstepConfig.ScreenShakeRadiusTRex);
    float ShakeScale = FootstepConfig.ScreenShakeIntensity * ProximityFactor;

    // ClientStartCameraShake requires a camera shake class — log intent for Blueprint wiring
    UE_LOG(LogTemp, Log, TEXT("AudioZone — T-Rex stomp shake: dist=%.0f scale=%.2f"),
           Distance, ShakeScale);

    // Console command fallback for editor testing
    UWorld* W = GetWorld();
    if (W)
    {
        FString Cmd = FString::Printf(TEXT("shake %.2f"), ShakeScale);
        UGameplayStatics::PlayWorldCameraShake(W, nullptr, GetActorLocation(),
                                               0.0f, FootstepConfig.ScreenShakeRadiusTRex,
                                               ShakeScale, false);
    }
}

// ─── TriggerDamageFlash ───────────────────────────────────────────────────

void AAudio_ZoneManager::TriggerDamageFlash()
{
    // Damage flash is handled by HUD/UMG — this logs the event for Blueprint binding
    UE_LOG(LogTemp, Warning, TEXT("AudioZone — DamageFlash triggered (wire to HUD BP)"));

    // Play damage audio sting if component has a sound assigned
    if (AmbientAudioComponent)
    {
        // Momentary volume spike to simulate impact audio
        AmbientAudioComponent->SetVolumeMultiplier(ZoneConfig.DangerStingVolume);
    }
}

// ─── CheckNearbyPredators ─────────────────────────────────────────────────

void AAudio_ZoneManager::CheckNearbyPredators()
{
    UWorld* W = GetWorld();
    if (!W) return;

    APlayerController* PC = W->GetFirstPlayerController();
    if (!PC || !PC->GetCharacter()) return;

    FVector PlayerLoc = PC->GetCharacter()->GetActorLocation();

    // Scan all actors for dinosaur labels (matches MinPlayableMap naming convention)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(W, AActor::StaticClass(), AllActors);

    float ClosestRaptor = TNumericLimits<float>::Max();
    float ClosestTRex   = TNumericLimits<float>::Max();

    for (AActor* A : AllActors)
    {
        if (!A) continue;
        FString Label = A->GetActorLabel().ToLower();
        float Dist = FVector::Dist(A->GetActorLocation(), PlayerLoc);

        if (Label.Contains(TEXT("raptor")))
        {
            ClosestRaptor = FMath::Min(ClosestRaptor, Dist);
        }
        else if (Label.Contains(TEXT("trex")) || Label.Contains(TEXT("t-rex")))
        {
            ClosestTRex = FMath::Min(ClosestTRex, Dist);
            if (Dist < FootstepConfig.ScreenShakeRadiusTRex)
            {
                TriggerTRexStompShake(Dist);
            }
        }
    }

    // Determine danger level from closest predator
    EAudio_DangerLevel NewDanger = EAudio_DangerLevel::Safe;

    if (ClosestTRex < TRexDangerRadius * 0.4f || ClosestRaptor < RaptorDangerRadius * 0.3f)
    {
        NewDanger = EAudio_DangerLevel::Imminent;
    }
    else if (ClosestTRex < TRexDangerRadius || ClosestRaptor < RaptorDangerRadius)
    {
        NewDanger = EAudio_DangerLevel::Caution;
    }

    SetDangerLevel(NewDanger);
}

// ─── BlendAmbientVolume ───────────────────────────────────────────────────

void AAudio_ZoneManager::BlendAmbientVolume(float TargetVolume, float DeltaTime)
{
    if (!AmbientAudioComponent) return;

    float Current = AmbientAudioComponent->VolumeMultiplier;
    float BlendSpeed = 0.5f; // volume units per second
    float NewVol = FMath::FInterpTo(Current, TargetVolume, DeltaTime, BlendSpeed);
    AmbientAudioComponent->SetVolumeMultiplier(NewVol);
}
