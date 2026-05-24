#include "Combat_AIController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"

ACombat_AIController::ACombat_AIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Setup sight configuration
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = FieldOfView;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Setup hearing configuration
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure AI Perception
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Initialize state
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    bIsFleeingFromThreat = false;
    bIsPackLeader = false;
    LastKnownTargetLocation = FVector::ZeroVector;
}

void ACombat_AIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnTargetPerceptionUpdated);
    }
    
    // Start behavior tree if available
    if (BehaviorTree && BlackboardComponent)
    {
        UseBlackboard(BehaviorTree->BlackboardAsset);
        RunBehaviorTree(BehaviorTree);
    }
    
    // Initialize pack behavior
    UpdatePackBehavior();
}

void ACombat_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCombatState();
    HandleCombatBehavior();
}

void ACombat_AIController::SetCombatTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent && NewTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), NewTarget->GetActorLocation());
        LastKnownTargetLocation = NewTarget->GetActorLocation();
        
        // Update AI state
        CurrentAIState = ECombat_AIState::Combat;
        BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentAIState));
        
        // Alert pack members
        BroadcastThreatToPackMembers(NewTarget);
    }
}

void ACombat_AIController::ClearCombatTarget()
{
    CurrentTarget = nullptr;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->ClearValue(TEXT("TargetActor"));
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownTargetLocation"), LastKnownTargetLocation);
        
        // Return to patrol state
        CurrentAIState = ECombat_AIState::Patrol;
        BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentAIState));
    }
}

bool ACombat_AIController::IsInCombat() const
{
    return CurrentTarget != nullptr && CurrentAIState == ECombat_AIState::Combat;
}

void ACombat_AIController::ExecuteAttack()
{
    if (!CanAttackTarget())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return;
    }
    
    LastAttackTime = CurrentTime;
    
    // Execute attack based on dinosaur species
    switch (DinosaurSpecies)
    {
        case ECombat_DinosaurSpecies::TRex:
            // Powerful bite attack
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("T-Rex executes devastating bite attack!"));
            }
            break;
            
        case ECombat_DinosaurSpecies::Raptor:
            // Quick claw strike
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Raptor executes swift claw strike!"));
            }
            break;
            
        case ECombat_DinosaurSpecies::Triceratops:
            // Charge attack
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Triceratops charges with horns!"));
            }
            break;
            
        default:
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("Dinosaur executes basic attack!"));
            }
            break;
    }
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("LastAttackTime"), LastAttackTime);
    }
}

void ACombat_AIController::ExecuteFlee()
{
    bIsFleeingFromThreat = true;
    CurrentAIState = ECombat_AIState::Flee;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentAIState));
        BlackboardComponent->SetValueAsBool(TEXT("IsFleeingFromThreat"), true);
        
        // Set flee direction away from threat
        if (CurrentTarget && GetPawn())
        {
            FVector FleeDirection = (GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
            FVector FleeLocation = GetPawn()->GetActorLocation() + (FleeDirection * 2000.0f);
            BlackboardComponent->SetValueAsVector(TEXT("FleeLocation"), FleeLocation);
        }
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Dinosaur is fleeing from threat!"));
    }
}

void ACombat_AIController::CallForBackup()
{
    // Alert nearby pack members
    for (ACombat_AIController* PackMember : PackMembers)
    {
        if (PackMember && PackMember != this && CurrentTarget)
        {
            PackMember->SetCombatTarget(CurrentTarget);
        }
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Called %d pack members for backup!"), PackMembers.Num()));
    }
}

void ACombat_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>() && Actor != GetPawn())
        {
            // Check if this is a potential threat (player or other dinosaur)
            if (Actor->ActorHasTag(TEXT("Player")) || Actor->ActorHasTag(TEXT("Dinosaur")))
            {
                SetCombatTarget(Actor);
                break;
            }
        }
    }
}

void ACombat_AIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }
    
    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        if (Actor->ActorHasTag(TEXT("Player")))
        {
            SetCombatTarget(Actor);
        }
    }
    else
    {
        // Target lost
        if (Actor == CurrentTarget)
        {
            LastKnownTargetLocation = Actor->GetActorLocation();
            ClearCombatTarget();
        }
    }
}

void ACombat_AIController::UpdateCombatState()
{
    if (!GetPawn())
    {
        return;
    }
    
    // Check health for flee condition
    // Note: This would typically check the pawn's health component
    // For now, we'll use a placeholder condition
    
    // Update target distance if we have a target
    if (CurrentTarget)
    {
        float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), DistanceToTarget);
        }
        
        // Check if target is in attack range
        if (DistanceToTarget <= AttackRange && CurrentAIState == ECombat_AIState::Combat)
        {
            ExecuteAttack();
        }
    }
}

void ACombat_AIController::HandleCombatBehavior()
{
    switch (CurrentAIState)
    {
        case ECombat_AIState::Patrol:
            // Patrol behavior handled by behavior tree
            break;
            
        case ECombat_AIState::Combat:
            if (CurrentTarget)
            {
                // Combat behavior handled by behavior tree and ExecuteAttack
            }
            else
            {
                // No target, return to patrol
                CurrentAIState = ECombat_AIState::Patrol;
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentAIState));
                }
            }
            break;
            
        case ECombat_AIState::Flee:
            // Flee behavior handled by behavior tree
            break;
            
        case ECombat_AIState::Hunt:
            // Hunt behavior for predators
            break;
    }
}

void ACombat_AIController::UpdatePackBehavior()
{
    // Find nearby pack members of the same species
    PackMembers.Empty();
    
    if (GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombat_AIController::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            ACombat_AIController* OtherController = Cast<ACombat_AIController>(Actor);
            if (OtherController && OtherController != this && OtherController->DinosaurSpecies == DinosaurSpecies)
            {
                float Distance = FVector::Dist(GetActorLocation(), OtherController->GetActorLocation());
                if (Distance <= 3000.0f) // Pack range
                {
                    PackMembers.Add(OtherController);
                }
            }
        }
    }
    
    // Determine pack leadership (first controller becomes leader)
    bIsPackLeader = PackMembers.Num() > 0;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsPackLeader"), bIsPackLeader);
        BlackboardComponent->SetValueAsInt(TEXT("PackSize"), PackMembers.Num() + 1);
    }
}

bool ACombat_AIController::CanAttackTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return DistanceToTarget <= AttackRange;
}

void ACombat_AIController::BroadcastThreatToPackMembers(AActor* Threat)
{
    for (ACombat_AIController* PackMember : PackMembers)
    {
        if (PackMember && PackMember != this)
        {
            PackMember->SetCombatTarget(Threat);
        }
    }
}