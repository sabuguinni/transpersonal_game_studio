#include "Audio_ProximityAlertSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

AAudio_ProximityAlertSystem::AAudio_ProximityAlertSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create audio component
    AlertAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AlertAudioComponent"));
    AlertAudioComponent->SetupAttachment(RootComponent);
    AlertAudioComponent->bAutoActivate = false;

    // Initialize default configuration
    AlertConfig.AlertType = EAudio_AlertType::TRexProximity;
    AlertConfig.DetectionRadius = 1000.0f;
    AlertConfig.AlertCooldown = 30.0f;
    AlertConfig.bRequiresLineOfSight = false;
    AlertConfig.VolumeMultiplier = 1.0f;

    // Initialize state
    bAlertActive = false;
    LastAlertTime = 0.0f;
    ScanTimer = 0.0f;
    ScanInterval = 1.0f;
    MinimumThreatDistance = 500.0f;
    bIsActive = true;
}

void AAudio_ProximityAlertSystem::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (DetectionSphere)
    {
        DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ProximityAlertSystem::OnDetectionSphereBeginOverlap);
        DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ProximityAlertSystem::OnDetectionSphereEndOverlap);
        DetectionSphere->SetSphereRadius(AlertConfig.DetectionRadius);
    }

    InitializeAlertSounds();
    ValidateConfiguration();
}

void AAudio_ProximityAlertSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsActive)
        return;

    ScanTimer += DeltaTime;
    if (ScanTimer >= ScanInterval)
    {
        UpdateDetection();
        ScanTimer = 0.0f;
    }

    // Update alert state
    if (bAlertActive && GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastAlertTime > AlertConfig.AlertCooldown)
        {
            DeactivateAlert();
        }
    }
}

void AAudio_ProximityAlertSystem::UpdateDetection()
{
    if (!GetWorld())
        return;

    ScanForThreats();

    // Process detected threats
    for (AActor* Threat : DetectedThreats)
    {
        if (IsValid(Threat))
        {
            ProcessThreatDistance(Threat);
        }
    }

    // Clean up invalid threats
    DetectedThreats.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
}

void AAudio_ProximityAlertSystem::ScanForThreats()
{
    if (!GetWorld())
        return;

    TArray<AActor*> OverlappingActors;
    DetectionSphere->GetOverlappingActors(OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (IsValidThreat(Actor) && !DetectedThreats.Contains(Actor))
        {
            DetectedThreats.Add(Actor);
        }
    }
}

bool AAudio_ProximityAlertSystem::IsValidThreat(AActor* Actor) const
{
    if (!IsValid(Actor))
        return false;

    // Check if actor is in target classes
    for (const TSubclassOf<APawn>& TargetClass : TargetClasses)
    {
        if (Actor->IsA(TargetClass))
        {
            return true;
        }
    }

    // Check for specific threat patterns
    FString ActorName = Actor->GetName();
    if (ActorName.Contains("TRex") || ActorName.Contains("Carnotaurus") || 
        ActorName.Contains("Raptor") || ActorName.Contains("Predator"))
    {
        return true;
    }

    return false;
}

void AAudio_ProximityAlertSystem::ProcessThreatDistance(AActor* ThreatActor)
{
    if (!IsValid(ThreatActor))
        return;

    float Distance = FVector::Dist(GetActorLocation(), ThreatActor->GetActorLocation());
    
    if (Distance <= MinimumThreatDistance && CanTriggerAlert())
    {
        // Determine alert type based on threat
        EAudio_AlertType AlertType = EAudio_AlertType::TRexProximity;
        
        FString ThreatName = ThreatActor->GetName();
        if (ThreatName.Contains("TRex"))
        {
            AlertType = EAudio_AlertType::TRexProximity;
        }
        else if (ThreatName.Contains("Volcanic") || ThreatName.Contains("Lava"))
        {
            AlertType = EAudio_AlertType::VolcanicHazard;
        }
        else if (ThreatName.Contains("Storm") || ThreatName.Contains("Weather"))
        {
            AlertType = EAudio_AlertType::StormWarning;
        }
        else if (ThreatName.Contains("Pack") || ThreatName.Contains("Raptor"))
        {
            AlertType = EAudio_AlertType::PredatorPack;
        }
        else
        {
            AlertType = EAudio_AlertType::EnvironmentalDanger;
        }

        TriggerAlert(ThreatActor, AlertType);
    }
}

bool AAudio_ProximityAlertSystem::HasLineOfSightToThreat(AActor* ThreatActor) const
{
    if (!IsValid(ThreatActor) || !GetWorld())
        return false;

    if (!AlertConfig.bRequiresLineOfSight)
        return true;

    FVector Start = GetActorLocation();
    FVector End = ThreatActor->GetActorLocation();

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(ThreatActor);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );

    return !bHit;
}

