#include "CombatAIManager.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    ThreatScanRadius = 2000.0f;
    ThreatScanInterval = 1.0f;
    PackCoordinationRadius = 1500.0f;
    MaxPackSize = 6;
    bEnablePackHunting = true;
    bEnableTacticalRetreat = true;

    LastThreatScanTime = 0.0f;
    LastPackUpdateTime = 0.0f;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager initialized at location: %s"), *GetActorLocation().ToString());
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Periodic threat scanning
    if (CurrentTime - LastThreatScanTime >= ThreatScanInterval)
    {
        ScanForThreats();
        LastThreatScanTime = CurrentTime;
    }

    // Update pack formations every 2 seconds
    if (CurrentTime - LastPackUpdateTime >= 2.0f)
    {
        UpdatePackFormations();
        LastPackUpdateTime = CurrentTime;
    }
}

void ACombatAIManager::ScanForThreats()
{
    if (!GetWorld()) return;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    // Clear old threats
    ActiveThreats.Empty();

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == this) continue;

        APawn* Pawn = Cast<APawn>(Actor);
        if (!Pawn) continue;

        // Check if this is a player or hostile entity
        bool bIsPlayer = Pawn->IsPlayerControlled();
        bool bIsHostile = Actor->GetName().Contains(TEXT("TRex")) || 
                         Actor->GetName().Contains(TEXT("Raptor"));

        if (bIsPlayer || bIsHostile)
        {
            FCombat_ThreatAssessment Threat;
            Threat.ThreatActor = Actor;
            Threat.ThreatLocation = Actor->GetActorLocation();
            Threat.ThreatLevel = bIsPlayer ? 8.0f : 6.0f;
            Threat.LastAssessmentTime = GetWorld()->GetTimeSeconds();

            ActiveThreats.Add(Threat);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CombatAI: Scanned and found %d active threats"), ActiveThreats.Num());
}

