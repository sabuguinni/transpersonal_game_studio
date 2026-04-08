#include "Audio3DDetectionSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "../Characters/DinosaurCharacter.h"

UAudio3DDetectionSystem::UAudio3DDetectionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 updates per second by default
    
    MaxDetectionRange = 5000.0f;
    MinAudibleIntensity = 0.1f;
    DetectionUpdateRate = 10.0f;
    bUseOcclusion = true;
    bUseDirectionalHearing = true;
    bDetectAllDinosaurs = true;
    
    DetectionSensitivity = 1.0f;
    LastDetectionUpdate = 0.0f;
    DetectionUpdateInterval = 1.0f / DetectionUpdateRate;
}

void UAudio3DDetectionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize detection system
    DetectionUpdateInterval = 1.0f / FMath::Max(DetectionUpdateRate, 1.0f);
    
    // Clear initial state
    CurrentDetections.Empty();
    LastDetectionTimes.Empty();
}

void UAudio3DDetectionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastDetectionUpdate += DeltaTime;
    
    if (LastDetectionUpdate >= DetectionUpdateInterval)
    {
        UpdateDetections();
        LastDetectionUpdate = 0.0f;
    }
    
    // Update timing for existing detections
    for (FAudioDetectionData& Detection : CurrentDetections)
    {
        Detection.TimeSinceDetected += DeltaTime;
    }
}

void UAudio3DDetectionSystem::UpdateDetections()
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<FAudioDetectionData> NewDetections;
    
    // Find all dinosaurs in the world
    for (TActorIterator<ADinosaur> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        ADinosaur* Dinosaur = *ActorIterator;
        if (IsValid(Dinosaur) && ShouldDetectDinosaur(Dinosaur))
        {
            ProcessDinosaur(Dinosaur);
        }
    }
    
    // Clean up old detections
    CleanupOldDetections();
    
    // Update closest threat and loudest sound
    UpdateClosestThreat();
    UpdateLoudestSound();
}

void UAudio3DDetectionSystem::ProcessDinosaur(ADinosaur* Dinosaur)
{
    if (!IsValid(Dinosaur) || !IsValid(GetOwner()))
    {
        return;
    }
    
    const FVector OwnerLocation = GetOwner()->GetActorLocation();
    const FVector DinosaurLocation = Dinosaur->GetActorLocation();
    const float Distance = FVector::Dist(OwnerLocation, DinosaurLocation);
    
    // Check if within detection range
    if (Distance > MaxDetectionRange)
    {
        return;
    }
    
    // Calculate audio intensity
    const float AudioIntensity = CalculateAudioIntensity(Dinosaur, Distance);
    
    if (AudioIntensity < MinAudibleIntensity)
    {
        return;
    }
    
    // Check occlusion if enabled
    if (bUseOcclusion && IsOccluded(OwnerLocation, DinosaurLocation))
    {
        return; // Sound is blocked
    }
    
    // Create detection data
    FAudioDetectionData DetectionData = CreateDetectionData(Dinosaur);
    
    // Check if this is a new detection or update
    bool bIsNewDetection = true;
    for (int32 i = 0; i < CurrentDetections.Num(); ++i)
    {
        if (CurrentDetections[i].Source == Dinosaur)
        {
            // Update existing detection
            const float OldTimeSinceDetected = CurrentDetections[i].TimeSinceDetected;
            CurrentDetections[i] = DetectionData;
            CurrentDetections[i].TimeSinceDetected = OldTimeSinceDetected;
            
            OnDetectionUpdated.Broadcast(DetectionData);
            bIsNewDetection = false;
            break;
        }
    }
    
    if (bIsNewDetection)
    {
        CurrentDetections.Add(DetectionData);
        OnNewDetection.Broadcast(DetectionData);
    }
    
    // Update last detection time
    LastDetectionTimes.Add(Dinosaur, GetWorld()->GetTimeSeconds());
}

FAudioDetectionData UAudio3DDetectionSystem::CreateDetectionData(ADinosaur* Dinosaur)
{
    FAudioDetectionData DetectionData;
    
    if (!IsValid(Dinosaur) || !IsValid(GetOwner()))
    {
        return DetectionData;
    }
    
    const FVector OwnerLocation = GetOwner()->GetActorLocation();
    const FVector DinosaurLocation = Dinosaur->GetActorLocation();
    
    DetectionData.Source = Dinosaur;
    DetectionData.Direction = (DinosaurLocation - OwnerLocation).GetSafeNormal();
    DetectionData.Distance = FVector::Dist(OwnerLocation, DinosaurLocation);
    DetectionData.Intensity = CalculateAudioIntensity(Dinosaur, DetectionData.Distance);
    DetectionData.bIsVisible = false; // TODO: Implement line of sight check
    DetectionData.TimeSinceDetected = 0.0f;
    
    return DetectionData;
}

