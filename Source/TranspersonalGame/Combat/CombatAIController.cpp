#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure Sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    
    // Configure Hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Initialize default values
    CurrentState = ECombat_AIState::Idle;
    DinosaurType = ECombat_DinosaurType::Predator;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    TimeWithoutTarget = 0.0f;
    
    // Default combat stats
    CombatStats.Health = 100.0f;
    CombatStats.MaxHealth = 100.0f;
    CombatStats.AttackDamage = 25.0f;
    CombatStats.AttackRange = 200.0f;
    CombatStats.DetectionRange = 1500.0f;
    CombatStats.MovementSpeed = 600.0f;
    CombatStats.Aggression = 0.7f;
    CombatStats.PackCoordination = 0.5f;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    }
    
    // Start behavior tree if available
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
        
        // Initialize blackboard values
        GetBlackboardComponent()->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentState));
        GetBlackboardComponent()->SetValueAsEnum(TEXT("DinosaurType"), static_cast<uint8>(DinosaurType));
        GetBlackboardComponent()->SetValueAsFloat(TEXT("AttackRange"), CombatStats.AttackRange);
        GetBlackboardComponent()->SetValueAsFloat(TEXT("DetectionRange"), CombatStats.DetectionRange);
    }
    
    // Set movement speed
    if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
    {
        if (UCharacterMovementComponent* MovementComp = ControlledCharacter->GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = CombatStats.MovementSpeed;
        }
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCombatLogic();
    UpdatePackBehavior();
    ProcessPerceptionData();
    
    // Update time without target
    if (!CurrentTarget)
    {
        TimeWithoutTarget += DeltaTime;
    }
    else
    {
        TimeWithoutTarget = 0.0f;
    }
}

void ACombatAIController::SetCombatState(ECombat_AIState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentState));
        }
        
        UE_LOG(LogTemp, Log, TEXT("Combat AI State changed to: %d"), static_cast<int32>(CurrentState));
    }
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
        
        if (CurrentTarget)
        {
            GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
            LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        }
    }
    
    if (CurrentTarget)
    {
        AddToKnownEnemies(CurrentTarget);
        SetCombatState(ECombat_AIState::Combat);
    }
}

void ACombatAIController::AddToKnownEnemies(AActor* Enemy)
{
    if (Enemy && !KnownEnemies.Contains(Enemy))
    {
        KnownEnemies.Add(Enemy);
        UE_LOG(LogTemp, Log, TEXT("Added enemy to known list: %s"), *Enemy->GetName());
    }
}

void ACombatAIController::RemoveFromKnownEnemies(AActor* Enemy)
{
    if (Enemy)
    {
        KnownEnemies.Remove(Enemy);
        UE_LOG(LogTemp, Log, TEXT("Removed enemy from known list: %s"), *Enemy->GetName());
    }
}

bool ACombatAIController::IsInCombatRange(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= CombatStats.AttackRange;
}

bool ACombatAIController::IsInDetectionRange(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= CombatStats.DetectionRange;
}

