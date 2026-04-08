#include "CombatAIManager.h"
#include "CombatAIController.h"
#include "../Components/CombatBehaviorComponent.h"
#include "Components/BlackboardComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    ThreatAssessmentInterval = 0.5f;
    MaxCombatDistance = 2000.0f;
    PackCoordinationRadius = 1000.0f;
    MaxPackSize = 6;
    LastThreatAssessmentTime = 0.0f;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager initialized"));
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update threat assessments at intervals
    if (CurrentTime - LastThreatAssessmentTime >= ThreatAssessmentInterval)
    {
        UpdateThreatAssessments();
        UpdatePackCoordination();
        LastThreatAssessmentTime = CurrentTime;
    }
}

void ACombatAIManager::RegisterCombatController(ACombatAIController* Controller)
{
    if (Controller && !ActiveCombatControllers.Contains(Controller))
    {
        ActiveCombatControllers.Add(Controller);
        UE_LOG(LogTemp, Log, TEXT("Registered combat controller: %s"), *Controller->GetName());
    }
}

void ACombatAIManager::UnregisterCombatController(ACombatAIController* Controller)
{
    if (Controller)
    {
        ActiveCombatControllers.Remove(Controller);
        UE_LOG(LogTemp, Log, TEXT("Unregistered combat controller: %s"), *Controller->GetName());
    }
}

TArray<FCombatThreat> ACombatAIManager::AssessThreats(AActor* ForActor, float AssessmentRadius)
{
    TArray<FCombatThreat> Threats;
    
    if (!ForActor)
    {
        return Threats;
    }
    
    // Find all potential threats within radius
    TArray<AActor*> NearbyActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        ForActor->GetActorLocation(),
        AssessmentRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>{ForActor},
        NearbyActors
    );
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == ForActor)
        {
            continue;
        }
        
        // Calculate threat level
        float ThreatLevel = CalculateThreatLevel(Actor, ForActor);
        
        if (ThreatLevel > 0.1f) // Only consider significant threats
        {
            FCombatThreat Threat;
            Threat.ThreatActor = Actor;
            Threat.ThreatLevel = ThreatLevel;
            Threat.Distance = FVector::Dist(ForActor->GetActorLocation(), Actor->GetActorLocation());
            Threat.EngagementType = DetermineEngagementType(ForActor, Actor);
            Threat.LastSeenTime = GetWorld()->GetTimeSeconds();
            Threat.LastKnownPosition = Actor->GetActorLocation();
            
            Threats.Add(Threat);
        }
    }
    
    // Sort threats by level (highest first)
    Threats.Sort([](const FCombatThreat& A, const FCombatThreat& B) {
        return A.ThreatLevel > B.ThreatLevel;
    });
    
    return Threats;
}

float ACombatAIManager::CalculateThreatLevel(AActor* ThreatActor, AActor* AssessingActor)
{
    if (!ThreatActor || !AssessingActor)
    {
        return 0.0f;
    }
    
    float BaseThreat = 0.0f;
    float Distance = FVector::Dist(AssessingActor->GetActorLocation(), ThreatActor->GetActorLocation());
    
    // Distance modifier (closer = more threatening)
    float DistanceModifier = FMath::Clamp(1.0f - (Distance / MaxCombatDistance), 0.1f, 1.0f);
    
    // Check if this is the player
    if (ThreatActor == GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        BaseThreat = 0.8f; // Player is always a significant threat
    }
    else
    {
        // Check for combat behavior component to assess AI threat
        if (UCombatBehaviorComponent* CombatComp = ThreatActor->FindComponentByClass<UCombatBehaviorComponent>())
        {
            // Base threat from combat capability
            BaseThreat = 0.5f;
            
            // Modify based on current combat state
            // This would require accessing the combat state from the component
            // BaseThreat *= CombatComp->GetAggressionMultiplier();
        }
        else
        {
            BaseThreat = 0.2f; // Non-combat entities are low threat
        }
    }
    
    return BaseThreat * DistanceModifier;
}

