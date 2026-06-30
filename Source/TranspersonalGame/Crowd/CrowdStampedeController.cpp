#include "CrowdStampedeController.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// UCrowdHerdComponent Implementation
// ============================================================

UCrowdHerdComponent::UCrowdHerdComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    HerdState = ECrowd_HerdState::Grazing;
    Species = ECrowd_HerbivoreSpecies::Triceratops;
    HerdSize = 12;
    DetectionRadius = 1500.f;
    CohesionRadius = 400.f;
    SeparationRadius = 120.f;
    bIsStampeding = false;
    StampedeTimer = 0.f;
    GrazingTimer = 0.f;
}

void UCrowdHerdComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCrowdHerdComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsStampeding)
    {
        StampedeTimer -= DeltaTime;
        if (StampedeTimer <= 0.f)
        {
            CalmHerd();
        }
        else
        {
            UpdateFlockingBehavior(DeltaTime);
        }
    }
    else
    {
        GrazingTimer += DeltaTime;
        // Periodic wander every 8-12 seconds
        if (GrazingTimer > FMath::RandRange(8.f, 12.f))
        {
            GrazingTimer = 0.f;
            if (HerdState == ECrowd_HerdState::Grazing)
            {
                HerdState = ECrowd_HerdState::Wandering;
            }
            else if (HerdState == ECrowd_HerdState::Wandering)
            {
                HerdState = ECrowd_HerdState::Grazing;
            }
        }
    }
}

void UCrowdHerdComponent::InitializeHerd(ECrowd_HerbivoreSpecies InSpecies, int32 InSize, FVector CenterLocation)
{
    Species = InSpecies;
    HerdSize = FMath::Clamp(InSize, 1, 50);
    HerdMembers.Empty();

    for (int32 i = 0; i < HerdSize; i++)
    {
        FCrowd_HerdMember Member;
        float Angle = (float)i / (float)HerdSize * 2.f * PI;
        float Radius = FMath::RandRange(100.f, 600.f);
        Member.Location = CenterLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.f
        );
        Member.Health = 100.f;
        Member.bIsAlpha = (i == 0);
        Member.Species = InSpecies;
        Member.Velocity = FVector::ZeroVector;
        HerdMembers.Add(Member);
    }

    HerdState = ECrowd_HerdState::Grazing;
    UE_LOG(LogTemp, Log, TEXT("CrowdHerd: Initialized herd of %d %s members"), HerdSize, *UEnum::GetValueAsString(InSpecies));
}

void UCrowdHerdComponent::TriggerStampede(FVector ThreatLocation)
{
    if (HerdMembers.Num() == 0) return;

    // Calculate flee direction — away from threat
    FVector HerdCenter = FVector::ZeroVector;
    for (const FCrowd_HerdMember& M : HerdMembers)
    {
        HerdCenter += M.Location;
    }
    HerdCenter /= (float)HerdMembers.Num();

    ActiveStampede.ThreatLocation = ThreatLocation;
    ActiveStampede.FleeDirection = (HerdCenter - ThreatLocation).GetSafeNormal();
    ActiveStampede.StampedeSpeed = 800.f;
    ActiveStampede.PanicRadius = 2000.f;
    ActiveStampede.Duration = FMath::RandRange(12.f, 20.f);

    StampedeTimer = ActiveStampede.Duration;
    bIsStampeding = true;
    HerdState = ECrowd_HerdState::Stampeding;

    UE_LOG(LogTemp, Log, TEXT("CrowdHerd: STAMPEDE triggered! Flee direction: %s, Duration: %.1fs"),
        *ActiveStampede.FleeDirection.ToString(), ActiveStampede.Duration);
}

void UCrowdHerdComponent::CalmHerd()
{
    bIsStampeding = false;
    StampedeTimer = 0.f;
    HerdState = ECrowd_HerdState::Grazing;
    ActiveStampede = FCrowd_StampedeData();
    UE_LOG(LogTemp, Log, TEXT("CrowdHerd: Herd calmed down, returning to grazing"));
}

void UCrowdHerdComponent::UpdateFlockingBehavior(float DeltaTime)
{
    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        FCrowd_HerdMember& Member = HerdMembers[i];

        FVector Cohesion = CalculateCohesionForce(i);
        FVector Separation = CalculateSeparationForce(i);
        FVector Alignment = CalculateAlignmentForce(i);
        FVector FleeForce = ActiveStampede.FleeDirection * ActiveStampede.StampedeSpeed;

        // Weighted combination: flee dominates during stampede
        FVector TotalForce = FleeForce * 0.6f + Cohesion * 0.2f + Separation * 0.15f + Alignment * 0.05f;

        Member.Velocity = FMath::VInterpTo(Member.Velocity, TotalForce, DeltaTime, 3.f);
        Member.Location += Member.Velocity * DeltaTime;
    }
}

