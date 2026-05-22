#include "Combat_DinosaurAI.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

// UCombat_DinosaurBehavior Implementation
UCombat_DinosaurBehavior::UCombat_DinosaurBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default stats
    Stats.Health = 100.0f;
    Stats.MaxHealth = 100.0f;
    Stats.AttackDamage = 25.0f;
    Stats.AttackRange = 300.0f;
    Stats.SightRange = 2000.0f;
    Stats.HearingRange = 1500.0f;
    Stats.MovementSpeed = 400.0f;
    Stats.Aggression = 0.5f;
    Stats.TerritorialRadius = 1000.0f;
}

void UCombat_DinosaurBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStats();
    SetDinosaurState(ECombat_DinosaurState::Idle);
    StateChangeTime = GetWorld()->GetTimeSeconds();
}

void UCombat_DinosaurBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update behavior based on current state
    switch (CurrentState)
    {
        case ECombat_DinosaurState::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Patrolling:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Hunting:
            UpdateHuntingBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Attacking:
            UpdateAttackingBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Fleeing:
            UpdateFleeingBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Feeding:
            UpdateFeedingBehavior(DeltaTime);
            break;
        case ECombat_DinosaurState::Territorial:
            UpdateTerritorialBehavior(DeltaTime);
            break;
    }
}

void UCombat_DinosaurBehavior::SetDinosaurState(ECombat_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        // Log state change for debugging
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s changed state to %d"), 
               *GetOwner()->GetName(), (int32)NewState);
    }
}

void UCombat_DinosaurBehavior::SetSpecies(ECombat_DinosaurSpecies Species)
{
    DinosaurSpecies = Species;
    InitializeStats();
}

void UCombat_DinosaurBehavior::InitializeStats()
{
    // Set species-specific stats
    switch (DinosaurSpecies)
    {
        case ECombat_DinosaurSpecies::TRex:
            Stats.MaxHealth = 500.0f;
            Stats.AttackDamage = 75.0f;
            Stats.AttackRange = 400.0f;
            Stats.SightRange = 3000.0f;
            Stats.MovementSpeed = 600.0f;
            Stats.Aggression = 0.9f;
            Stats.TerritorialRadius = 2000.0f;
            break;
            
        case ECombat_DinosaurSpecies::Velociraptor:
            Stats.MaxHealth = 150.0f;
            Stats.AttackDamage = 35.0f;
            Stats.AttackRange = 200.0f;
            Stats.SightRange = 2500.0f;
            Stats.MovementSpeed = 800.0f;
            Stats.Aggression = 0.8f;
            Stats.TerritorialRadius = 800.0f;
            break;
            
        case ECombat_DinosaurSpecies::Triceratops:
            Stats.MaxHealth = 400.0f;
            Stats.AttackDamage = 50.0f;
            Stats.AttackRange = 350.0f;
            Stats.SightRange = 1800.0f;
            Stats.MovementSpeed = 300.0f;
            Stats.Aggression = 0.3f;
            Stats.TerritorialRadius = 1200.0f;
            break;
            
        case ECombat_DinosaurSpecies::Brachiosaurus:
            Stats.MaxHealth = 800.0f;
            Stats.AttackDamage = 30.0f;
            Stats.AttackRange = 500.0f;
            Stats.SightRange = 2200.0f;
            Stats.MovementSpeed = 200.0f;
            Stats.Aggression = 0.1f;
            Stats.TerritorialRadius = 1500.0f;
            break;
            
        case ECombat_DinosaurSpecies::Ankylosaurus:
            Stats.MaxHealth = 350.0f;
            Stats.AttackDamage = 40.0f;
            Stats.AttackRange = 300.0f;
            Stats.SightRange = 1500.0f;
            Stats.MovementSpeed = 250.0f;
            Stats.Aggression = 0.4f;
            Stats.TerritorialRadius = 1000.0f;
            break;
            
        case ECombat_DinosaurSpecies::Parasaurolophus:
            Stats.MaxHealth = 200.0f;
            Stats.AttackDamage = 20.0f;
            Stats.AttackRange = 250.0f;
            Stats.SightRange = 2800.0f;
            Stats.HearingRange = 3000.0f;
            Stats.MovementSpeed = 500.0f;
            Stats.Aggression = 0.2f;
            Stats.TerritorialRadius = 600.0f;
            break;
    }
    
    Stats.Health = Stats.MaxHealth;
}

void UCombat_DinosaurBehavior::AttackTarget(AActor* Target)
{
    if (!Target || !CanAttackTarget(Target))
        return;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
        return;
        
    LastAttackTime = CurrentTime;
    
    // Apply damage to target if it has a health component
    if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
    {
        // TODO: Apply damage through damage system
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacks %s for %.1f damage"), 
               *GetOwner()->GetName(), *Target->GetName(), Stats.AttackDamage);
    }
    
    SetDinosaurState(ECombat_DinosaurState::Attacking);
}

