#include "Combat_DinosaurAI.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ACombat_DinosaurAI::ACombat_DinosaurAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Initialize Behavior Tree components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Initialize sight config
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 2000.0f;
        SightConfig->LoseSightRadius = 2200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    }

    // Initialize hearing config
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1500.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    }

    // Default combat parameters
    SightRange = 2000.0f;
    HearingRange = 1500.0f;
    AttackRange = 300.0f;
    FleeThreshold = 0.3f;
    AggressionLevel = 0.7f;
    bIsPackHunter = false;
    bIsApexPredator = false;

    // Initialize state
    CurrentState = ECombat_DinosaurState::Idle;
    PrimaryThreat = FCombat_ThreatAssessment();
    PackBehavior = FCombat_PackBehavior();
}

void ACombat_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAIPerception();
    InitializeBehaviorTree();
    
    // Set initial blackboard values
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), static_cast<uint8>(CurrentState));
        BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
        BlackboardComponent->SetValueAsBool(TEXT("IsPackHunter"), bIsPackHunter);
        BlackboardComponent->SetValueAsBool(TEXT("IsApexPredator"), bIsApexPredator);
    }
}

void ACombat_DinosaurAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateThreatList();
    SelectPrimaryThreat();
    UpdateCombatState();
    ExecuteCombatBehavior();
    
    if (bIsPackHunter)
    {
        UpdatePackBehavior();
    }
}

void ACombat_DinosaurAI::InitializeAIPerception()
{
    if (AIPerceptionComponent && SightConfig && HearingConfig)
    {
        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
        
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurAI::OnPerceptionUpdated);
    }
}

void ACombat_DinosaurAI::InitializeBehaviorTree()
{
    if (BlackboardComponent)
    {
        UseBlackboard(nullptr); // Will be set via Blueprint or C++
    }
}

void ACombat_DinosaurAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (IsValidThreat(Actor))
        {
            AssessThreat(Actor);
        }
    }
}

void ACombat_DinosaurAI::AssessThreat(AActor* PotentialThreat)
{
    if (!PotentialThreat || !IsValidThreat(PotentialThreat))
    {
        return;
    }

    FCombat_ThreatAssessment NewThreat;
    NewThreat.ThreatTarget = PotentialThreat;
    NewThreat.ThreatDistance = FVector::Dist(GetPawn()->GetActorLocation(), PotentialThreat->GetActorLocation());
    NewThreat.ThreatScore = CalculateThreatScore(PotentialThreat);
    NewThreat.LastSeenTime = GetWorld()->GetTimeSeconds();

    // Determine threat level based on score and distance
    if (NewThreat.ThreatScore > 0.8f)
    {
        NewThreat.ThreatLevel = ECombat_ThreatLevel::Critical;
    }
    else if (NewThreat.ThreatScore > 0.6f)
    {
        NewThreat.ThreatLevel = ECombat_ThreatLevel::High;
    }
    else if (NewThreat.ThreatScore > 0.4f)
    {
        NewThreat.ThreatLevel = ECombat_ThreatLevel::Medium;
    }
    else if (NewThreat.ThreatScore > 0.2f)
    {
        NewThreat.ThreatLevel = ECombat_ThreatLevel::Low;
    }
    else
    {
        NewThreat.ThreatLevel = ECombat_ThreatLevel::None;
    }

    // Add or update threat in list
    bool bFoundExisting = false;
    for (FCombat_ThreatAssessment& ExistingThreat : ThreatList)
    {
        if (ExistingThreat.ThreatTarget == PotentialThreat)
        {
            ExistingThreat = NewThreat;
            bFoundExisting = true;
            break;
        }
    }

    if (!bFoundExisting)
    {
        ThreatList.Add(NewThreat);
    }
}

void ACombat_DinosaurAI::UpdateThreatList()
{
    CleanupThreatList();
    
    // Update distances and scores for existing threats
    for (FCombat_ThreatAssessment& Threat : ThreatList)
    {
        if (Threat.ThreatTarget)
        {
            Threat.ThreatDistance = FVector::Dist(GetPawn()->GetActorLocation(), Threat.ThreatTarget->GetActorLocation());
            Threat.ThreatScore = CalculateThreatScore(Threat.ThreatTarget);
        }
    }
}

void ACombat_DinosaurAI::SelectPrimaryThreat()
{
    FCombat_ThreatAssessment BestThreat;
    float HighestScore = 0.0f;

    for (const FCombat_ThreatAssessment& Threat : ThreatList)
    {
        if (Threat.ThreatScore > HighestScore)
        {
            HighestScore = Threat.ThreatScore;
            BestThreat = Threat;
        }
    }

    PrimaryThreat = BestThreat;
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("PrimaryThreat"), PrimaryThreat.ThreatTarget);
        BlackboardComponent->SetValueAsFloat(TEXT("ThreatDistance"), PrimaryThreat.ThreatDistance);
        BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(PrimaryThreat.ThreatLevel));
    }
}

void ACombat_DinosaurAI::UpdateCombatState()
{
    ECombat_DinosaurState NewState = CurrentState;

    if (PrimaryThreat.ThreatTarget)
    {
        float HealthRatio = 1.0f; // TODO: Get actual health ratio from pawn
        
        if (PrimaryThreat.ThreatLevel == ECombat_ThreatLevel::Critical && HealthRatio < FleeThreshold)
        {
            NewState = ECombat_DinosaurState::Flee;
        }
        else if (PrimaryThreat.ThreatDistance <= AttackRange)
        {
            NewState = ECombat_DinosaurState::Attack;
        }
        else if (PrimaryThreat.ThreatLevel >= ECombat_ThreatLevel::Medium)
        {
            NewState = ECombat_DinosaurState::Hunt;
        }
        else
        {
            NewState = ECombat_DinosaurState::Patrol;
        }
    }
    else
    {
        NewState = ECombat_DinosaurState::Idle;
    }

    if (bIsPackHunter && PackBehavior.bIsPackHunting)
    {
        NewState = ECombat_DinosaurState::Pack;
    }

    CurrentState = NewState;
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), static_cast<uint8>(CurrentState));
    }
}

