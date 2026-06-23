#include "AudioZoneManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
// UAudio_ZoneComponent
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for audio blending
}

void UAudio_ZoneComponent::BeginPlay()
{
    Super::BeginPlay();

    ZoneState.ZoneName = ZoneConfig.ZoneName;
    ZoneState.CurrentVolume = 0.0f;
    ZoneState.bPlayerInside = false;

    // Cache player pawn reference
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        CachedPlayerPawn = PC->GetPawn();
    }

    // Register with subsystem
    if (UWorld* World = GetWorld())
    {
        if (UAudio_ZoneManagerSubsystem* Mgr = World->GetSubsystem<UAudio_ZoneManagerSubsystem>())
        {
            Mgr->RegisterZone(this);
        }
    }
}

void UAudio_ZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Refresh player pawn if lost
    if (!CachedPlayerPawn)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            CachedPlayerPawn = PC->GetPawn();
        }
    }

    if (!CachedPlayerPawn || !GetOwner())
    {
        return;
    }

    // Compute distance from zone origin to player
    FVector ZoneOrigin = GetOwner()->GetActorLocation();
    FVector PlayerLoc = CachedPlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(ZoneOrigin, PlayerLoc);

    ZoneState.PlayerDistance = Distance;
    ZoneState.bPlayerInside = (Distance <= ZoneConfig.OuterRadius);

    // Compute target volume
    TargetVolume = ComputeVolumeForDistance(Distance) * ZoneConfig.MasterVolume;

    // Interpolate current volume toward target
    float InterpDelta = VolumeInterpSpeed * DeltaTime;
    ZoneState.CurrentVolume = FMath::FInterpTo(ZoneState.CurrentVolume, TargetVolume, DeltaTime, VolumeInterpSpeed);

    // Apply volume to audio components
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->SetVolumeMultiplier(ZoneState.CurrentVolume);
        if (ZoneState.CurrentVolume > 0.01f && !PrimaryAudioComponent->IsPlaying())
        {
            PrimaryAudioComponent->Play();
        }
        else if (ZoneState.CurrentVolume <= 0.01f && PrimaryAudioComponent->IsPlaying())
        {
            PrimaryAudioComponent->Stop();
        }
    }

    if (SecondaryAudioComponent)
    {
        // Secondary plays at half volume of primary
        SecondaryAudioComponent->SetVolumeMultiplier(ZoneState.CurrentVolume * 0.5f);
        if (ZoneState.CurrentVolume > 0.01f && !SecondaryAudioComponent->IsPlaying())
        {
            SecondaryAudioComponent->Play();
        }
        else if (ZoneState.CurrentVolume <= 0.01f && SecondaryAudioComponent->IsPlaying())
        {
            SecondaryAudioComponent->Stop();
        }
    }
}

float UAudio_ZoneComponent::ComputeVolumeForDistance(float Distance) const
{
    if (Distance <= ZoneConfig.InnerRadius)
    {
        return 1.0f;
    }
    if (Distance >= ZoneConfig.OuterRadius)
    {
        return 0.0f;
    }

    // Linear falloff between inner and outer radius
    float Range = ZoneConfig.OuterRadius - ZoneConfig.InnerRadius;
    float Alpha = (Distance - ZoneConfig.InnerRadius) / Range;
    return FMath::Clamp(1.0f - Alpha, 0.0f, 1.0f);
}

void UAudio_ZoneComponent::ForceVolume(float Volume)
{
    ZoneState.CurrentVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    TargetVolume = ZoneState.CurrentVolume;

    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->SetVolumeMultiplier(ZoneState.CurrentVolume);
    }
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->SetVolumeMultiplier(ZoneState.CurrentVolume * 0.5f);
    }
}

// ============================================================
// AAudio_ZoneActor
// ============================================================

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create audio zone component
    AudioZoneComponent = CreateDefaultSubobject<UAudio_ZoneComponent>(TEXT("AudioZoneComponent"));
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootScene")));

    // Create audio components
    PrimarySound = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimarySound"));
    PrimarySound->SetupAttachment(GetRootComponent());
    PrimarySound->bAutoActivate = false;

    SecondarySound = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondarySound"));
    SecondarySound->SetupAttachment(GetRootComponent());
    SecondarySound->bAutoActivate = false;
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Wire audio components into the zone component
    if (AudioZoneComponent)
    {
        AudioZoneComponent->PrimaryAudioComponent = PrimarySound;
        AudioZoneComponent->SecondaryAudioComponent = SecondarySound;
    }
}

