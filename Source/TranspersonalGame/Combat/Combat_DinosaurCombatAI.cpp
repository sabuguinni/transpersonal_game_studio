#include "Combat_DinosaurCombatAI.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Math/UnrealMathUtility.h"

ACombat_DinosaurCombatAI::ACombat_DinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Create Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    
    // Create Blackboard Component
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Set default values
    CurrentState = ECombat_DinosaurState::Idle;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    AttackCooldown = 2.0f;
    StateTimer = 0.0f;
    bIsPatrolling = false;
    NextDecisionTime = 0.0f;

    // Initialize dinosaur stats
    InitializeDinosaurStats();
}

void ACombat_DinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    
    SetupAIPerception();
    ConfigureBlackboard();
    
    PatrolStartLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    PatrolTargetLocation = PatrolStartLocation;
    
    SetDinosaurState(ECombat_DinosaurState::Patrolling);
}

void ACombat_DinosaurCombatAI::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (BehaviorTreeAsset && BlackboardComponent)
    {
        BlackboardComponent->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);
        BehaviorTreeComponent->StartTree(*BehaviorTreeAsset);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Combat AI Controller possessed pawn: %s"), 
           InPawn ? *InPawn->GetName() : TEXT("None"));
}

void ACombat_DinosaurCombatAI::OnUnPossess()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
    
    Super::OnUnPossess();
}

void ACombat_DinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateAIBehavior(DeltaTime);
    StateTimer += DeltaTime;
}

void ACombat_DinosaurCombatAI::SetDinosaurState(ECombat_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ECombat_DinosaurState PreviousState = CurrentState;
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), (uint8)NewState);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur AI State changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

bool ACombat_DinosaurCombatAI::DetectPlayer()
{
    if (!GetPawn())
        return false;
    
    // Find all player characters in the world
    TArray<AActor*> PlayerCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), PlayerCharacters);
    
    for (AActor* Actor : PlayerCharacters)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            float Distance = GetDistanceToTarget(Actor);
            if (Distance <= DinosaurStats.DetectionRange)
            {
                CurrentTarget = Actor;
                
                // Update blackboard
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
                    BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
                }
                
                UE_LOG(LogTemp, Log, TEXT("Player detected at distance: %f"), Distance);
                return true;
            }
        }
    }
    
    return false;
}

void ACombat_DinosaurCombatAI::AttackTarget(AActor* Target)
{
    if (!Target || !GetPawn())
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
        return;
    
    float Distance = GetDistanceToTarget(Target);
    if (Distance <= DinosaurStats.AttackRange)
    {
        LastAttackTime = CurrentTime;
        
        // Apply damage to target if it's a character
        if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
        {
            // In a full implementation, this would use a damage system
            UE_LOG(LogTemp, Warning, TEXT("Dinosaur attacking player for %f damage!"), DinosaurStats.AttackDamage);
        }
        
        SetDinosaurState(ECombat_DinosaurState::Attacking);
    }
}

void ACombat_DinosaurCombatAI::MoveToTarget(AActor* Target)
{
    if (!Target)
        return;
    
    FVector TargetLocation = Target->GetActorLocation();
    MoveToLocation(TargetLocation, DinosaurStats.AttackRange * 0.8f);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("MoveToLocation"), TargetLocation);
    }
}

void ACombat_DinosaurCombatAI::StartPatrolling()
{
    if (!GetPawn())
        return;
    
    bIsPatrolling = true;
    
    // Generate random patrol point around start location
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0; // Keep on ground level
    RandomDirection.Normalize();
    
    float PatrolRadius = 1000.0f;
    PatrolTargetLocation = PatrolStartLocation + (RandomDirection * PatrolRadius);
    
    MoveToLocation(PatrolTargetLocation, 100.0f);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolLocation"), PatrolTargetLocation);
    }
    
    SetDinosaurState(ECombat_DinosaurState::Patrolling);
}

void ACombat_DinosaurCombatAI::FleeFromThreat(AActor* Threat)
{
    if (!Threat || !GetPawn())
        return;
    
    FVector ThreatLocation = Threat->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector FleeDirection = (MyLocation - ThreatLocation).GetSafeNormal();
    
    FVector FleeLocation = MyLocation + (FleeDirection * 2000.0f);
    MoveToLocation(FleeLocation, 100.0f);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("FleeLocation"), FleeLocation);
    }
    
    SetDinosaurState(ECombat_DinosaurState::Fleeing);
}

float ACombat_DinosaurCombatAI::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
        return FLT_MAX;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

bool ACombat_DinosaurCombatAI::CanAttackTarget(AActor* Target) const
{
    if (!Target)
        return false;
    
    float Distance = GetDistanceToTarget(Target);
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    return (Distance <= DinosaurStats.AttackRange) && 
           (CurrentTime - LastAttackTime >= AttackCooldown);
}