void AAudio_ProximityAlertSystem::TriggerAlert(AActor* ThreatActor, EAudio_AlertType AlertType)
{
    if (!CanTriggerAlert() || !IsValid(ThreatActor))
        return;

    // Check line of sight if required
    if (AlertConfig.bRequiresLineOfSight && !HasLineOfSightToThreat(ThreatActor))
        return;

    bAlertActive = true;
    LastAlertTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    PlayAlertSound(AlertType);

    // Broadcast event
    OnAlertTriggered.Broadcast(AlertType, ThreatActor);

    // Debug logging
    if (GEngine)
    {
        FString AlertMessage = FString::Printf(
            TEXT("PROXIMITY ALERT: %s detected at distance %.1f"),
            *UEnum::GetValueAsString(AlertType),
            FVector::Dist(GetActorLocation(), ThreatActor->GetActorLocation())
        );
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, AlertMessage);
    }
}

void AAudio_ProximityAlertSystem::DeactivateAlert()
{
    if (!bAlertActive)
        return;

    bAlertActive = false;

    if (AlertAudioComponent && AlertAudioComponent->IsPlaying())
    {
        AlertAudioComponent->Stop();
    }

    OnAlertDeactivated.Broadcast();
}

bool AAudio_ProximityAlertSystem::CanTriggerAlert() const
{
    if (!bIsActive || bAlertActive)
        return false;

    if (GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        return (CurrentTime - LastAlertTime) >= AlertConfig.AlertCooldown;
    }

    return true;
}

void AAudio_ProximityAlertSystem::SetAlertConfiguration(const FAudio_AlertConfiguration& NewConfig)
{
    AlertConfig = NewConfig;

    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(AlertConfig.DetectionRadius);
    }

    if (AlertAudioComponent)
    {
        AlertAudioComponent->SetVolumeMultiplier(AlertConfig.VolumeMultiplier);
    }

    ValidateConfiguration();
}

float AAudio_ProximityAlertSystem::GetDistanceToNearestThreat() const
{
    float MinDistance = FLT_MAX;

    for (const AActor* Threat : DetectedThreats)
    {
        if (IsValid(Threat))
        {
            float Distance = FVector::Dist(GetActorLocation(), Threat->GetActorLocation());
            MinDistance = FMath::Min(MinDistance, Distance);
        }
    }

    return MinDistance == FLT_MAX ? -1.0f : MinDistance;
}

void AAudio_ProximityAlertSystem::PlayAlertSound(EAudio_AlertType AlertType)
{
    if (!AlertAudioComponent)
        return;

    USoundBase* AlertSound = GetAlertSoundForType(AlertType);
    if (!AlertSound)
        AlertSound = AlertConfig.AlertSound;

    if (AlertSound)
    {
        AlertAudioComponent->SetSound(AlertSound);
        AlertAudioComponent->SetVolumeMultiplier(AlertConfig.VolumeMultiplier);
        AlertAudioComponent->Play();
    }
}

USoundBase* AAudio_ProximityAlertSystem::GetAlertSoundForType(EAudio_AlertType AlertType) const
{
    if (const USoundBase* const* FoundSound = AlertSoundMap.Find(AlertType))
    {
        return *FoundSound;
    }
    return nullptr;
}

void AAudio_ProximityAlertSystem::InitializeAlertSounds()
{
    // Initialize sound map - these would be loaded from assets in a real implementation
    AlertSoundMap.Empty();
    
    // For now, use the configured alert sound for all types
    // In production, each alert type would have its own sound asset
    if (AlertConfig.AlertSound)
    {
        AlertSoundMap.Add(EAudio_AlertType::TRexProximity, AlertConfig.AlertSound);
        AlertSoundMap.Add(EAudio_AlertType::VolcanicHazard, AlertConfig.AlertSound);
        AlertSoundMap.Add(EAudio_AlertType::StormWarning, AlertConfig.AlertSound);
        AlertSoundMap.Add(EAudio_AlertType::PredatorPack, AlertConfig.AlertSound);
        AlertSoundMap.Add(EAudio_AlertType::EnvironmentalDanger, AlertConfig.AlertSound);
    }
}

void AAudio_ProximityAlertSystem::ValidateConfiguration()
{
    // Ensure valid detection radius
    if (AlertConfig.DetectionRadius <= 0.0f)
    {
        AlertConfig.DetectionRadius = 1000.0f;
    }

    // Ensure valid cooldown
    if (AlertConfig.AlertCooldown < 0.0f)
    {
        AlertConfig.AlertCooldown = 30.0f;
    }

    // Ensure valid volume multiplier
    AlertConfig.VolumeMultiplier = FMath::Clamp(AlertConfig.VolumeMultiplier, 0.0f, 2.0f);
}

void AAudio_ProximityAlertSystem::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
    const FHitResult& SweepResult)
{
    if (IsValidThreat(OtherActor) && !DetectedThreats.Contains(OtherActor))
    {
        DetectedThreats.Add(OtherActor);
    }
}

void AAudio_ProximityAlertSystem::OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    DetectedThreats.Remove(OtherActor);
}