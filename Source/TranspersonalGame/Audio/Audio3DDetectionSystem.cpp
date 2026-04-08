#include "Audio3DDetectionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "../Characters/DinosaurCharacter.h"

UAudio3DDetectionSystem::UAudio3DDetectionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance
    
    // Default settings
    MaxDetectionRange = 5000.0f;
    MinAudibleIntensity = 0.1f;
    DetectionUpdateRate = 10.0f;
    bUseOcclusion = true;
    bUseDirectionalHearing = true;
    bDetectAllDinosaurs = true;
    
    DetectionSensitivity = 1.0f;
    DetectionUpdateInterval = 1.0f / DetectionUpdateRate;
}

void UAudio3DDetectionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize detection data
    CurrentDetections.Empty();
    LastDetectionTimes.Empty();
    
    // Start detection updates
    LastDetectionUpdate = 0.0f;
}

void UAudio3DDetectionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Update detection at specified rate
    LastDetectionUpdate += DeltaTime;
    if (LastDetectionUpdate >= DetectionUpdateInterval)
    {
        UpdateDetections();
        LastDetectionUpdate = 0.0f;
    }
}

void UAudio3DDetectionSystem::UpdateDetections()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Clear old detections
    TArray<FAudioDetectionData> NewDetections;
    
    // Find all dinosaurs in range
    TArray<AActor*> FoundDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurCharacter::StaticClass(), FoundDinosaurs);
    
    for (AActor* Actor : FoundDinosaurs)
    {
        if (ADinosaurCharacter* Dinosaur = Cast<ADinosaurCharacter>(Actor))
        {
            ProcessDinosaur(Dinosaur);
        }
    }
    
    // Update closest threat and loudest sound
    UpdateClosestThreat();
    UpdateLoudestSound();
    
    // Clean up old detections
    CleanupOldDetections();
}

void UAudio3DDetectionSystem::ProcessDinosaur(ADinosaurCharacter* Dinosaur)
{
    if (!Dinosaur || !ShouldDetectDinosaur(Dinosaur))
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
    bool bIsOccluded = false;
    if (bUseOcclusion)
    {
        bIsOccluded = IsOccluded(OwnerLocation, DinosaurLocation);
    }
    
    // Create detection data
    FAudioDetectionData DetectionData = CreateDetectionData(Dinosaur);
    DetectionData.Distance = Distance;
    DetectionData.Intensity = AudioIntensity * (bIsOccluded ? 0.3f : 1.0f); // Reduce intensity if occluded
    DetectionData.Direction = (DinosaurLocation - OwnerLocation).GetSafeNormal();
    DetectionData.bIsVisible = !bIsOccluded;
    DetectionData.TimeSinceDetected = GetWorld()->GetTimeSeconds() - LastDetectionTimes.FindOrAdd(Dinosaur, GetWorld()->GetTimeSeconds());
    
    // Check if this is a new detection or update
    bool bIsNewDetection = true;
    for (int32 i = 0; i < CurrentDetections.Num(); i++)
    {
        if (CurrentDetections[i].Source == Dinosaur)
        {
            // Update existing detection
            CurrentDetections[i] = DetectionData;
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

FAudioDetectionData UAudio3DDetectionSystem::CreateDetectionData(ADinosaurCharacter* Dinosaur)
{
    FAudioDetectionData DetectionData;
    DetectionData.Source = Dinosaur;
    DetectionData.Direction = FVector::ZeroVector;
    DetectionData.Distance = 0.0f;
    DetectionData.Intensity = 0.0f;
    DetectionData.bIsVisible = false;
    DetectionData.TimeSinceDetected = 0.0f;
    
    return DetectionData;
}

float UAudio3DDetectionSystem::CalculateAudioIntensity(ADinosaurCharacter* Dinosaur, float Distance) const
{
    if (!Dinosaur)
    {
        return 0.0f;
    }
    
    // Base intensity from dinosaur threat level
    float BaseIntensity = GetDinosaurThreatLevel(Dinosaur);
    
    // Apply distance attenuation
    float AttenuatedIntensity = BaseIntensity * GetDistanceAttenuation(Distance);
    
    // Apply sensitivity modifier
    AttenuatedIntensity *= DetectionSensitivity;
    
    return FMath::Clamp(AttenuatedIntensity, 0.0f, 1.0f);
}

bool UAudio3DDetectionSystem::IsOccluded(const FVector& Start, const FVector& End) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    // Perform line trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );
    
    return bHit;
}

bool UAudio3DDetectionSystem::ShouldDetectDinosaur(ADinosaurCharacter* Dinosaur) const
{
    if (!Dinosaur)
    {
        return false;
    }
    
    // If detecting all dinosaurs, return true
    if (bDetectAllDinosaurs)
    {
        return true;
    }
    
    // Check if dinosaur type is in detectable list
    for (const TSubclassOf<ADinosaurCharacter>& DetectableType : DetectableDinosaurTypes)
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
    float ClosestDistance = MAX_FLT;
    
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
    const float MaxDetectionAge = 5.0f; // Remove detections older than 5 seconds
    
    for (int32 i = CurrentDetections.Num() - 1; i >= 0; i--)
    {
        const FAudioDetectionData& Detection = CurrentDetections[i];
        const float* LastDetectionTime = LastDetectionTimes.Find(Detection.Source);
        
        if (!LastDetectionTime || (CurrentTime - *LastDetectionTime) > MaxDetectionAge)
        {
            // Remove old detection
            OnDetectionLost.Broadcast(Detection.Source);
            CurrentDetections.RemoveAt(i);
            
            if (Detection.Source)
            {
                LastDetectionTimes.Remove(Detection.Source);
            }
        }
    }
}

float UAudio3DDetectionSystem::GetDinosaurThreatLevel(ADinosaurCharacter* Dinosaur) const
{
    if (!Dinosaur)
    {
        return 0.0f;
    }
    
    // This would be implemented based on dinosaur properties
    // For now, return a default threat level
    return 0.7f; // Medium threat
}

float UAudio3DDetectionSystem::GetDistanceAttenuation(float Distance) const
{
    if (Distance <= 0.0f)
    {
        return 1.0f;
    }
    
    // Linear attenuation over max detection range
    const float AttenuationFactor = 1.0f - (Distance / MaxDetectionRange);
    return FMath::Max(0.0f, AttenuationFactor);
}

// Public interface functions
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
    if (ClosestThreat.Source)
    {
        return ClosestThreat.Direction;
    }
    
    return FVector::ZeroVector;
}

void UAudio3DDetectionSystem::SetDetectionSensitivity(float Sensitivity)
{
    DetectionSensitivity = FMath::Clamp(Sensitivity, 0.0f, 1.0f);
}

float UAudio3DDetectionSystem::GetTotalThreatLevel() const
{
    float TotalThreat = 0.0f;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        const float ThreatLevel = GetDinosaurThreatLevel(Detection.Source);
        const float DistanceModifier = 1.0f - (Detection.Distance / MaxDetectionRange);
        TotalThreat += ThreatLevel * DistanceModifier * Detection.Intensity;
    }
    
    return FMath::Clamp(TotalThreat, 0.0f, 1.0f);
}