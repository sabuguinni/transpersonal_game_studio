#include "DinosaurAudioDetector.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UDinosaurAudioDetector::UDinosaurAudioDetector()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Default detection parameters
    DetectionRadius = 2000.0f; // 20 meters
    UpdateFrequency = 4.0f; // 4 times per second
    UpdateInterval = 1.0f / UpdateFrequency;
    LastUpdateTime = 0.0f;

    // Audio response parameters
    TensionIncreaseRate = 2.0f;
    TensionDecreaseRate = 0.5f;

    // Initialize state
    CurrentThreatLevel = 0.0f;
    bAnyDinosaursDetected = false;
}

void UDinosaurAudioDetector::BeginPlay()
{
    Super::BeginPlay();

    // Find AudioManager if not set
    if (!AudioManager)
    {
        AudioManager = Cast<AAudioManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAudioManager::StaticClass()));
    }
}

void UDinosaurAudioDetector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastUpdateTime += DeltaTime;

    // Update detection at specified frequency
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateDinosaurDetection();
        LastUpdateTime = 0.0f;
    }
}

void UDinosaurAudioDetector::UpdateDinosaurDetection()
{
    if (!GetWorld())
        return;

    NearbyDinosaurs.Empty();

    // Get all actors in detection radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), DinosaurBaseClass ? DinosaurBaseClass : AActor::StaticClass(), FoundActors);

    FVector PlayerLocation = GetOwner()->GetActorLocation();

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == GetOwner())
            continue;

        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());

        if (Distance <= DetectionRadius)
        {
            FDinosaurAudioData DinosaurData;
            DinosaurData.DinosaurActor = Actor;
            DinosaurData.Distance = Distance;

            // Try to get dinosaur properties from tags or components
            // This is a simplified implementation - in practice you'd have a proper dinosaur component
            if (Actor->Tags.Contains("Carnivore"))
            {
                DinosaurData.bIsHerbivore = false;
                DinosaurData.ThreatLevel = 0.8f;
            }
            else if (Actor->Tags.Contains("Herbivore"))
            {
                DinosaurData.bIsHerbivore = true;
                DinosaurData.ThreatLevel = 0.2f;
            }
            else
            {
                // Default values
                DinosaurData.bIsHerbivore = true;
                DinosaurData.ThreatLevel = 0.3f;
            }

            // Check if aggressive (could be based on AI state)
            DinosaurData.bIsAggressive = Actor->Tags.Contains("Aggressive");
            if (DinosaurData.bIsAggressive)
            {
                DinosaurData.ThreatLevel *= 1.5f;
            }

            NearbyDinosaurs.Add(DinosaurData);
        }
    }

    bAnyDinosaursDetected = NearbyDinosaurs.Num() > 0;

    // Calculate overall threat level
    CalculateThreatLevel();

    // Update audio system
    UpdateAudioSystem();
}

void UDinosaurAudioDetector::CalculateThreatLevel()
{
    if (NearbyDinosaurs.Num() == 0)
    {
        CurrentThreatLevel = 0.0f;
        return;
    }

    float MaxThreat = 0.0f;
    float CumulativeThreat = 0.0f;

    for (const FDinosaurAudioData& DinosaurData : NearbyDinosaurs)
    {
        float IndividualThreat = CalculateIndividualThreat(DinosaurData);
        MaxThreat = FMath::Max(MaxThreat, IndividualThreat);
        CumulativeThreat += IndividualThreat * 0.3f; // Multiple dinosaurs add to tension
    }

    // Combine max threat with cumulative threat
    CurrentThreatLevel = FMath::Clamp(MaxThreat + CumulativeThreat, 0.0f, 1.0f);
}

float UDinosaurAudioDetector::CalculateIndividualThreat(const FDinosaurAudioData& DinosaurData) const
{
    float BaseThreat = DinosaurData.ThreatLevel;

    // Distance falloff
    float DistanceFactor = 1.0f - (DinosaurData.Distance / DetectionRadius);
    DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);

    // Apply distance falloff with exponential curve for more dramatic close encounters
    DistanceFactor = FMath::Pow(DistanceFactor, 0.5f);

    return BaseThreat * DistanceFactor;
}

void UDinosaurAudioDetector::UpdateAudioSystem()
{
    if (!AudioManager)
        return;

    // Notify audio manager about dinosaur detection
    AudioManager->OnDinosaurDetected(bAnyDinosaursDetected, GetClosestDinosaurDistance());

    // Set audio state based on threat level
    if (CurrentThreatLevel > 0.8f)
    {
        AudioManager->SetAudioState(EAudioState::Danger, 0.5f);
    }
    else if (CurrentThreatLevel > 0.5f)
    {
        AudioManager->SetAudioState(EAudioState::Tension, 1.0f);
    }
    else if (CurrentThreatLevel > 0.2f)
    {
        AudioManager->SetAudioState(EAudioState::Exploration, 2.0f);
    }
    else if (bAnyDinosaursDetected)
    {
        AudioManager->SetAudioState(EAudioState::Discovery, 1.5f);
    }
}

void UDinosaurAudioDetector::SetDetectionRadius(float NewRadius)
{
    DetectionRadius = FMath::Max(NewRadius, 100.0f); // Minimum 1 meter
}

float UDinosaurAudioDetector::GetClosestDinosaurDistance() const
{
    if (NearbyDinosaurs.Num() == 0)
        return -1.0f;

    float ClosestDistance = FLT_MAX;
    for (const FDinosaurAudioData& DinosaurData : NearbyDinosaurs)
    {
        ClosestDistance = FMath::Min(ClosestDistance, DinosaurData.Distance);
    }

    return ClosestDistance;
}

float UDinosaurAudioDetector::GetHighestThreatLevel() const
{
    if (NearbyDinosaurs.Num() == 0)
        return 0.0f;

    float HighestThreat = 0.0f;
    for (const FDinosaurAudioData& DinosaurData : NearbyDinosaurs)
    {
        float IndividualThreat = CalculateIndividualThreat(DinosaurData);
        HighestThreat = FMath::Max(HighestThreat, IndividualThreat);
    }

    return HighestThreat;
}

void UDinosaurAudioDetector::SetAudioManager(AAudioManager* NewAudioManager)
{
    AudioManager = NewAudioManager;
}