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
    
    // Initialize detection arrays
    CurrentDetections.Empty();
    LastDetectionTimes.Empty();
    
    // Start detection updates
    LastDetectionUpdate = GetWorld()->GetTimeSeconds();
}

void UAudio3DDetectionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update detections at specified rate
    if (CurrentTime - LastDetectionUpdate >= DetectionUpdateInterval)
    {
        UpdateDetections();
        LastDetectionUpdate = CurrentTime;
    }
    
    // Clean up old detections
    CleanupOldDetections();
}

void UAudio3DDetectionSystem::UpdateDetections()
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<FAudioDetectionData> NewDetections;
    
    // Find all dinosaurs in range
    TArray<AActor*> FoundDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurCharacter::StaticClass(), FoundDinosaurs);
    
    for (AActor* Actor : FoundDinosaurs)
    {
        if (ADinosaurCharacter* Dinosaur = Cast<ADinosaurCharacter>(Actor))
        {
            if (ShouldDetectDinosaur(Dinosaur))
            {
                ProcessDinosaur(Dinosaur);
            }
        }
    }
    
    // Update closest threat and loudest sound
    UpdateClosestThreat();
    UpdateLoudestSound();
}

void UAudio3DDetectionSystem::ProcessDinosaur(ADinosaurCharacter* Dinosaur)
{
    if (!Dinosaur || !GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    float Distance = FVector::Dist(OwnerLocation, DinosaurLocation);
    
    // Check if within detection range
    if (Distance > MaxDetectionRange)
    {
        return;
    }
    
    // Create detection data
    FAudioDetectionData DetectionData = CreateDetectionData(Dinosaur);
    
    // Check if this is a new detection or update
    bool bIsNewDetection = !LastDetectionTimes.Contains(Dinosaur);
    
    if (bIsNewDetection)
    {
        CurrentDetections.Add(DetectionData);
        OnNewDetection.Broadcast(DetectionData);
    }
    else
    {
        // Update existing detection
        for (FAudioDetectionData& ExistingDetection : CurrentDetections)
        {
            if (ExistingDetection.Source == Dinosaur)
            {
                ExistingDetection = DetectionData;
                OnDetectionUpdated.Broadcast(DetectionData);
                break;
            }
        }
    }
    
    LastDetectionTimes.Add(Dinosaur, GetWorld()->GetTimeSeconds());
}

FAudioDetectionData UAudio3DDetectionSystem::CreateDetectionData(ADinosaurCharacter* Dinosaur)
{
    FAudioDetectionData DetectionData;
    
    if (!Dinosaur || !GetOwner())
    {
        return DetectionData;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    
    DetectionData.Source = Dinosaur;
    DetectionData.Direction = (DinosaurLocation - OwnerLocation).GetSafeNormal();
    DetectionData.Distance = FVector::Dist(OwnerLocation, DinosaurLocation);
    DetectionData.Intensity = CalculateAudioIntensity(Dinosaur, DetectionData.Distance);
    DetectionData.bIsVisible = !IsOccluded(OwnerLocation, DinosaurLocation);
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
    float DistanceAttenuation = GetDistanceAttenuation(Distance);
    
    // Apply sensitivity modifier
    float FinalIntensity = BaseIntensity * DistanceAttenuation * DetectionSensitivity;
    
    return FMath::Clamp(FinalIntensity, 0.0f, 1.0f);
}

bool UAudio3DDetectionSystem::IsOccluded(const FVector& Start, const FVector& End) const
{
    if (!bUseOcclusion || !GetWorld())
    {
        return false;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
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
    
    // Check if we should detect all dinosaurs
    if (bDetectAllDinosaurs)
    {
        return true;
    }
    
    // Check if this dinosaur type is in our detection list
    for (TSubclassOf<ADinosaurCharacter> DetectableType : DetectableDinosaurTypes)
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
        if (Detection.Intensity >= MinAudibleIntensity && Detection.Distance < ClosestDistance)
        {
            ClosestThreat = Detection;
            ClosestDistance = Detection.Distance;
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
            LoudestSound = Detection;
            HighestIntensity = Detection.Intensity;
        }
    }
}

void UAudio3DDetectionSystem::CleanupOldDetections()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    const float MaxDetectionAge = 5.0f; // Remove detections older than 5 seconds
    
    for (int32 i = CurrentDetections.Num() - 1; i >= 0; --i)
    {
        const FAudioDetectionData& Detection = CurrentDetections[i];
        
        if (Detection.Source && LastDetectionTimes.Contains(Detection.Source))
        {
            float LastDetectionTime = LastDetectionTimes[Detection.Source];
            if (CurrentTime - LastDetectionTime > MaxDetectionAge)
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

float UAudio3DDetectionSystem::GetDinosaurThreatLevel(ADinosaurCharacter* Dinosaur) const
{
    if (!Dinosaur)
    {
        return 0.0f;
    }
    
    // This should be implemented based on dinosaur properties
    // For now, return a default value
    return 0.7f;
}

float UAudio3DDetectionSystem::GetDistanceAttenuation(float Distance) const
{
    if (Distance <= 0.0f)
    {
        return 1.0f;
    }
    
    // Logarithmic falloff
    float NormalizedDistance = Distance / MaxDetectionRange;
    return FMath::Clamp(1.0f - (NormalizedDistance * NormalizedDistance), 0.0f, 1.0f);
}

// Public interface functions
TArray<FAudioDetectionData> UAudio3DDetectionSystem::GetNearbyThreats(float MaxDistance) const
{
    TArray<FAudioDetectionData> NearbyThreats;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        if (Detection.Distance <= MaxDistance && Detection.Intensity >= MinAudibleIntensity)
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
        if (Detection.Distance <= MaxDistance && Detection.Intensity >= 0.5f)
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
    DetectionSensitivity = FMath::Clamp(Sensitivity, 0.0f, 2.0f);
}

float UAudio3DDetectionSystem::GetTotalThreatLevel() const
{
    float TotalThreat = 0.0f;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        TotalThreat += Detection.Intensity;
    }
    
    return FMath::Clamp(TotalThreat, 0.0f, 1.0f);
}