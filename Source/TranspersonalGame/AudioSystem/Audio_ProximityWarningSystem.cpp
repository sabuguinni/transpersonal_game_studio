#include "Audio_ProximityWarningSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Pawn.h"

UAudio_ProximityWarningSystem::UAudio_ProximityWarningSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 times per second

    // Initialize detection ranges (in Unreal Units - 1 UU = 1 cm)
    MaxDetectionRange = 5000.0f;    // 50 meters
    CriticalRange = 500.0f;         // 5 meters
    HighThreatRange = 1500.0f;      // 15 meters
    MediumThreatRange = 3000.0f;    // 30 meters

    // Audio settings
    BaseVolume = 0.5f;
    MaxVolume = 1.0f;
    WarningCooldown = 3.0f;

    // Initialize status
    bIsPlayingWarning = false;
    LastWarningTime = 0.0f;
    LastScanTime = 0.0f;
    ScanInterval = 0.5f; // Scan twice per second

    // Create audio components
    ProximityAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ProximityAudioComponent"));
    HeartbeatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("HeartbeatAudioComponent"));

    if (ProximityAudioComponent)
    {
        ProximityAudioComponent->bAutoActivate = false;
        ProximityAudioComponent->SetVolumeMultiplier(BaseVolume);
    }

    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->bAutoActivate = false;
        HeartbeatAudioComponent->SetVolumeMultiplier(0.3f);
    }
}

void UAudio_ProximityWarningSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize current threat
    CurrentThreat = FAudio_ThreatData();

    UE_LOG(LogTemp, Warning, TEXT("Audio Proximity Warning System initialized"));
}

void UAudio_ProximityWarningSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Scan for threats at regular intervals
    if (CurrentTime - LastScanTime >= ScanInterval)
    {
        ScanForThreats();
        LastScanTime = CurrentTime;
    }
}

void UAudio_ProximityWarningSystem::ScanForThreats()
{
    if (!GetOwner())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FAudio_ThreatData NewThreat;
    float ClosestDistance = MaxDetectionRange + 1.0f;

    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == GetOwner())
        {
            continue;
        }

        // Check if this is a dinosaur or threat actor
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("TRex")) || 
            ActorName.Contains(TEXT("Raptor")) || 
            ActorName.Contains(TEXT("Brachiosaurus")) ||
            ActorName.Contains(TEXT("Dinosaur")) ||
            Actor->ActorHasTag(FName("Dinosaur")) ||
            Actor->ActorHasTag(FName("Threat")))
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            
            if (Distance <= MaxDetectionRange && Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                NewThreat.Distance = Distance;
                NewThreat.ThreatLocation = Actor->GetActorLocation();
                NewThreat.ThreatActor = Actor;
                NewThreat.ThreatLevel = CalculateThreatLevel(Distance, Actor);
            }
        }
    }

    // Update current threat and play warning if necessary
    if (NewThreat.ThreatActor)
    {
        bool bThreatChanged = (CurrentThreat.ThreatActor != NewThreat.ThreatActor) ||
                             (CurrentThreat.ThreatLevel != NewThreat.ThreatLevel);

        CurrentThreat = NewThreat;

        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (bThreatChanged && (CurrentTime - LastWarningTime >= WarningCooldown))
        {
            PlayThreatWarning(CurrentThreat);
        }

        UpdateHeartbeatIntensity(CurrentThreat.ThreatLevel);
    }
    else
    {
        // No threats detected
        if (CurrentThreat.ThreatLevel != EAudio_ThreatLevel::None)
        {
            CurrentThreat = FAudio_ThreatData(); // Reset to none
            StopThreatWarning();
            UpdateHeartbeatIntensity(EAudio_ThreatLevel::None);
        }
    }
}

void UAudio_ProximityWarningSystem::PlayThreatWarning(const FAudio_ThreatData& ThreatData)
{
    if (!ProximityAudioComponent || !ThreatData.ThreatActor)
    {
        return;
    }

    USoundCue* SoundToPlay = GetSoundForActor(ThreatData.ThreatActor);
    if (!SoundToPlay)
    {
        return;
    }

    float Volume = CalculateVolumeForDistance(ThreatData.Distance);
    
    ProximityAudioComponent->SetSound(SoundToPlay);
    ProximityAudioComponent->SetVolumeMultiplier(Volume);
    ProximityAudioComponent->Play();

    bIsPlayingWarning = true;
    LastWarningTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Warning, TEXT("Playing threat warning for %s at distance %.1f"), 
           *ThreatData.ThreatActor->GetName(), ThreatData.Distance);
}

