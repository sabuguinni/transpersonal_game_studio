#include "NPC_DinosaurAI.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UNPC_DinosaurAI::UNPC_DinosaurAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Create sensing component
    PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
    PawnSensingComponent->SightRadius = 1500.0f;
    PawnSensingComponent->HearingThreshold = 1200.0f;
    PawnSensingComponent->LOSHearingThreshold = 800.0f;
    PawnSensingComponent->SensingInterval = 0.2f;
    PawnSensingComponent->SetPeripheralVisionAngle(90.0f);

    // Initialize default stats
    Stats.Health = 100.0f;
    Stats.MaxHealth = 100.0f;
    Stats.Hunger = 50.0f;
    Stats.Aggression = 30.0f;
    Stats.Fear = 0.0f;
    Stats.TerritorialRadius = 2000.0f;
    Stats.DetectionRange = 1500.0f;
    Stats.AttackDamage = 25.0f;
    Stats.MovementSpeed = 400.0f;

    // Initialize pack behavior
    PackBehavior.PackCohesionRadius = 1000.0f;
    PackBehavior.MaxPackSize = 6;
    PackBehavior.bIsPackLeader = false;

    // Initialize state
    CurrentState = ENPC_DinosaurState::Idle;
    Species = ENPC_DinosaurSpecies::TRex;
}

void UNPC_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();

    // Cache references
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        AIController = Cast<AAIController>(OwnerPawn->GetController());
        if (AIController && AIController->GetBlackboardComponent())
        {
            BlackboardComponent = AIController->GetBlackboardComponent();
        }
    }

    // Set patrol center to current location
    if (OwnerPawn)
    {
        PatrolCenter = OwnerPawn->GetActorLocation();
    }

    // Bind sensing events
    if (PawnSensingComponent)
    {
        PawnSensingComponent->OnSeePawn.AddDynamic(this, &UNPC_DinosaurAI::OnSeePawn);
        PawnSensingComponent->OnHearNoise.AddDynamic(this, &UNPC_DinosaurAI::OnHearNoise);
    }

    // Initialize species-specific traits
    InitializeSpeciesTraits();

    // Start with patrolling behavior
    StartPatrolling();
}

void UNPC_DinosaurAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerPawn)
        return;

    // Update AI state
    UpdateAIState(DeltaTime);
    
    // Update stats
    UpdateHunger(DeltaTime);
    UpdateFear(DeltaTime);
    
    // Update blackboard values
    UpdateBlackboardValues();
    
    // Update timers
    StateChangeTimer += DeltaTime;
    TimeSincePlayerSeen += DeltaTime;
}

void UNPC_DinosaurAI::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState == NewState)
        return;

    ENPC_DinosaurState PreviousState = CurrentState;
    CurrentState = NewState;
    StateChangeTimer = 0.0f;

    // Log state change
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state from %d to %d"), 
           *GetOwner()->GetName(), (int32)PreviousState, (int32)NewState);

    // Update movement speed based on state
    if (ACharacter* Character = Cast<ACharacter>(OwnerPawn))
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            switch (CurrentState)
            {
                case ENPC_DinosaurState::Hunting:
                    MovementComp->MaxWalkSpeed = Stats.MovementSpeed * 1.5f;
                    break;
                case ENPC_DinosaurState::Fleeing:
                    MovementComp->MaxWalkSpeed = Stats.MovementSpeed * 2.0f;
                    break;
                case ENPC_DinosaurState::Patrolling:
                    MovementComp->MaxWalkSpeed = Stats.MovementSpeed * 0.7f;
                    break;
                default:
                    MovementComp->MaxWalkSpeed = Stats.MovementSpeed;
                    break;
            }
        }
    }
}

void UNPC_DinosaurAI::StartHunting(AActor* Target)
{
    if (!Target)
        return;

    CurrentTarget = Target;
    LastKnownPlayerLocation = Target->GetActorLocation();
    TimeSincePlayerSeen = 0.0f;
    SetDinosaurState(ENPC_DinosaurState::Hunting);

    // If this is a pack leader, call pack to hunt
    if (PackBehavior.bIsPackLeader && Species == ENPC_DinosaurSpecies::Raptor)
    {
        CallPackToHunt(Target);
    }
}

void UNPC_DinosaurAI::StartFleeing(AActor* ThreatSource)
{
    if (!ThreatSource)
        return;

    CurrentTarget = ThreatSource;
    Stats.Fear = FMath::Min(Stats.Fear + 30.0f, 100.0f);
    SetDinosaurState(ENPC_DinosaurState::Fleeing);
}