void ACombat_DinosaurAI::ExecuteCombatBehavior()
{
    switch (CurrentState)
    {
        case ECombat_DinosaurState::Hunt:
            ExecuteHuntingPattern();
            break;
            
        case ECombat_DinosaurState::Attack:
            if (ShouldAmbush(PrimaryThreat.ThreatTarget))
            {
                ExecuteAmbushTactic();
            }
            break;
            
        case ECombat_DinosaurState::Pack:
            CoordinatePackAttack();
            break;
            
        default:
            break;
    }
}

float ACombat_DinosaurAI::CalculateThreatScore(AActor* Target)
{
    if (!Target)
    {
        return 0.0f;
    }

    float Score = 0.0f;
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    
    // Distance factor (closer = higher threat)
    Score += FMath::Clamp(1.0f - (Distance / SightRange), 0.0f, 1.0f) * 0.4f;
    
    // Target type factor
    if (Target->IsA<ACharacter>())
    {
        Score += 0.6f; // Players are high priority
    }
    else if (Target->IsA<APawn>())
    {
        Score += 0.3f; // Other pawns are medium priority
    }
    
    // Aggression modifier
    Score *= AggressionLevel;
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

bool ACombat_DinosaurAI::IsValidThreat(AActor* Target)
{
    if (!Target || Target == GetPawn())
    {
        return false;
    }
    
    // Don't consider pack members as threats
    if (bIsPackHunter && PackBehavior.PackMembers.Contains(Target))
    {
        return false;
    }
    
    return true;
}

void ACombat_DinosaurAI::CleanupThreatList()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    ThreatList.RemoveAll([CurrentTime](const FCombat_ThreatAssessment& Threat)
    {
        return !Threat.ThreatTarget || 
               !IsValid(Threat.ThreatTarget) || 
               (CurrentTime - Threat.LastSeenTime) > 10.0f; // Remove threats not seen for 10 seconds
    });
}

void ACombat_DinosaurAI::JoinPack(ACombat_DinosaurAI* PackLeader)
{
    if (PackLeader && PackLeader != this)
    {
        PackBehavior.PackLeader = PackLeader;
        PackLeader->PackBehavior.PackMembers.AddUnique(GetPawn());
        bIsPackHunter = true;
    }
}

void ACombat_DinosaurAI::LeavePack()
{
    if (PackBehavior.PackLeader)
    {
        if (ACombat_DinosaurAI* Leader = Cast<ACombat_DinosaurAI>(PackBehavior.PackLeader))
        {
            Leader->PackBehavior.PackMembers.Remove(GetPawn());
        }
    }
    
    PackBehavior = FCombat_PackBehavior();
    bIsPackHunter = false;
}

void ACombat_DinosaurAI::UpdatePackBehavior()
{
    if (!bIsPackHunter || PackBehavior.PackMembers.Num() == 0)
    {
        return;
    }

    // Calculate pack center
    FVector CenterSum = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (AActor* Member : PackBehavior.PackMembers)
    {
        if (Member && IsValid(Member))
        {
            CenterSum += Member->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers > 0)
    {
        PackBehavior.PackCenter = CenterSum / ValidMembers;
        PackBehavior.PackCohesion = FMath::Clamp(1.0f - (ValidMembers * 0.1f), 0.5f, 1.0f);
    }
}

void ACombat_DinosaurAI::CoordinatePackAttack()
{
    if (!PrimaryThreat.ThreatTarget || !bIsPackHunter)
    {
        return;
    }

    PackBehavior.bIsPackHunting = true;
    
    // Coordinate with pack members for flanking
    FVector FlankingPos = CalculateFlankingPosition(PrimaryThreat.ThreatTarget);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("FlankingPosition"), FlankingPos);
        BlackboardComponent->SetValueAsBool(TEXT("IsPackHunting"), true);
    }
}

FVector ACombat_DinosaurAI::CalculateFlankingPosition(AActor* Target)
{
    if (!Target)
    {
        return GetPawn()->GetActorLocation();
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the side
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector).GetSafeNormal();
    FVector FlankingPosition = TargetLocation + (FlankDirection * AttackRange * 1.5f);
    
    return FlankingPosition;
}

bool ACombat_DinosaurAI::ShouldAmbush(AActor* Target)
{
    if (!Target)
    {
        return false;
    }

    // Ambush if target is unaware and we have advantage
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance > AttackRange * 0.5f && Distance < AttackRange * 2.0f && AggressionLevel > 0.6f;
}

void ACombat_DinosaurAI::ExecuteAmbushTactic()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("ShouldAmbush"), true);
        BlackboardComponent->SetValueAsVector(TEXT("AmbushPosition"), GetPawn()->GetActorLocation());
    }
}

void ACombat_DinosaurAI::ExecuteHuntingPattern()
{
    if (!PrimaryThreat.ThreatTarget)
    {
        return;
    }

    // Move towards target with hunting behavior
    FVector TargetLocation = PrimaryThreat.ThreatTarget->GetActorLocation();
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("HuntTarget"), TargetLocation);
        BlackboardComponent->SetValueAsBool(TEXT("IsHunting"), true);
    }
}