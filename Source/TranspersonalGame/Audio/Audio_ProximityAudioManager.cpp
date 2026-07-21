#include "Audio_ProximityAudioManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

UAudio_ProximityAudioManager::UAudio_ProximityAudioManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize proximity data
    TRexProximityData.DetectionRadius = 3000.0f;
    TRexProximityData.MaxAudioDistance = 6000.0f;
    TRexProximityData.VolumeMultiplier = 2.0f;
    TRexProximityData.ThreatLevel = EAudio_ProximityThreatLevel::Critical;

    RaptorProximityData.DetectionRadius = 1500.0f;
    RaptorProximityData.MaxAudioDistance = 3000.0f;
    RaptorProximityData.VolumeMultiplier = 1.5f;
    RaptorProximityData.ThreatLevel = EAudio_ProximityThreatLevel::High;

    GeneralDinosaurData.DetectionRadius = 1000.0f;
    GeneralDinosaurData.MaxAudioDistance = 2000.0f;
    GeneralDinosaurData.VolumeMultiplier = 1.0f;
    GeneralDinosaurData.ThreatLevel = EAudio_ProximityThreatLevel::Medium;

    ProximityCheckInterval = 0.5f;
    HeartbeatIntensityMultiplier = 2.0f;
    bEnableProximityAudio = true;
    bEnableHeartbeatEffect = true;

    CurrentThreatLevel = EAudio_ProximityThreatLevel::None;
    CurrentProximityIntensity = 0.0f;
    NearestThreatActor = nullptr;
    LastProximityCheck = 0.0f;
    CurrentHeartbeatRate = 1.0f;
}

void UAudio_ProximityAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Create audio components
    ProximityAudioComponent = NewObject<UAudioComponent>(this);
    if (ProximityAudioComponent)
    {
        ProximityAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        ProximityAudioComponent->SetVolumeMultiplier(0.0f);
    }

    HeartbeatAudioComponent = NewObject<UAudioComponent>(this);
    if (HeartbeatAudioComponent)
    {
        HeartbeatAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        HeartbeatAudioComponent->SetVolumeMultiplier(0.0f);
    }

    // Find all dinosaur actors in the level
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains("trex") || ActorName.Contains("raptor") || 
                ActorName.Contains("dinosaur") || ActorName.Contains("dino"))
            {
                AddTrackedDinosaur(Actor);
            }
        }
    }
}

void UAudio_ProximityAudioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableProximityAudio)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastProximityCheck >= ProximityCheckInterval)
    {
        UpdateProximityAudio();
        LastProximityCheck = CurrentTime;
    }

    // Update heartbeat effect
    if (bEnableHeartbeatEffect && HeartbeatAudioComponent)
    {
        UpdateHeartbeatEffect(CurrentProximityIntensity);
    }
}

void UAudio_ProximityAudioManager::UpdateProximityAudio()
{
    if (!GetOwner())
        return;

    FVector PlayerLocation = GetOwner()->GetActorLocation();
    float ClosestDistance = FLT_MAX;
    AActor* ClosestThreat = nullptr;
    EAudio_ProximityThreatLevel HighestThreatLevel = EAudio_ProximityThreatLevel::None;

    // Check all tracked dinosaurs
    for (AActor* Dinosaur : TrackedDinosaurs)
    {
        if (!Dinosaur || !IsValid(Dinosaur))
            continue;

        float Distance = FVector::Dist(PlayerLocation, Dinosaur->GetActorLocation());
        
        // Determine threat level based on dinosaur type and distance
        EAudio_ProximityThreatLevel ThreatLevel = EAudio_ProximityThreatLevel::None;
        float DetectionRadius = GeneralDinosaurData.DetectionRadius;
        
        FString DinosaurName = Dinosaur->GetName().ToLower();
        if (DinosaurName.Contains("trex"))
        {
            ThreatLevel = TRexProximityData.ThreatLevel;
            DetectionRadius = TRexProximityData.DetectionRadius;
        }
        else if (DinosaurName.Contains("raptor"))
        {
            ThreatLevel = RaptorProximityData.ThreatLevel;
            DetectionRadius = RaptorProximityData.DetectionRadius;
        }
        else
        {
            ThreatLevel = GeneralDinosaurData.ThreatLevel;
            DetectionRadius = GeneralDinosaurData.DetectionRadius;
        }

        // Check if within detection range
        if (Distance <= DetectionRadius)
        {
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestThreat = Dinosaur;
                HighestThreatLevel = ThreatLevel;
            }
        }
    }

    // Update threat state
    NearestThreatActor = ClosestThreat;
    CurrentThreatLevel = HighestThreatLevel;
    
    if (ClosestThreat)
    {
        CurrentProximityIntensity = CalculateProximityIntensity(ClosestThreat);
        PlayProximityWarning(HighestThreatLevel);
    }
    else
    {
        CurrentProximityIntensity = 0.0f;
        SetThreatLevel(EAudio_ProximityThreatLevel::None);
    }
}