void UNPC_DinosaurAI::StartPatrolling()
{
    CurrentTarget = nullptr;
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
}

bool UNPC_DinosaurAI::IsInTerritory(const FVector& Location) const
{
    float DistanceFromCenter = FVector::Dist(PatrolCenter, Location);
    return DistanceFromCenter <= Stats.TerritorialRadius;
}

float UNPC_DinosaurAI::GetDistanceToTarget() const
{
    if (!CurrentTarget.IsValid() || !OwnerPawn)
        return -1.0f;

    return FVector::Dist(OwnerPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
}

void UNPC_DinosaurAI::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    Stats.Health = FMath::Max(0.0f, Stats.Health - DamageAmount);
    
    if (Stats.Health <= 0.0f)
    {
        // Handle death
        SetDinosaurState(ENPC_DinosaurState::Idle);
        if (OwnerPawn)
        {
            OwnerPawn->Destroy();
        }
        return;
    }

    // Increase fear and aggression
    Stats.Fear += DamageAmount * 0.5f;
    Stats.Aggression += DamageAmount * 0.3f;

    // Decide whether to flee or fight
    if (ShouldFleeFromThreat(DamageSource))
    {
        StartFleeing(DamageSource);
    }
    else
    {
        StartHunting(DamageSource);
    }
}

void UNPC_DinosaurAI::InitializeSpeciesTraits()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Stats.MaxHealth = 200.0f;
            Stats.Health = 200.0f;
            Stats.AttackDamage = 50.0f;
            Stats.MovementSpeed = 600.0f;
            Stats.Aggression = 70.0f;
            Stats.TerritorialRadius = 3000.0f;
            Stats.DetectionRange = 2000.0f;
            PackBehavior.MaxPackSize = 1; // Solitary
            break;

        case ENPC_DinosaurSpecies::Raptor:
            Stats.MaxHealth = 80.0f;
            Stats.Health = 80.0f;
            Stats.AttackDamage = 30.0f;
            Stats.MovementSpeed = 800.0f;
            Stats.Aggression = 60.0f;
            Stats.TerritorialRadius = 1500.0f;
            Stats.DetectionRange = 1800.0f;
            PackBehavior.MaxPackSize = 6; // Pack hunters
            break;

        case ENPC_DinosaurSpecies::Triceratops:
            Stats.MaxHealth = 150.0f;
            Stats.Health = 150.0f;
            Stats.AttackDamage = 40.0f;
            Stats.MovementSpeed = 400.0f;
            Stats.Aggression = 40.0f;
            Stats.TerritorialRadius = 2000.0f;
            Stats.DetectionRange = 1200.0f;
            PackBehavior.MaxPackSize = 4; // Small herds
            break;

        case ENPC_DinosaurSpecies::Brachiosaurus:
            Stats.MaxHealth = 300.0f;
            Stats.Health = 300.0f;
            Stats.AttackDamage = 20.0f;
            Stats.MovementSpeed = 200.0f;
            Stats.Aggression = 10.0f;
            Stats.TerritorialRadius = 4000.0f;
            Stats.DetectionRange = 1000.0f;
            PackBehavior.MaxPackSize = 8; // Large herds
            break;

        case ENPC_DinosaurSpecies::Stegosaurus:
            Stats.MaxHealth = 120.0f;
            Stats.Health = 120.0f;
            Stats.AttackDamage = 35.0f;
            Stats.MovementSpeed = 300.0f;
            Stats.Aggression = 30.0f;
            Stats.TerritorialRadius = 1800.0f;
            Stats.DetectionRange = 1100.0f;
            PackBehavior.MaxPackSize = 3; // Small groups
            break;
    }

    // Update sensing component based on species
    if (PawnSensingComponent)
    {
        PawnSensingComponent->SightRadius = Stats.DetectionRange;
    }
}

bool UNPC_DinosaurAI::ShouldFleeFromThreat(AActor* Threat) const
{
    if (!Threat)
        return false;

    // Base flee chance on fear level and health
    float FleeChance = (Stats.Fear + (100.0f - Stats.Health)) / 200.0f;
    
    // Species-specific flee behavior
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            FleeChance *= 0.3f; // T-Rex rarely flees
            break;
        case ENPC_DinosaurSpecies::Raptor:
            FleeChance *= 0.6f; // Raptors are tactical
            break;
        case ENPC_DinosaurSpecies::Brachiosaurus:
            FleeChance *= 1.2f; // Herbivores flee more readily
            break;
        default:
            break;
    }

    return FMath::RandRange(0.0f, 1.0f) < FleeChance;
}

