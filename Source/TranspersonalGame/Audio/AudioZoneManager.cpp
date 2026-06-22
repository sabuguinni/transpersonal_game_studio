// AudioZoneManager.cpp
// Agent #16 — Audio Agent
// Implementation of adaptive audio zone manager

#include "AudioZoneManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

// ============================================================
// UAudio_ZoneComponent
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetSphereRadius(1000.0f);
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UAudio_ZoneComponent::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    CurrentThreatLevel = NewLevel;
}

float UAudio_ZoneComponent::GetBlendWeight(const FVector& PlayerLocation) const
{
    const float Distance = FVector::Dist(PlayerLocation, GetComponentLocation());
    const float Radius = ZoneConfig.BlendRadius;
    if (Distance >= Radius) return 0.0f;
    // Smooth blend: 1.0 at centre, 0.0 at edge
    const float Alpha = 1.0f - (Distance / Radius);
    return FMath::SmoothStep(0.0f, 1.0f, Alpha);
}

// ============================================================
// AAudio_ZoneManager
// ============================================================

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 500ms — not every frame
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();

    // Discover all zone components in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        UAudio_ZoneComponent* ZoneComp = Actor->FindComponentByClass<UAudio_ZoneComponent>();
        if (ZoneComp)
        {
            RegisteredZones.Add(ZoneComp);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Registered %d audio zones"), RegisteredZones.Num());
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AudioUpdateTimer += DeltaTime;
    if (AudioUpdateTimer < AudioUpdateInterval) return;
    AudioUpdateTimer = 0.0f;

    // Cache player location
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        CachedPlayerLocation = PC->GetPawn()->GetActorLocation();
    }

    UpdateActiveZone();
    EvaluateDayNightTransition();
}

void AAudio_ZoneManager::UpdateActiveZone()
{
    if (RegisteredZones.Num() == 0) return;

    float MaxWeight = 0.0f;
    EAudio_ZoneType DominantZone = EAudio_ZoneType::Forest;

    for (UAudio_ZoneComponent* Zone : RegisteredZones)
    {
        if (!Zone) continue;
        const float Weight = Zone->GetBlendWeight(CachedPlayerLocation);
        if (Weight > MaxWeight)
        {
            MaxWeight = Weight;
            DominantZone = Zone->GetZoneType();
        }
    }

    if (DominantZone != ActiveZoneType)
    {
        ActiveZoneType = DominantZone;
        UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Zone transition -> %d"), (int32)ActiveZoneType);
    }
}

void AAudio_ZoneManager::EvaluateDayNightTransition()
{
    const bool bShouldBeNight = (TimeOfDay < 6.0f || TimeOfDay > 20.0f);
    if (bShouldBeNight != bIsNight)
    {
        bIsNight = bShouldBeNight;
        UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Day/Night transition -> %s"),
            bIsNight ? TEXT("NIGHT") : TEXT("DAY"));
    }
}

void AAudio_ZoneManager::UpdateThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (NewLevel != GlobalThreatLevel)
    {
        GlobalThreatLevel = NewLevel;
        UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Threat level -> %d"), (int32)GlobalThreatLevel);
    }
}

void AAudio_ZoneManager::UpdateTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
}

void AAudio_ZoneManager::RegisterZone(UAudio_ZoneComponent* Zone)
{
    if (Zone && !RegisteredZones.Contains(Zone))
    {
        RegisteredZones.Add(Zone);
    }
}

EAudio_ZoneType AAudio_ZoneManager::GetDominantZoneAtLocation(const FVector& Location) const
{
    float MaxWeight = 0.0f;
    EAudio_ZoneType Result = EAudio_ZoneType::Forest;

    for (UAudio_ZoneComponent* Zone : RegisteredZones)
    {
        if (!Zone) continue;
        const float Weight = Zone->GetBlendWeight(Location);
        if (Weight > MaxWeight)
        {
            MaxWeight = Weight;
            Result = Zone->GetZoneType();
        }
    }
    return Result;
}

void AAudio_ZoneManager::OnDinosaurProximityEnter(float DinosaurDangerRating)
{
    // Escalate threat based on dinosaur danger rating (0-1 scale)
    if (DinosaurDangerRating >= 0.8f)
    {
        UpdateThreatLevel(EAudio_ThreatLevel::Extreme);
    }
    else if (DinosaurDangerRating >= 0.5f)
    {
        UpdateThreatLevel(EAudio_ThreatLevel::Danger);
    }
    else
    {
        UpdateThreatLevel(EAudio_ThreatLevel::Cautious);
    }
}

void AAudio_ZoneManager::OnDinosaurProximityExit()
{
    // Gradually de-escalate — go to Cautious first, then Safe on next call
    if (GlobalThreatLevel == EAudio_ThreatLevel::Extreme || GlobalThreatLevel == EAudio_ThreatLevel::Danger)
    {
        UpdateThreatLevel(EAudio_ThreatLevel::Cautious);
    }
    else
    {
        UpdateThreatLevel(EAudio_ThreatLevel::Safe);
    }
}