FVector UCrowdHerdComponent::CalculateCohesionForce(int32 MemberIndex) const
{
    if (HerdMembers.Num() <= 1) return FVector::ZeroVector;

    FVector Center = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdMember& Self = HerdMembers[MemberIndex];

    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        if (i == MemberIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdMembers[i].Location);
        if (Dist < CohesionRadius)
        {
            Center += HerdMembers[i].Location;
            Count++;
        }
    }

    if (Count == 0) return FVector::ZeroVector;
    Center /= (float)Count;
    return (Center - Self.Location).GetSafeNormal() * 200.f;
}

FVector UCrowdHerdComponent::CalculateSeparationForce(int32 MemberIndex) const
{
    FVector SeparationForce = FVector::ZeroVector;
    const FCrowd_HerdMember& Self = HerdMembers[MemberIndex];

    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        if (i == MemberIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdMembers[i].Location);
        if (Dist < SeparationRadius && Dist > 0.f)
        {
            FVector Away = (Self.Location - HerdMembers[i].Location).GetSafeNormal();
            SeparationForce += Away * (SeparationRadius / Dist);
        }
    }

    return SeparationForce * 150.f;
}

FVector UCrowdHerdComponent::CalculateAlignmentForce(int32 MemberIndex) const
{
    if (HerdMembers.Num() <= 1) return FVector::ZeroVector;

    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdMember& Self = HerdMembers[MemberIndex];

    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        if (i == MemberIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdMembers[i].Location);
        if (Dist < CohesionRadius)
        {
            AvgVelocity += HerdMembers[i].Velocity;
            Count++;
        }
    }

    if (Count == 0) return FVector::ZeroVector;
    AvgVelocity /= (float)Count;
    return AvgVelocity.GetSafeNormal() * 100.f;
}

// ============================================================
// ACrowdStampedeController Implementation
// ============================================================

ACrowdStampedeController::ACrowdStampedeController()
{
    PrimaryActorTick.bCanEverTick = true;

    ThreatDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ThreatDetectionSphere"));
    ThreatDetectionSphere->SetSphereRadius(3000.f);
    ThreatDetectionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
    RootComponent = ThreatDetectionSphere;

    HerdComponent = CreateDefaultSubobject<UCrowdHerdComponent>(TEXT("HerdComponent"));

    GlobalThreatRadius = 3000.f;
    MaxSimultaneousHerds = 10;
}

void ACrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();

    ThreatDetectionSphere->OnComponentBeginOverlap.AddDynamic(
        this, &ACrowdStampedeController::OnThreatSphereOverlap);

    // Auto-initialize herd at spawn location
    if (HerdComponent)
    {
        HerdComponent->InitializeHerd(HerdComponent->Species, HerdComponent->HerdSize, GetActorLocation());
    }
}

void ACrowdStampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACrowdStampedeController::BroadcastThreatToNearbyHerds(FVector ThreatLocation, float ThreatRadius)
{
    for (ACrowdStampedeController* NearbyHerd : NearbyHerds)
    {
        if (!NearbyHerd) continue;
        float Dist = FVector::Dist(GetActorLocation(), NearbyHerd->GetActorLocation());
        if (Dist < ThreatRadius && NearbyHerd->HerdComponent)
        {
            NearbyHerd->HerdComponent->TriggerStampede(ThreatLocation);
            UE_LOG(LogTemp, Log, TEXT("CrowdStampede: Broadcast threat to herd at %s"),
                *NearbyHerd->GetActorLocation().ToString());
        }
    }
}

void ACrowdStampedeController::OnPredatorEnterRange(AActor* PredatorActor)
{
    if (!PredatorActor || !HerdComponent) return;

    FVector ThreatLoc = PredatorActor->GetActorLocation();
    HerdComponent->TriggerStampede(ThreatLoc);
    BroadcastThreatToNearbyHerds(ThreatLoc, GlobalThreatRadius);

    UE_LOG(LogTemp, Log, TEXT("CrowdStampede: Predator '%s' triggered stampede at %s"),
        *PredatorActor->GetName(), *ThreatLoc.ToString());
}

void ACrowdStampedeController::SpawnHerdAtLocation(FVector Location, ECrowd_HerbivoreSpecies Species, int32 Count)
{
    if (HerdComponent)
    {
        HerdComponent->InitializeHerd(Species, Count, Location);
        SetActorLocation(Location);
    }
}

void ACrowdStampedeController::OnThreatSphereOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    // Check if the overlapping actor is a predator (DinosaurCombatAI or player)
    FString ActorName = OtherActor->GetName().ToLower();
    bool bIsThreat = ActorName.Contains(TEXT("raptor")) ||
                     ActorName.Contains(TEXT("trex")) ||
                     ActorName.Contains(TEXT("predator")) ||
                     ActorName.Contains(TEXT("player")) ||
                     ActorName.Contains(TEXT("character"));

    if (bIsThreat)
    {
        OnPredatorEnterRange(OtherActor);
    }
}