void UAudio_ProximityWarningSystem::StopThreatWarning()
{
    if (ProximityAudioComponent && bIsPlayingWarning)
    {
        ProximityAudioComponent->Stop();
        bIsPlayingWarning = false;
    }
}

EAudio_ThreatLevel UAudio_ProximityWarningSystem::CalculateThreatLevel(float Distance, AActor* ThreatActor)
{
    if (!ThreatActor)
    {
        return EAudio_ThreatLevel::None;
    }

    if (Distance <= CriticalRange)
    {
        return EAudio_ThreatLevel::Critical;
    }
    else if (Distance <= HighThreatRange)
    {
        return EAudio_ThreatLevel::High;
    }
    else if (Distance <= MediumThreatRange)
    {
        return EAudio_ThreatLevel::Medium;
    }
    else if (Distance <= MaxDetectionRange)
    {
        return EAudio_ThreatLevel::Low;
    }

    return EAudio_ThreatLevel::None;
}

USoundCue* UAudio_ProximityWarningSystem::GetSoundForActor(AActor* ThreatActor)
{
    if (!ThreatActor)
    {
        return GenericThreatSound;
    }

    FString ActorName = ThreatActor->GetName();
    
    if (ActorName.Contains(TEXT("TRex")))
    {
        return TRexApproachSound ? TRexApproachSound : GenericThreatSound;
    }
    else if (ActorName.Contains(TEXT("Raptor")))
    {
        return RaptorPackSound ? RaptorPackSound : GenericThreatSound;
    }
    else if (ActorName.Contains(TEXT("Brachiosaurus")))
    {
        return BrachiosaurusSound ? BrachiosaurusSound : GenericThreatSound;
    }

    return GenericThreatSound;
}

float UAudio_ProximityWarningSystem::CalculateVolumeForDistance(float Distance)
{
    if (Distance <= CriticalRange)
    {
        return MaxVolume;
    }
    
    // Linear interpolation from MaxVolume to BaseVolume
    float NormalizedDistance = (Distance - CriticalRange) / (MaxDetectionRange - CriticalRange);
    NormalizedDistance = FMath::Clamp(NormalizedDistance, 0.0f, 1.0f);
    
    return FMath::Lerp(MaxVolume, BaseVolume, NormalizedDistance);
}

void UAudio_ProximityWarningSystem::UpdateHeartbeatIntensity(EAudio_ThreatLevel ThreatLevel)
{
    if (!HeartbeatAudioComponent)
    {
        return;
    }

    float HeartbeatVolume = 0.0f;
    float HeartbeatPitch = 1.0f;

    switch (ThreatLevel)
    {
        case EAudio_ThreatLevel::Critical:
            HeartbeatVolume = 0.8f;
            HeartbeatPitch = 1.5f;
            break;
        case EAudio_ThreatLevel::High:
            HeartbeatVolume = 0.6f;
            HeartbeatPitch = 1.3f;
            break;
        case EAudio_ThreatLevel::Medium:
            HeartbeatVolume = 0.4f;
            HeartbeatPitch = 1.1f;
            break;
        case EAudio_ThreatLevel::Low:
            HeartbeatVolume = 0.2f;
            HeartbeatPitch = 1.0f;
            break;
        default:
            HeartbeatVolume = 0.0f;
            HeartbeatPitch = 1.0f;
            break;
    }

    HeartbeatAudioComponent->SetVolumeMultiplier(HeartbeatVolume);
    HeartbeatAudioComponent->SetPitchMultiplier(HeartbeatPitch);

    if (HeartbeatVolume > 0.0f && !HeartbeatAudioComponent->IsPlaying())
    {
        HeartbeatAudioComponent->Play();
    }
    else if (HeartbeatVolume <= 0.0f && HeartbeatAudioComponent->IsPlaying())
    {
        HeartbeatAudioComponent->Stop();
    }
}