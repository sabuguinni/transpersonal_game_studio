// BiomeAudioSystem.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_003
// Implementation of biome-aware ambient audio trigger zones.

#include "BiomeAudioSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

ABiomeAudioZone::ABiomeAudioZone()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(2000.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));

    // Ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);
}

void ABiomeAudioZone::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    TriggerSphere->OnActorBeginOverlap.AddDynamic(this, &ABiomeAudioZone::OnPlayerEnterZone);
    TriggerSphere->OnActorEndOverlap.AddDynamic(this, &ABiomeAudioZone::OnPlayerExitZone);

    // Sync sphere radius from config
    TriggerSphere->SetSphereRadius(BiomeConfig.TriggerRadius);

    UE_LOG(LogTemp, Log, TEXT("BiomeAudioZone [%s] initialised — Biome: %s, Radius: %.0f"),
        *GetActorLabel(),
        *GetBiomeDebugName(),
        BiomeConfig.TriggerRadius);
}

void ABiomeAudioZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickAudioFade(DeltaTime);

    if (bIsPlayerInZone && BiomeConfig.bPlayDinoCallsInZone)
    {
        TickDinoCallTimer(DeltaTime);
    }
}

void ABiomeAudioZone::OnPlayerEnterZone(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor) return;

    // Check if it's the player (has PlayerController)
    APlayerController* PC = OtherActor->GetWorld() ?
        OtherActor->GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (PlayerPawn != OtherActor) return;

    bIsPlayerInZone = true;
    FadeInAmbientAudio();

    UE_LOG(LogTemp, Log, TEXT("BiomeAudioZone [%s]: Player ENTERED — fading in ambient audio"),
        *GetActorLabel());
}

void ABiomeAudioZone::OnPlayerExitZone(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor) return;

    APlayerController* PC = OtherActor->GetWorld() ?
        OtherActor->GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (PlayerPawn != OtherActor) return;

    bIsPlayerInZone = false;
    FadeOutAmbientAudio();

    UE_LOG(LogTemp, Log, TEXT("BiomeAudioZone [%s]: Player EXITED — fading out ambient audio"),
        *GetActorLabel());
}

void ABiomeAudioZone::FadeInAmbientAudio()
{
    if (!AmbientAudioComponent) return;

    AmbientAudioComponent->Play();
    bFadingIn = true;
    bFadingOut = false;
    CurrentFadeProgress = AmbientAudioComponent->VolumeMultiplier;
}

void ABiomeAudioZone::FadeOutAmbientAudio()
{
    if (!AmbientAudioComponent) return;

    bFadingOut = true;
    bFadingIn = false;
    CurrentFadeProgress = AmbientAudioComponent->VolumeMultiplier;
}

void ABiomeAudioZone::TickAudioFade(float DeltaTime)
{
    if (!AmbientAudioComponent) return;

    if (bFadingIn)
    {
        float Target = BiomeConfig.AmbientVolume;
        float Speed = (BiomeConfig.FadeInTime > 0.0f) ? (Target / BiomeConfig.FadeInTime) : Target;
        CurrentFadeProgress = FMath::Min(CurrentFadeProgress + Speed * DeltaTime, Target);
        AmbientAudioComponent->SetVolumeMultiplier(CurrentFadeProgress);

        if (FMath::IsNearlyEqual(CurrentFadeProgress, Target, 0.01f))
        {
            bFadingIn = false;
        }
    }
    else if (bFadingOut)
    {
        float Speed = (BiomeConfig.FadeOutTime > 0.0f) ?
            (BiomeConfig.AmbientVolume / BiomeConfig.FadeOutTime) : BiomeConfig.AmbientVolume;
        CurrentFadeProgress = FMath::Max(CurrentFadeProgress - Speed * DeltaTime, 0.0f);
        AmbientAudioComponent->SetVolumeMultiplier(CurrentFadeProgress);

        if (FMath::IsNearlyEqual(CurrentFadeProgress, 0.0f, 0.01f))
        {
            bFadingOut = false;
            AmbientAudioComponent->Stop();
        }
    }
}

void ABiomeAudioZone::TickDinoCallTimer(float DeltaTime)
{
    DinoCallTimer += DeltaTime;

    if (DinoCallTimer >= BiomeConfig.DinoCallInterval)
    {
        DinoCallTimer = 0.0f;
        // Randomise next interval slightly (±30%)
        BiomeConfig.DinoCallInterval = FMath::RandRange(
            BiomeConfig.DinoCallInterval * 0.7f,
            BiomeConfig.DinoCallInterval * 1.3f
        );

        UE_LOG(LogTemp, Log, TEXT("BiomeAudioZone [%s]: Dino call trigger — biome %s"),
            *GetActorLabel(), *GetBiomeDebugName());
        // Audio Agent (#16) will bind actual sound cues to this event via Blueprint
    }
}

FString ABiomeAudioZone::GetBiomeDebugName() const
{
    switch (BiomeConfig.BiomeType)
    {
        case EWorld_BiomeAudioType::Forest:   return TEXT("Dense Forest");
        case EWorld_BiomeAudioType::Plains:   return TEXT("Open Plains");
        case EWorld_BiomeAudioType::River:    return TEXT("River Bank");
        case EWorld_BiomeAudioType::Rocky:    return TEXT("Rocky Highlands");
        case EWorld_BiomeAudioType::Volcanic: return TEXT("Volcanic Region");
        case EWorld_BiomeAudioType::Cave:     return TEXT("Cave Interior");
        default:                              return TEXT("Unknown");
    }
}
