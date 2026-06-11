#include "CombatAIManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Initialize default values
    ThreatUpdateInterval = 1.0f;
    MaxCombatRange = 5000.0f;
    FlankingAngle = 45.0f;
    RetreatHealthThreshold = 0.3f;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();

    // Start threat assessment timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ThreatAssessmentTimer,
            this,
            &ACombatAIManager::UpdateThreatAssessments,
            ThreatUpdateInterval,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager initialized and active"));
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ProcessCombatLogic();
}

FCombat_ThreatAssessment ACombatAIManager::AssessThreat(AActor* PotentialThreat, AActor* Assessor)
{
    FCombat_ThreatAssessment Assessment;

    if (!PotentialThreat || !Assessor)
    {
        return Assessment;
    }

    // Calculate distance
    float Distance = FVector::Dist(PotentialThreat->GetActorLocation(), Assessor->GetActorLocation());
    Assessment.ThreatDistance = Distance;
    Assessment.ThreatSource = PotentialThreat;

    // Calculate threat score
    float ThreatScore = CalculateThreatScore(PotentialThreat, Assessor);

    // Determine threat level based on distance and score
    if (Distance > MaxCombatRange)
    {
        Assessment.ThreatLevel = ECombat_ThreatLevel::None;
    }
    else if (ThreatScore > 0.8f)
    {
        Assessment.ThreatLevel = ECombat_ThreatLevel::Critical;
    }
    else if (ThreatScore > 0.6f)
    {
        Assessment.ThreatLevel = ECombat_ThreatLevel::High;
    }
    else if (ThreatScore > 0.4f)
    {
        Assessment.ThreatLevel = ECombat_ThreatLevel::Medium;
    }
    else if (ThreatScore > 0.2f)
    {
        Assessment.ThreatLevel = ECombat_ThreatLevel::Low;
    }

    Assessment.AggressionModifier = ThreatScore;

    return Assessment;
}

void ACombatAIManager::RegisterCombatant(AActor* Combatant)
{
    if (Combatant && IsValidCombatant(Combatant))
    {
        ActiveCombatants.AddUnique(Combatant);
        UE_LOG(LogTemp, Log, TEXT("Registered combatant: %s"), *Combatant->GetName());
    }
}

void ACombatAIManager::UnregisterCombatant(AActor* Combatant)
{
    if (Combatant)
    {
        ActiveCombatants.Remove(Combatant);
        ThreatAssessments.Remove(Combatant);
        UE_LOG(LogTemp, Log, TEXT("Unregistered combatant: %s"), *Combatant->GetName());
    }
}

TArray<AActor*> ACombatAIManager::GetNearbyThreats(AActor* Assessor, float SearchRadius)
{
    TArray<AActor*> NearbyThreats;

    if (!Assessor)
    {
        return NearbyThreats;
    }

    FVector AssessorLocation = Assessor->GetActorLocation();

    for (AActor* Combatant : ActiveCombatants)
    {
        if (Combatant && Combatant != Assessor)
        {
            float Distance = FVector::Dist(Combatant->GetActorLocation(), AssessorLocation);
            if (Distance <= SearchRadius)
            {
                NearbyThreats.Add(Combatant);
            }
        }
    }

    return NearbyThreats;
}

void ACombatAIManager::InitiateCombatEncounter(AActor* Aggressor, AActor* Target)
{
    if (!Aggressor || !Target)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Combat initiated: %s vs %s"), 
           *Aggressor->GetName(), *Target->GetName());

    // Register both as combatants if not already registered
    RegisterCombatant(Aggressor);
    RegisterCombatant(Target);

    // Assess mutual threats
    FCombat_ThreatAssessment AggressorThreat = AssessThreat(Target, Aggressor);
    FCombat_ThreatAssessment TargetThreat = AssessThreat(Aggressor, Target);

    ThreatAssessments.Add(Aggressor, AggressorThreat);
    ThreatAssessments.Add(Target, TargetThreat);
}

void ACombatAIManager::EndCombatEncounter(AActor* Combatant)
{
    if (Combatant)
    {
        ThreatAssessments.Remove(Combatant);
        UE_LOG(LogTemp, Log, TEXT("Combat ended for: %s"), *Combatant->GetName());
    }
}

FVector ACombatAIManager::CalculateFlankingPosition(AActor* Attacker, AActor* Target)
{
    if (!Attacker || !Target)
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector AttackerLocation = Attacker->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - AttackerLocation).GetSafeNormal();

    // Calculate flanking position at 45-degree angle
    FVector FlankDirection = DirectionToTarget.RotateAngleAxis(FlankingAngle, FVector::UpVector);
    FVector FlankingPosition = TargetLocation + (FlankDirection * 800.0f);

    return FlankingPosition;
}

