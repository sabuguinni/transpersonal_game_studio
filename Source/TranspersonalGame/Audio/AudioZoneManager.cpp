#include "AudioZoneManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;

    ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
    ZoneSphere->SetSphereRadius(1000.0f);
    ZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ZoneSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    ZoneSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    RootComponent = ZoneSphere;

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.0f);
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    if (ZoneConfig.bLooping && AmbientAudioComponent)
    {
        AmbientAudioComponent->Play();
        AmbientAudioComponent->SetVolumeMultiplier(0.0f);
    }
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateVolumeByDistance(DeltaTime);
}

void AAudio_ZoneManager::SetZoneVolume(float NewVolume)
{
    CurrentVolume = FMath::Clamp(NewVolume, 0.0f, ZoneConfig.MaxVolume);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);
    }
}

void AAudio_ZoneManager::FadeInAudio()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->FadeIn(ZoneConfig.FadeInTime, ZoneConfig.MaxVolume);
    }
}

void AAudio_ZoneManager::FadeOutAudio()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->FadeOut(ZoneConfig.FadeOutTime, 0.0f);
    }
}

float AAudio_ZoneManager::GetDistanceToPlayer() const
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return TNumericLimits<float>::Max();

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return TNumericLimits<float>::Max();

    return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
}

bool AAudio_ZoneManager::IsPlayerInZone() const
{
    float Dist = GetDistanceToPlayer();
    return Dist <= ZoneSphere->GetScaledSphereRadius();
}

void AAudio_ZoneManager::UpdateVolumeByDistance(float DeltaTime)
{
    float Dist = GetDistanceToPlayer();
    float Radius = ZoneSphere->GetScaledSphereRadius();
    float BlendStart = Radius - ZoneConfig.BlendRadius;

    float TargetVolume = 0.0f;

    if (Dist <= BlendStart)
    {
        TargetVolume = ZoneConfig.MaxVolume;
        if (!bPlayerInZone)
        {
            bPlayerInZone = true;
        }
    }
    else if (Dist <= Radius)
    {
        float Alpha = 1.0f - ((Dist - BlendStart) / ZoneConfig.BlendRadius);
        TargetVolume = Alpha * ZoneConfig.MaxVolume;
        bPlayerInZone = true;
    }
    else
    {
        TargetVolume = 0.0f;
        if (bPlayerInZone)
        {
            bPlayerInZone = false;
        }
    }

    float FadeSpeed = (TargetVolume > CurrentVolume)
        ? (1.0f / FMath::Max(ZoneConfig.FadeInTime, 0.01f))
        : (1.0f / FMath::Max(ZoneConfig.FadeOutTime, 0.01f));

    CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);

    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(CurrentVolume);
    }
}
