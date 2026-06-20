#include "AudioZoneManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

// ---- UAudio_ZoneComponent ----

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetSphereRadius(500.0f);
}

void UAudio_ZoneComponent::ActivateZone()
{
    ZoneConfig.bIsActive = true;
}

void UAudio_ZoneComponent::DeactivateZone()
{
    ZoneConfig.bIsActive = false;
}

float UAudio_ZoneComponent::GetBlendWeight(const FVector& ListenerLocation) const
{
    if (!ZoneConfig.bIsActive)
    {
        return 0.0f;
    }

    const float Distance = FVector::Dist(GetComponentLocation(), ListenerLocation);
    const float Radius = ZoneConfig.BlendRadius;

    if (Distance >= Radius)
    {
        return 0.0f;
    }

    // Smooth blend using cosine curve
    const float NormalizedDist = Distance / Radius;
    return FMath::Clamp(0.5f * (1.0f + FMath::Cos(NormalizedDist * PI)), 0.0f, 1.0f);
}

// ---- AAudio_ZoneManager ----

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.25f;
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-discover zone components in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        UAudio_ZoneComponent* ZoneComp = Actor->FindComponentByClass<UAudio_ZoneComponent>();
        if (ZoneComp)
        {
            RegisterZone(ZoneComp);
        }
    }
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableDynamicMixing)
    {
        return;
    }

    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate < UpdateInterval)
    {
        return;
    }
    TimeSinceLastUpdate = 0.0f;

    // Get listener location from first local player
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        UpdateAudioMix(PC->GetPawn()->GetActorLocation());
    }
}

void AAudio_ZoneManager::RegisterZone(UAudio_ZoneComponent* Zone)
{
    if (Zone && !RegisteredZones.Contains(Zone))
    {
        RegisteredZones.Add(Zone);
    }
}

void AAudio_ZoneManager::UnregisterZone(UAudio_ZoneComponent* Zone)
{
    RegisteredZones.Remove(Zone);
}

EAudio_ZoneType AAudio_ZoneManager::GetDominantZoneForListener(const FVector& ListenerLocation) const
{
    EAudio_ZoneType DominantType = EAudio_ZoneType::OpenPlain;
    float MaxWeight = 0.0f;

    for (const UAudio_ZoneComponent* Zone : RegisteredZones)
    {
        if (!Zone)
        {
            continue;
        }

        const float Weight = Zone->GetBlendWeight(ListenerLocation);
        if (Weight > MaxWeight)
        {
            MaxWeight = Weight;
            DominantType = Zone->ZoneConfig.ZoneType;
        }
    }

    return DominantType;
}

float AAudio_ZoneManager::GetDangerLevel() const
{
    return CurrentDangerLevel;
}

void AAudio_ZoneManager::SetDangerLevel(float NewDangerLevel)
{
    CurrentDangerLevel = FMath::Clamp(NewDangerLevel, 0.0f, 1.0f);
}

void AAudio_ZoneManager::UpdateAudioMix(const FVector& ListenerLocation)
{
    if (RegisteredZones.Num() == 0)
    {
        return;
    }

    // Calculate blended danger from all active danger zones
    float TotalDanger = 0.0f;
    int32 DangerZoneCount = 0;

    for (const UAudio_ZoneComponent* Zone : RegisteredZones)
    {
        if (!Zone || !Zone->ZoneConfig.bIsActive)
        {
            continue;
        }

        if (Zone->ZoneConfig.ZoneType == EAudio_ZoneType::DangerZone)
        {
            const float Weight = Zone->GetBlendWeight(ListenerLocation);
            TotalDanger += Weight * Zone->ZoneConfig.DangerIntensity;
            ++DangerZoneCount;
        }
    }

    if (DangerZoneCount > 0)
    {
        CurrentDangerLevel = FMath::Clamp(TotalDanger / DangerZoneCount, 0.0f, 1.0f);
    }
    else
    {
        // Decay danger level when not in danger zone
        CurrentDangerLevel = FMath::FInterpTo(CurrentDangerLevel, 0.0f, UpdateInterval, 0.5f);
    }
}
