#include "AudioZoneManager.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerVolume;

    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudio"));
    AmbienceAudioComponent->SetupAttachment(RootComponent);
    AmbienceAudioComponent->bAutoActivate = false;
    AmbienceAudioComponent->VolumeMultiplier = 0.0f;
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneManager::OnTriggerBeginOverlap);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneManager::OnTriggerEndOverlap);

    ActiveMusicLayer = ZoneConfig.MusicLayer;
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smooth fade in/out of ambience volume
    if (bZoneActive && CurrentFadeAlpha < 1.0f)
    {
        float FadeRate = (ZoneConfig.FadeInTime > 0.0f) ? (DeltaTime / ZoneConfig.FadeInTime) : 1.0f;
        CurrentFadeAlpha = FMath::Clamp(CurrentFadeAlpha + FadeRate, 0.0f, 1.0f);
        AmbienceAudioComponent->SetVolumeMultiplier(CurrentFadeAlpha * ZoneConfig.AmbienceVolume);
    }
    else if (!bZoneActive && CurrentFadeAlpha > 0.0f)
    {
        float FadeRate = (ZoneConfig.FadeOutTime > 0.0f) ? (DeltaTime / ZoneConfig.FadeOutTime) : 1.0f;
        CurrentFadeAlpha = FMath::Clamp(CurrentFadeAlpha - FadeRate, 0.0f, 1.0f);
        AmbienceAudioComponent->SetVolumeMultiplier(CurrentFadeAlpha * ZoneConfig.AmbienceVolume);

        if (CurrentFadeAlpha <= 0.0f)
        {
            AmbienceAudioComponent->Stop();
        }
    }
}

void AAudio_ZoneManager::ActivateZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    if (ZoneConfig.bTriggerOnce && bHasTriggeredOnce) return;

    bZoneActive = true;
    bHasTriggeredOnce = true;

    if (!AmbienceAudioComponent->IsPlaying())
    {
        AmbienceAudioComponent->Play();
    }

    // Fire tension/dread stings based on zone type
    switch (ZoneConfig.ZoneType)
    {
        case EAudio_ZoneType::Tension:
            TriggerTensionSting();
            break;
        case EAudio_ZoneType::Dread:
            TriggerDreadSting();
            break;
        default:
            break;
    }
}

void AAudio_ZoneManager::DeactivateZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    bZoneActive = false;
}

float AAudio_ZoneManager::GetCurrentAmbienceVolume() const
{
    return CurrentFadeAlpha * ZoneConfig.AmbienceVolume;
}

void AAudio_ZoneManager::SetMusicLayer(EAudio_MusicLayer NewLayer)
{
    ActiveMusicLayer = NewLayer;
}

void AAudio_ZoneManager::TriggerTensionSting()
{
    // Blueprint-implementable: plays a one-shot tension sting cue
    // In full implementation, this calls into MetaSounds parameter bus
    UE_LOG(LogTemp, Log, TEXT("AudioZone[%s]: TENSION_STING triggered"), *GetActorLabel());
}

void AAudio_ZoneManager::TriggerDreadSting()
{
    // Blueprint-implementable: plays a one-shot dread sting cue
    UE_LOG(LogTemp, Log, TEXT("AudioZone[%s]: DREAD_STING triggered"), *GetActorLabel());
}

void AAudio_ZoneManager::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar)
    {
        ActivateZone(OtherActor);
    }
}

void AAudio_ZoneManager::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar)
    {
        DeactivateZone(OtherActor);
    }
}
