#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/HealthComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI perception component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Initialize default values
    AggressionLevel = ECombat_AggressionLevel::Moderate;
    TacticalRole = ECombat_TacticalRole::Flanker;
    AttackRange = 300.0f;
    FleeHealthThreshold = 0.3f;
    CurrentCombatState = ECombat_CombatState::Idle;
    CombatTarget = nullptr;
    PackLeader = nullptr;
    bIsPackLeader = false;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    SetupPerception();
    
    // Start combat behavior updates
    GetWorld()->GetTimerManager().SetTimer(CombatUpdateTimer, this, 
        &ACombatAIController::UpdateCombatBehavior, 0.5f, true);
    
    GetWorld()->GetTimerManager().SetTimer(ThreatAssessmentTimer, this, 
        &ACombatAIController::AssessThreats, 1.0f, true);
}

void ACombatAIController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);
    
    if (CombatBehaviorTree && CombatBlackboard)
    {
        RunBehaviorTree(CombatBehaviorTree);
        
        // Initialize blackboard values
        UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
            BlackboardComp->SetValueAsEnum(TEXT("AggressionLevel"), static_cast<uint8>(AggressionLevel));
            BlackboardComp->SetValueAsEnum(TEXT("TacticalRole"), static_cast<uint8>(TacticalRole));
            BlackboardComp->SetValueAsFloat(TEXT("AttackRange"), AttackRange);
            BlackboardComp->SetValueAsFloat(TEXT("FleeHealthThreshold"), FleeHealthThreshold);
        }
    }
}

void ACombatAIController::UnPossess()
{
    // Clean up timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(CombatUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(ThreatAssessmentTimer);
    }
    
    // Leave pack if part of one
    if (PackLeader || bIsPackLeader)
    {
        LeavePack();
    }
    
    Super::UnPossess();
}

void ACombatAIController::SetupPerception()
{
    if (!AIPerceptionComponent)
        return;

    ConfigureSight();
    ConfigureHearing();

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
}

void ACombatAIController::ConfigureSight()
{
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = FieldOfView / 2.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        
        // Configure detection filters
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }
}

void ACombatAIController::ConfigureHearing()
{
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        
        // Configure detection filters
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor || Actor == GetPawn())
            continue;
            
        // Check if this is a potential threat
        if (ShouldEngageTarget(Actor))
        {
            if (!CombatTarget || AssessThreatLevel(Actor) > AssessThreatLevel(CombatTarget))
            {
                SetCombatTarget(Actor);
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || Actor == GetPawn())
        return;
        
    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        if (ShouldEngageTarget(Actor))
        {
            SetCombatTarget(Actor);
            SetCombatState(ECombat_CombatState::Engaging);
        }
    }
    else
    {
        // Target lost
        if (Actor == CombatTarget)
        {
            SetCombatState(ECombat_CombatState::Searching);
        }
    }
}

void ACombatAIController::SetCombatTarget(AActor* NewTarget)
{
    if (CombatTarget != NewTarget)
    {
        CombatTarget = NewTarget;
        
        // Update blackboard
        UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsObject(TEXT("CombatTarget"), CombatTarget);
        }
        
        // Notify pack members if leader
        if (bIsPackLeader && CombatTarget)
        {
            CoordinatePackAttack(CombatTarget);
        }
    }
}

AActor* ACombatAIController::GetCombatTarget() const
{
    return CombatTarget;
}

bool ACombatAIController::ShouldEngageTarget(AActor* Target) const
{
    if (!Target || Target == GetPawn())
        return false;
        
    // Check if target is a character (player or NPC)
    ACharacter* TargetCharacter = Cast<ACharacter>(Target);
    if (!TargetCharacter)
        return false;
        
    // Don't engage if we're too weak
    if (ShouldFleeFromCombat())
        return false;
        
    // Check aggression level
    float ThreatLevel = AssessThreatLevel(Target);
    switch (AggressionLevel)
    {
        case ECombat_AggressionLevel::Passive:
            return false; // Never engage
            
        case ECombat_AggressionLevel::Defensive:
            return ThreatLevel > 0.7f; // Only engage strong threats
            
        case ECombat_AggressionLevel::Moderate:
            return ThreatLevel > 0.3f; // Engage moderate threats
            
        case ECombat_AggressionLevel::Aggressive:
            return ThreatLevel > 0.1f; // Engage most targets
            
        case ECombat_AggressionLevel::Berserk:
            return true; // Engage everything
    }
    
    return false;
}

