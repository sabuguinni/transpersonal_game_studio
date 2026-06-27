// AudioZoneSystem.cpp
// Agent #16 — Audio Agent
// Cycle: PROD_CYCLE_AUTO_20260627_007

#include "AudioZoneSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_ZoneActor
// ─────────────────────────────────────────────────────────────────────────────

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = TriggerSphere;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);

    bPlayerInZone = false;
    CurrentVolume = 0.0f;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    FadeTimer = 0.0f;
    bFadingIn = false;
    bFadingOut = false;
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    TriggerSphere->SetSphereRadius(ZoneConfig.BlendRadius);

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneActor::OnSphereBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneActor::OnSphereEndOverlap);

    if (AmbientSound)
    {
        AmbientAudioComponent->SetSound(AmbientSound);
    }
}

void AAudio_ZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

void AAudio_ZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bFadingIn)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / FMath::Max(ZoneConfig.FadeInTime, 0.01f), 0.0f, 1.0f);
        CurrentVolume = Alpha * ZoneConfig.MaxVolume;
        AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);

        if (Alpha >= 1.0f)
        {
            bFadingIn = false;
            FadeTimer = 0.0f;
        }
    }
    else if (bFadingOut)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / FMath::Max(ZoneConfig.FadeOutTime, 0.01f), 0.0f, 1.0f);
        CurrentVolume = (1.0f - Alpha) * ZoneConfig.MaxVolume;
        AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);

        if (Alpha >= 1.0f)
        {
            bFadingOut = false;
            FadeTimer = 0.0f;
            AmbientAudioComponent->Stop();
        }
    }
}

void AAudio_ZoneActor::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    CurrentDangerLevel = NewLevel;

    // Adjust volume multiplier based on danger — danger zones are louder
    switch (NewLevel)
    {
        case EAudio_DangerLevel::Safe:
            ZoneConfig.MaxVolume = 0.6f;
            break;
        case EAudio_DangerLevel::Caution:
            ZoneConfig.MaxVolume = 0.8f;
            break;
        case EAudio_DangerLevel::Danger:
            ZoneConfig.MaxVolume = 1.0f;
            break;
        case EAudio_DangerLevel::Critical:
            ZoneConfig.MaxVolume = 1.2f;
            break;
    }
}

void AAudio_ZoneActor::FadeAudioIn()
{
    if (!AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }
    bFadingIn = true;
    bFadingOut = false;
    FadeTimer = 0.0f;
}

void AAudio_ZoneActor::FadeAudioOut()
{
    bFadingOut = true;
    bFadingIn = false;
    FadeTimer = 0.0f;
}

void AAudio_ZoneActor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                              bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        bPlayerInZone = true;
        FadeAudioIn();
    }
}

void AAudio_ZoneActor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        bPlayerInZone = false;
        FadeAudioOut();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_AdaptiveMusicManager
// ─────────────────────────────────────────────────────────────────────────────

AAudio_AdaptiveMusicManager::AAudio_AdaptiveMusicManager()
{
    PrimaryActorTick.bCanEverTick = true;

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->bAutoActivate = false;
    RootComponent = MusicComponent;

    GlobalDangerLevel = EAudio_DangerLevel::Safe;
    PreviousDangerLevel = EAudio_DangerLevel::Safe;
    TransitionTimer = 0.0f;
    bTransitioning = false;
    MusicTransitionTime = 3.0f;
}

void AAudio_AdaptiveMusicManager::BeginPlay()
{
    Super::BeginPlay();

    // Start with exploration music if assigned
    if (ExplorationMusic)
    {
        MusicComponent->SetSound(ExplorationMusic);
        MusicComponent->SetVolumeMultiplier(0.7f);
        MusicComponent->Play();
    }
}

void AAudio_AdaptiveMusicManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bTransitioning)
    {
        TransitionTimer += DeltaTime;
        float Alpha = FMath::Clamp(TransitionTimer / FMath::Max(MusicTransitionTime, 0.01f), 0.0f, 1.0f);

        // Fade out old track in first half, fade in new track in second half
        if (Alpha < 0.5f)
        {
            float FadeOut = 1.0f - (Alpha * 2.0f);
            MusicComponent->SetVolumeMultiplier(FadeOut * 0.7f);
        }
        else
        {
            float FadeIn = (Alpha - 0.5f) * 2.0f;
            MusicComponent->SetVolumeMultiplier(FadeIn * 0.7f);
        }

        if (Alpha >= 1.0f)
        {
            bTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }
}

void AAudio_AdaptiveMusicManager::SetGlobalDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (NewLevel == GlobalDangerLevel) return;

    PreviousDangerLevel = GlobalDangerLevel;
    GlobalDangerLevel = NewLevel;

    USoundBase* TargetTrack = nullptr;

    switch (NewLevel)
    {
        case EAudio_DangerLevel::Safe:
            TargetTrack = ExplorationMusic;
            break;
        case EAudio_DangerLevel::Caution:
            TargetTrack = NightMusic;
            break;
        case EAudio_DangerLevel::Danger:
            TargetTrack = DangerMusic;
            break;
        case EAudio_DangerLevel::Critical:
            TargetTrack = CombatMusic;
            break;
    }

    if (TargetTrack)
    {
        TransitionToTrack(TargetTrack);
    }
}

void AAudio_AdaptiveMusicManager::TransitionToTrack(USoundBase* NewTrack)
{
    if (!NewTrack) return;

    MusicComponent->SetSound(NewTrack);
    if (!MusicComponent->IsPlaying())
    {
        MusicComponent->Play();
    }

    bTransitioning = true;
    TransitionTimer = 0.0f;
}
