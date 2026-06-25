#include "AudioZoneManager.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// ─────────────────────────────────────────────
// UAudio_ZoneManager
// ─────────────────────────────────────────────

UAudio_ZoneManager::UAudio_ZoneManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentBlendAlpha = 0.0f;
    TargetVolume = 0.0f;
    BlendSpeed = 2.0f;
}

void UAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Attempt to find an AudioComponent on the owner
    if (AActor* Owner = GetOwner())
    {
        AudioComp = Owner->FindComponentByClass<UAudioComponent>();
    }
}

void UAudio_ZoneManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Smooth blend toward target volume
    CurrentBlendAlpha = FMath::FInterpTo(CurrentBlendAlpha, TargetVolume, DeltaTime, BlendSpeed);

    if (AudioComp)
    {
        AudioComp->SetVolumeMultiplier(CurrentBlendAlpha * ZoneConfig.MaxVolume);
    }
}

void UAudio_ZoneManager::SetZoneVolume(float NewVolume)
{
    TargetVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void UAudio_ZoneManager::OnPlayerEnterZone(float DistanceToCenter)
{
    if (ZoneConfig.BlendRadius <= 0.0f)
    {
        SetZoneVolume(1.0f);
        return;
    }

    // Closer to center = louder
    float Alpha = 1.0f - FMath::Clamp(DistanceToCenter / ZoneConfig.BlendRadius, 0.0f, 1.0f);
    SetZoneVolume(Alpha);
}

// ─────────────────────────────────────────────
// AAudio_ZoneActor
// ─────────────────────────────────────────────

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->InitSphereRadius(800.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    ZoneManager = CreateDefaultSubobject<UAudio_ZoneManager>(TEXT("ZoneManager"));
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Sync config to manager
    ZoneManager->ZoneConfig = ZoneConfig;
    TriggerSphere->SetSphereRadius(ZoneConfig.BlendRadius);

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneActor::OnSphereBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneActor::OnSphereEndOverlap);
}

void AAudio_ZoneActor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // Only respond to player character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    float Distance = FVector::Dist(OtherActor->GetActorLocation(), GetActorLocation());
    ZoneManager->OnPlayerEnterZone(Distance);
}

void AAudio_ZoneActor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    // Fade out when player leaves zone
    ZoneManager->SetZoneVolume(0.0f);
}
