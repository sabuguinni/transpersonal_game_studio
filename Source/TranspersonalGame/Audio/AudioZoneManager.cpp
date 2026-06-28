#include "AudioZoneManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Trigger sphere for zone detection
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);

    // Music layer audio component
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;
    MusicAudioComponent->SetVolumeMultiplier(0.0f);
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerEnterZone);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerExitZone);

    // Set sphere radius from config
    TriggerSphere->SetSphereRadius(ZoneConfig.BlendRadius);

    // Assign sounds if set
    if (AmbientSound && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetSound(AmbientSound);
    }
    if (MusicLayer && MusicAudioComponent)
    {
        MusicAudioComponent->SetSound(MusicLayer);
    }
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smooth blend toward target intensity
    if (FMath::Abs(CurrentIntensity - TargetIntensity) > 0.001f)
    {
        float BlendSpeed = bPlayerInZone
            ? (1.0f / FMath::Max(ZoneConfig.FadeInTime, 0.1f))
            : (1.0f / FMath::Max(ZoneConfig.FadeOutTime, 0.1f));

        CurrentIntensity = FMath::FInterpTo(CurrentIntensity, TargetIntensity, DeltaTime, BlendSpeed);

        // Apply volume to audio components
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetVolumeMultiplier(CurrentIntensity * ZoneConfig.AmbientVolume);
        }
        if (MusicAudioComponent)
        {
            MusicAudioComponent->SetVolumeMultiplier(CurrentIntensity * ZoneConfig.MusicIntensity);
        }
    }
}

void AAudio_ZoneManager::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Check if the overlapping actor is the player character
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character) return;

    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (!PC) return;

    bPlayerInZone = true;
    TargetIntensity = 1.0f;

    // Start audio playback if not already playing
    if (AmbientAudioComponent && !AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }
    if (MusicAudioComponent && ZoneConfig.MusicIntensity > 0.0f && !MusicAudioComponent->IsPlaying())
    {
        MusicAudioComponent->Play();
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered zone type %d"), (int32)ZoneConfig.ZoneType);
}

void AAudio_ZoneManager::OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character) return;

    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (!PC) return;

    bPlayerInZone = false;
    TargetIntensity = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited zone type %d"), (int32)ZoneConfig.ZoneType);
}

void AAudio_ZoneManager::SetZoneActive(bool bActive)
{
    if (bActive)
    {
        TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    else
    {
        TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        bPlayerInZone = false;
        TargetIntensity = 0.0f;
    }
}