void ACombat_DinosaurCombatAI::InitializeDinosaurStats()
{
    switch (DinosaurSpecies)
    {
        case ECombat_DinosaurSpecies::TRex:
            DinosaurStats.Health = 200.0f;
            DinosaurStats.AttackDamage = 50.0f;
            DinosaurStats.AttackRange = 400.0f;
            DinosaurStats.DetectionRange = 2000.0f;
            DinosaurStats.MovementSpeed = 600.0f;
            DinosaurStats.Aggression = 0.9f;
            break;
            
        case ECombat_DinosaurSpecies::Velociraptor:
            DinosaurStats.Health = 80.0f;
            DinosaurStats.AttackDamage = 30.0f;
            DinosaurStats.AttackRange = 200.0f;
            DinosaurStats.DetectionRange = 1800.0f;
            DinosaurStats.MovementSpeed = 800.0f;
            DinosaurStats.Aggression = 0.8f;
            break;
            
        case ECombat_DinosaurSpecies::Triceratops:
            DinosaurStats.Health = 300.0f;
            DinosaurStats.AttackDamage = 40.0f;
            DinosaurStats.AttackRange = 350.0f;
            DinosaurStats.DetectionRange = 1200.0f;
            DinosaurStats.MovementSpeed = 400.0f;
            DinosaurStats.Aggression = 0.5f;
            break;
            
        case ECombat_DinosaurSpecies::Brachiosaurus:
            DinosaurStats.Health = 500.0f;
            DinosaurStats.AttackDamage = 60.0f;
            DinosaurStats.AttackRange = 600.0f;
            DinosaurStats.DetectionRange = 1000.0f;
            DinosaurStats.MovementSpeed = 300.0f;
            DinosaurStats.Aggression = 0.3f;
            break;
            
        case ECombat_DinosaurSpecies::Pteranodon:
            DinosaurStats.Health = 60.0f;
            DinosaurStats.AttackDamage = 20.0f;
            DinosaurStats.AttackRange = 300.0f;
            DinosaurStats.DetectionRange = 2500.0f;
            DinosaurStats.MovementSpeed = 1000.0f;
            DinosaurStats.Aggression = 0.6f;
            break;
    }
}

void ACombat_DinosaurCombatAI::UpdateAIBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECombat_DinosaurState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ECombat_DinosaurState::Hunting:
            HandleHuntingState(DeltaTime);
            break;
        case ECombat_DinosaurState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
        case ECombat_DinosaurState::Fleeing:
            HandleFleeingState(DeltaTime);
            break;
        case ECombat_DinosaurState::Patrolling:
            HandlePatrollingState(DeltaTime);
            break;
    }
}

void ACombat_DinosaurCombatAI::HandleIdleState(float DeltaTime)
{
    if (StateTimer > 3.0f)
    {
        if (DetectPlayer())
        {
            SetDinosaurState(ECombat_DinosaurState::Hunting);
        }
        else
        {
            StartPatrolling();
        }
    }
}

void ACombat_DinosaurCombatAI::HandleHuntingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ECombat_DinosaurState::Idle);
        return;
    }
    
    float Distance = GetDistanceToTarget(CurrentTarget);
    
    if (Distance > DinosaurStats.DetectionRange * 1.5f)
    {
        CurrentTarget = nullptr;
        SetDinosaurState(ECombat_DinosaurState::Idle);
    }
    else if (Distance <= DinosaurStats.AttackRange)
    {
        SetDinosaurState(ECombat_DinosaurState::Attacking);
    }
    else
    {
        MoveToTarget(CurrentTarget);
    }
}

void ACombat_DinosaurCombatAI::HandleAttackingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ECombat_DinosaurState::Idle);
        return;
    }
    
    float Distance = GetDistanceToTarget(CurrentTarget);
    
    if (Distance > DinosaurStats.AttackRange * 1.2f)
    {
        SetDinosaurState(ECombat_DinosaurState::Hunting);
    }
    else if (CanAttackTarget(CurrentTarget))
    {
        AttackTarget(CurrentTarget);
    }
}

void ACombat_DinosaurCombatAI::HandleFleeingState(float DeltaTime)
{
    if (StateTimer > 10.0f)
    {
        SetDinosaurState(ECombat_DinosaurState::Idle);
    }
}

void ACombat_DinosaurCombatAI::HandlePatrollingState(float DeltaTime)
{
    if (DetectPlayer())
    {
        SetDinosaurState(ECombat_DinosaurState::Hunting);
        return;
    }
    
    if (!GetPawn())
        return;
    
    float DistanceToPatrolTarget = FVector::Dist(GetPawn()->GetActorLocation(), PatrolTargetLocation);
    
    if (DistanceToPatrolTarget < 200.0f || StateTimer > 15.0f)
    {
        StartPatrolling(); // Generate new patrol point
    }
}

void ACombat_DinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            CurrentTarget = Actor;
            if (CurrentState == ECombat_DinosaurState::Idle || CurrentState == ECombat_DinosaurState::Patrolling)
            {
                SetDinosaurState(ECombat_DinosaurState::Hunting);
            }
            break;
        }
    }
}

void ACombat_DinosaurCombatAI::SetupAIPerception()
{
    if (!AIPerceptionComponent)
        return;
    
    // Configure sight sense
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = DinosaurStats.DetectionRange;
        SightConfig->LoseSightRadius = DinosaurStats.DetectionRange * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }
    
    // Bind perception update delegate
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatAI::OnPerceptionUpdated);
}

void ACombat_DinosaurCombatAI::ConfigureBlackboard()
{
    if (!BlackboardComponent)
        return;
    
    // Initialize blackboard values
    BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), (uint8)CurrentState);
    BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
    BlackboardComponent->SetValueAsVector(TEXT("PatrolLocation"), PatrolStartLocation);
}

AActor* ACombat_DinosaurCombatAI::FindNearestPlayer() const
{
    if (!GetPawn())
        return nullptr;
    
    TArray<AActor*> PlayerCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), PlayerCharacters);
    
    AActor* NearestPlayer = nullptr;
    float NearestDistance = FLT_MAX;
    
    for (AActor* Actor : PlayerCharacters)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            float Distance = GetDistanceToTarget(Actor);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestPlayer = Actor;
            }
        }
    }
    
    return NearestPlayer;
}

bool ACombat_DinosaurCombatAI::IsPlayerInRange(float Range) const
{
    AActor* NearestPlayer = FindNearestPlayer();
    if (!NearestPlayer)
        return false;
    
    return GetDistanceToTarget(NearestPlayer) <= Range;
}