void ACombatAIManager::UpdatePackFormations()
{
    if (!GetWorld()) return;

    TArray<AActor*> PredatorActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), PredatorActors);

    // Clear existing packs
    ActivePacks.Empty();

    TArray<AActor*> UnassignedPredators;
    
    // Find raptor-type predators
    for (AActor* Actor : PredatorActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Raptor")))
        {
            UnassignedPredators.Add(Actor);
        }
    }

    // Form packs from unassigned predators
    while (UnassignedPredators.Num() >= 2)
    {
        FCombat_PackFormation NewPack;
        NewPack.AlphaLeader = UnassignedPredators[0];
        NewPack.PackMembers.Add(UnassignedPredators[0]);
        NewPack.FormationCenter = UnassignedPredators[0]->GetActorLocation();

        UnassignedPredators.RemoveAt(0);

        // Add nearby predators to this pack
        for (int32 i = UnassignedPredators.Num() - 1; i >= 0; i--)
        {
            if (NewPack.PackMembers.Num() >= MaxPackSize) break;

            float Distance = FVector::Dist(NewPack.FormationCenter, UnassignedPredators[i]->GetActorLocation());
            if (Distance <= PackCoordinationRadius)
            {
                NewPack.PackMembers.Add(UnassignedPredators[i]);
                UnassignedPredators.RemoveAt(i);
            }
        }

        if (NewPack.PackMembers.Num() >= 2)
        {
            ActivePacks.Add(NewPack);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CombatAI: Formed %d active packs"), ActivePacks.Num());
}

void ACombatAIManager::RegisterCombatActor(AActor* Actor)
{
    if (!Actor) return;
    
    UE_LOG(LogTemp, Log, TEXT("CombatAI: Registered combat actor: %s"), *Actor->GetName());
}

void ACombatAIManager::UnregisterCombatActor(AActor* Actor)
{
    if (!Actor) return;
    
    // Remove from all packs
    for (FCombat_PackFormation& Pack : ActivePacks)
    {
        Pack.PackMembers.Remove(Actor);
        if (Pack.AlphaLeader == Actor && Pack.PackMembers.Num() > 0)
        {
            Pack.AlphaLeader = Pack.PackMembers[0];
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CombatAI: Unregistered combat actor: %s"), *Actor->GetName());
}

FCombat_ThreatAssessment ACombatAIManager::AssessThreat(AActor* PotentialThreat, AActor* Observer)
{
    FCombat_ThreatAssessment Assessment;
    
    if (!PotentialThreat || !Observer)
    {
        return Assessment;
    }

    Assessment.ThreatActor = PotentialThreat;
    Assessment.ThreatLocation = PotentialThreat->GetActorLocation();
    Assessment.LastAssessmentTime = GetWorld()->GetTimeSeconds();

    float Distance = FVector::Dist(Observer->GetActorLocation(), PotentialThreat->GetActorLocation());
    
    // Base threat calculation
    float BaseThreat = 1.0f;
    if (Cast<APawn>(PotentialThreat) && Cast<APawn>(PotentialThreat)->IsPlayerControlled())
    {
        BaseThreat = 8.0f; // Player is high threat
    }
    else if (PotentialThreat->GetName().Contains(TEXT("TRex")))
    {
        BaseThreat = 10.0f; // T-Rex is maximum threat
    }
    else if (PotentialThreat->GetName().Contains(TEXT("Raptor")))
    {
        BaseThreat = 6.0f; // Other raptors are moderate threat
    }

    // Distance modifier (closer = more threatening)
    float DistanceModifier = FMath::Clamp(2000.0f / FMath::Max(Distance, 100.0f), 0.1f, 2.0f);
    
    Assessment.ThreatLevel = BaseThreat * DistanceModifier;

    return Assessment;
}

bool ACombatAIManager::ShouldEngageTarget(AActor* Predator, AActor* Target)
{
    if (!Predator || !Target) return false;

    FCombat_ThreatAssessment Threat = AssessThreat(Target, Predator);
    
    // Don't engage if threat is too high for solo predator
    if (Threat.ThreatLevel > 7.0f)
    {
        // Check if we have pack support
        TArray<AActor*> Allies = GetNearbyAllies(Predator, PackCoordinationRadius);
        if (Allies.Num() < 2) return false;
    }

    float Distance = FVector::Dist(Predator->GetActorLocation(), Target->GetActorLocation());
    return Distance <= ThreatScanRadius && Threat.ThreatLevel > 0.5f;
}

bool ACombatAIManager::ShouldRetreat(AActor* Actor)
{
    if (!Actor || !bEnableTacticalRetreat) return false;

    TArray<AActor*> NearbyEnemies = GetNearbyEnemies(Actor, ThreatScanRadius * 0.5f);
    TArray<AActor*> NearbyAllies = GetNearbyAllies(Actor, PackCoordinationRadius);

    // Retreat if outnumbered 2:1 or more
    return NearbyEnemies.Num() >= (NearbyAllies.Num() + 1) * 2;
}

FVector ACombatAIManager::GetOptimalFlankingPosition(AActor* Predator, AActor* Target)
{
    if (!Predator || !Target) return FVector::ZeroVector;

    FVector TargetLocation = Target->GetActorLocation();
    FVector PredatorLocation = Predator->GetActorLocation();
    
    // Calculate perpendicular flanking positions
    FVector ToTarget = (TargetLocation - PredatorLocation).GetSafeNormal();
    FVector RightFlank = FVector::CrossProduct(ToTarget, FVector::UpVector) * 800.0f;
    FVector LeftFlank = -RightFlank;

    // Choose the flank position that's further from other pack members
    FVector RightPosition = TargetLocation + RightFlank;
    FVector LeftPosition = TargetLocation + LeftFlank;

    TArray<AActor*> Allies = GetNearbyAllies(Predator, PackCoordinationRadius);
    float RightDistance = 0.0f;
    float LeftDistance = 0.0f;

    for (AActor* Ally : Allies)
    {
        if (Ally == Predator) continue;
        RightDistance += FVector::Dist(Ally->GetActorLocation(), RightPosition);
        LeftDistance += FVector::Dist(Ally->GetActorLocation(), LeftPosition);
    }

    return (RightDistance > LeftDistance) ? RightPosition : LeftPosition;
}

void ACombatAIManager::CoordinatePackAttack(const FCombat_PackFormation& Pack, AActor* Target)
{
    if (!Target || Pack.PackMembers.Num() < 2) return;

    UE_LOG(LogTemp, Warning, TEXT("CombatAI: Coordinating pack attack with %d members against %s"), 
           Pack.PackMembers.Num(), *Target->GetName());

    // Alpha leads direct assault
    if (Pack.AlphaLeader)
    {
        // Direct alpha to target
    }

    // Other members flank
    for (int32 i = 1; i < Pack.PackMembers.Num(); i++)
    {
        if (Pack.PackMembers[i])
        {
            FVector FlankPosition = GetOptimalFlankingPosition(Pack.PackMembers[i], Target);
            // Direct pack member to flank position
        }
    }
}

void ACombatAIManager::InitiateTacticalRetreat(AActor* Actor)
{
    if (!Actor) return;

    UE_LOG(LogTemp, Warning, TEXT("CombatAI: Initiating tactical retreat for %s"), *Actor->GetName());
    
    // Find safe retreat position (away from threats, toward allies)
    FVector SafeDirection = FVector::ZeroVector;
    
    for (const FCombat_ThreatAssessment& Threat : ActiveThreats)
    {
        if (Threat.ThreatActor)
        {
            FVector AwayFromThreat = (Actor->GetActorLocation() - Threat.ThreatLocation).GetSafeNormal();
            SafeDirection += AwayFromThreat * Threat.ThreatLevel;
        }
    }

    SafeDirection.Normalize();
    FVector RetreatPosition = Actor->GetActorLocation() + SafeDirection * 1500.0f;
    
    // Direct actor to retreat position
}

TArray<AActor*> ACombatAIManager::GetNearbyAllies(AActor* Actor, float SearchRadius)
{
    TArray<AActor*> Allies;
    if (!Actor || !GetWorld()) return Allies;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);

    for (AActor* OtherActor : AllActors)
    {
        if (!OtherActor || OtherActor == Actor) continue;

        // Same species are allies (simple check by name)
        bool bSameSpecies = (Actor->GetName().Contains(TEXT("Raptor")) && OtherActor->GetName().Contains(TEXT("Raptor"))) ||
                           (Actor->GetName().Contains(TEXT("TRex")) && OtherActor->GetName().Contains(TEXT("TRex")));

        if (bSameSpecies)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), OtherActor->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                Allies.Add(OtherActor);
            }
        }
    }

    return Allies;
}

