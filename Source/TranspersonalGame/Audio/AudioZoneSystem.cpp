#include "AudioZoneSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Engine/World.h"

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(600.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
    AudioComponent->VolumeMultiplier = 0.0f;
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Apply config radius to sphere
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(ZoneConfig.TriggerRadius);
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneActor::HandleBeginOverlap);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneActor::HandleEndOverlap);
    }

    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor [%s] BeginPlay — ZoneType=%d Radius=%.0f"),
           *GetActorLabel(), (int32)ZoneConfig.ZoneType, ZoneConfig.TriggerRadius);
}

void AAudio_ZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(FadeInHandle);
    GetWorldTimerManager().ClearTimer(FadeOutHandle);
    GetWorldTimerManager().ClearTimer(CooldownHandle);

    Super::EndPlay(EndPlayReason);
}

void AAudio_ZoneActor::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                           bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;
    if (bOnCooldown) return;

    bPlayerInside = true;
    FadeInAudio();
    OnPlayerEnterZone(ZoneConfig.ZoneType);

    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor [%s] Player ENTERED — ZoneType=%d"),
           *GetActorLabel(), (int32)ZoneConfig.ZoneType);
}

void AAudio_ZoneActor::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    bPlayerInside = false;
    FadeOutAudio();
    OnPlayerExitZone(ZoneConfig.ZoneType);

    if (ZoneConfig.bOneShot)
    {
        bOnCooldown = true;
        GetWorldTimerManager().SetTimer(CooldownHandle, this,
            &AAudio_ZoneActor::ResetCooldown, ZoneConfig.CooldownSeconds, false);
    }

    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor [%s] Player EXITED — ZoneType=%d"),
           *GetActorLabel(), (int32)ZoneConfig.ZoneType);
}

void AAudio_ZoneActor::FadeInAudio()
{
    if (!AudioComponent) return;

    AudioComponent->Play();

    // Ramp volume from 0 to MaxVolume over FadeInDuration using timer ticks
    const float TickRate = 0.05f;
    const float Steps = ZoneConfig.FadeInDuration / TickRate;
    const float VolumeStep = ZoneConfig.MaxVolume / FMath::Max(Steps, 1.0f);

    // Simple immediate set for now — Blueprint can override with curve
    AudioComponent->SetVolumeMultiplier(ZoneConfig.MaxVolume);

    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor [%s] FadeIn — TargetVolume=%.2f Duration=%.1fs"),
           *GetActorLabel(), ZoneConfig.MaxVolume, ZoneConfig.FadeInDuration);
}

void AAudio_ZoneActor::FadeOutAudio()
{
    if (!AudioComponent) return;

    AudioComponent->FadeOut(ZoneConfig.FadeOutDuration, 0.0f);

    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor [%s] FadeOut — Duration=%.1fs"),
           *GetActorLabel(), ZoneConfig.FadeOutDuration);
}

void AAudio_ZoneActor::ResetCooldown()
{
    bOnCooldown = false;
    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor [%s] Cooldown reset — zone active again"),
           *GetActorLabel());
}

void AAudio_ZoneActor::SetZoneType(EAudio_ZoneType NewType)
{
    ZoneConfig.ZoneType = NewType;
    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor [%s] ZoneType set to %d"),
           *GetActorLabel(), (int32)NewType);
}

void AAudio_ZoneActor::OnPlayerEnterZone_Implementation(EAudio_ZoneType ZoneType)
{
    // Blueprint override hook — log zone entry with type for debugging
    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor OnPlayerEnterZone — ZoneType=%d (Blueprint can override)"),
           (int32)ZoneType);
}

void AAudio_ZoneActor::OnPlayerExitZone_Implementation(EAudio_ZoneType ZoneType)
{
    // Blueprint override hook — log zone exit
    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor OnPlayerExitZone — ZoneType=%d (Blueprint can override)"),
           (int32)ZoneType);
}