bool ACombatAIController::ShouldFleeFromCombat() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return true;
        
    // Check health component if available
    UHealthComponent* HealthComp = ControlledPawn->FindComponentByClass<UHealthComponent>();
    if (HealthComp)
    {
        float HealthPercent = HealthComp->GetHealthPercent();
        return HealthPercent <= FleeHealthThreshold;
    }
    
    return false;
}

void ACombatAIController::JoinPack(ACombatAIController* Leader)
{
    if (!Leader || Leader == this)
        return;
        
    // Leave current pack first
    if (PackLeader || bIsPackLeader)
    {
        LeavePack();
    }
    
    PackLeader = Leader;
    Leader->PackMembers.AddUnique(this);
    bIsPackLeader = false;
    
    // Update blackboard
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsObject(TEXT("PackLeader"), PackLeader);
        BlackboardComp->SetValueAsBool(TEXT("IsPackLeader"), bIsPackLeader);
    }
}

void ACombatAIController::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    if (bIsPackLeader)
    {
        // Transfer leadership or disband pack
        if (PackMembers.Num() > 0)
        {
            ACombatAIController* NewLeader = PackMembers[0];
            NewLeader->BecomePackLeader();
            
            // Transfer remaining members
            for (int32 i = 1; i < PackMembers.Num(); i++)
            {
                PackMembers[i]->JoinPack(NewLeader);
            }
        }
        
        PackMembers.Empty();
        bIsPackLeader = false;
    }
    
    // Update blackboard
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsObject(TEXT("PackLeader"), nullptr);
        BlackboardComp->SetValueAsBool(TEXT("IsPackLeader"), bIsPackLeader);
    }
}

void ACombatAIController::BecomePackLeader()
{
    bIsPackLeader = true;
    PackLeader = nullptr;
    
    // Update blackboard
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsObject(TEXT("PackLeader"), nullptr);
        BlackboardComp->SetValueAsBool(TEXT("IsPackLeader"), bIsPackLeader);
    }
}

void ACombatAIController::CoordinatePackAttack(AActor* Target)
{
    if (!bIsPackLeader || !Target)
        return;
        
    SendPackCommand(ECombat_PackCommand::AttackTarget, Target);
}

FVector ACombatAIController::GetFlankingPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
        return GetPawn()->GetActorLocation();
        
    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector ToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Get perpendicular vector for flanking
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
    
    // Randomize flanking side
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    // Calculate flanking position
    float FlankDistance = AttackRange * 0.8f;
    return TargetLocation + (FlankDirection * FlankDistance);
}

FVector ACombatAIController::GetAmbushPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
        return GetPawn()->GetActorLocation();
        
    // Find position behind target
    FVector TargetLocation = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    
    return TargetLocation - (TargetForward * AttackRange * 1.2f);
}

bool ACombatAIController::IsInOptimalAttackPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
        return false;
        
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange && Distance >= AttackRange * 0.5f;
}

ECombat_CombatState ACombatAIController::GetCombatState() const
{
    return CurrentCombatState;
}

void ACombatAIController::SetCombatState(ECombat_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        
        // Update blackboard
        UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        }
    }
}