TArray<AActor*> ACombatAIManager::GetNearbyEnemies(AActor* Actor, float SearchRadius)
{
    TArray<AActor*> Enemies;
    if (!Actor || !GetWorld()) return Enemies;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);

    for (AActor* OtherActor : AllActors)
    {
        if (!OtherActor || OtherActor == Actor) continue;

        // Different species or player are enemies
        bool bIsEnemy = false;
        
        APawn* OtherPawn = Cast<APawn>(OtherActor);
        if (OtherPawn && OtherPawn->IsPlayerControlled())
        {
            bIsEnemy = true; // Player is always enemy
        }
        else
        {
            // Different dinosaur species are enemies
            bool bActorIsRaptor = Actor->GetName().Contains(TEXT("Raptor"));
            bool bOtherIsRaptor = OtherActor->GetName().Contains(TEXT("Raptor"));
            bool bActorIsTRex = Actor->GetName().Contains(TEXT("TRex"));
            bool bOtherIsTRex = OtherActor->GetName().Contains(TEXT("TRex"));

            bIsEnemy = (bActorIsRaptor && !bOtherIsRaptor) || (bActorIsTRex && !bOtherIsTRex) ||
                      (!bActorIsRaptor && !bActorIsTRex && (bOtherIsRaptor || bOtherIsTRex));
        }

        if (bIsEnemy)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), OtherActor->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                Enemies.Add(OtherActor);
            }
        }
    }

    return Enemies;
}