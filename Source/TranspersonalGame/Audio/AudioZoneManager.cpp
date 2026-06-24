// AudioZoneManager.cpp — Agent #16 Audio Agent
// Implementation of spatial audio zone manager for prehistoric survival game
#include "AudioZoneManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
    ZoneSphere->InitSphereRadius(1500.0f);
    ZoneSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = ZoneSphere;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->VolumeMultiplier = 0.0f;
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerEnterZone);
    ZoneSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneManager::OnPlayerExitZone);

    // Start ambient audio silently — volume fades in when player enters
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Play();
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
    }
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!AmbientAudioComponent) return;

    // Smooth volume blend based on player presence and danger level
    float TargetVolume = bPlayerInZone ? ZoneConfig.AmbientVolume : 0.0f;

    // Danger level modifies music intensity
    float DangerMultiplier = 1.0f;
    switch (CurrentDangerLevel)
    {
        case EAudio_DangerLevel::Caution:   DangerMultiplier = 1.2f; break;
        case EAudio_DangerLevel::Danger:    DangerMultiplier = 1.5f; break;
        case EAudio_DangerLevel::Critical:  DangerMultiplier = 2.0f; break;
        default: break;
    }

    float CurrentVolume = AmbientAudioComponent->VolumeMultiplier;
    float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume * DangerMultiplier, DeltaTime, 2.0f);
    AmbientAudioComponent->SetVolumeMultiplier(NewVolume);

    // Blend danger alpha for pitch shifting effect
    float TargetDangerAlpha = (CurrentDangerLevel != EAudio_DangerLevel::Safe) ? 1.0f : 0.0f;
    DangerBlendAlpha = FMath::FInterpTo(DangerBlendAlpha, TargetDangerAlpha, DeltaTime, 1.5f);

    // Pitch rises slightly with danger (tension effect)
    float PitchMultiplier = FMath::Lerp(1.0f, 1.15f, DangerBlendAlpha);
    AmbientAudioComponent->SetPitchMultiplier(PitchMultiplier);
}

void AAudio_ZoneManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    CurrentDangerLevel = NewLevel;
}

void AAudio_ZoneManager::SetBiomeType(EAudio_BiomeType NewBiome)
{
    ZoneConfig.BiomeType = NewBiome;
}

float AAudio_ZoneManager::GetCurrentAmbientVolume() const
{
    if (AmbientAudioComponent)
    {
        return AmbientAudioComponent->VolumeMultiplier;
    }
    return 0.0f;
}

bool AAudio_ZoneManager::IsPlayerInZone() const
{
    return bPlayerInZone;
}

void AAudio_ZoneManager::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsPlayerControlled())
    {
        bPlayerInZone = true;
        UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered %s zone"),
            *UEnum::GetValueAsString(ZoneConfig.BiomeType));
    }
}

void AAudio_ZoneManager::OnPlayerExitZone(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsPlayerControlled())
    {
        bPlayerInZone = false;
        UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited %s zone"),
            *UEnum::GetValueAsString(ZoneConfig.BiomeType));
    }
}
