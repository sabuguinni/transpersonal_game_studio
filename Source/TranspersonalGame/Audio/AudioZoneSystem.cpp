#include "AudioZoneSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ─────────────────────────────────────────────
// AAudio_ZoneTrigger
// ─────────────────────────────────────────────

AAudio_ZoneTrigger::AAudio_ZoneTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);
}

void AAudio_ZoneTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerSphere->SetSphereRadius(ZoneConfig.BlendRadius);

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneTrigger::HandleBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneTrigger::HandleEndOverlap);
}

void AAudio_ZoneTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

void AAudio_ZoneTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bFadingIn)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / FMath::Max(ZoneConfig.FadeInTime, 0.01f), 0.0f, 1.0f);
        CurrentBlendAlpha = Alpha;
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(Alpha * ZoneConfig.AmbientVolume);
        }
        if (Alpha >= 1.0f)
        {
            bFadingIn = false;
        }
    }
    else if (bFadingOut)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / FMath::Max(ZoneConfig.FadeOutTime, 0.01f), 0.0f, 1.0f);
        CurrentBlendAlpha = 1.0f - Alpha;
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(CurrentBlendAlpha * ZoneConfig.AmbientVolume);
        }
        if (Alpha >= 1.0f)
        {
            bFadingOut = false;
            if (AmbientAudioComponent)
            {
                AmbientAudioComponent->Stop();
            }
        }
    }
}

void AAudio_ZoneTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        OnPlayerEnterZone(OtherActor);
    }
}

void AAudio_ZoneTrigger::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        OnPlayerExitZone(OtherActor);
    }
}

void AAudio_ZoneTrigger::OnPlayerEnterZone(AActor* OverlappingActor)
{
    bPlayerInZone = true;
    FadeAmbientIn();
}

void AAudio_ZoneTrigger::OnPlayerExitZone(AActor* OverlappingActor)
{
    bPlayerInZone = false;
    FadeAmbientOut();
}

void AAudio_ZoneTrigger::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    ZoneConfig.DangerLevel = NewLevel;

    // Adjust music volume based on danger
    switch (NewLevel)
    {
    case EAudio_DangerLevel::Safe:
        ZoneConfig.MusicVolume = 0.4f;
        break;
    case EAudio_DangerLevel::Caution:
        ZoneConfig.MusicVolume = 0.6f;
        break;
    case EAudio_DangerLevel::Danger:
        ZoneConfig.MusicVolume = 0.85f;
        break;
    case EAudio_DangerLevel::Critical:
        ZoneConfig.MusicVolume = 1.0f;
        break;
    default:
        break;
    }
}

EAudio_DangerLevel AAudio_ZoneTrigger::GetDangerLevel() const
{
    return ZoneConfig.DangerLevel;
}

void AAudio_ZoneTrigger::FadeAmbientIn()
{
    if (!AmbientAudioComponent) return;

    if (!AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }

    FadeTimer = 0.0f;
    bFadingIn = true;
    bFadingOut = false;
}

void AAudio_ZoneTrigger::FadeAmbientOut()
{
    if (!AmbientAudioComponent) return;

    FadeTimer = 0.0f;
    bFadingOut = true;
    bFadingIn = false;
}

// ─────────────────────────────────────────────
// AAudio_DinosaurSoundEmitter
// ─────────────────────────────────────────────

AAudio_DinosaurSoundEmitter::AAudio_DinosaurSoundEmitter()
{
    PrimaryActorTick.bCanEverTick = true;

    RoarAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RoarAudio"));
    RoarAudioComponent->bAutoActivate = false;
    RootComponent = RoarAudioComponent;

    FootstepAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FootstepAudio"));
    FootstepAudioComponent->SetupAttachment(RootComponent);
    FootstepAudioComponent->bAutoActivate = false;
}

void AAudio_DinosaurSoundEmitter::BeginPlay()
{
    Super::BeginPlay();
    ScheduleNextRoar();
}

void AAudio_DinosaurSoundEmitter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RoarTimer += DeltaTime;
    if (RoarTimer >= TimeUntilNextRoar)
    {
        TriggerRoar();
        ScheduleNextRoar();
    }
}

void AAudio_DinosaurSoundEmitter::TriggerRoar()
{
    if (RoarAudioComponent && RoarAudioComponent->Sound)
    {
        RoarAudioComponent->Play();
    }
}

void AAudio_DinosaurSoundEmitter::TriggerFootstep()
{
    if (FootstepAudioComponent && FootstepAudioComponent->Sound)
    {
        FootstepAudioComponent->Play();
    }
}

void AAudio_DinosaurSoundEmitter::SetDistanceAttenuation(float MaxDistance)
{
    MaxHearingDistance = MaxDistance;
    if (RoarAudioComponent)
    {
        RoarAudioComponent->SetVolumeMultiplier(1.0f);
    }
}

void AAudio_DinosaurSoundEmitter::ScheduleNextRoar()
{
    RoarTimer = 0.0f;
    TimeUntilNextRoar = FMath::RandRange(RoarCooldownMin, RoarCooldownMax);
}
