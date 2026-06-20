#include "AudioZoneSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ============================================================
// UAudio_ZoneComponent
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float UAudio_ZoneComponent::GetBlendWeightForPlayer(const FVector& PlayerLocation) const
{
    AActor* Owner = GetOwner();
    if (!Owner) return 0.0f;

    const float Distance = FVector::Dist(PlayerLocation, Owner->GetActorLocation());
    const float Radius = ZoneConfig.BlendRadius;

    if (Distance >= Radius) return 0.0f;
    if (Distance <= 0.0f)   return 1.0f;

    // Smooth falloff: cosine curve for natural audio blend
    const float Alpha = Distance / Radius;
    return FMath::Clamp(FMath::Cos(Alpha * PI * 0.5f), 0.0f, 1.0f);
}

// ============================================================
// AAudio_ZoneActor
// ============================================================

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = false;

    AudioZoneComponent = CreateDefaultSubobject<UAudio_ZoneComponent>(TEXT("AudioZoneComponent"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Sync config to component on play
    if (AudioZoneComponent)
    {
        AudioZoneComponent->ZoneConfig = ZoneConfig;
    }

    // Auto-register with ZoneManager
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAudio_ZoneManager* Mgr = GI->GetSubsystem<UAudio_ZoneManager>())
        {
            Mgr->RegisterZone(this);
        }
    }
}

void AAudio_ZoneActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (AudioZoneComponent)
    {
        AudioZoneComponent->ZoneConfig = ZoneConfig;
    }
}

void AAudio_ZoneActor::PreviewZoneConfig()
{
#if WITH_EDITOR
    UE_LOG(LogTemp, Log, TEXT("[AudioZone] PreviewZoneConfig — Zone: %s | Radius: %.0f | DangerMusic: %s"),
        *GetActorLabel(),
        ZoneConfig.BlendRadius,
        ZoneConfig.bDangerMusic ? TEXT("YES") : TEXT("NO"));
#endif
}

// ============================================================
// UAudio_ZoneManager
// ============================================================

void UAudio_ZoneManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisteredZones.Empty();
    UE_LOG(LogTemp, Log, TEXT("[AudioZoneManager] Initialized"));
}

void UAudio_ZoneManager::Deinitialize()
{
    RegisteredZones.Empty();
    Super::Deinitialize();
}

void UAudio_ZoneManager::RegisterZone(AAudio_ZoneActor* Zone)
{
    if (Zone && !RegisteredZones.Contains(Zone))
    {
        RegisteredZones.Add(Zone);
        UE_LOG(LogTemp, Log, TEXT("[AudioZoneManager] Registered zone: %s"), *Zone->GetActorLabel());
    }
}

void UAudio_ZoneManager::UnregisterZone(AAudio_ZoneActor* Zone)
{
    RegisteredZones.Remove(Zone);
}

EAudio_ZoneType UAudio_ZoneManager::GetDominantZoneForPlayer(const FVector& PlayerLocation) const
{
    EAudio_ZoneType Dominant = EAudio_ZoneType::OpenPlain;
    float BestWeight = 0.0f;

    for (AAudio_ZoneActor* Zone : RegisteredZones)
    {
        if (!Zone || !Zone->AudioZoneComponent) continue;

        const float Weight = Zone->AudioZoneComponent->GetBlendWeightForPlayer(PlayerLocation);
        if (Weight > BestWeight)
        {
            BestWeight = Weight;
            Dominant = Zone->ZoneConfig.ZoneType;
        }
    }

    return Dominant;
}

float UAudio_ZoneManager::GetDangerLevel(const FVector& PlayerLocation) const
{
    float TotalDanger = 0.0f;

    for (AAudio_ZoneActor* Zone : RegisteredZones)
    {
        if (!Zone || !Zone->AudioZoneComponent) continue;
        if (!Zone->ZoneConfig.bDangerMusic) continue;

        const float Weight = Zone->AudioZoneComponent->GetBlendWeightForPlayer(PlayerLocation);
        TotalDanger = FMath::Max(TotalDanger, Weight * Zone->ZoneConfig.MusicIntensity);
    }

    return FMath::Clamp(TotalDanger, 0.0f, 1.0f);
}