void UAudio_ProximityAudioManager::SetThreatLevel(EAudio_ProximityThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel == NewThreatLevel)
        return;

    CurrentThreatLevel = NewThreatLevel;

    // Adjust audio based on threat level
    if (ProximityAudioComponent)
    {
        float VolumeMultiplier = 0.0f;
        switch (NewThreatLevel)
        {
            case EAudio_ProximityThreatLevel::Low:
                VolumeMultiplier = 0.3f;
                break;
            case EAudio_ProximityThreatLevel::Medium:
                VolumeMultiplier = 0.6f;
                break;
            case EAudio_ProximityThreatLevel::High:
                VolumeMultiplier = 0.8f;
                break;
            case EAudio_ProximityThreatLevel::Critical:
                VolumeMultiplier = 1.0f;
                break;
            default:
                VolumeMultiplier = 0.0f;
                break;
        }
        
        ProximityAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    }
}

void UAudio_ProximityAudioManager::AddTrackedDinosaur(AActor* DinosaurActor)
{
    if (DinosaurActor && !TrackedDinosaurs.Contains(DinosaurActor))
    {
        TrackedDinosaurs.Add(DinosaurActor);
    }
}

void UAudio_ProximityAudioManager::RemoveTrackedDinosaur(AActor* DinosaurActor)
{
    TrackedDinosaurs.Remove(DinosaurActor);
}

float UAudio_ProximityAudioManager::CalculateProximityIntensity(AActor* ThreatActor)
{
    if (!ThreatActor || !GetOwner())
        return 0.0f;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
    
    // Determine max distance based on threat type
    float MaxDistance = GeneralDinosaurData.MaxAudioDistance;
    FString ThreatName = ThreatActor->GetName().ToLower();
    
    if (ThreatName.Contains("trex"))
    {
        MaxDistance = TRexProximityData.MaxAudioDistance;
    }
    else if (ThreatName.Contains("raptor"))
    {
        MaxDistance = RaptorProximityData.MaxAudioDistance;
    }

    // Calculate intensity (1.0 at close range, 0.0 at max distance)
    float Intensity = FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);
    return Intensity;
}

void UAudio_ProximityAudioManager::PlayProximityWarning(EAudio_ProximityThreatLevel ThreatLevel)
{
    if (!ProximityAudioComponent)
        return;

    // Play appropriate warning sound based on threat level
    // This would be connected to MetaSound assets in Blueprint
    
    SetThreatLevel(ThreatLevel);
}

void UAudio_ProximityAudioManager::UpdateHeartbeatEffect(float Intensity)
{
    if (!HeartbeatAudioComponent)
        return;

    // Increase heartbeat rate based on proximity intensity
    CurrentHeartbeatRate = 1.0f + (Intensity * HeartbeatIntensityMultiplier);
    
    // Set heartbeat volume and pitch
    HeartbeatAudioComponent->SetVolumeMultiplier(Intensity * 0.7f);
    HeartbeatAudioComponent->SetPitchMultiplier(CurrentHeartbeatRate);
}

bool UAudio_ProximityAudioManager::IsPlayerInDanger() const
{
    return CurrentThreatLevel >= EAudio_ProximityThreatLevel::Medium;
}

float UAudio_ProximityAudioManager::GetDistanceToNearestThreat() const
{
    if (!NearestThreatActor || !GetOwner())
        return FLT_MAX;

    return FVector::Dist(GetOwner()->GetActorLocation(), NearestThreatActor->GetActorLocation());
}