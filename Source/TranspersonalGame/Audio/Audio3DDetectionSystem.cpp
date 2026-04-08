#include "Audio3DDetectionSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UAudio3DDetectionSystem::UAudio3DDetectionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 updates per second by default
    
    // Initialize default values
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
    
    // Update detection interval based on rate
    DetectionUpdateInterval = 1.0f / FMath::Max(DetectionUpdateRate, 1.0f);
    
    // Initialize detection arrays
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
}

void UAudio3DDetectionSystem::UpdateDetections()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Clear previous detections
    TArray<FAudioDetectionData> PreviousDetections = CurrentDetections;
    CurrentDetections.Empty();
    
    // Find all dinosaurs in the world
    TArray<AActor*> FoundDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaur::StaticClass(), FoundDinosaurs);
    
    // Process each dinosaur
    for (AActor* Actor : FoundDinosaurs)
    {
        if (ADinosaur* Dinosaur = Cast<ADinosaur>(Actor))
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
    
    // Clean up old detections
    CleanupOldDetections();
    
    // Fire events for new/updated/lost detections
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        bool bWasDetected = PreviousDetections.ContainsByPredicate([&](const FAudioDetectionData& Prev)
        {
            return Prev.Source == Detection.Source;
        });
        
        if (bWasDetected)
        {
            OnDetectionUpdated.Broadcast(Detection);
        }
        else
        {
            OnNewDetection.Broadcast(Detection);
        }
    }
    
    // Check for lost detections
    for (const FAudioDetectionData& PrevDetection : PreviousDetections)
    {
        bool bStillDetected = CurrentDetections.ContainsByPredicate([&](const FAudioDetectionData& Current)
        {
            return Current.Source == PrevDetection.Source;
        });
        
        if (!bStillDetected)
        {
            OnDetectionLost.Broadcast(PrevDetection.Source);
        }
    }
}

void UAudio3DDetectionSystem::ProcessDinosaur(ADinosaur* Dinosaur)
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
    
    // Calculate audio intensity
    float AudioIntensity = CalculateAudioIntensity(Dinosaur, Distance);
    
    // Check if audible
    if (AudioIntensity < MinAudibleIntensity)
    {
        return;
    }
    
    // Check occlusion if enabled
    if (bUseOcclusion && IsOccluded(OwnerLocation, DinosaurLocation))
    {
        AudioIntensity *= 0.3f; // Reduce intensity for occluded sounds
        if (AudioIntensity < MinAudibleIntensity)
        {
            return;
        }
    }
    
    // Create detection data
    FAudioDetectionData DetectionData = CreateDetectionData(Dinosaur);
    DetectionData.Distance = Distance;
    DetectionData.Intensity = AudioIntensity;
    
    // Add to current detections
    CurrentDetections.Add(DetectionData);
    
    // Update last detection time
    LastDetectionTimes.Add(Dinosaur, GetWorld()->GetTimeSeconds());
}

FAudioDetectionData UAudio3DDetectionSystem::CreateDetectionData(ADinosaur* Dinosaur)
{
    FAudioDetectionData DetectionData;
    DetectionData.Source = Dinosaur;
    
    if (GetOwner())
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        FVector DinosaurLocation = Dinosaur->GetActorLocation();
        
        DetectionData.Direction = (DinosaurLocation - OwnerLocation).GetSafeNormal();
        DetectionData.Distance = FVector::Dist(OwnerLocation, DinosaurLocation);
    }
    
    // Check visibility with line trace
    if (GetWorld() && GetOwner())
    {
        FHitResult HitResult;
        FVector Start = GetOwner()->GetActorLocation();
        FVector End = Dinosaur->GetActorLocation();
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());
        QueryParams.AddIgnoredActor(Dinosaur);
        
        DetectionData.bIsVisible = !GetWorld()->LineTraceSingleByChannel(
            HitResult, Start, End, ECC_Visibility, QueryParams);
    }
    
    DetectionData.TimeSinceDetected = 0.0f;
    
    return DetectionData;
}

float UAudio3DDetectionSystem::CalculateAudioIntensity(ADinosaur* Dinosaur, float Distance) const
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
    if (!GetWorld())
    {
        return false;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    return GetWorld()->LineTraceSingleByChannel(
        HitResult, Start, End, ECC_WorldStatic, QueryParams);
}

bool UAudio3DDetectionSystem::ShouldDetectDinosaur(ADinosaur* Dinosaur) const
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
    float LoudestIntensity = 0.0f;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        if (Detection.Intensity > LoudestIntensity)
        {
            LoudestIntensity = Detection.Intensity;
            LoudestSound = Detection;
        }
    }
}

void UAudio3DDetectionSystem::CleanupOldDetections()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    const float MaxAge = 5.0f; // Remove detections older than 5 seconds
    
    TArray<ADinosaur*> KeysToRemove;
    
    for (auto& Pair : LastDetectionTimes)
    {
        if (CurrentTime - Pair.Value > MaxAge)
        {
            KeysToRemove.Add(Pair.Key);
        }
    }
    
    for (ADinosaur* Key : KeysToRemove)
    {
        LastDetectionTimes.Remove(Key);
    }
}

float UAudio3DDetectionSystem::GetDinosaurThreatLevel(ADinosaur* Dinosaur) const
{
    if (!Dinosaur)
    {
        return 0.0f;
    }
    
    // This would be implemented based on dinosaur properties
    // For now, return a default threat level
    return 0.7f;
}

float UAudio3DDetectionSystem::GetDistanceAttenuation(float Distance) const
{
    if (Distance <= 0.0f)
    {
        return 1.0f;
    }
    
    // Logarithmic attenuation
    float NormalizedDistance = Distance / MaxDetectionRange;
    return FMath::Clamp(1.0f - (NormalizedDistance * NormalizedDistance), 0.0f, 1.0f);
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
    DetectionSensitivity = FMath::Clamp(Sensitivity, 0.0f, 2.0f);
}

float UAudio3DDetectionSystem::GetTotalThreatLevel() const
{
    float TotalThreat = 0.0f;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        float ThreatLevel = GetDinosaurThreatLevel(Detection.Source);
        float DistanceModifier = 1.0f - (Detection.Distance / MaxDetectionRange);
        TotalThreat += ThreatLevel * DistanceModifier * Detection.Intensity;
    }
    
    return FMath::Clamp(TotalThreat, 0.0f, 1.0f);
}