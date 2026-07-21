#include "Arch_StructuralAcousticsSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"

UArch_StructuralAcousticsSystem::UArch_StructuralAcousticsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default acoustic properties
    AcousticSettings.ReverbAmount = 0.3f;
    AcousticSettings.EchoDelay = 0.1f;
    AcousticSettings.SoundDamping = 0.5f;
    AcousticSettings.AmbientVolume = 1.0f;
    
    DetectionRadius = 1000.0f;
    bAutoDetectZoneType = true;
    ZoneType = EArch_AcousticZoneType::OpenAir;
}

void UArch_StructuralAcousticsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    
    if (bAutoDetectZoneType)
    {
        DetectNearbyStructures();
        ZoneType = DetermineZoneTypeFromEnvironment();
    }
    
    ApplyAcousticSettings();
}

void UArch_StructuralAcousticsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoDetectZoneType)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastDetectionTime >= DetectionInterval)
        {
            DetectNearbyStructures();
            EArch_AcousticZoneType NewZoneType = DetermineZoneTypeFromEnvironment();
            
            if (NewZoneType != ZoneType)
            {
                EArch_AcousticZoneType OldZone = ZoneType;
                SetAcousticZone(NewZoneType);
                OnAcousticZoneChanged(OldZone, NewZoneType);
            }
            
            LastDetectionTime = CurrentTime;
        }
    }
}

void UArch_StructuralAcousticsSystem::InitializeAudioComponents()
{
    if (!AmbientAudioComponent)
    {
        AmbientAudioComponent = NewObject<UAudioComponent>(GetOwner());
        if (AmbientAudioComponent && GetOwner())
        {
            AmbientAudioComponent->AttachToComponent(
                GetOwner()->GetRootComponent(),
                FAttachmentTransformRules::KeepWorldTransform
            );
            AmbientAudioComponent->bAutoActivate = true;
            AmbientAudioComponent->SetVolumeMultiplier(AcousticSettings.AmbientVolume);
        }
    }
}

void UArch_StructuralAcousticsSystem::DetectNearbyStructures()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    NearbyStructures.Empty();
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Get all actors within detection radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsInRadiusOfClass(
        GetWorld(),
        OwnerLocation,
        DetectionRadius,
        AActor::StaticClass(),
        FoundActors
    );
    
    // Filter for structural elements
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != GetOwner())
        {
            FString ActorName = Actor->GetName().ToLower();
            
            // Check if actor represents a structural element
            if (ActorName.Contains("cave") || 
                ActorName.Contains("rock") || 
                ActorName.Contains("stone") ||
                ActorName.Contains("arch") ||
                ActorName.Contains("ruin") ||
                ActorName.Contains("shelter") ||
                ActorName.Contains("wall") ||
                ActorName.Contains("building"))
            {
                NearbyStructures.Add(Actor);
            }
        }
    }
}

EArch_AcousticZoneType UArch_StructuralAcousticsSystem::DetermineZoneTypeFromEnvironment()
{
    if (NearbyStructures.Num() == 0)
    {
        return EArch_AcousticZoneType::OpenAir;
    }
    
    int32 CaveCount = 0;
    int32 RuinCount = 0;
    int32 ShelterCount = 0;
    int32 ForestCount = 0;
    
    for (const TWeakObjectPtr<AActor>& ActorPtr : NearbyStructures)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            FString ActorName = Actor->GetName().ToLower();
            
            if (ActorName.Contains("cave"))
            {
                CaveCount++;
            }
            else if (ActorName.Contains("ruin") || ActorName.Contains("arch"))
            {
                RuinCount++;
            }
            else if (ActorName.Contains("shelter") || ActorName.Contains("building"))
            {
                ShelterCount++;
            }
            else if (ActorName.Contains("tree") || ActorName.Contains("forest"))
            {
                ForestCount++;
            }
        }
    }
    
    // Determine zone type based on nearby structures
    if (CaveCount > 0)
    {
        return EArch_AcousticZoneType::Cave;
    }
    else if (RuinCount > 0)
    {
        return EArch_AcousticZoneType::Ruins;
    }
    else if (ShelterCount > 0)
    {
        return EArch_AcousticZoneType::Shelter;
    }
    else if (ForestCount > 2)
    {
        return EArch_AcousticZoneType::Forest;
    }
    
    return EArch_AcousticZoneType::OpenAir;
}

