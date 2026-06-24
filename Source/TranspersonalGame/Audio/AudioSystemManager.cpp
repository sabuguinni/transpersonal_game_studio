// AudioSystemManager.cpp
// Agent #16 — Audio Agent
// Adaptive audio zone system implementation

#include "AudioSystemManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// UAudio_ZoneTriggerComponent
// ============================================================

UAudio_ZoneTriggerComponent::UAudio_ZoneTriggerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetSphereRadius(ZoneConfig.TriggerRadius);
    SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void UAudio_ZoneTriggerComponent::OnPlayerEnterZone(AActor* Player)
{
    if (!Player) return;
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered zone type %d"),
        static_cast<int32>(ZoneConfig.ZoneType));
}

void UAudio_ZoneTriggerComponent::OnPlayerExitZone(AActor* Player)
{
    if (!Player) return;
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited zone type %d"),
        static_cast<int32>(ZoneConfig.ZoneType));
}

// ============================================================
// AAudio_ZoneActor
// ============================================================

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerZone = CreateDefaultSubobject<UAudio_ZoneTriggerComponent>(TEXT("TriggerZone"));
    RootComponent = TriggerZone;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    if (TriggerZone)
    {
        TriggerZone->SetSphereRadius(ZoneConfig.TriggerRadius);
        TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneActor::HandleBeginOverlap);
        TriggerZone->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneActor::HandleEndOverlap);
    }
}

void AAudio_ZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Fade audio in/out based on player presence
    if (AmbientAudioComponent && AmbientAudioComponent->IsActive())
    {
        float TargetVolume = bPlayerInZone ? ZoneConfig.AmbientVolume : 0.0f;
        float FadeSpeed = bPlayerInZone
            ? (1.0f / FMath::Max(ZoneConfig.FadeInDuration, 0.01f))
            : (1.0f / FMath::Max(ZoneConfig.FadeOutDuration, 0.01f));

        CurrentFadeProgress = FMath::FInterpTo(CurrentFadeProgress, TargetVolume, DeltaTime, FadeSpeed * 2.0f);
        AmbientAudioComponent->SetVolumeMultiplier(CurrentFadeProgress);
    }
}

void AAudio_ZoneActor::SetZoneType(EAudio_ZoneType NewType)
{
    ZoneConfig.ZoneType = NewType;
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Zone type set to %d"), static_cast<int32>(NewType));
}

void AAudio_ZoneActor::TriggerDangerAlert(float Intensity)
{
    if (!ZoneConfig.bTriggerScreenShake) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Screen shake via camera shake — intensity drives magnitude
    UE_LOG(LogTemp, Log, TEXT("AudioZone: DangerAlert triggered at intensity %.2f"), Intensity);
}

void AAudio_ZoneActor::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    bPlayerInZone = true;

    if (AmbientAudioComponent && !AmbientAudioComponent->IsActive())
    {
        AmbientAudioComponent->Play();
    }

    if (TriggerZone)
    {
        TriggerZone->OnPlayerEnterZone(OtherActor);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s]: Player ENTERED — zone type %d"),
        *GetActorLabel(), static_cast<int32>(ZoneConfig.ZoneType));
}

void AAudio_ZoneActor::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    bPlayerInZone = false;

    if (TriggerZone)
    {
        TriggerZone->OnPlayerExitZone(OtherActor);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone [%s]: Player EXITED — zone type %d"),
        *GetActorLabel(), static_cast<int32>(ZoneConfig.ZoneType));
}