bool UCombat_DinosaurBehavior::CanAttackTarget(AActor* Target) const
{
    if (!Target)
        return false;
        
    float Distance = GetDistanceToTarget(Target);
    return Distance <= Stats.AttackRange;
}

void UCombat_DinosaurBehavior::TakeDamage(float Damage, AActor* Attacker)
{
    Stats.Health = FMath::Max(0.0f, Stats.Health - Damage);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s takes %.1f damage (Health: %.1f/%.1f)"), 
           *GetOwner()->GetName(), Damage, Stats.Health, Stats.MaxHealth);
    
    if (Stats.Health <= 0.0f)
    {
        // Handle death
        UE_LOG(LogTemp, Error, TEXT("Dinosaur %s has died!"), *GetOwner()->GetName());
        return;
    }
    
    // React to damage
    if (Attacker && Stats.Aggression > 0.3f)
    {
        CurrentTarget = Attacker;
        SetDinosaurState(ECombat_DinosaurState::Hunting);
    }
    else if (Stats.Aggression < 0.5f)
    {
        SetDinosaurState(ECombat_DinosaurState::Fleeing);
    }
}

void UCombat_DinosaurBehavior::OnTargetSighted(AActor* Target)
{
    if (!Target)
        return;
        
    // Check if target is a threat based on species behavior
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(Target))
    {
        CurrentTarget = Target;
        LastKnownTargetLocation = Target->GetActorLocation();
        
        if (Stats.Aggression > 0.6f)
        {
            SetDinosaurState(ECombat_DinosaurState::Hunting);
        }
        else if (Stats.Aggression < 0.3f)
        {
            SetDinosaurState(ECombat_DinosaurState::Fleeing);
        }
        else
        {
            SetDinosaurState(ECombat_DinosaurState::Territorial);
        }
    }
}

void UCombat_DinosaurBehavior::OnTargetLost(AActor* Target)
{
    if (Target == CurrentTarget)
    {
        LastKnownTargetLocation = Target->GetActorLocation();
        
        // Continue hunting for a short time
        if (CurrentState == ECombat_DinosaurState::Hunting)
        {
            // Will timeout and return to patrol/idle
        }
        else
        {
            CurrentTarget = nullptr;
            SetDinosaurState(ECombat_DinosaurState::Patrolling);
        }
    }
}

void UCombat_DinosaurBehavior::OnSoundHeard(FVector SoundLocation, float SoundIntensity)
{
    if (CurrentState == ECombat_DinosaurState::Idle || CurrentState == ECombat_DinosaurState::Patrolling)
    {
        LastKnownTargetLocation = SoundLocation;
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
    }
}

// Behavior update functions
void UCombat_DinosaurBehavior::UpdateIdleBehavior(float DeltaTime)
{
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    
    // Randomly start patrolling after being idle for a while
    if (TimeSinceStateChange > FMath::RandRange(5.0f, 15.0f))
    {
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
    }
}

void UCombat_DinosaurBehavior::UpdatePatrolBehavior(float DeltaTime)
{
    // Simple patrol behavior - move in random directions
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f;
        RandomDirection.Normalize();
        
        FVector NewLocation = OwnerPawn->GetActorLocation() + RandomDirection * Stats.MovementSpeed * DeltaTime;
        OwnerPawn->SetActorLocation(NewLocation);
    }
    
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    if (TimeSinceStateChange > FMath::RandRange(10.0f, 30.0f))
    {
        SetDinosaurState(ECombat_DinosaurState::Idle);
    }
}

void UCombat_DinosaurBehavior::UpdateHuntingBehavior(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
        return;
    }
    
    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
    
    if (DistanceToTarget <= Stats.AttackRange)
    {
        AttackTarget(CurrentTarget);
    }
    else if (DistanceToTarget <= Stats.SightRange)
    {
        MoveTowardsTarget(CurrentTarget, DeltaTime);
        RotateTowardsTarget(CurrentTarget, DeltaTime);
    }
    else
    {
        // Target too far, give up hunt
        CurrentTarget = nullptr;
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
    }
}

void UCombat_DinosaurBehavior::UpdateAttackingBehavior(float DeltaTime)
{
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    
    // Attack animation duration
    if (TimeSinceStateChange > 1.5f)
    {
        if (CurrentTarget && GetDistanceToTarget(CurrentTarget) <= Stats.SightRange)
        {
            SetDinosaurState(ECombat_DinosaurState::Hunting);
        }
        else
        {
            SetDinosaurState(ECombat_DinosaurState::Patrolling);
        }
    }
}

void UCombat_DinosaurBehavior::UpdateFleeingBehavior(float DeltaTime)
{
    if (CurrentTarget)
    {
        MoveAwayFromTarget(CurrentTarget, DeltaTime);
    }
    
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    if (TimeSinceStateChange > 10.0f || !CurrentTarget)
    {
        CurrentTarget = nullptr;
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
    }
}