float UNPC_DinosaurAI::GetOptimalAttackRange() const
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            return 300.0f;
        case ENPC_DinosaurSpecies::Raptor:
            return 200.0f;
        case ENPC_DinosaurSpecies::Triceratops:
            return 400.0f;
        default:
            return 250.0f;
    }
}

void UNPC_DinosaurAI::UpdateAIState(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            UpdateIdleState(DeltaTime);
            break;
        case ENPC_DinosaurState::Patrolling:
            UpdatePatrollingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Hunting:
            UpdateHuntingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Fleeing:
            UpdateFleeingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Feeding:
            UpdateFeedingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Territorial:
            UpdateTerritorialState(DeltaTime);
            break;
    }
}

void UNPC_DinosaurAI::UpdateHunger(float DeltaTime)
{
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - DeltaTime * 2.0f);
    
    if (Stats.Hunger < 20.0f && CurrentState != ENPC_DinosaurState::Feeding)
    {
        // Look for food or start hunting
        if (Species == ENPC_DinosaurSpecies::TRex || Species == ENPC_DinosaurSpecies::Raptor)
        {
            // Carnivores hunt when hungry
            if (CurrentTarget.IsValid())
            {
                StartHunting(CurrentTarget.Get());
            }
        }
    }
}

void UNPC_DinosaurAI::UpdateFear(float DeltaTime)
{
    // Fear decays over time
    Stats.Fear = FMath::Max(0.0f, Stats.Fear - DeltaTime * 5.0f);
}

void UNPC_DinosaurAI::OnSeePawn(APawn* Pawn)
{
    if (!Pawn || Pawn == OwnerPawn)
        return;

    // Check if it's a player
    if (Pawn->IsPlayerControlled())
    {
        LastKnownPlayerLocation = Pawn->GetActorLocation();
        TimeSincePlayerSeen = 0.0f;

        float DistanceToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), Pawn->GetActorLocation());
        
        // Decide behavior based on species and distance
        if (DistanceToPlayer < Stats.DetectionRange * 0.5f)
        {
            if (Stats.Aggression > 50.0f)
            {
                StartHunting(Pawn);
            }
            else if (ShouldFleeFromThreat(Pawn))
            {
                StartFleeing(Pawn);
            }
        }
    }
}

void UNPC_DinosaurAI::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    if (!NoiseInstigator || NoiseInstigator == OwnerPawn)
        return;

    // React to loud noises
    if (Volume > 0.7f)
    {
        Stats.Fear += 10.0f;
        
        if (NoiseInstigator->IsPlayerControlled())
        {
            LastKnownPlayerLocation = Location;
            
            if (Stats.Aggression > Stats.Fear)
            {
                StartHunting(NoiseInstigator);
            }
            else
            {
                StartFleeing(NoiseInstigator);
            }
        }
    }
}

void UNPC_DinosaurAI::UpdateIdleState(float DeltaTime)
{
    // Transition to patrolling after some time
    if (StateChangeTimer > 5.0f)
    {
        StartPatrolling();
    }
}

void UNPC_DinosaurAI::UpdatePatrollingState(float DeltaTime)
{
    if (!AIController)
        return;

    // Move to random patrol points
    NextPatrolPointTimer += DeltaTime;
    if (NextPatrolPointTimer > 10.0f)
    {
        FVector PatrolPoint = GetRandomPatrolPoint();
        AIController->MoveToLocation(PatrolPoint, 100.0f);
        NextPatrolPointTimer = 0.0f;
    }
}

void UNPC_DinosaurAI::UpdateHuntingState(float DeltaTime)
{
    if (!AIController || !CurrentTarget.IsValid())
    {
        StartPatrolling();
        return;
    }

    float DistanceToTarget = GetDistanceToTarget();
    
    if (DistanceToTarget < GetOptimalAttackRange())
    {
        // Attack target
        if (CurrentTarget.IsValid())
        {
            // Deal damage to target
            UE_LOG(LogTemp, Log, TEXT("Dinosaur %s attacks target for %f damage"), 
                   *GetOwner()->GetName(), Stats.AttackDamage);
        }
    }
    else
    {
        // Move towards target
        AIController->MoveToActor(CurrentTarget.Get(), GetOptimalAttackRange() * 0.8f);
    }

    // Give up hunting if target is too far or hasn't been seen for too long
    if (DistanceToTarget > Stats.DetectionRange * 2.0f || TimeSincePlayerSeen > 30.0f)
    {
        StartPatrolling();
    }
}

