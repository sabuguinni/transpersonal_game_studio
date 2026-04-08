#include "Audio3DDetectionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "../Characters/DinosaurCharacter.h"
#include "../AI/DinosaurAIController.h"

UAudio3DDetectionSystem::UAudio3DDetectionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS default
    
    // Default values
    MaxDetectionRange = 5000.0f;
    MinAudibleIntensity = 0.1f;
    DetectionUpdateRate = 10.0f;
    bUseOcclusion = true;
    bUseDirectionalHearing = true;
    bDetectAllDinosaurs = true;
    DetectionSensitivity = 1.0f;
    
    // Calculate update interval
    DetectionUpdateInterval = 1.0f / DetectionUpdateRate;
}

void UAudio3DDetectionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize detection data
    CurrentDetections.Empty();
    LastDetectionTimes.Empty();
    LastDetectionUpdate = GetWorld()->GetTimeSeconds();
    
    // Update tick interval based on detection rate
    PrimaryComponentTick.TickInterval = DetectionUpdateInterval;
    
    UE_LOG(LogTemp, Log, TEXT("Audio3DDetectionSystem initialized with range: %f"), MaxDetectionRange);
}

void UAudio3DDetectionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastDetectionUpdate >= DetectionUpdateInterval)
    {
        UpdateDetections();
        LastDetectionUpdate = CurrentTime;
    }
}

void UAudio3DDetectionSystem::UpdateDetections()
{
    if (!GetWorld() || !GetOwner())
        return;
        
    TArray<FAudioDetectionData> PreviousDetections = CurrentDetections;
    CurrentDetections.Empty();
    
    // Find all dinosaurs in range
    TArray<AActor*> FoundDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurCharacter::StaticClass(), FoundDinosaurs);
    
    FVector ListenerLocation = GetOwner()->GetActorLocation();
    
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
    
    // Update threat and loudest sound
    UpdateClosestThreat();
    UpdateLoudestSound();
    
    // Clean up old detections
    CleanupOldDetections();
    
    // Fire events for new detections
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        bool bIsNewDetection = true;
        for (const FAudioDetectionData& PrevDetection : PreviousDetections)
        {
            if (PrevDetection.Source == Detection.Source)
            {
                bIsNewDetection = false;
                OnDetectionUpdated.Broadcast(Detection);
                break;
            }
        }
        
        if (bIsNewDetection)
        {
            OnNewDetection.Broadcast(Detection);
        }
    }
    
    // Fire events for lost detections
    for (const FAudioDetectionData& PrevDetection : PreviousDetections)
    {
        bool bStillDetected = false;
        for (const FAudioDetectionData& Detection : CurrentDetections)
        {
            if (Detection.Source == PrevDetection.Source)
            {
                bStillDetected = true;
                break;
            }
        }
        
        if (!bStillDetected && PrevDetection.Source)
        {
            OnDetectionLost.Broadcast(Cast<ADinosaur>(PrevDetection.Source));
        }
    }
}

void UAudio3DDetectionSystem::ProcessDinosaur(ADinosaurCharacter* Dinosaur)
{
    if (!Dinosaur || !GetOwner())
        return;
        
    FVector ListenerLocation = GetOwner()->GetActorLocation();
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    float Distance = FVector::Dist(ListenerLocation, DinosaurLocation);
    
    // Check if in range
    if (Distance > MaxDetectionRange)
        return;
        
    // Calculate audio intensity
    float AudioIntensity = CalculateAudioIntensity(Dinosaur, Distance);
    
    // Apply sensitivity modifier
    AudioIntensity *= DetectionSensitivity;
    
    // Check if audible
    if (AudioIntensity < MinAudibleIntensity)
        return;
        
    // Check occlusion if enabled
    if (bUseOcclusion && IsOccluded(ListenerLocation, DinosaurLocation))
    {
        AudioIntensity *= 0.3f; // Reduce intensity for occluded sounds
        if (AudioIntensity < MinAudibleIntensity)
            return;
    }
    
    // Create detection data
    FAudioDetectionData DetectionData = CreateDetectionData(Dinosaur);
    DetectionData.Distance = Distance;
    DetectionData.Intensity = AudioIntensity;
    DetectionData.Direction = (DinosaurLocation - ListenerLocation).GetSafeNormal();
    
    // Check visibility (simple line trace)
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        ListenerLocation,
        DinosaurLocation,
        ECC_Visibility
    );
    
    DetectionData.bIsVisible = !bHit || HitResult.GetActor() == Dinosaur;
    
    // Update time since detected
    float* LastTime = LastDetectionTimes.Find(Dinosaur);
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (LastTime)
    {
        DetectionData.TimeSinceDetected = CurrentTime - *LastTime;
    }
    else
    {
        DetectionData.TimeSinceDetected = 0.0f;
        LastDetectionTimes.Add(Dinosaur, CurrentTime);
    }
    
    CurrentDetections.Add(DetectionData);
}

