#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "DinosaurCombatComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    CombatComponent = CreateDefaultSubobject<UDinosaurCombatComponent>(TEXT("CombatComponent"));

    // Set blackboard component
    SetBlackboardComponent(BlackboardComponent);
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerception();
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateThreatAssessment();
    CleanupOldThreats();
    UpdateBlackboardKeys();
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (CombatComponent)
    {
        CombatComponent->Initialize(InPawn);
    }
}

void ACombatAIController::InitializePerception()
{
    if (!AIPerceptionComponent) return;

    // Configure Sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 1500.0f;
        SightConfig->LoseSightRadius = 1600.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure Hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1200.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Configure Damage
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    if (DamageConfig)
    {
        DamageConfig->SetMaxAge(10.0f);
        AIPerceptionComponent->ConfigureSense(*DamageConfig);
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        HandleCombatStateTransition(NewState);
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }
    }
}

void ACombatAIController::AssessThreat(AActor* PotentialThreat)
{
    if (!PotentialThreat || PotentialThreat == GetPawn()) return;

    // Find existing threat or create new one
    FThreatAssessment* ExistingThreat = ThreatList.FindByPredicate([PotentialThreat](const FThreatAssessment& Threat)
    {
        return Threat.ThreatActor == PotentialThreat;
    });

    FThreatAssessment ThreatData;
    if (ExistingThreat)
    {
        ThreatData = *ExistingThreat;
    }
    else
    {
        ThreatData.ThreatActor = PotentialThreat;
    }

    // Update threat data
    ThreatData.Distance = FVector::Dist(GetPawn()->GetActorLocation(), PotentialThreat->GetActorLocation());
    ThreatData.LastSeenTime = GetWorld()->GetTimeSeconds();
    ThreatData.LastKnownLocation = PotentialThreat->GetActorLocation();
    
    // Line of sight check
    FHitResult HitResult;
    FVector StartLocation = GetPawn()->GetActorLocation();
    FVector EndLocation = PotentialThreat->GetActorLocation();
    
    ThreatData.bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
        HitResult, 
        StartLocation, 
        EndLocation, 
        ECC_Visibility
    ) || HitResult.GetActor() == PotentialThreat;

    // Calculate threat score
    ThreatData.ThreatScore = CalculateThreatScore(PotentialThreat);
    
    // Determine threat level
    if (ThreatData.ThreatScore > 0.8f)
        ThreatData.ThreatLevel = EThreatLevel::Critical;
    else if (ThreatData.ThreatScore > 0.6f)
        ThreatData.ThreatLevel = EThreatLevel::High;
    else if (ThreatData.ThreatScore > 0.4f)
        ThreatData.ThreatLevel = EThreatLevel::Medium;
    else if (ThreatData.ThreatScore > 0.2f)
        ThreatData.ThreatLevel = EThreatLevel::Low;
    else
        ThreatData.ThreatLevel = EThreatLevel::None;

    // Update or add to threat list
    if (ExistingThreat)
    {
        *ExistingThreat = ThreatData;
    }
    else
    {
        ThreatList.Add(ThreatData);
    }
}

FThreatAssessment ACombatAIController::GetHighestThreat() const
{
    FThreatAssessment HighestThreat;
    float HighestScore = 0.0f;

    for (const FThreatAssessment& Threat : ThreatList)
    {
        if (Threat.ThreatScore > HighestScore)
        {
            HighestScore = Threat.ThreatScore;
            HighestThreat = Threat;
        }
    }

    return HighestThreat;
}

void ACombatAIController::UpdateBlackboardKeys()
{
    if (!BlackboardComponent) return;

    FThreatAssessment HighestThreat = GetHighestThreat();
    
    // Update target information
    BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), HighestThreat.ThreatActor);
    BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), HighestThreat.LastKnownLocation);
    BlackboardComponent->SetValueAsFloat(TEXT("TargetDistance"), HighestThreat.Distance);
    BlackboardComponent->SetValueAsBool(TEXT("HasLineOfSight"), HighestThreat.bHasLineOfSight);
    BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(HighestThreat.ThreatLevel));
    
    // Combat state
    BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    
    // Tactical information
    BlackboardComponent->SetValueAsBool(TEXT("CanAttack"), 
        HighestThreat.Distance <= AttackRange && 
        GetWorld()->GetTimeSeconds() - LastAttackTime >= AttackCooldown);
    
    BlackboardComponent->SetValueAsBool(TEXT("ShouldFlee"), ShouldFleeFromTarget(HighestThreat.ThreatActor));
    BlackboardComponent->SetValueAsBool(TEXT("ShouldEngage"), ShouldEngageTarget(HighestThreat.ThreatActor));
    
    // Pack information
    if (bIsPackHunter)
    {
        TArray<ACombatAIController*> NearbyPack = GetNearbyPackMembers();
        BlackboardComponent->SetValueAsInt(TEXT("PackSize"), NearbyPack.Num());
        BlackboardComponent->SetValueAsBool(TEXT("HasPackSupport"), NearbyPack.Num() > 0);
    }
}

