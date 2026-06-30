// AudioSystemManager.cpp
// Audio Agent #16 — Transpersonal Game Studio
// Adaptive audio: ambient layers, danger proximity, heartbeat, T-Rex screen shake

#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

// ============================================================
// UAudio_ProximityComponent
// ============================================================

UAudio_ProximityComponent::UAudio_ProximityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float UAudio_ProximityComponent::GetProximityVolume(float DistanceToPlayer) const
{
    if (DistanceToPlayer >= ProximityRadius)
    {
        return 0.0f;
    }
    // Linear falloff from 1.0 at distance=0 to 0.0 at ProximityRadius
    float Alpha = 1.0f - (DistanceToPlayer / ProximityRadius);
    return FMath::Clamp(Alpha, 0.0f, 1.0f);
}

bool UAudio_ProximityComponent::IsPlayerInDangerZone(float DistanceToPlayer) const
{
    return DistanceToPlayer <= HeartbeatThresholdDistance;
}

// ============================================================
// AAudio_SystemManager
// ============================================================

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20fps tick for audio — sufficient

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetupAttachment(GetRootComponent());

    DangerAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DangerAudio"));
    DangerAudioComponent->bAutoActivate = false;
    DangerAudioComponent->SetupAttachment(GetRootComponent());
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Cache player pawn reference
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        CachedPlayerPawn = PC->GetPawn();
    }

    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    CurrentTimeOfDay = EAudio_TimeOfDay::Day;
    HeartbeatTimer = 0.0f;
    CurrentHeartbeatInterval = HeartbeatMaxInterval;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Initialized — ambient layers: %d"), AmbientLayers.Num());
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ScanForDanger(DeltaTime);
    UpdateAmbientLayers(DeltaTime);
    UpdateHeartbeat(DeltaTime);
}

// ---- Danger Scanning ----

void AAudio_SystemManager::ScanForDanger(float DeltaTime)
{
    DangerScanTimer += DeltaTime;
    if (DangerScanTimer < DangerScanInterval)
    {
        return;
    }
    DangerScanTimer = 0.0f;

    EAudio_DangerLevel EvaluatedLevel = EvaluateDangerFromZones();

    if (EvaluatedLevel != CurrentDangerLevel)
    {
        SetDangerLevel(EvaluatedLevel);
    }

    // T-Rex screen shake check
    float NearestDist = GetNearestDangerDistance();
    if (NearestDist < TRexShakeRadius && CurrentDangerLevel >= EAudio_DangerLevel::Threatened)
    {
        ApplyTRexScreenShake();
    }
}

EAudio_DangerLevel AAudio_SystemManager::EvaluateDangerFromZones() const
{
    if (!CachedPlayerPawn)
    {
        return EAudio_DangerLevel::Safe;
    }

    FVector PlayerLoc = CachedPlayerPawn->GetActorLocation();
    EAudio_DangerLevel HighestLevel = EAudio_DangerLevel::Safe;

    for (const FAudio_DangerZone& Zone : ActiveDangerZones)
    {
        float Dist = FVector::Dist(PlayerLoc, Zone.Location);
        if (Dist <= Zone.Radius)
        {
            if (Zone.DangerLevel > HighestLevel)
            {
                HighestLevel = Zone.DangerLevel;
            }
        }
    }

    return HighestLevel;
}

float AAudio_SystemManager::GetNearestDangerDistance() const
{
    if (!CachedPlayerPawn || ActiveDangerZones.Num() == 0)
    {
        return TRexShakeRadius * 2.0f;
    }

    FVector PlayerLoc = CachedPlayerPawn->GetActorLocation();
    float MinDist = TRexShakeRadius * 2.0f;

    for (const FAudio_DangerZone& Zone : ActiveDangerZones)
    {
        float Dist = FVector::Dist(PlayerLoc, Zone.Location);
        if (Dist < MinDist)
        {
            MinDist = Dist;
        }
    }

    return MinDist;
}

// ---- Ambient Layers ----

void AAudio_SystemManager::UpdateAmbientLayers(float DeltaTime)
{
    for (FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        float TargetVolume = 0.0f;

        // Active if time of day matches
        if (Layer.ActiveTimeOfDay == CurrentTimeOfDay)
        {
            // Scale by danger: reduce ambient volume when danger is high
            float DangerScale = 1.0f;
            switch (CurrentDangerLevel)
            {
                case EAudio_DangerLevel::Aware:      DangerScale = 0.7f; break;
                case EAudio_DangerLevel::Threatened: DangerScale = 0.4f; break;
                case EAudio_DangerLevel::Critical:   DangerScale = 0.1f; break;
                default:                             DangerScale = 1.0f; break;
            }
            TargetVolume = Layer.BaseVolume * DangerScale;
        }

        // Smooth crossfade
        Layer.CurrentVolume = FMath::FInterpTo(Layer.CurrentVolume, TargetVolume, DeltaTime, AmbientCrossfadeSpeed);
    }
}