FAudioDetectionData UAudio3DDetectionSystem::CreateDetectionData(ADinosaurCharacter* Dinosaur)
{
    FAudioDetectionData Data;
    Data.Source = Cast<ADinosaur>(Dinosaur);
    Data.Direction = FVector::ZeroVector;
    Data.Distance = 0.0f;
    Data.Intensity = 0.0f;
    Data.bIsVisible = false;
    Data.TimeSinceDetected = 0.0f;
    return Data;
}

float UAudio3DDetectionSystem::CalculateAudioIntensity(ADinosaurCharacter* Dinosaur, float Distance) const
{
    if (!Dinosaur)
        return 0.0f;
        
    // Base intensity based on dinosaur size/threat level
    float BaseIntensity = GetDinosaurThreatLevel(Dinosaur);
    
    // Apply distance attenuation
    float DistanceAttenuation = GetDistanceAttenuation(Distance);
    
    // Check if dinosaur is making noise (moving, vocalizing, etc.)
    float ActivityMultiplier = 1.0f;
    
    // Get dinosaur velocity for movement noise
    FVector Velocity = Dinosaur->GetVelocity();
    float Speed = Velocity.Size();
    if (Speed > 100.0f) // Moving
    {
        ActivityMultiplier *= FMath::Clamp(Speed / 600.0f, 1.0f, 2.0f);
    }
    
    // Check if dinosaur is in combat or aggressive state
    if (ADinosaurAIController* AIController = Cast<ADinosaurAIController>(Dinosaur->GetController()))
    {
        // Assume we have a way to check AI state
        // ActivityMultiplier *= AIController->IsInCombat() ? 2.0f : 1.0f;
    }
    
    return BaseIntensity * DistanceAttenuation * ActivityMultiplier;
}

bool UAudio3DDetectionSystem::IsOccluded(const FVector& Start, const FVector& End) const
{
    if (!GetWorld())
        return false;
        
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_WorldStatic,
        QueryParams
    );
    
    return bHit;
}

bool UAudio3DDetectionSystem::ShouldDetectDinosaur(ADinosaurCharacter* Dinosaur) const
{
    if (!Dinosaur)
        return false;
        
    // Check if we should detect all dinosaurs
    if (bDetectAllDinosaurs)
        return true;
        
    // Check against specific types
    for (const TSubclassOf<ADinosaur>& DinosaurType : DetectableDinosaurTypes)
    {
        if (Dinosaur->IsA(DinosaurType))
            return true;
    }
    
    return false;
}

void UAudio3DDetectionSystem::UpdateClosestThreat()
{
    ClosestThreat = FAudioDetectionData();
    float ClosestDistance = MAX_FLT;
    
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        float ThreatLevel = GetDinosaurThreatLevel(Cast<ADinosaurCharacter>(Detection.Source));
        if (ThreatLevel > 0.5f && Detection.Distance < ClosestDistance)
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
    float CurrentTime = GetWorld()->GetTimeSeconds();
    const float MaxAge = 5.0f; // Remove detections older than 5 seconds
    
    TArray<ADinosaurCharacter*> ToRemove;
    for (auto& Pair : LastDetectionTimes)
    {
        if (CurrentTime - Pair.Value > MaxAge)
        {
            ToRemove.Add(Pair.Key);
        }
    }
    
    for (ADinosaurCharacter* Dinosaur : ToRemove)
    {
        LastDetectionTimes.Remove(Dinosaur);
    }
}

float UAudio3DDetectionSystem::GetDinosaurThreatLevel(ADinosaurCharacter* Dinosaur) const
{
    if (!Dinosaur)
        return 0.0f;
        
    // This would be based on dinosaur properties
    // For now, return a default value based on size or type
    return 0.7f; // Medium threat level
}

float UAudio3DDetectionSystem::GetDistanceAttenuation(float Distance) const
{
    if (Distance <= 0.0f)
        return 1.0f;
        
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
        if (Detection.Distance <= MaxDistance)
        {
            float ThreatLevel = GetDinosaurThreatLevel(Cast<ADinosaurCharacter>(Detection.Source));
            if (ThreatLevel > 0.5f)
            {
                NearbyThreats.Add(Detection);
            }
        }
    }
    
    return NearbyThreats;
}

bool UAudio3DDetectionSystem::IsThreateningDinosaurNearby(float MaxDistance) const
{
    for (const FAudioDetectionData& Detection : CurrentDetections)
    {
        if (Detection.Distance <= MaxDistance)
        {
            float ThreatLevel = GetDinosaurThreatLevel(Cast<ADinosaurCharacter>(Detection.Source));
            if (ThreatLevel > 0.5f)
            {
                return true;
            }
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
        float ThreatLevel = GetDinosaurThreatLevel(Cast<ADinosaurCharacter>(Detection.Source));
        float DistanceModifier = 1.0f - (Detection.Distance / MaxDetectionRange);
        TotalThreat += ThreatLevel * DistanceModifier * Detection.Intensity;
    }
    
    return FMath::Clamp(TotalThreat, 0.0f, 1.0f);
}