void ACombatAIController::AttackTarget()
{
    if (!CurrentTarget || !GetPawn())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return;
    }
    
    if (IsInCombatRange(CurrentTarget))
    {
        LastAttackTime = CurrentTime;
        
        // Face the target
        FVector Direction = (CurrentTarget->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
        FRotator NewRotation = Direction.Rotation();
        GetPawn()->SetActorRotation(NewRotation);
        
        // Apply damage if target is a character
        if (ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
        {
            // Apply damage logic here
            UE_LOG(LogTemp, Log, TEXT("Combat AI attacking target: %s for %f damage"), 
                   *CurrentTarget->GetName(), CombatStats.AttackDamage);
        }
        
        // Coordinate pack attack if in pack
        if (PackMembers.Num() > 0)
        {
            CoordinatePackAttack(CurrentTarget);
        }
    }
}

void ACombatAIController::FleeFromThreat()
{
    if (!GetPawn())
    {
        return;
    }
    
    SetCombatState(ECombat_AIState::Flee);
    
    // Find flee direction (opposite of threat)
    FVector FleeDirection = FVector::ZeroVector;
    
    if (CurrentTarget)
    {
        FleeDirection = (GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
    }
    else if (KnownEnemies.Num() > 0)
    {
        // Flee from average position of known enemies
        FVector EnemyCenter = FVector::ZeroVector;
        int32 ValidEnemies = 0;
        
        for (AActor* Enemy : KnownEnemies)
        {
            if (IsValid(Enemy))
            {
                EnemyCenter += Enemy->GetActorLocation();
                ValidEnemies++;
            }
        }
        
        if (ValidEnemies > 0)
        {
            EnemyCenter /= ValidEnemies;
            FleeDirection = (GetPawn()->GetActorLocation() - EnemyCenter).GetSafeNormal();
        }
    }
    
    if (!FleeDirection.IsZero())
    {
        FVector FleeLocation = GetPawn()->GetActorLocation() + (FleeDirection * 2000.0f);
        
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsVector(TEXT("FleeLocation"), FleeLocation);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Combat AI fleeing to location: %s"), *FleeLocation.ToString());
    }
}

void ACombatAIController::JoinPack(ACombatAIController* PackLeader)
{
    if (PackLeader && PackLeader != this)
    {
        if (!PackMembers.Contains(PackLeader))
        {
            PackMembers.Add(PackLeader);
        }
        
        // Add this AI to the pack leader's pack
        if (!PackLeader->PackMembers.Contains(this))
        {
            PackLeader->PackMembers.Add(this);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Joined pack with leader: %s"), *PackLeader->GetName());
    }
}

void ACombatAIController::LeavePack()
{
    // Remove this AI from all pack members' lists
    for (ACombatAIController* Member : PackMembers)
    {
        if (IsValid(Member))
        {
            Member->PackMembers.Remove(this);
        }
    }
    
    PackMembers.Empty();
    UE_LOG(LogTemp, Log, TEXT("Left pack"));
}

void ACombatAIController::CoordinatePackAttack(AActor* Target)
{
    if (!Target || PackMembers.Num() == 0)
    {
        return;
    }
    
    // Signal pack members to attack the same target
    for (ACombatAIController* Member : PackMembers)
    {
        if (IsValid(Member) && Member != this)
        {
            Member->SetTarget(Target);
            Member->SetCombatState(ECombat_AIState::Combat);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Coordinated pack attack on: %s"), *Target->GetName());
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        // Check if this is a player character
        if (Actor->IsA<ACharacter>() && Actor->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            if (IsInDetectionRange(Actor))
            {
                SetTarget(Actor);
                UE_LOG(LogTemp, Log, TEXT("Combat AI detected player: %s"), *Actor->GetName());
            }
        }
    }
}

void ACombatAIController::UpdateCombatLogic()
{
    if (!GetPawn())
    {
        return;
    }
    
    switch (CurrentState)
    {
        case ECombat_AIState::Idle:
            if (KnownEnemies.Num() > 0)
            {
                SetCombatState(ECombat_AIState::Alert);
            }
            break;
            
        case ECombat_AIState::Alert:
            if (CurrentTarget && IsInDetectionRange(CurrentTarget))
            {
                SetCombatState(ECombat_AIState::Combat);
            }
            else if (TimeWithoutTarget > 10.0f)
            {
                SetCombatState(ECombat_AIState::Patrol);
            }
            break;
            
        case ECombat_AIState::Combat:
            if (CurrentTarget)
            {
                if (IsInCombatRange(CurrentTarget))
                {
                    AttackTarget();
                }
                else if (!IsInDetectionRange(CurrentTarget))
                {
                    SetCombatState(ECombat_AIState::Alert);
                    CurrentTarget = nullptr;
                }
            }
            else
            {
                SetCombatState(ECombat_AIState::Alert);
            }
            break;
            
        case ECombat_AIState::Flee:
            // Check if threat is gone
            if (TimeWithoutTarget > 5.0f)
            {
                SetCombatState(ECombat_AIState::Patrol);
            }
            break;
            
        default:
            break;
    }
}

void ACombatAIController::UpdatePackBehavior()
{
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    // Clean up invalid pack members
    PackMembers.RemoveAll([](ACombatAIController* Member) {
        return !IsValid(Member);
    });
    
    // Share target information with pack
    if (CurrentTarget)
    {
        for (ACombatAIController* Member : PackMembers)
        {
            if (IsValid(Member) && !Member->CurrentTarget)
            {
                Member->SetTarget(CurrentTarget);
            }
        }
    }
}

void ACombatAIController::ProcessPerceptionData()
{
    if (!AIPerceptionComponent)
    {
        return;
    }
    
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);
    
    // Clean up known enemies list
    KnownEnemies.RemoveAll([this](AActor* Enemy) {
        return !IsValid(Enemy) || !IsInDetectionRange(Enemy);
    });
}