#include "AudioZoneManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AAudioZoneManager::AAudioZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(1500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
    AudioComponent->VolumeMultiplier = 1.0f;
}

void AAudioZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Apply config to sphere radius
    TriggerSphere->SetSphereRadius(ZoneConfig.TriggerRadius);
    AudioComponent->VolumeMultiplier = ZoneConfig.VolumeMultiplier;

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudioZoneManager::OnPlayerEnterZone);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudioZoneManager::OnPlayerExitZone);

    // Looping ambient zones auto-start
    if (ZoneConfig.bLooping && !ZoneConfig.bOneShot)
    {
        if (AudioComponent->Sound)
        {
            AudioComponent->Play();
        }
    }
}

void AAudioZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Reserved for adaptive volume fade based on player distance
}

void AAudioZoneManager::ActivateZone()
{
    if (!bHasPlayedOneShot || !ZoneConfig.bOneShot)
    {
        if (AudioComponent && AudioComponent->Sound)
        {
            AudioComponent->Play();
            if (ZoneConfig.bOneShot)
            {
                bHasPlayedOneShot = true;
            }
        }
    }
}

void AAudioZoneManager::DeactivateZone()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->FadeOut(2.0f, 0.0f);
    }
}

void AAudioZoneManager::SetVolumeMultiplier(float NewVolume)
{
    ZoneConfig.VolumeMultiplier = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    if (AudioComponent)
    {
        AudioComponent->VolumeMultiplier = ZoneConfig.VolumeMultiplier;
    }
}

bool AAudioZoneManager::IsPlayerInZone() const
{
    return bPlayerInZone;
}

EAudio_ZoneType AAudioZoneManager::GetZoneType() const
{
    return ZoneConfig.ZoneType;
}

FString AAudioZoneManager::GetLinkedVoiceURL() const
{
    return ZoneConfig.VoiceLineURL;
}

void AAudioZoneManager::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC) return;

    bPlayerInZone = true;

    // Danger zones fade in urgently
    if (ZoneConfig.ZoneType == EAudio_ZoneType::Danger || ZoneConfig.ZoneType == EAudio_ZoneType::Combat)
    {
        if (AudioComponent && AudioComponent->Sound)
        {
            AudioComponent->FadeIn(0.5f, ZoneConfig.VolumeMultiplier);
        }
    }
    else
    {
        ActivateZone();
    }
}

void AAudioZoneManager::OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC) return;

    bPlayerInZone = false;

    // Soft zones fade out gently; danger zones cut faster
    float FadeTime = (ZoneConfig.ZoneType == EAudio_ZoneType::Danger) ? 1.0f : 3.0f;
    DeactivateZone();
}