TArray<ACombatAIController*> ACombatAIManager::GetNearbyPackMembers(ACombatAIController* Controller, float Radius)
{
    TArray<ACombatAIController*> PackMembers;
    
    if (!Controller || !Controller->GetPawn())
    {
        return PackMembers;
    }
    
    FVector ControllerLocation = Controller->GetPawn()->GetActorLocation();
    
    for (ACombatAIController* OtherController : ActiveCombatControllers)
    {
        if (OtherController == Controller || !OtherController->GetPawn())
        {
            continue;
        }
        
        float Distance = FVector::Dist(ControllerLocation, OtherController->GetPawn()->GetActorLocation());
        
        if (Distance <= Radius && AreCompatiblePackMembers(Controller, OtherController))
        {
            PackMembers.Add(OtherController);
        }
    }
    
    return PackMembers;
}

void ACombatAIManager::CoordinatePackAttack(const TArray<ACombatAIController*>& PackMembers, AActor* Target)
{
    if (!Target || PackMembers.Num() == 0)
    {
        return;
    }
    
    // Assign roles based on pack position and capabilities
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        ACombatAIController* Member = PackMembers[i];
        if (!Member || !Member->GetBlackboardComponent())
        {
            continue;
        }
        
        UBlackboardComponent* Blackboard = Member->GetBlackboardComponent();
        
        // Set common target
        Blackboard->SetValueAsObject(TEXT("CombatTarget"), Target);
        
        // Assign pack role
        if (i == 0)
        {
            // Alpha - direct engagement
            Blackboard->SetValueAsEnum(TEXT("PackRole"), 0); // Alpha
            Blackboard->SetValueAsVector(TEXT("PreferredPosition"), Target->GetActorLocation());
        }
        else if (i < PackMembers.Num() / 2)
        {
            // Flankers - attack from sides
            FVector FlankPosition = CalculateFlankPosition(Target, Member->GetPawn(), i % 2 == 0);
            Blackboard->SetValueAsEnum(TEXT("PackRole"), 1); // Flanker
            Blackboard->SetValueAsVector(TEXT("PreferredPosition"), FlankPosition);
        }
        else
        {
            // Support - maintain distance, harass
            FVector SupportPosition = CalculateSupportPosition(Target, Member->GetPawn());
            Blackboard->SetValueAsEnum(TEXT("PackRole"), 2); // Support
            Blackboard->SetValueAsVector(TEXT("PreferredPosition"), SupportPosition);
        }
        
        // Set pack coordination flag
        Blackboard->SetValueAsBool(TEXT("IsPackHunting"), true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Coordinated pack attack with %d members against %s"), 
           PackMembers.Num(), *Target->GetName());
}

FVector ACombatAIManager::FindOptimalCombatPosition(AActor* Attacker, AActor* Target, const FCombatTactics& Tactics)
{
    if (!Attacker || !Target)
    {
        return FVector::ZeroVector;
    }
    
    FVector AttackerPos = Attacker->GetActorLocation();
    FVector TargetPos = Target->GetActorLocation();
    FVector DirectionToTarget = (TargetPos - AttackerPos).GetSafeNormal();
    
    // Generate potential positions in a circle around the target
    TArray<FVector> CandidatePositions;
    int32 NumCandidates = 16;
    float AngleStep = 360.0f / NumCandidates;
    
    for (int32 i = 0; i < NumCandidates; i++)
    {
        float Angle = i * AngleStep;
        FVector Direction = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)),
            FMath::Sin(FMath::DegreesToRadians(Angle)),
            0.0f
        );
        
        FVector CandidatePos = TargetPos + Direction * Tactics.PreferredEngagementDistance;
        CandidatePositions.Add(CandidatePos);
    }
    
    // Evaluate each position
    FVector BestPosition = AttackerPos;
    float BestScore = -1.0f;
    
    for (const FVector& Position : CandidatePositions)
    {
        float Score = CalculatePositionScore(Position, Attacker, Target, Tactics);
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestPosition = Position;
        }
    }
    
    return BestPosition;
}

bool ACombatAIManager::IsPositionTacticallyAdvantaged(const FVector& Position, AActor* ForActor, AActor* Target)
{
    if (!ForActor || !Target)
    {
        return false;
    }
    
    // Check terrain advantage
    bool bHasTerrainAdvantage = HasTerrainAdvantage(Position, ForActor);
    
    // Check line of sight
    FHitResult HitResult;
    bool bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Position + FVector(0, 0, 100), // Slightly elevated
        Target->GetActorLocation() + FVector(0, 0, 100),
        ECC_Visibility
    );
    
    // Check distance advantage
    float Distance = FVector::Dist(Position, Target->GetActorLocation());
    bool bOptimalDistance = Distance >= 150.0f && Distance <= 400.0f;
    
    return bHasTerrainAdvantage || (bHasLineOfSight && bOptimalDistance);
}