float UAudio3DDetectionSystem::CalculateAudioIntensity(ADinosaur* Dinosaur, float Distance) const
{
    if (!IsValid(Dinosaur))
    {
        return 0.0f;
    }
    
    // Base intensity from dinosaur size/threat level
    float BaseIntensity = GetDinosaurThreatLevel(Dinosaur);
    
    // Apply distance attenuation
    float DistanceAttenuation = GetDistanceAttenuation(Distance);
    
    // Apply sensitivity modifier
    float FinalIntensity = BaseIntensity * DistanceAttenuation * DetectionSensitivity;
    
    return FMath::Clamp(FinalIntensity, 0.0f, 1.0f);
}

bool UAudio3DDetectionSystem::IsOccluded(const FVector& Start, const FVector& End) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    return GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
}

bool UAudio3DDetectionSystem::ShouldDetectDinosaur(ADinosaur* Dinosaur) const
{
    if (!IsValid(Dinosaur))
    {
        return false;
    }
    
    if (bDetectAllDinosaurs)
    {
        return true;
    }
    
    // Check if dinosaur type is in detectable types
    for (const TSubclassOf<ADinosaur>& DetectableType : DetectableDinosaurTypes)
    {
        if (Dinosaur->IsA(DetectableType))
        {
            return true;
        }
    }
    
    return false;
}

void UAudio3DDetectionSystem::UpdateClosestThreat()
{
    ClosestThreat = FAudioDetectionData();
    float ClosestDistance = MaxDetectionRange + 1.0f;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        if (Detection.Distance < ClosestDistance && GetDinosaurThreatLevel(Detection.Source) > 0.5f)
        {
            ClosestDistance = Detection.Distance;
            ClosestThreat = Detection;
        }
    }
}

void UAudio3DDetectionSystem::UpdateLoudestSound()
{
    LoudestSound = FAudioDetectionData();
    float HighestIntensity = 0.0f;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        if (Detection.Intensity > HighestIntensity)
        {
            HighestIntensity = Detection.Intensity;
            LoudestSound = Detection;
        }
    }
}

void UAudio3DDetectionSystem::CleanupOldDetections()
{
    const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const float MaxAge = 5.0f; // Remove detections older than 5 seconds
    
    for (int32 i = CurrentDetections.Num() - 1; i >= 0; --i)
    {
        const FAudioDetectionData& Detection = CurrentDetections[i];
        
        if (Detection.Source)
        {
            float* LastDetectionTime = LastDetectionTimes.Find(Detection.Source);
            if (LastDetectionTime && (CurrentTime - *LastDetectionTime) > MaxAge)
            {
                OnDetectionLost.Broadcast(Detection.Source);
                CurrentDetections.RemoveAt(i);
                LastDetectionTimes.Remove(Detection.Source);
            }
        }
        else
        {
            // Remove invalid detections
            CurrentDetections.RemoveAt(i);
        }
    }
}

float UAudio3DDetectionSystem::GetDinosaurThreatLevel(ADinosaur* Dinosaur) const
{
    if (!IsValid(Dinosaur))
    {
        return 0.0f;
    }
    
    // TODO: Get threat level from dinosaur properties
    // For now, return a default value based on size/type
    return 0.7f; // Default threat level
}

float UAudio3DDetectionSystem::GetDistanceAttenuation(float Distance) const
{
    if (Distance <= 0.0f)
    {
        return 1.0f;
    }
    
    // Linear attenuation for now - could be improved with more complex curves
    const float NormalizedDistance = Distance / MaxDetectionRange;
    return FMath::Clamp(1.0f - NormalizedDistance, 0.0f, 1.0f);
}

// Public interface implementations
TArray<FAudioDetectionData> UAudio3DDetectionSystem::GetNearbyThreats(float MaxDistance) const
{
    TArray<FAudioDetectionData> NearbyThreats;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        if (Detection.Distance <= MaxDistance && GetDinosaurThreatLevel(Detection.Source) > 0.5f)
        {
            NearbyThreats.Add(Detection);
        }
    }
    
    return NearbyThreats;
}

bool UAudio3DDetectionSystem::IsThreateningDinosaurNearby(float MaxDistance) const
{
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        if (Detection.Distance <= MaxDistance && GetDinosaurThreatLevel(Detection.Source) > 0.5f)
        {
            return true;
        }
    }
    
    return false;
}

FVector UAudio3DDetectionSystem::GetDirectionToClosestThreat() const
{
    return ClosestThreat.Source ? ClosestThreat.Direction : FVector::ZeroVector;
}

void UAudio3DDetectionSystem::SetDetectionSensitivity(float Sensitivity)
{
    DetectionSensitivity = FMath::Clamp(Sensitivity, 0.0f, 2.0f);
}

float UAudio3DDetectionSystem::GetTotalThreatLevel() const
{
    float TotalThreat = 0.0f;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        TotalThreat += Detection.Intensity * GetDinosaurThreatLevel(Detection.Source);
    }
    
    return FMath::Clamp(TotalThreat, 0.0f, 1.0f);
}