#include "AudioZoneManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
    AudioComponent->bIsUISound = false;

    bPlayerInZone = false;
    CurrentVolume = 0.0f;
    TargetVolume = 0.0f;
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Apply zone config to sphere radius
    TriggerSphere->SetSphereRadius(ZoneConfig.TriggerRadius);
    AudioComponent->SetVolumeMultiplier(0.0f);

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Zone '%s' initialized (Type=%d, Radius=%.0f)"),
        *GetActorLabel(), (int32)ZoneConfig.ZoneType, ZoneConfig.TriggerRadius);
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateVolumeFade(DeltaTime);
}

void AAudio_ZoneManager::OnPlayerEnterZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    bPlayerInZone = true;
    TargetVolume = ZoneConfig.MaxVolume;

    if (AudioComponent && !AudioComponent->IsPlaying())
    {
        AudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Player entered zone '%s' — audio starting"),
            *GetActorLabel());
    }
}

void AAudio_ZoneManager::OnPlayerExitZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    bPlayerInZone = false;
    TargetVolume = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Player exited zone '%s' — audio fading out"),
        *GetActorLabel());
}

void AAudio_ZoneManager::SetZoneType(EAudio_ZoneType NewType)
{
    ZoneConfig.ZoneType = NewType;
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Zone '%s' type changed to %d"),
        *GetActorLabel(), (int32)NewType);
}

float AAudio_ZoneManager::GetDistanceToPlayer() const
{
    UWorld* World = GetWorld();
    if (!World) return -1.0f;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return -1.0f;

    return FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
}

void AAudio_ZoneManager::UpdateVolumeFade(float DeltaTime)
{
    if (FMath::IsNearlyEqual(CurrentVolume, TargetVolume, 0.01f))
    {
        CurrentVolume = TargetVolume;
    }
    else
    {
        float FadeSpeed = (TargetVolume > CurrentVolume)
            ? (ZoneConfig.MaxVolume / FMath::Max(ZoneConfig.FadeInTime, 0.1f))
            : (ZoneConfig.MaxVolume / FMath::Max(ZoneConfig.FadeOutTime, 0.1f));

        CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);
    }

    if (AudioComponent)
    {
        AudioComponent->SetVolumeMultiplier(CurrentVolume);

        // Stop audio when fully faded out and not in zone
        if (!bPlayerInZone && CurrentVolume <= 0.01f && AudioComponent->IsPlaying())
        {
            AudioComponent->Stop();
        }
    }
}
