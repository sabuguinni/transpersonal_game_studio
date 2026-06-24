#include "AudioZoneSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"

// ============================================================
// UAudio_ZoneComponent
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UAudio_ZoneComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentVolume = 0.0f;
    TargetVolume = 0.0f;
}

void UAudio_ZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Smooth volume blend
    if (!FMath::IsNearlyEqual(CurrentVolume, TargetVolume, 0.001f))
    {
        CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, VolumeBlendSpeed);
    }
}

void UAudio_ZoneComponent::OnPlayerEnterZone()
{
    bPlayerInZone = true;
    TargetVolume = ZoneConfig.MaxVolume;
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered zone [%s] tone=%d"),
        *ZoneConfig.ZoneDescription, (int32)ZoneConfig.Tone);
}

void UAudio_ZoneComponent::OnPlayerExitZone()
{
    bPlayerInZone = false;
    TargetVolume = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited zone [%s]"),
        *ZoneConfig.ZoneDescription);
}

EAudio_ZoneTone UAudio_ZoneComponent::GetCurrentTone() const
{
    return ZoneConfig.Tone;
}

float UAudio_ZoneComponent::GetVolumeForDistance(float Distance) const
{
    if (Distance <= 0.0f) return ZoneConfig.MaxVolume;
    if (Distance >= ZoneConfig.BlendRadius) return 0.0f;
    // Linear falloff from center to edge
    float Alpha = 1.0f - (Distance / ZoneConfig.BlendRadius);
    return FMath::Clamp(Alpha * ZoneConfig.MaxVolume, 0.0f, ZoneConfig.MaxVolume);
}

// ============================================================
// AAudio_ZoneActor
// ============================================================

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
    ZoneSphere->SetSphereRadius(800.0f);
    ZoneSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = ZoneSphere;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    AudioZoneComponent = CreateDefaultSubobject<UAudio_ZoneComponent>(TEXT("AudioZoneComponent"));
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneActor::OnSphereBeginOverlap);
    ZoneSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneActor::OnSphereEndOverlap);

    // Sync zone config to component
    if (AudioZoneComponent)
    {
        AudioZoneComponent->ZoneConfig = ZoneConfig;
    }

    UE_LOG(LogTemp, Log, TEXT("AAudio_ZoneActor: BeginPlay — zone [%s] tone=%d radius=%.0f"),
        *ZoneConfig.ZoneDescription, (int32)ZoneConfig.Tone, ZoneConfig.BlendRadius);
}

void AAudio_ZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_ZoneActor::ActivateZone()
{
    bIsActive = true;
    if (AudioComponent && !AudioComponent->IsPlaying())
    {
        AudioComponent->Play();
    }
}

void AAudio_ZoneActor::DeactivateZone()
{
    bIsActive = false;
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
}

bool AAudio_ZoneActor::IsPlayerInRange() const
{
    return AudioZoneComponent ? AudioZoneComponent->bPlayerInZone : false;
}

void AAudio_ZoneActor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (!Char) return;

    APlayerController* PC = Cast<APlayerController>(Char->GetController());
    if (!PC) return;

    if (AudioZoneComponent)
    {
        AudioZoneComponent->OnPlayerEnterZone();
    }
    ActivateZone();
}

void AAudio_ZoneActor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (!Char) return;

    APlayerController* PC = Cast<APlayerController>(Char->GetController());
    if (!PC) return;

    if (AudioZoneComponent)
    {
        AudioZoneComponent->OnPlayerExitZone();
    }
    DeactivateZone();
}

// ============================================================
// AAudio_TRexProximityActor
// ============================================================

AAudio_TRexProximityActor::AAudio_TRexProximityActor()
{
    PrimaryActorTick.bCanEverTick = true;

    ProximitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphere"));
    ProximitySphere->SetSphereRadius(1200.0f);
    ProximitySphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = ProximitySphere;
}

void AAudio_TRexProximityActor::BeginPlay()
{
    Super::BeginPlay();

    ProximitySphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_TRexProximityActor::OnProximityBeginOverlap);
    ProximitySphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_TRexProximityActor::OnProximityEndOverlap);

    FootstepTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AAudio_TRexProximityActor: BeginPlay — shake radius=%.0f interval=%.1fs"),
        ShakeConfig.TriggerRadius, FootstepInterval);
}

void AAudio_TRexProximityActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bPlayerInRange) return;

    FootstepTimer += DeltaTime;
    if (FootstepTimer >= FootstepInterval)
    {
        FootstepTimer = 0.0f;
        TriggerFootstepShake();
    }
}

void AAudio_TRexProximityActor::TriggerFootstepShake()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Camera shake via console command — Blueprint-friendly fallback
    // In full implementation: PC->ClientStartCameraShake(ShakeClass, ShakeConfig.ShakeIntensity)
    UE_LOG(LogTemp, Log, TEXT("TRexProximity: FOOTSTEP SHAKE — intensity=%.2f duration=%.2f freq=%.1f"),
        ShakeConfig.ShakeIntensity, ShakeConfig.ShakeDuration, ShakeConfig.ShakeFrequency);

    // Apply shake via console command as fallback
    UGameplayStatics::PlayWorldCameraShake(
        World,
        nullptr, // ShakeClass — assigned in Blueprint
        GetActorLocation(),
        0.0f,
        ShakeConfig.TriggerRadius,
        ShakeConfig.ShakeIntensity
    );
}

void AAudio_TRexProximityActor::TriggerRoarShake()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    UE_LOG(LogTemp, Log, TEXT("TRexProximity: ROAR SHAKE — intensity=%.2f (x3 footstep)"),
        ShakeConfig.ShakeIntensity * 3.0f);

    UGameplayStatics::PlayWorldCameraShake(
        World,
        nullptr,
        GetActorLocation(),
        0.0f,
        ShakeConfig.TriggerRadius * 1.5f,
        ShakeConfig.ShakeIntensity * 3.0f
    );
}

void AAudio_TRexProximityActor::OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (!Char) return;

    APlayerController* PC = Cast<APlayerController>(Char->GetController());
    if (!PC) return;

    bPlayerInRange = true;
    FootstepTimer = 0.0f;
    UE_LOG(LogTemp, Warning, TEXT("TRexProximity: Player entered T-Rex zone — screen shake ACTIVE"));

    // Immediate roar shake on entry
    TriggerRoarShake();
}

void AAudio_TRexProximityActor::OnProximityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (!Char) return;

    APlayerController* PC = Cast<APlayerController>(Char->GetController());
    if (!PC) return;

    bPlayerInRange = false;
    FootstepTimer = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("TRexProximity: Player exited T-Rex zone — screen shake DEACTIVATED"));
}