void UCombat_DinosaurBehavior::UpdateFeedingBehavior(float DeltaTime)
{
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    
    // Feeding duration
    if (TimeSinceStateChange > FMath::RandRange(15.0f, 30.0f))
    {
        SetDinosaurState(ECombat_DinosaurState::Idle);
    }
}

void UCombat_DinosaurBehavior::UpdateTerritorialBehavior(float DeltaTime)
{
    if (CurrentTarget)
    {
        float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
        
        if (DistanceToTarget > Stats.TerritorialRadius)
        {
            // Target left territory
            CurrentTarget = nullptr;
            SetDinosaurState(ECombat_DinosaurState::Idle);
        }
        else if (DistanceToTarget < Stats.TerritorialRadius * 0.5f)
        {
            // Target too close, become aggressive
            if (Stats.Aggression > 0.4f)
            {
                SetDinosaurState(ECombat_DinosaurState::Hunting);
            }
            else
            {
                // Display warning behavior
                RotateTowardsTarget(CurrentTarget, DeltaTime);
            }
        }
    }
    else
    {
        SetDinosaurState(ECombat_DinosaurState::Patrolling);
    }
}

// Utility functions
bool UCombat_DinosaurBehavior::IsTargetInRange(AActor* Target, float Range) const
{
    if (!Target)
        return false;
        
    return GetDistanceToTarget(Target) <= Range;
}

float UCombat_DinosaurBehavior::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
        return FLT_MAX;
        
    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
}

void UCombat_DinosaurBehavior::MoveTowardsTarget(AActor* Target, float DeltaTime)
{
    if (!Target || !GetOwner())
        return;
        
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
        return;
        
    FVector Direction = (Target->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = OwnerPawn->GetActorLocation() + Direction * Stats.MovementSpeed * DeltaTime;
    
    OwnerPawn->SetActorLocation(NewLocation);
}

void UCombat_DinosaurBehavior::MoveAwayFromTarget(AActor* Target, float DeltaTime)
{
    if (!Target || !GetOwner())
        return;
        
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
        return;
        
    FVector Direction = (OwnerPawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = OwnerPawn->GetActorLocation() + Direction * Stats.MovementSpeed * DeltaTime;
    
    OwnerPawn->SetActorLocation(NewLocation);
}

void UCombat_DinosaurBehavior::RotateTowardsTarget(AActor* Target, float DeltaTime)
{
    if (!Target || !GetOwner())
        return;
        
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
        return;
        
    FVector Direction = (Target->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = OwnerPawn->GetActorRotation();
    
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 2.0f);
    OwnerPawn->SetActorRotation(NewRotation);
}

// ACombat_DinosaurAIController Implementation
ACombat_DinosaurAIController::ACombat_DinosaurAIController()
{
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    SetupPerception();
}

void ACombat_DinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    SetupBlackboard();
    
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurAIController::OnPerceptionUpdated);
    }
}

void ACombat_DinosaurAIController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);
    
    StartBehaviorTree();
}

void ACombat_DinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ACharacter* PlayerCharacter = Cast<ACharacter>(Actor))
        {
            // Notify behavior component
            if (APawn* ControlledPawn = GetPawn())
            {
                if (UCombat_DinosaurBehavior* BehaviorComp = ControlledPawn->FindComponentByClass<UCombat_DinosaurBehavior>())
                {
                    BehaviorComp->OnTargetSighted(Actor);
                }
            }
            
            // Update blackboard
            SetBlackboardValue(TEXT("TargetActor"), Actor);
            SetBlackboardVector(TEXT("TargetLocation"), Actor->GetActorLocation());
        }
    }
}

void ACombat_DinosaurAIController::StartBehaviorTree()
{
    if (BehaviorTreeAsset && BehaviorTreeComponent && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }
}

void ACombat_DinosaurAIController::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ACombat_DinosaurAIController::SetBlackboardValue(const FName& KeyName, UObject* Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(KeyName, Value);
    }
}

void ACombat_DinosaurAIController::SetBlackboardVector(const FName& KeyName, FVector Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(KeyName, Value);
    }
}

void ACombat_DinosaurAIController::SetBlackboardFloat(const FName& KeyName, float Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(KeyName, Value);
    }
}

void ACombat_DinosaurAIController::SetupPerception()
{
    if (!AIPerceptionComponent)
        return;
        
    // Setup sight perception
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 2000.0f;
        SightConfig->LoseSightRadius = 2200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }
    
    // Setup hearing perception
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1500.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }
    
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ACombat_DinosaurAIController::SetupBlackboard()
{
    if (BlackboardAsset && BlackboardComponent)
    {
        UseBlackboard(BlackboardAsset);
    }
}