void AAudio_ZoneActor::SetZoneType(EAudio_ZoneType NewType)
{
    if (AudioZoneComponent)
    {
        AudioZoneComponent->ZoneConfig.ZoneType = NewType;

        // Apply zone-type-specific defaults
        switch (NewType)
        {
        case EAudio_ZoneType::TRexTerritory:
            AudioZoneComponent->ZoneConfig.ZoneName = TEXT("TRex Territory");
            AudioZoneComponent->ZoneConfig.InnerRadius = 1200.0f;
            AudioZoneComponent->ZoneConfig.OuterRadius = 3500.0f;
            AudioZoneComponent->ZoneConfig.bDominant = true;
            break;

        case EAudio_ZoneType::RiverCrossing:
            AudioZoneComponent->ZoneConfig.ZoneName = TEXT("River Crossing");
            AudioZoneComponent->ZoneConfig.InnerRadius = 600.0f;
            AudioZoneComponent->ZoneConfig.OuterRadius = 2000.0f;
            AudioZoneComponent->ZoneConfig.bDominant = false;
            break;

        case EAudio_ZoneType::TribeCamp:
            AudioZoneComponent->ZoneConfig.ZoneName = TEXT("Tribe Camp");
            AudioZoneComponent->ZoneConfig.InnerRadius = 500.0f;
            AudioZoneComponent->ZoneConfig.OuterRadius = 1800.0f;
            AudioZoneComponent->ZoneConfig.bDominant = false;
            break;

        case EAudio_ZoneType::FlintDeposit:
            AudioZoneComponent->ZoneConfig.ZoneName = TEXT("Flint Deposit");
            AudioZoneComponent->ZoneConfig.InnerRadius = 300.0f;
            AudioZoneComponent->ZoneConfig.OuterRadius = 1000.0f;
            AudioZoneComponent->ZoneConfig.bDominant = false;
            break;

        default:
            break;
        }
    }
}

EAudio_ZoneType AAudio_ZoneActor::GetZoneType() const
{
    if (AudioZoneComponent)
    {
        return AudioZoneComponent->ZoneConfig.ZoneType;
    }
    return EAudio_ZoneType::Custom;
}

bool AAudio_ZoneActor::IsPlayerInside() const
{
    if (AudioZoneComponent)
    {
        return AudioZoneComponent->ZoneState.bPlayerInside;
    }
    return false;
}

// ============================================================
// UAudio_ZoneManagerSubsystem
// ============================================================

void UAudio_ZoneManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisteredZones.Empty();
    UE_LOG(LogTemp, Log, TEXT("[AudioZoneManager] Subsystem initialized."));
}

void UAudio_ZoneManagerSubsystem::Deinitialize()
{
    RegisteredZones.Empty();
    Super::Deinitialize();
}

void UAudio_ZoneManagerSubsystem::RegisterZone(UAudio_ZoneComponent* Zone)
{
    if (Zone && !RegisteredZones.Contains(Zone))
    {
        RegisteredZones.Add(Zone);
        UE_LOG(LogTemp, Log, TEXT("[AudioZoneManager] Registered zone: %s"), *Zone->ZoneConfig.ZoneName);
    }
}

void UAudio_ZoneManagerSubsystem::UnregisterZone(UAudio_ZoneComponent* Zone)
{
    RegisteredZones.Remove(Zone);
}

TArray<FAudio_ZoneState> UAudio_ZoneManagerSubsystem::GetAllZoneStates() const
{
    TArray<FAudio_ZoneState> States;
    for (const UAudio_ZoneComponent* Zone : RegisteredZones)
    {
        if (Zone)
        {
            States.Add(Zone->ZoneState);
        }
    }
    return States;
}

bool UAudio_ZoneManagerSubsystem::HasDominantZoneActive() const
{
    for (const UAudio_ZoneComponent* Zone : RegisteredZones)
    {
        if (Zone && Zone->ZoneConfig.bDominant && Zone->ZoneState.bPlayerInside)
        {
            return true;
        }
    }
    return false;
}