void UArch_StructuralAcousticsSystem::SetAcousticZone(EArch_AcousticZoneType NewZoneType)
{
    ZoneType = NewZoneType;
    
    // Update acoustic properties based on zone type
    switch (ZoneType)
    {
        case EArch_AcousticZoneType::Cave:
            AcousticSettings.ReverbAmount = 0.8f;
            AcousticSettings.EchoDelay = 0.5f;
            AcousticSettings.SoundDamping = 0.3f;
            AcousticSettings.AmbientVolume = 0.6f;
            if (AmbientAudioComponent && CaveAmbientSound)
            {
                AmbientAudioComponent->SetSound(CaveAmbientSound);
            }
            break;
            
        case EArch_AcousticZoneType::Forest:
            AcousticSettings.ReverbAmount = 0.2f;
            AcousticSettings.EchoDelay = 0.05f;
            AcousticSettings.SoundDamping = 0.7f;
            AcousticSettings.AmbientVolume = 0.8f;
            if (AmbientAudioComponent && ForestAmbientSound)
            {
                AmbientAudioComponent->SetSound(ForestAmbientSound);
            }
            break;
            
        case EArch_AcousticZoneType::Ruins:
            AcousticSettings.ReverbAmount = 0.5f;
            AcousticSettings.EchoDelay = 0.3f;
            AcousticSettings.SoundDamping = 0.4f;
            AcousticSettings.AmbientVolume = 0.7f;
            if (AmbientAudioComponent && RuinsAmbientSound)
            {
                AmbientAudioComponent->SetSound(RuinsAmbientSound);
            }
            break;
            
        case EArch_AcousticZoneType::Shelter:
            AcousticSettings.ReverbAmount = 0.4f;
            AcousticSettings.EchoDelay = 0.2f;
            AcousticSettings.SoundDamping = 0.5f;
            AcousticSettings.AmbientVolume = 0.5f;
            break;
            
        case EArch_AcousticZoneType::Canyon:
            AcousticSettings.ReverbAmount = 0.9f;
            AcousticSettings.EchoDelay = 0.8f;
            AcousticSettings.SoundDamping = 0.2f;
            AcousticSettings.AmbientVolume = 0.9f;
            break;
            
        default: // OpenAir
            AcousticSettings.ReverbAmount = 0.1f;
            AcousticSettings.EchoDelay = 0.02f;
            AcousticSettings.SoundDamping = 0.8f;
            AcousticSettings.AmbientVolume = 1.0f;
            break;
    }
    
    ApplyAcousticSettings();
}

void UArch_StructuralAcousticsSystem::ApplyAcousticSettings()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AcousticSettings.AmbientVolume);
        
        // Apply reverb and other audio effects here
        // This would typically involve setting up audio effects chains
        // For now, we'll just adjust the basic volume
    }
}

void UArch_StructuralAcousticsSystem::UpdateAcousticProperties(const FArch_AcousticProperties& NewProperties)
{
    AcousticSettings = NewProperties;
    ApplyAcousticSettings();
}

FArch_AcousticProperties UArch_StructuralAcousticsSystem::GetCurrentAcousticProperties() const
{
    return AcousticSettings;
}

void UArch_StructuralAcousticsSystem::PlayStructuralSound(USoundCue* SoundToPlay, float VolumeMultiplier)
{
    if (SoundToPlay && GetOwner())
    {
        float AdjustedVolume = VolumeMultiplier * AcousticSettings.AmbientVolume;
        
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            SoundToPlay,
            GetOwner()->GetActorLocation(),
            AdjustedVolume
        );
    }
}