bool ACombatAIController::ShouldEngageTarget(AActor* Target) const
{
    if (!Target) return false;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    
    // Consider aggression level, distance, and pack support
    float EngagementScore = AggressionLevel;
    
    // Closer targets are more likely to be engaged
    if (Distance < TerritorialRadius)
    {
        EngagementScore += 0.3f;
    }
    
    // Pack support increases engagement likelihood
    if (bIsPackHunter && GetNearbyPackMembers().Num() > 0)
    {
        EngagementScore += 0.2f;
    }
    
    return EngagementScore > 0.5f;
}

bool ACombatAIController::ShouldFleeFromTarget(AActor* Target) const
{
    if (!Target) return false;
    
    FThreatAssessment Threat = GetHighestThreat();
    
    // Flee if threat level is too high or we're overwhelmed
    return Threat.ThreatLevel >= EThreatLevel::Critical || 
           Threat.ThreatScore > FearThreshold;
}

FVector ACombatAIController::GetOptimalAttackPosition(AActor* Target) const
{
    if (!Target) return GetPawn()->GetActorLocation();
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    // Position at optimal attack range
    FVector OptimalPosition = TargetLocation - (DirectionToTarget * AttackRange * 0.8f);
    
    // Use navigation system to find valid location
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(OptimalPosition, NavLocation, FVector(100.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return OptimalPosition;
}

FVector ACombatAIController::GetFleeDirection(AActor* ThreatSource) const
{
    if (!ThreatSource) return GetPawn()->GetActorLocation();
    
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector ThreatLocation = ThreatSource->GetActorLocation();
    FVector FleeDirection = (CurrentLocation - ThreatLocation).GetSafeNormal();
    
    FVector FleePosition = CurrentLocation + (FleeDirection * FleeDistance);
    
    // Use navigation system to find valid flee location
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(FleePosition, NavLocation, FVector(200.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return FleePosition;
}

void ACombatAIController::CoordinatePackAttack(AActor* Target)
{
    if (!bIsPackHunter || !Target) return;
    
    TArray<ACombatAIController*> PackMembers = GetNearbyPackMembers();
    
    for (ACombatAIController* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->AssessThreat(Target);
            Member->SetCombatState(ECombatState::Hunting);
        }
    }
}

TArray<ACombatAIController*> ACombatAIController::GetNearbyPackMembers(float Radius) const
{
    TArray<ACombatAIController*> NearbyMembers;
    
    if (!bIsPackHunter) return NearbyMembers;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatAIController::StaticClass(), FoundActors);
    
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        ACombatAIController* OtherController = Cast<ACombatAIController>(Actor);
        if (OtherController && OtherController != this && OtherController->bIsPackHunter)
        {
            if (OtherController->PackTag == PackTag)
            {
                float Distance = FVector::Dist(MyLocation, OtherController->GetPawn()->GetActorLocation());
                if (Distance <= Radius)
                {
                    NearbyMembers.Add(OtherController);
                }
            }
        }
    }
    
    return NearbyMembers;
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        AssessThreat(Actor);
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor)
    {
        AssessThreat(Actor);
        
        // React to losing sight of target
        if (Stimulus.WasSuccessfullySensed() == false && Actor == CurrentTarget)
        {
            SetCombatState(ECombatState::Hunting);
        }
    }
}

void ACombatAIController::UpdateThreatAssessment()
{
    for (FThreatAssessment& Threat : ThreatList)
    {
        if (Threat.ThreatActor)
        {
            AssessThreat(Threat.ThreatActor);
        }
    }
}

void ACombatAIController::CleanupOldThreats()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    ThreatList.RemoveAll([CurrentTime](const FThreatAssessment& Threat)
    {
        return !Threat.ThreatActor || 
               CurrentTime - Threat.LastSeenTime > 30.0f; // Remove threats not seen for 30 seconds
    });
}

float ACombatAIController::CalculateThreatScore(AActor* Actor) const
{
    if (!Actor) return 0.0f;
    
    float Score = 0.0f;
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
    
    // Distance factor (closer = higher threat)
    Score += FMath::Clamp(1.0f - (Distance / 1500.0f), 0.0f, 0.4f);
    
    // Player character is always high threat
    if (Actor->IsA<ACharacter>() && Cast<ACharacter>(Actor)->IsPlayerControlled())
    {
        Score += 0.5f;
    }
    
    // Line of sight increases threat
    FThreatAssessment* ExistingThreat = ThreatList.FindByPredicate([Actor](const FThreatAssessment& Threat)
    {
        return Threat.ThreatActor == Actor;
    });
    
    if (ExistingThreat && ExistingThreat->bHasLineOfSight)
    {
        Score += 0.2f;
    }
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

void ACombatAIController::HandleCombatStateTransition(ECombatState NewState)
{
    switch (NewState)
    {
        case ECombatState::Attacking:
            LastAttackTime = GetWorld()->GetTimeSeconds();
            break;
            
        case ECombatState::Fleeing:
            // Clear current target when fleeing
            CurrentTarget = nullptr;
            break;
            
        case ECombatState::Hunting:
            // Set target to highest threat when hunting
            CurrentTarget = GetHighestThreat().ThreatActor;
            break;
            
        default:
            break;
    }
}