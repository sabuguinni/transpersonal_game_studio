#include "Combat_ThreatAssessmentSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ACombat_ThreatAssessmentSystem::ACombat_ThreatAssessmentSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    AssessmentUpdateRate = 0.5f;
    MaxDetectionRange = 2000.0f;
    bVisualizeThreatZones = true;
    PreviousThreatLevel = ECombat_ThreatLevel::None;

    InitializeComponents();
}

void ACombat_ThreatAssessmentSystem::InitializeComponents()
{
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(MaxDetectionRange);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    DetectionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    VisualizationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualizationMesh->SetVisibility(bVisualizeThreatZones);

    // Try to load sphere mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        VisualizationMesh->SetStaticMesh(SphereMeshAsset.Object);
        VisualizationMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    SetupDetectionSphere();
}

void ACombat_ThreatAssessmentSystem::SetupDetectionSphere()
{
    if (DetectionSphere)
    {
        DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACombat_ThreatAssessmentSystem::OnDetectionSphereBeginOverlap);
        DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ACombat_ThreatAssessmentSystem::OnDetectionSphereEndOverlap);
    }
}

void ACombat_ThreatAssessmentSystem::BeginPlay()
{
    Super::BeginPlay();

    // Start threat assessment timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AssessmentTimerHandle,
            this,
            &ACombat_ThreatAssessmentSystem::UpdateThreatAssessment,
            AssessmentUpdateRate,
            true
        );
    }

    UpdateVisualization();
}

void ACombat_ThreatAssessmentSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update visualization if enabled
    if (bVisualizeThreatZones)
    {
        UpdateVisualization();
    }
}

void ACombat_ThreatAssessmentSystem::UpdateThreatAssessment()
{
    if (!GetWorld())
    {
        return;
    }

    // Clear old threat data
    CurrentThreatData.ThreatLevel = ECombat_ThreatLevel::None;
    CurrentThreatData.ThreatIntensity = 0.0f;
    CurrentThreatData.bIsActiveThreat = false;

    float HighestThreatValue = 0.0f;
    AActor* PrimaryThreat = nullptr;

    // Assess all tracked threats
    for (int32 i = TrackedThreats.Num() - 1; i >= 0; --i)
    {
        AActor* ThreatActor = TrackedThreats[i];
        
        if (!IsValid(ThreatActor))
        {
            TrackedThreats.RemoveAt(i);
            continue;
        }

        float Distance = FVector::Dist(GetActorLocation(), ThreatActor->GetActorLocation());
        if (Distance > MaxDetectionRange)
        {
            TrackedThreats.RemoveAt(i);
            OnThreatRemoved(ThreatActor);
            continue;
        }

        float ThreatValue = CalculateActorThreatValue(ThreatActor);
        float DistanceModifier = CalculateDistanceThreatModifier(Distance);
        float FinalThreatValue = ThreatValue * DistanceModifier;

        if (FinalThreatValue > HighestThreatValue)
        {
            HighestThreatValue = FinalThreatValue;
            PrimaryThreat = ThreatActor;
            CurrentThreatData.ThreatOrigin = ThreatActor->GetActorLocation();
        }
    }

    // Update threat level based on highest threat value
    ECombat_ThreatLevel NewThreatLevel = ECombat_ThreatLevel::None;
    if (HighestThreatValue > 0.8f)
    {
        NewThreatLevel = ECombat_ThreatLevel::Critical;
    }
    else if (HighestThreatValue > 0.6f)
    {
        NewThreatLevel = ECombat_ThreatLevel::High;
    }
    else if (HighestThreatValue > 0.4f)
    {
        NewThreatLevel = ECombat_ThreatLevel::Medium;
    }
    else if (HighestThreatValue > 0.2f)
    {
        NewThreatLevel = ECombat_ThreatLevel::Low;
    }

    CurrentThreatData.ThreatLevel = NewThreatLevel;
    CurrentThreatData.ThreatIntensity = HighestThreatValue;
    CurrentThreatData.bIsActiveThreat = (NewThreatLevel != ECombat_ThreatLevel::None);

    // Notify if threat level changed
    if (NewThreatLevel != PreviousThreatLevel)
    {
        OnThreatLevelChanged(NewThreatLevel);
        PreviousThreatLevel = NewThreatLevel;
    }
}

ECombat_ThreatLevel ACombat_ThreatAssessmentSystem::CalculateThreatLevel(AActor* ThreatActor)
{
    if (!IsValid(ThreatActor))
    {
        return ECombat_ThreatLevel::None;
    }

    float Distance = FVector::Dist(GetActorLocation(), ThreatActor->GetActorLocation());
    float ThreatValue = CalculateActorThreatValue(ThreatActor);
    float DistanceModifier = CalculateDistanceThreatModifier(Distance);
    float FinalThreatValue = ThreatValue * DistanceModifier;

    if (FinalThreatValue > 0.8f)
    {
        return ECombat_ThreatLevel::Critical;
    }
    else if (FinalThreatValue > 0.6f)
    {
        return ECombat_ThreatLevel::High;
    }
    else if (FinalThreatValue > 0.4f)
    {
        return ECombat_ThreatLevel::Medium;
    }
    else if (FinalThreatValue > 0.2f)
    {
        return ECombat_ThreatLevel::Low;
    }

    return ECombat_ThreatLevel::None;
}

