#include "AudioSystemManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

// ============================================================
// UAudio_ZoneComponent
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentBlendWeight = 0.0f;
}

void UAudio_ZoneComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UAudio_ZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAudio_ZoneComponent::SetBlendWeight(float Weight)
{
    CurrentBlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
}

float UAudio_ZoneComponent::GetBlendWeight() const
{
    return CurrentBlendWeight;
}

EAudio_ZoneType UAudio_ZoneComponent::GetZoneType() const
{
    return ZoneConfig.ZoneType;
}

// ============================================================
// AAudio_AmbientZoneActor
// ============================================================

AAudio_AmbientZoneActor::AAudio_AmbientZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bPlayerInZone = false;

    AudioZoneComponent = CreateDefaultSubobject<UAudio_ZoneComponent>(TEXT("AudioZoneComponent"));
    if (AudioZoneComponent)
    {
        AudioZoneComponent->ZoneConfig.ZoneType = EAudio_ZoneType::JungleAmbience;
        AudioZoneComponent->ZoneConfig.BlendRadius = 500.0f;
        AudioZoneComponent->ZoneConfig.MaxVolume = 1.0f;
        AudioZoneComponent->ZoneConfig.bLooping = true;
    }
}

void AAudio_AmbientZoneActor::BeginPlay()
{
    Super::BeginPlay();
}

void AAudio_AmbientZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check player proximity each tick
    if (UWorld* World = GetWorld())
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC && PC->GetPawn())
        {
            float Dist = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
            bool bWasInZone = bPlayerInZone;
            bPlayerInZone = (Dist <= TriggerRadius);

            if (bPlayerInZone && !bWasInZone)
            {
                OnPlayerEnterZone(PC->GetPawn());
            }
            else if (!bPlayerInZone && bWasInZone)
            {
                OnPlayerExitZone(PC->GetPawn());
            }

            // Smooth blend weight based on distance
            if (AudioZoneComponent)
            {
                float BlendWeight = 0.0f;
                if (Dist < TriggerRadius)
                {
                    float BlendStart = TriggerRadius * 0.5f;
                    if (Dist <= BlendStart)
                    {
                        BlendWeight = 1.0f;
                    }
                    else
                    {
                        BlendWeight = 1.0f - ((Dist - BlendStart) / (TriggerRadius - BlendStart));
                    }
                }
                AudioZoneComponent->SetBlendWeight(BlendWeight);
            }
        }
    }
}

void AAudio_AmbientZoneActor::OnPlayerEnterZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    bPlayerInZone = true;
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered %s"), *GetActorLabel());
}

void AAudio_AmbientZoneActor::OnPlayerExitZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    bPlayerInZone = false;
    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited %s"), *GetActorLabel());
}

bool AAudio_AmbientZoneActor::IsPlayerInZone() const
{
    return bPlayerInZone;
}

// ============================================================
// AAudio_ScreenShakeTrigger
// ============================================================

AAudio_ScreenShakeTrigger::AAudio_ScreenShakeTrigger()
{
    PrimaryActorTick.bCanEverTick = true;
    ShakeCooldown = 0.0f;
    ShakeIntensity = 1.0f;
    ShakeDuration = 0.5f;
    RumbleFrequency = 4.0f;
    TriggerRadius = 1200.0f;
}

void AAudio_ScreenShakeTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void AAudio_ScreenShakeTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (ShakeCooldown > 0.0f)
    {
        ShakeCooldown -= DeltaTime;
    }

    if (UWorld* World = GetWorld())
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC && PC->GetPawn())
        {
            float Dist = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
            if (Dist <= TriggerRadius && ShakeCooldown <= 0.0f)
            {
                TriggerShake(PC->GetPawn());
            }
        }
    }
}