float ACombatAIController::AssessThreatLevel(AActor* Target) const
{
    if (!Target)
        return 0.0f;
        
    float ThreatLevel = 0.5f; // Base threat level
    
    // Distance factor (closer = more threatening)
    if (GetPawn())
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
        float DistanceFactor = FMath::Clamp(1.0f - (Distance / SightRadius), 0.0f, 1.0f);
        ThreatLevel += DistanceFactor * 0.3f;
    }
    
    // Health factor (if available)
    UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
    if (TargetHealth)
    {
        float HealthFactor = TargetHealth->GetHealthPercent();
        ThreatLevel += HealthFactor * 0.2f;
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

bool ACombatAIController::IsTargetTooStrong(AActor* Target) const
{
    return AssessThreatLevel(Target) > 0.8f && !bIsPackLeader;
}

bool ACombatAIController::HasAdvantageousPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
        return false;
        
    // Check if we're in flanking position or have height advantage
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    
    // Height advantage
    bool bHasHeightAdvantage = MyLocation.Z > TargetLocation.Z + 100.0f;
    
    // Flanking position (target not facing us directly)
    FVector TargetForward = Target->GetActorForwardVector();
    FVector ToMe = (MyLocation - TargetLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(TargetForward, ToMe);
    bool bIsFlanking = DotProduct < 0.5f; // Less than 60 degrees
    
    return bHasHeightAdvantage || bIsFlanking;
}

void ACombatAIController::UpdatePackFormation()
{
    if (!bIsPackLeader || PackMembers.Num() == 0)
        return;
        
    // Simple formation update - spread pack members around target
    if (CombatTarget)
    {
        FVector TargetLocation = CombatTarget->GetActorLocation();
        float AngleStep = 360.0f / PackMembers.Num();
        
        for (int32 i = 0; i < PackMembers.Num(); i++)
        {
            if (PackMembers[i] && PackMembers[i]->GetPawn())
            {
                float Angle = AngleStep * i;
                FVector FormationPos = TargetLocation + FVector(
                    FMath::Cos(FMath::DegreesToRadians(Angle)) * AttackRange,
                    FMath::Sin(FMath::DegreesToRadians(Angle)) * AttackRange,
                    0.0f
                );
                
                // Set formation position in blackboard
                UBlackboardComponent* MemberBlackboard = PackMembers[i]->GetBlackboardComponent();
                if (MemberBlackboard)
                {
                    MemberBlackboard->SetValueAsVector(TEXT("FormationPosition"), FormationPos);
                }
            }
        }
    }
}

void ACombatAIController::SendPackCommand(ECombat_PackCommand Command, AActor* Target)
{
    for (ACombatAIController* Member : PackMembers)
    {
        if (Member)
        {
            Member->ReceivePackCommand(Command, Target);
        }
    }
}

void ACombatAIController::ReceivePackCommand(ECombat_PackCommand Command, AActor* Target)
{
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp)
        return;
        
    switch (Command)
    {
        case ECombat_PackCommand::AttackTarget:
            SetCombatTarget(Target);
            SetCombatState(ECombat_CombatState::Engaging);
            break;
            
        case ECombat_PackCommand::Retreat:
            SetCombatState(ECombat_CombatState::Fleeing);
            break;
            
        case ECombat_PackCommand::FormUp:
            SetCombatState(ECombat_CombatState::Patrolling);
            break;
            
        case ECombat_PackCommand::HoldPosition:
            SetCombatState(ECombat_CombatState::Idle);
            break;
    }
    
    BlackboardComp->SetValueAsEnum(TEXT("LastPackCommand"), static_cast<uint8>(Command));
}

void ACombatAIController::UpdateCombatBehavior()
{
    if (!GetPawn())
        return;
        
    // Update pack formation if leader
    if (bIsPackLeader)
    {
        UpdatePackFormation();
    }
    
    // Check if we should flee
    if (ShouldFleeFromCombat() && CurrentCombatState != ECombat_CombatState::Fleeing)
    {
        SetCombatState(ECombat_CombatState::Fleeing);
        SetCombatTarget(nullptr);
    }
    
    // Validate current target
    if (CombatTarget)
    {
        if (!IsValid(CombatTarget) || !ShouldEngageTarget(CombatTarget))
        {
            SetCombatTarget(nullptr);
            SetCombatState(ECombat_CombatState::Idle);
        }
    }
}

void ACombatAIController::AssessThreats()
{
    if (!AIPerceptionComponent)
        return;
        
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);
    
    AActor* HighestThreat = nullptr;
    float HighestThreatLevel = 0.0f;
    
    for (AActor* Actor : PerceivedActors)
    {
        if (ShouldEngageTarget(Actor))
        {
            float ThreatLevel = AssessThreatLevel(Actor);
            if (ThreatLevel > HighestThreatLevel)
            {
                HighestThreat = Actor;
                HighestThreatLevel = ThreatLevel;
            }
        }
    }
    
    if (HighestThreat && HighestThreat != CombatTarget)
    {
        SetCombatTarget(HighestThreat);
    }
}