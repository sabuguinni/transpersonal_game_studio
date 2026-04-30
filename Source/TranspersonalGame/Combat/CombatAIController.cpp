#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Initialize sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Initialize hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure AI Perception
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // Initialize combat parameters
    AttackRange = 300.0f;
    DetectionRange = 1500.0f;
    AttackCooldown = 2.0f;
    FleeHealthThreshold = 0.2f;
    
    CurrentCombatState = ECombat_AIState::Idle;
    CurrentTarget = nullptr;
    ThreatLevel = 0.0f;
    LastAttackTime = 0.0f;
    
    // Species defaults
    DinosaurSpecies = EDinosaur_Species::TRex;
    bIsPackHunter = false;
    bIsAmbushPredator = false;
    bIsTerritorial = true;

    // Blackboard keys
    TargetActorKey = TEXT("TargetActor");
    CombatStateKey = TEXT("CombatState");
    ThreatLevelKey = TEXT("ThreatLevel");
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAIPerception();
    InitializeBlackboard();
    
    // Start behavior tree if assigned
    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateThreatLevel();
    UpdateBlackboard();
    
    // Handle combat state logic
    switch (CurrentCombatState)
    {
        case ECombat_AIState::Idle:
            // Look for threats
            if (CurrentTarget && ThreatLevel > 0.3f)
            {
                SetCombatState(ECombat_AIState::Alert);
            }
            break;
            
        case ECombat_AIState::Alert:
            if (!CurrentTarget || ThreatLevel < 0.1f)
            {
                SetCombatState(ECombat_AIState::Idle);
            }
            else if (ThreatLevel > 0.7f && IsTargetInAttackRange())
            {
                SetCombatState(ECombat_AIState::Aggressive);
            }
            break;
            
        case ECombat_AIState::Aggressive:
            if (!CurrentTarget || ShouldFlee())
            {
                SetCombatState(ECombat_AIState::Fleeing);
            }
            else if (CanAttack())
            {
                ExecuteAttack();
            }
            break;
            
        case ECombat_AIState::Fleeing:
            if (!CurrentTarget || ThreatLevel < 0.2f)
            {
                SetCombatState(ECombat_AIState::Idle);
            }
            break;
    }
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        // Configure perception for this pawn
        if (AIPerceptionComponent)
        {
            AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
        }
    }
}

void ACombatAIController::InitializeAIPerception()
{
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    }
}

void ACombatAIController::InitializeBlackboard()
{
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsObject(TargetActorKey, CurrentTarget);
        GetBlackboardComponent()->SetValueAsEnum(CombatStateKey, static_cast<uint8>(CurrentCombatState));
        GetBlackboardComponent()->SetValueAsFloat(ThreatLevelKey, ThreatLevel);
    }
}

void ACombatAIController::UpdateBlackboard()
{
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsObject(TargetActorKey, CurrentTarget);
        GetBlackboardComponent()->SetValueAsEnum(CombatStateKey, static_cast<uint8>(CurrentCombatState));
        GetBlackboardComponent()->SetValueAsFloat(ThreatLevelKey, ThreatLevel);
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            // Check if this is a player or potential threat
            if (Actor->IsA<ACharacter>() || Actor->GetName().Contains(TEXT("Character")))
            {
                float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
                
                if (Distance <= DetectionRange)
                {
                    SetTarget(Actor);
                    UE_LOG(LogTemp, Warning, TEXT("CombatAI: Target detected - %s at distance %f"), *Actor->GetName(), Distance);
                }
            }
        }
    }
}

void ACombatAIController::SetCombatState(ECombat_AIState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        UpdateBlackboard();
        
        UE_LOG(LogTemp, Warning, TEXT("CombatAI: State changed to %d"), static_cast<int32>(NewState));
    }
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    UpdateBlackboard();
    
    if (NewTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatAI: Target set to %s"), *NewTarget->GetName());
    }
}

bool ACombatAIController::CanAttack() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    
    float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;
    return TimeSinceLastAttack >= AttackCooldown && IsTargetInAttackRange();
}