void UNPC_DinosaurAI::UpdateFleeingState(float DeltaTime)
{
    if (!AIController || !CurrentTarget.IsValid())
    {
        StartPatrolling();
        return;
    }

    // Move away from threat
    FVector FleeDirection = (OwnerPawn->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
    FVector FleeLocation = OwnerPawn->GetActorLocation() + FleeDirection * 2000.0f;
    
    AIController->MoveToLocation(FleeLocation, 100.0f);

    // Stop fleeing after some time or when far enough
    if (StateChangeTimer > 15.0f || GetDistanceToTarget() > Stats.DetectionRange * 1.5f)
    {
        StartPatrolling();
    }
}

void UNPC_DinosaurAI::UpdateFeedingState(float DeltaTime)
{
    // Feeding restores hunger
    Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + DeltaTime * 10.0f);
    
    // Stop feeding when full
    if (Stats.Hunger > 80.0f || StateChangeTimer > 20.0f)
    {
        StartPatrolling();
    }
}

void UNPC_DinosaurAI::UpdateTerritorialState(float DeltaTime)
{
    // Similar to patrolling but more aggressive to intruders
    UpdatePatrollingState(DeltaTime);
}

FVector UNPC_DinosaurAI::GetRandomPatrolPoint() const
{
    if (!OwnerPawn)
        return FVector::ZeroVector;

    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    FVector PatrolPoint = PatrolCenter + RandomDirection * RandomDistance;
    
    return PatrolPoint;
}

void UNPC_DinosaurAI::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), (uint8)CurrentState);
    BlackboardComponent->SetValueAsFloat(TEXT("Health"), Stats.Health);
    BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), Stats.Hunger);
    BlackboardComponent->SetValueAsFloat(TEXT("Fear"), Stats.Fear);
    BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), Stats.Aggression);
    
    if (CurrentTarget.IsValid())
    {
        BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget.Get());
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
    }
    else
    {
        BlackboardComponent->ClearValue(TEXT("Target"));
    }
    
    BlackboardComponent->SetValueAsVector(TEXT("PatrolCenter"), PatrolCenter);
    BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), LastKnownPlayerLocation);
}

void UNPC_DinosaurAI::JoinPack(UNPC_DinosaurAI* PackLeaderAI)
{
    if (!PackLeaderAI || PackLeaderAI == this)
        return;

    PackBehavior.PackLeader = PackLeaderAI->GetOwner();
    PackBehavior.bIsPackLeader = false;
    
    // Add this dinosaur to the leader's pack
    if (PackLeaderAI->PackBehavior.PackMembers.Num() < PackLeaderAI->PackBehavior.MaxPackSize)
    {
        PackLeaderAI->PackBehavior.PackMembers.Add(OwnerPawn);
    }
}

void UNPC_DinosaurAI::LeavePack()
{
    if (PackBehavior.PackLeader.IsValid())
    {
        // Remove from leader's pack
        if (UNPC_DinosaurAI* LeaderAI = PackBehavior.PackLeader->FindComponentByClass<UNPC_DinosaurAI>())
        {
            LeaderAI->PackBehavior.PackMembers.Remove(OwnerPawn);
        }
    }
    
    PackBehavior.PackLeader = nullptr;
    PackBehavior.bIsPackLeader = false;
    PackBehavior.PackMembers.Empty();
}

void UNPC_DinosaurAI::CallPackToHunt(AActor* Target)
{
    if (!PackBehavior.bIsPackLeader || !Target)
        return;

    for (TWeakObjectPtr<APawn>& PackMember : PackBehavior.PackMembers)
    {
        if (PackMember.IsValid())
        {
            if (UNPC_DinosaurAI* MemberAI = PackMember->FindComponentByClass<UNPC_DinosaurAI>())
            {
                MemberAI->StartHunting(Target);
            }
        }
    }
}

TArray<APawn*> UNPC_DinosaurAI::GetNearbyPackMembers(float Radius)
{
    TArray<APawn*> NearbyMembers;
    
    if (!OwnerPawn)
        return NearbyMembers;

    for (TWeakObjectPtr<APawn>& PackMember : PackBehavior.PackMembers)
    {
        if (PackMember.IsValid())
        {
            float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), PackMember->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyMembers.Add(PackMember.Get());
            }
        }
    }
    
    return NearbyMembers;
}