void ACombat_ThreatAssessmentSystem::AddThreatTarget(AActor* NewThreat)
{
    if (IsValid(NewThreat) && !TrackedThreats.Contains(NewThreat))
    {
        TrackedThreats.Add(NewThreat);
        OnNewThreatDetected(NewThreat);
    }
}

void ACombat_ThreatAssessmentSystem::RemoveThreatTarget(AActor* ThreatToRemove)
{
    if (TrackedThreats.Contains(ThreatToRemove))
    {
        TrackedThreats.Remove(ThreatToRemove);
        OnThreatRemoved(ThreatToRemove);
    }
}

FCombat_ThreatData ACombat_ThreatAssessmentSystem::GetCurrentThreatData() const
{
    return CurrentThreatData;
}

TArray<AActor*> ACombat_ThreatAssessmentSystem::GetThreatsInRange(float Range) const
{
    TArray<AActor*> ThreatsInRange;
    
    for (AActor* Threat : TrackedThreats)
    {
        if (IsValid(Threat))
        {
            float Distance = FVector::Dist(GetActorLocation(), Threat->GetActorLocation());
            if (Distance <= Range)
            {
                ThreatsInRange.Add(Threat);
            }
        }
    }

    return ThreatsInRange;
}

bool ACombat_ThreatAssessmentSystem::IsLocationSafe(FVector Location, float SafetyRadius) const
{
    for (AActor* Threat : TrackedThreats)
    {
        if (IsValid(Threat))
        {
            float Distance = FVector::Dist(Location, Threat->GetActorLocation());
            if (Distance < SafetyRadius)
            {
                return false;
            }
        }
    }

    return true;
}

void ACombat_ThreatAssessmentSystem::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValid(OtherActor) && OtherActor != this)
    {
        // Check if actor is a potential threat (pawn, enemy, etc.)
        if (OtherActor->IsA<APawn>())
        {
            AddThreatTarget(OtherActor);
        }
    }
}

void ACombat_ThreatAssessmentSystem::OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValid(OtherActor))
    {
        RemoveThreatTarget(OtherActor);
    }
}

float ACombat_ThreatAssessmentSystem::CalculateDistanceThreatModifier(float Distance) const
{
    if (Distance <= 0.0f || MaxDetectionRange <= 0.0f)
    {
        return 1.0f;
    }

    // Closer threats are more dangerous
    float NormalizedDistance = Distance / MaxDetectionRange;
    return FMath::Clamp(1.0f - NormalizedDistance, 0.1f, 1.0f);
}

float ACombat_ThreatAssessmentSystem::CalculateActorThreatValue(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return 0.0f;
    }

    float BaseThreatValue = 0.5f;

    // Check actor name for threat indicators
    FString ActorName = Actor->GetName().ToLower();
    
    if (ActorName.Contains("trex") || ActorName.Contains("tyrannosaurus"))
    {
        BaseThreatValue = 1.0f;
    }
    else if (ActorName.Contains("raptor") || ActorName.Contains("velociraptor"))
    {
        BaseThreatValue = 0.8f;
    }
    else if (ActorName.Contains("carnotaurus") || ActorName.Contains("allosaurus"))
    {
        BaseThreatValue = 0.9f;
    }
    else if (ActorName.Contains("dinosaur") || ActorName.Contains("predator"))
    {
        BaseThreatValue = 0.7f;
    }
    else if (Actor->IsA<APawn>())
    {
        BaseThreatValue = 0.6f;
    }

    return BaseThreatValue;
}

void ACombat_ThreatAssessmentSystem::UpdateVisualization()
{
    if (!VisualizationMesh)
    {
        return;
    }

    // Update visibility
    VisualizationMesh->SetVisibility(bVisualizeThreatZones);

    if (!bVisualizeThreatZones)
    {
        return;
    }

    // Update color based on threat level
    FLinearColor ThreatColor = FLinearColor::Green;
    
    switch (CurrentThreatData.ThreatLevel)
    {
        case ECombat_ThreatLevel::Low:
            ThreatColor = FLinearColor::Yellow;
            break;
        case ECombat_ThreatLevel::Medium:
            ThreatColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
            break;
        case ECombat_ThreatLevel::High:
            ThreatColor = FLinearColor::Red;
            break;
        case ECombat_ThreatLevel::Critical:
            ThreatColor = FLinearColor(0.8f, 0.0f, 0.2f); // Dark Red
            break;
        default:
            ThreatColor = FLinearColor::Green;
            break;
    }

    // Update scale based on threat intensity
    float ScaleMultiplier = FMath::Lerp(0.3f, 1.0f, CurrentThreatData.ThreatIntensity);
    VisualizationMesh->SetWorldScale3D(FVector(ScaleMultiplier));
}