// ---- Heartbeat ----

void AAudio_SystemManager::UpdateHeartbeat(float DeltaTime)
{
    if (CurrentDangerLevel < EAudio_DangerLevel::Threatened)
    {
        return;
    }

    HeartbeatTimer += DeltaTime;

    // Map danger level to heartbeat interval
    float TargetInterval = HeartbeatMaxInterval;
    switch (CurrentDangerLevel)
    {
        case EAudio_DangerLevel::Threatened:
            TargetInterval = FMath::Lerp(HeartbeatMaxInterval, HeartbeatMinInterval, 0.5f);
            break;
        case EAudio_DangerLevel::Critical:
            TargetInterval = HeartbeatMinInterval;
            break;
        default:
            break;
    }

    CurrentHeartbeatInterval = FMath::FInterpTo(CurrentHeartbeatInterval, TargetInterval, DeltaTime, DangerTransitionSpeed);

    if (HeartbeatTimer >= CurrentHeartbeatInterval)
    {
        HeartbeatTimer = 0.0f;
        if (HeartbeatSound && CachedPlayerPawn)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeartbeatSound, CachedPlayerPawn->GetActorLocation(), 1.0f);
        }
    }
}

// ---- Screen Shake ----

void AAudio_SystemManager::ApplyTRexScreenShake()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        return;
    }

    float NearestDist = GetNearestDangerDistance();
    float ShakeAlpha = 1.0f - FMath::Clamp(NearestDist / TRexShakeRadius, 0.0f, 1.0f);
    float FinalIntensity = TRexShakeIntensity * ShakeAlpha;

    if (FinalIntensity > 0.05f)
    {
        // Apply camera shake via console command (works without TSubclassOf reference)
        FString Cmd = FString::Printf(TEXT("shake CameraShake_TRex %f"), FinalIntensity);
        PC->ConsoleCommand(Cmd);
    }
}

// ---- Public API ----

void AAudio_SystemManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (NewLevel == CurrentDangerLevel)
    {
        return;
    }

    CurrentDangerLevel = NewLevel;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Danger level changed to: %d"), (int32)NewLevel);
}

void AAudio_SystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay)
{
    if (NewTimeOfDay == CurrentTimeOfDay)
    {
        return;
    }

    CurrentTimeOfDay = NewTimeOfDay;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Time of day changed to: %d"), (int32)NewTimeOfDay);
}

void AAudio_SystemManager::RegisterDangerZone(FVector Location, float Radius, EAudio_DangerLevel Level, FString SourceName)
{
    // Update existing zone if same source
    for (FAudio_DangerZone& Zone : ActiveDangerZones)
    {
        if (Zone.SourceActorName == SourceName)
        {
            Zone.Location = Location;
            Zone.Radius = Radius;
            Zone.DangerLevel = Level;
            return;
        }
    }

    // Add new zone
    FAudio_DangerZone NewZone;
    NewZone.Location = Location;
    NewZone.Radius = Radius;
    NewZone.DangerLevel = Level;
    NewZone.SourceActorName = SourceName;
    ActiveDangerZones.Add(NewZone);

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Danger zone registered: %s (radius=%.0f)"), *SourceName, Radius);
}

void AAudio_SystemManager::UnregisterDangerZone(FString SourceName)
{
    ActiveDangerZones.RemoveAll([&SourceName](const FAudio_DangerZone& Zone)
    {
        return Zone.SourceActorName == SourceName;
    });
}

void AAudio_SystemManager::TriggerImpactSound(FVector ImpactLocation, float Magnitude)
{
    // Magnitude drives screen shake intensity for impacts (e.g., T-Rex footfall)
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !CachedPlayerPawn)
    {
        return;
    }

    float DistToPlayer = FVector::Dist(ImpactLocation, CachedPlayerPawn->GetActorLocation());
    float ShakeAlpha = FMath::Clamp(1.0f - (DistToPlayer / TRexShakeRadius), 0.0f, 1.0f);
    float FinalIntensity = Magnitude * ShakeAlpha;

    if (FinalIntensity > 0.05f)
    {
        FString Cmd = FString::Printf(TEXT("shake CameraShake_Impact %f"), FinalIntensity);
        PC->ConsoleCommand(Cmd);
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioSystemManager] Impact sound triggered at (%.0f,%.0f,%.0f) mag=%.2f shake=%.2f"),
        ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z, Magnitude, FinalIntensity);
}