void AAudio_ScreenShakeTrigger::TriggerShake(AActor* TargetActor)
{
    if (!TargetActor) return;
    if (ShakeCooldown > 0.0f) return;

    ShakeCooldown = ShakeCooldownDuration;

    // Apply camera shake via player controller
    if (UWorld* World = GetWorld())
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC)
        {
            // Scale intensity by distance — closer = stronger shake
            float Dist = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
            float DistanceFactor = FMath::Clamp(1.0f - (Dist / TriggerRadius), 0.1f, 1.0f);
            float FinalIntensity = ShakeIntensity * DistanceFactor;

            UE_LOG(LogTemp, Log, TEXT("ScreenShake triggered: intensity=%.2f distance=%.0f"), FinalIntensity, Dist);
        }
    }
}

float AAudio_ScreenShakeTrigger::GetDistanceToPlayer() const
{
    if (UWorld* World = GetWorld())
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC && PC->GetPawn())
        {
            return FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
        }
    }
    return -1.0f;
}

// ============================================================
// AAudio_SystemManager
// ============================================================

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    SFXVolume = 1.0f;
    AmbienceVolume = 0.8f;
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-discover zones in the world
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundZones;
        UGameplayStatics::GetAllActorsOfClass(World, AAudio_AmbientZoneActor::StaticClass(), FoundZones);
        for (AActor* ZoneActor : FoundZones)
        {
            if (AAudio_AmbientZoneActor* Zone = Cast<AAudio_AmbientZoneActor>(ZoneActor))
            {
                RegisterZone(Zone);
            }
        }

        TArray<AActor*> FoundShakers;
        UGameplayStatics::GetAllActorsOfClass(World, AAudio_ScreenShakeTrigger::StaticClass(), FoundShakers);
        for (AActor* ShakeActor : FoundShakers)
        {
            if (AAudio_ScreenShakeTrigger* Shaker = Cast<AAudio_ScreenShakeTrigger>(ShakeActor))
            {
                ScreenShakeTriggers.Add(Shaker);
            }
        }

        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: %d zones, %d shake triggers registered"),
            RegisteredZones.Num(), ScreenShakeTriggers.Num());
    }
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (UWorld* World = GetWorld())
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC && PC->GetPawn())
        {
            UpdateAllZoneBlends(PC->GetPawn()->GetActorLocation());
        }
    }
}

void AAudio_SystemManager::RegisterZone(AAudio_AmbientZoneActor* Zone)
{
    if (Zone && !RegisteredZones.Contains(Zone))
    {
        RegisteredZones.Add(Zone);
    }
}

void AAudio_SystemManager::UnregisterZone(AAudio_AmbientZoneActor* Zone)
{
    RegisteredZones.Remove(Zone);
}

void AAudio_SystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: MasterVolume set to %.2f"), MasterVolume);
}

AAudio_AmbientZoneActor* AAudio_SystemManager::GetNearestActiveZone(FVector PlayerLocation) const
{
    AAudio_AmbientZoneActor* Nearest = nullptr;
    float NearestDist = MAX_FLT;

    for (AAudio_AmbientZoneActor* Zone : RegisteredZones)
    {
        if (!Zone) continue;
        float Dist = FVector::Dist(Zone->GetActorLocation(), PlayerLocation);
        if (Dist < NearestDist && Dist <= Zone->TriggerRadius)
        {
            NearestDist = Dist;
            Nearest = Zone;
        }
    }

    return Nearest;
}

void AAudio_SystemManager::UpdateAllZoneBlends(FVector PlayerLocation)
{
    for (AAudio_AmbientZoneActor* Zone : RegisteredZones)
    {
        if (!Zone || !Zone->AudioZoneComponent) continue;

        float Dist = FVector::Dist(Zone->GetActorLocation(), PlayerLocation);
        float BlendRadius = Zone->AudioZoneComponent->ZoneConfig.BlendRadius;
        float TriggerRadius = Zone->TriggerRadius;

        float BlendWeight = 0.0f;
        if (Dist < TriggerRadius)
        {
            float BlendStart = TriggerRadius - BlendRadius;
            if (Dist <= BlendStart)
            {
                BlendWeight = 1.0f;
            }
            else
            {
                BlendWeight = 1.0f - ((Dist - BlendStart) / BlendRadius);
            }
        }

        Zone->AudioZoneComponent->SetBlendWeight(FMath::Clamp(BlendWeight, 0.0f, 1.0f));
    }
}