bool ACombatAIManager::ShouldRetreat(AActor* Combatant)
{
    if (!Combatant)
    {
        return false;
    }

    // Check if combatant has health component and is below threshold
    // This is a simplified check - in practice would use actual health component
    FCombat_ThreatAssessment* Assessment = ThreatAssessments.Find(Combatant);
    if (Assessment && Assessment->ThreatLevel == ECombat_ThreatLevel::Critical)
    {
        return true;
    }

    return false;
}

void ACombatAIManager::CoordinatePackAttack(TArray<AActor*> PackMembers, AActor* Target)
{
    if (!Target || PackMembers.Num() == 0)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Coordinating pack attack with %d members against %s"), 
           PackMembers.Num(), *Target->GetName());

    // Assign roles to pack members
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        AActor* PackMember = PackMembers[i];
        if (!PackMember)
        {
            continue;
        }

        FVector FlankingPos = CalculateFlankingPosition(PackMember, Target);
        
        // In a real implementation, this would send commands to AI controllers
        // For now, we log the tactical positioning
        UE_LOG(LogTemp, Log, TEXT("Pack member %s assigned flanking position"), 
               *PackMember->GetName());
    }
}

void ACombatAIManager::UpdateThreatAssessments()
{
    // Update threat assessments for all active combatants
    for (AActor* Combatant : ActiveCombatants)
    {
        if (!IsValid(Combatant))
        {
            continue;
        }

        TArray<AActor*> NearbyThreats = GetNearbyThreats(Combatant, MaxCombatRange);
        
        // Find the highest threat
        FCombat_ThreatAssessment HighestThreat;
        for (AActor* Threat : NearbyThreats)
        {
            FCombat_ThreatAssessment CurrentThreat = AssessThreat(Threat, Combatant);
            if (CurrentThreat.ThreatLevel > HighestThreat.ThreatLevel)
            {
                HighestThreat = CurrentThreat;
            }
        }

        ThreatAssessments.Add(Combatant, HighestThreat);
    }
}

void ACombatAIManager::ProcessCombatLogic()
{
    // Process combat logic for all active combatants
    for (AActor* Combatant : ActiveCombatants)
    {
        if (!IsValid(Combatant))
        {
            continue;
        }

        FCombat_ThreatAssessment* Assessment = ThreatAssessments.Find(Combatant);
        if (!Assessment)
        {
            continue;
        }

        // Check if should retreat
        if (ShouldRetreat(Combatant))
        {
            UE_LOG(LogTemp, Log, TEXT("%s should retreat"), *Combatant->GetName());
        }

        // Process threat response based on level
        switch (Assessment->ThreatLevel)
        {
            case ECombat_ThreatLevel::Critical:
                // Engage in combat or flee
                break;
            case ECombat_ThreatLevel::High:
                // Prepare for combat
                break;
            case ECombat_ThreatLevel::Medium:
                // Stay alert
                break;
            case ECombat_ThreatLevel::Low:
                // Monitor situation
                break;
            case ECombat_ThreatLevel::None:
                // Return to normal behavior
                break;
        }
    }
}

float ACombatAIManager::CalculateThreatScore(AActor* Threat, AActor* Assessor)
{
    if (!Threat || !Assessor)
    {
        return 0.0f;
    }

    float ThreatScore = 0.0f;
    float Distance = FVector::Dist(Threat->GetActorLocation(), Assessor->GetActorLocation());

    // Distance factor (closer = more threatening)
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / MaxCombatRange), 0.0f, 1.0f);
    ThreatScore += DistanceFactor * 0.4f;

    // Size factor (larger = more threatening)
    FVector ThreatScale = Threat->GetActorScale3D();
    float SizeFactor = FMath::Clamp(ThreatScale.Size() / 3.0f, 0.0f, 1.0f);
    ThreatScore += SizeFactor * 0.3f;

    // Movement factor (moving towards = more threatening)
    FVector ThreatVelocity = Threat->GetVelocity();
    FVector DirectionToAssessor = (Assessor->GetActorLocation() - Threat->GetActorLocation()).GetSafeNormal();
    float MovementFactor = FMath::Max(0.0f, FVector::DotProduct(ThreatVelocity.GetSafeNormal(), DirectionToAssessor));
    ThreatScore += MovementFactor * 0.3f;

    return FMath::Clamp(ThreatScore, 0.0f, 1.0f);
}

bool ACombatAIManager::IsValidCombatant(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }

    // Check if actor is a pawn or has combat-relevant components
    return Actor->IsA<APawn>() || Actor->GetName().Contains(TEXT("Dino")) || 
           Actor->GetName().Contains(TEXT("Character"));
}