void ACombatAIManager::NotifyCombatStateChange(ACombatAIController* Controller, ECombatState NewState)
{
    if (Controller)
    {
        OnCombatStateChanged.Broadcast(Controller, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Combat state changed for %s: %d"), 
               *Controller->GetName(), (int32)NewState);
    }
}

void ACombatAIManager::NotifyCombatEngagement(ACombatAIController* Attacker, AActor* Target, ECombatEngagementType EngagementType)
{
    if (Attacker && Target)
    {
        OnCombatEngagement.Broadcast(Attacker, Target, EngagementType);
        
        UE_LOG(LogTemp, Log, TEXT("Combat engagement: %s vs %s (Type: %d)"), 
               *Attacker->GetName(), *Target->GetName(), (int32)EngagementType);
    }
}

bool ACombatAIManager::CanEngageInCombat(AActor* Attacker, AActor* Target)
{
    if (!Attacker || !Target)
    {
        return false;
    }
    
    // Check distance
    float Distance = FVector::Dist(Attacker->GetActorLocation(), Target->GetActorLocation());
    if (Distance > MaxCombatDistance)
    {
        return false;
    }
    
    // Check if both actors are valid for combat
    UCombatBehaviorComponent* AttackerCombat = Attacker->FindComponentByClass<UCombatBehaviorComponent>();
    if (!AttackerCombat)
    {
        return false; // Attacker has no combat capability
    }
    
    return true;
}

ECombatEngagementType ACombatAIManager::DetermineEngagementType(AActor* Attacker, AActor* Target)
{
    if (!Attacker || !Target)
    {
        return ECombatEngagementType::Defensive;
    }
    
    // Check if target is player
    if (Target == GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        return ECombatEngagementType::Territorial;
    }
    
    // Default to defensive for now
    // This could be expanded based on dinosaur archetypes and AI behavior
    return ECombatEngagementType::Defensive;
}

void ACombatAIManager::UpdateThreatAssessments()
{
    // Update threat assessments for all active controllers
    for (ACombatAIController* Controller : ActiveCombatControllers)
    {
        if (!Controller || !Controller->GetPawn())
        {
            continue;
        }
        
        TArray<FCombatThreat> Threats = AssessThreats(Controller->GetPawn(), 1000.0f);
        
        // Update controller's blackboard with threat information
        if (UBlackboardComponent* Blackboard = Controller->GetBlackboardComponent())
        {
            if (Threats.Num() > 0)
            {
                FCombatThreat HighestThreat = Threats[0];
                Blackboard->SetValueAsObject(TEXT("HighestThreat"), HighestThreat.ThreatActor);
                Blackboard->SetValueAsFloat(TEXT("ThreatLevel"), HighestThreat.ThreatLevel);
                Blackboard->SetValueAsVector(TEXT("ThreatLocation"), HighestThreat.LastKnownPosition);
            }
            else
            {
                Blackboard->ClearValue(TEXT("HighestThreat"));
                Blackboard->SetValueAsFloat(TEXT("ThreatLevel"), 0.0f);
            }
        }
    }
}

float ACombatAIManager::CalculateSizeThreatModifier(EDinosaurSize AttackerSize, EDinosaurSize TargetSize)
{
    int32 SizeDifference = (int32)TargetSize - (int32)AttackerSize;
    
    // Larger targets are more threatening
    if (SizeDifference > 0)
    {
        return 1.0f + (SizeDifference * 0.3f);
    }
    else if (SizeDifference < 0)
    {
        return FMath::Max(0.2f, 1.0f + (SizeDifference * 0.2f));
    }
    
    return 1.0f;
}

float ACombatAIManager::CalculateDietThreatModifier(EDinosaurDiet AttackerDiet, EDinosaurDiet TargetDiet)
{
    // Carnivores are generally more threatening
    if (TargetDiet == EDinosaurDiet::Carnivore)
    {
        return 1.5f;
    }
    else if (TargetDiet == EDinosaurDiet::Omnivore)
    {
        return 1.2f;
    }
    
    return 1.0f;
}

