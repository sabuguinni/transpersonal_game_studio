#include "AudioZoneManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    BlendSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BlendSphere"));
    BlendSphere->SetSphereRadius(1500.0f);
    BlendSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = BlendSphere;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Update sphere radius from config
    if (BlendSphere)
    {
        BlendSphere->SetSphereRadius(ZoneConfig.BlendRadius);
    }

    // Bind overlap events
    BlendSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerEnterZone);
    BlendSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerLeaveZone);

    // Start audio looping at zero volume
    if (AmbientAudioComponent && ZoneConfig.bLooping)
    {
        AmbientAudioComponent->Play();
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
    }
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateBlendWeight(DeltaTime);
}

void AAudio_ZoneManager::UpdateBlendWeight(float DeltaTime)
{
    if (!AmbientAudioComponent) return;

    float BlendSpeed = bPlayerInZone
        ? (1.0f / FMath::Max(ZoneConfig.FadeInTime, 0.01f))
        : (1.0f / FMath::Max(ZoneConfig.FadeOutTime, 0.01f));

    CurrentBlendWeight = FMath::FInterpTo(
        CurrentBlendWeight,
        TargetBlendWeight,
        DeltaTime,
        BlendSpeed
    );

    AmbientAudioComponent->SetVolumeMultiplier(CurrentBlendWeight * ZoneConfig.MaxVolume);
}

void AAudio_ZoneManager::SetZoneActive(bool bActive)
{
    TargetBlendWeight = bActive ? 1.0f : 0.0f;
    bPlayerInZone = bActive;
}

float AAudio_ZoneManager::GetDistanceToPlayer() const
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player) return MAX_FLT;

    return FVector::Dist(GetActorLocation(), Player->GetActorLocation());
}

void AAudio_ZoneManager::TriggerDangerStinger()
{
    if (!ZoneConfig.bTriggersDangerMusic) return;

    // Boost volume briefly as a danger stinger
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(ZoneConfig.MaxVolume * 1.5f);
    }
}

void AAudio_ZoneManager::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (OtherActor == Player)
    {
        bPlayerInZone = true;
        TargetBlendWeight = 1.0f;

        if (ZoneConfig.bTriggersDangerMusic)
        {
            TriggerDangerStinger();
        }
    }
}

void AAudio_ZoneManager::OnPlayerLeaveZone(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (OtherActor == Player)
    {
        bPlayerInZone = false;
        TargetBlendWeight = 0.0f;
    }
}