void ACombatAIController::ExecuteAttack()
{
    if (!CanAttack())
    {
        return;
    }
    
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Basic attack logic - could be expanded with animations, damage, etc.
    UE_LOG(LogTemp, Warning, TEXT("CombatAI: Executing attack on %s"), CurrentTarget ? *CurrentTarget->GetName() : TEXT("None"));
    
    // If this is a pack hunter, coordinate with pack
    if (bIsPackHunter)
    {
        CoordinatePackAttack();
    }
}

void ACombatAIController::UpdateThreatLevel()
{
    if (!CurrentTarget || !GetPawn())
    {
        ThreatLevel = 0.0f;
        return;
    }
    
    ThreatLevel = CalculateThreatLevel(CurrentTarget);
}

bool ACombatAIController::ShouldFlee() const
{
    // Check health threshold (simplified - would need actual health component)
    // For now, use threat level as proxy
    return ThreatLevel > 1.5f || (CurrentTarget && !IsTargetVisible());
}

TArray<ACombatAIController*> ACombatAIController::GetNearbyPackMembers(float Radius)
{
    TArray<ACombatAIController*> PackMembers;
    
    if (!GetPawn())
    {
        return PackMembers;
    }
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatAIController::StaticClass(), FoundActors);
    
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (ACombatAIController* OtherController = Cast<ACombatAIController>(Actor))
        {
            if (OtherController != this && OtherController->bIsPackHunter && 
                OtherController->DinosaurSpecies == DinosaurSpecies)
            {
                if (OtherController->GetPawn())
                {
                    float Distance = FVector::Dist(MyLocation, OtherController->GetPawn()->GetActorLocation());
                    if (Distance <= Radius)
                    {
                        PackMembers.Add(OtherController);
                    }
                }
            }
        }
    }
    
    return PackMembers;
}

void ACombatAIController::CoordinatePackAttack()
{
    if (!bIsPackHunter)
    {
        return;
    }
    
    TArray<ACombatAIController*> PackMembers = GetNearbyPackMembers();
    
    for (ACombatAIController* Member : PackMembers)
    {
        if (Member && Member->CurrentCombatState == ECombat_AIState::Alert)
        {
            Member->SetTarget(CurrentTarget);
            Member->SetCombatState(ECombat_AIState::Aggressive);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAI: Coordinating pack attack with %d members"), PackMembers.Num());
}

float ACombatAIController::CalculateThreatLevel(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return 0.0f;
    }
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / DetectionRange), 0.0f, 1.0f);
    
    // Base threat level based on distance
    float Threat = DistanceFactor;
    
    // Increase threat if target is moving towards us
    FVector ToTarget = (Target->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
    FVector TargetVelocity = Target->GetVelocity().GetSafeNormal();
    float MovementThreat = FMath::Max(0.0f, FVector::DotProduct(TargetVelocity, ToTarget));
    
    Threat += MovementThreat * 0.5f;
    
    return FMath::Clamp(Threat, 0.0f, 2.0f);
}

bool ACombatAIController::IsTargetInAttackRange() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Distance <= AttackRange;
}

bool ACombatAIController::IsTargetVisible() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    
    // Simple line trace for visibility
    FHitResult HitResult;
    FVector Start = GetPawn()->GetActorLocation();
    FVector End = CurrentTarget->GetActorLocation();
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility
    );
    
    return !bHit || HitResult.GetActor() == CurrentTarget;
}

void ACombatAIController::SendPackSignal(ECombat_PackSignal Signal)
{
    // Implementation for pack communication
    UE_LOG(LogTemp, Warning, TEXT("CombatAI: Sending pack signal %d"), static_cast<int32>(Signal));
}

void ACombatAIController::ReceivePackSignal(ECombat_PackSignal Signal, ACombatAIController* Sender)
{
    // Implementation for receiving pack signals
    UE_LOG(LogTemp, Warning, TEXT("CombatAI: Received pack signal %d from %s"), 
           static_cast<int32>(Signal), 
           Sender ? *Sender->GetName() : TEXT("Unknown"));
}