bool ACombatAIManager::IsNaturalPredator(EDinosaurDiet AttackerDiet, EDinosaurDiet TargetDiet)
{
    return (AttackerDiet == EDinosaurDiet::Herbivore && TargetDiet == EDinosaurDiet::Carnivore) ||
           (AttackerDiet == EDinosaurDiet::Carnivore && TargetDiet == EDinosaurDiet::Herbivore);
}

void ACombatAIManager::UpdatePackCoordination()
{
    // Group compatible controllers for pack behavior
    TArray<ACombatAIController*> ProcessedControllers;
    
    for (ACombatAIController* Controller : ActiveCombatControllers)
    {
        if (!Controller || ProcessedControllers.Contains(Controller))
        {
            continue;
        }
        
        TArray<ACombatAIController*> PackMembers = GetNearbyPackMembers(Controller, PackCoordinationRadius);
        
        if (PackMembers.Num() > 0)
        {
            PackMembers.Add(Controller);
            
            // Find common target for pack
            AActor* CommonTarget = nullptr;
            for (ACombatAIController* Member : PackMembers)
            {
                if (UBlackboardComponent* BB = Member->GetBlackboardComponent())
                {
                    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("HighestThreat")));
                    if (Target)
                    {
                        CommonTarget = Target;
                        break;
                    }
                }
            }
            
            if (CommonTarget)
            {
                CoordinatePackAttack(PackMembers, CommonTarget);
            }
            
            ProcessedControllers.Append(PackMembers);
        }
    }
}

bool ACombatAIManager::AreCompatiblePackMembers(ACombatAIController* Controller1, ACombatAIController* Controller2)
{
    // For now, simple compatibility check
    // This could be expanded to check species, size, diet, etc.
    return Controller1 && Controller2 && Controller1 != Controller2;
}

bool ACombatAIManager::HasTerrainAdvantage(const FVector& Position, AActor* ForActor)
{
    if (!ForActor)
    {
        return false;
    }
    
    // Check if position is elevated
    FVector ActorPos = ForActor->GetActorLocation();
    float HeightDifference = Position.Z - ActorPos.Z;
    
    // Higher ground provides advantage
    return HeightDifference > 50.0f;
}

float ACombatAIManager::CalculatePositionScore(const FVector& Position, AActor* Attacker, AActor* Target, const FCombatTactics& Tactics)
{
    float Score = 0.0f;
    
    // Distance score
    float Distance = FVector::Dist(Position, Target->GetActorLocation());
    float DistanceScore = 1.0f - FMath::Abs(Distance - Tactics.PreferredEngagementDistance) / Tactics.PreferredEngagementDistance;
    Score += DistanceScore * 0.4f;
    
    // Terrain advantage score
    if (HasTerrainAdvantage(Position, Attacker))
    {
        Score += 0.3f;
    }
    
    // Line of sight score
    FHitResult HitResult;
    bool bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Position + FVector(0, 0, 100),
        Target->GetActorLocation() + FVector(0, 0, 100),
        ECC_Visibility
    );
    
    if (bHasLineOfSight)
    {
        Score += 0.3f;
    }
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

FVector ACombatAIManager::CalculateFlankPosition(AActor* Target, AActor* Attacker, bool bLeftFlank)
{
    if (!Target || !Attacker)
    {
        return FVector::ZeroVector;
    }
    
    FVector TargetPos = Target->GetActorLocation();
    FVector AttackerPos = Attacker->GetActorLocation();
    FVector DirectionToAttacker = (AttackerPos - TargetPos).GetSafeNormal();
    
    // Calculate perpendicular direction for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToAttacker, FVector::UpVector);
    if (!bLeftFlank)
    {
        FlankDirection *= -1.0f;
    }
    
    return TargetPos + FlankDirection * 300.0f;
}

FVector ACombatAIManager::CalculateSupportPosition(AActor* Target, AActor* Attacker)
{
    if (!Target || !Attacker)
    {
        return FVector::ZeroVector;
    }
    
    FVector TargetPos = Target->GetActorLocation();
    FVector AttackerPos = Attacker->GetActorLocation();
    FVector DirectionToAttacker = (AttackerPos - TargetPos).GetSafeNormal();
    
    // Support position is behind and to the side
    FVector SupportDirection = -DirectionToAttacker + FVector::CrossProduct(DirectionToAttacker, FVector::UpVector) * 0.5f;
    SupportDirection.Normalize();
    
    return TargetPos + SupportDirection * 500.0f;
}