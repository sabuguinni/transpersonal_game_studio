#include "AudioZoneManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    BlendSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BlendSphere"));
    BlendSphere->SetSphereRadius(500.0f);
    BlendSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = BlendSphere;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->bIsUISound = false;
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Update sphere radius from config
    if (BlendSphere)
    {
        BlendSphere->SetSphereRadius(ZoneConfig.BlendRadius);
        BlendSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerEnterZone);
        BlendSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerExitZone);
    }

    TargetVolume = ZoneConfig.BaseVolume;
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smoothly blend audio volume
    if (AmbientAudioComponent && AmbientAudioComponent->IsActive())
    {
        float CurrentVol = AmbientAudioComponent->VolumeMultiplier;
        float BlendedVol = FMath::FInterpTo(CurrentVol, TargetVolume, DeltaTime, 2.0f);
        AmbientAudioComponent->SetVolumeMultiplier(BlendedVol);
    }
}

void AAudio_ZoneManager::SetZoneType(EAudio_ZoneType NewType)
{
    ZoneConfig.ZoneType = NewType;
}

void AAudio_ZoneManager::SetDangerLevel(float DangerLevel)
{
    CurrentDangerLevel = FMath::Clamp(DangerLevel, 0.0f, 1.0f);

    // Increase volume/intensity based on danger
    TargetVolume = ZoneConfig.BaseVolume + (CurrentDangerLevel * ZoneConfig.DangerMultiplier * 0.5f);
    TargetVolume = FMath::Clamp(TargetVolume, 0.0f, 2.0f);
}

void AAudio_ZoneManager::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsPlayerControlled())
    {
        if (AmbientAudioComponent && !AmbientAudioComponent->IsActive())
        {
            AmbientAudioComponent->Activate(true);
            AmbientAudioComponent->SetVolumeMultiplier(0.0f); // Fade in via Tick
            TargetVolume = ZoneConfig.BaseVolume;
        }
    }
}

void AAudio_ZoneManager::OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsPlayerControlled())
    {
        // Fade out — set target to 0, Tick will blend, then deactivate
        TargetVolume = 0.0f;
    }
}
