// AudioZoneManager.cpp
// Agent #16 — Audio Agent — PROD_CYCLE_AUTO_20260619_007
// Full implementation of adaptive audio zone manager.

#include "AudioZoneManager.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick — sufficient for audio

    // Trigger sphere — detects player entering the audio zone
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(1500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Ambient audio component — looping environmental sound
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->bIsUISound = false;

    // Voice audio component — NPC dialogue / narration
    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudio"));
    VoiceAudioComponent->SetupAttachment(RootComponent);
    VoiceAudioComponent->bAutoActivate = false;
    VoiceAudioComponent->bIsUISound = false;
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerEnterZone);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerExitZone);
    }
}

// ─── EndPlay ─────────────────────────────────────────────────────────────────

void AAudio_ZoneManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
    {
        VoiceAudioComponent->Stop();
    }

    Super::EndPlay(EndPlayReason);
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    WorldTimeSeconds += DeltaTime;

    // Periodic danger check
    DangerCheckTimer += DeltaTime;
    if (DangerCheckTimer >= DangerCheckInterval)
    {
        DangerCheckTimer = 0.0f;
        UpdateDangerFromDinoProximity();
    }
}

// ─── Zone Overlap ─────────────────────────────────────────────────────────────

void AAudio_ZoneManager::OnPlayerEnterZone(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only respond to player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC || !PC->IsLocalController()) return;

    FadeInAmbience(ZoneConfig.BlendRadius > 0.0f ? 2.0f : 0.5f);
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered %s zone"),
        *UEnum::GetValueAsString(ZoneConfig.BiomeZone));
}

void AAudio_ZoneManager::OnPlayerExitZone(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC || !PC->IsLocalController()) return;

    FadeOutAmbience(3.0f);
}

// ─── Danger Level ─────────────────────────────────────────────────────────────

void AAudio_ZoneManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;

    CurrentDangerLevel = NewLevel;

    // Adjust ambient volume based on danger
    if (AmbientAudioComponent)
    {
        switch (NewLevel)
        {
        case EAudio_DangerLevel::Safe:
            AmbientAudioComponent->SetVolumeMultiplier(1.0f);
            break;
        case EAudio_DangerLevel::Aware:
            AmbientAudioComponent->SetVolumeMultiplier(0.7f);
            break;
        case EAudio_DangerLevel::Danger:
            AmbientAudioComponent->SetVolumeMultiplier(0.4f);
            break;
        case EAudio_DangerLevel::Critical:
            AmbientAudioComponent->SetVolumeMultiplier(0.1f);
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Danger level -> %s"),
        *UEnum::GetValueAsString(NewLevel));
}

void AAudio_ZoneManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Time of day -> %s"),
        *UEnum::GetValueAsString(NewTime));
}

// ─── Voice Lines ──────────────────────────────────────────────────────────────

bool AAudio_ZoneManager::TryPlayVoiceLine(int32 VoiceLineIndex)
{
    if (!VoiceLines.IsValidIndex(VoiceLineIndex)) return false;

    FAudio_VoiceLine& Line = VoiceLines[VoiceLineIndex];

    // Cooldown check
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - Line.LastPlayedTime < Line.CooldownSeconds)
    {
        UE_LOG(LogTemp, Verbose, TEXT("AudioZone: Voice line %d on cooldown (%.1fs remaining)"),
            VoiceLineIndex, Line.CooldownSeconds - (CurrentTime - Line.LastPlayedTime));
        return false;
    }

    PlayVoiceLineImmediate(VoiceLineIndex);
    return true;
}

void AAudio_ZoneManager::PlayVoiceLineImmediate(int32 VoiceLineIndex)
{
    if (!VoiceLines.IsValidIndex(VoiceLineIndex)) return;

    FAudio_VoiceLine& Line = VoiceLines[VoiceLineIndex];
    Line.LastPlayedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Play audio asset if loaded
    USoundBase* SoundAsset = Line.AudioAsset.IsValid() ? Line.AudioAsset.Get() : nullptr;
    if (SoundAsset && VoiceAudioComponent)
    {
        VoiceAudioComponent->SetSound(SoundAsset);
        VoiceAudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("AudioZone: Playing voice line [%s]: %s"),
            *Line.CharacterName, *Line.DialogueText);
    }
    else
    {
        // Log URL for runtime streaming (MetaSounds integration)
        UE_LOG(LogTemp, Log, TEXT("AudioZone: Voice line [%s] URL=%s"),
            *Line.CharacterName, *Line.AudioURL);
    }
}

// ─── Ambience Fade ────────────────────────────────────────────────────────────

void AAudio_ZoneManager::FadeInAmbience(float FadeTime)
{
    if (!AmbientAudioComponent) return;

    if (!AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }
    AmbientAudioComponent->AdjustVolume(FadeTime, 1.0f);
}

void AAudio_ZoneManager::FadeOutAmbience(float FadeTime)
{
    if (!AmbientAudioComponent) return;
    AmbientAudioComponent->AdjustVolume(FadeTime, 0.0f);
}

// ─── Dino Proximity ───────────────────────────────────────────────────────────

float AAudio_ZoneManager::GetDistanceToNearestDino() const
{
    UWorld* World = GetWorld();
    if (!World) return TNumericLimits<float>::Max();

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return TNumericLimits<float>::Max();

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
    float MinDist = TNumericLimits<float>::Max();

    // Check all actors for dino tags
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        FString Label = Actor->GetActorLabel();
        if (Label.Contains(TEXT("TRex")) || Label.Contains(TEXT("Raptor")) ||
            Label.Contains(TEXT("Brachio")) || Label.Contains(TEXT("Dino")))
        {
            float Dist = FVector::Dist(PlayerLoc, Actor->GetActorLocation());
            MinDist = FMath::Min(MinDist, Dist);
        }
    }

    return MinDist;
}

void AAudio_ZoneManager::UpdateDangerFromDinoProximity()
{
    float NearestDino = GetDistanceToNearestDino();

    EAudio_DangerLevel NewLevel = EAudio_DangerLevel::Safe;

    if (NearestDino < RaptorProximityRadius * 0.5f)
    {
        NewLevel = EAudio_DangerLevel::Critical;
    }
    else if (NearestDino < RaptorProximityRadius)
    {
        NewLevel = EAudio_DangerLevel::Danger;
    }
    else if (NearestDino < TRexProximityRadius)
    {
        NewLevel = EAudio_DangerLevel::Aware;
    }

    SetDangerLevel(NewLevel);
}
