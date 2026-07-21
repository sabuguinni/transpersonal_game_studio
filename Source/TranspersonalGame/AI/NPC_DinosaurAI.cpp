#include "NPC_DinosaurAI.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"

UNPC_DinosaurAI::UNPC_DinosaurAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance

    // Initialize default stats based on T-Rex
    DinosaurStats.Health = 100.0f;
    DinosaurStats.MaxHealth = 100.0f;
    DinosaurStats.Hunger = 50.0f;
    DinosaurStats.Aggression = 70.0f;
    DinosaurStats.Fear = 10.0f;
    DinosaurStats.Stamina = 100.0f;
    DinosaurStats.MaxStamina = 100.0f;
    DinosaurStats.MovementSpeed = 400.0f;
    DinosaurStats.AttackDamage = 50.0f;
    DinosaurStats.DetectionRange = 3000.0f;
    DinosaurStats.AttackRange = 300.0f;

    // Initialize memory
    DinosaurMemory.TerritoryRadius = 5000.0f;
    DinosaurMemory.LastPlayerSightingTime = -1.0f;

    // Default behavior settings
    CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
    MinStateTime = 3.0f;
    StateChangeTimer = 0.0f;
    bIsPackAnimal = false;
    bIsNocturnal = false;
}

void UNPC_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner pawn reference
    OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("NPC_DinosaurAI: Owner is not a Pawn!"));
        return;
    }

    // Find player character
    PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    // Initialize species-specific traits
    InitializeSpeciesTraits();

    // Set home location to current position
    DinosaurMemory.HomeLocation = OwnerPawn->GetActorLocation();

    // Initialize patrol points around home location
    UpdatePatrolRoute();

    // Try to get AI controller and behavior tree components
    if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
    {
        BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
        BlackboardComponent = AIController->GetBlackboardComponent();
    }

    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurAI initialized for %s - Species: %d"), 
           *OwnerPawn->GetName(), (int32)Species);
}

void UNPC_DinosaurAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerPawn || !IsValid(OwnerPawn))
    {
        return;
    }

    // Update internal timers
    StateChangeTimer += DeltaTime;
    LastBehaviorUpdate += DeltaTime;

    // Update stats (hunger, stamina, etc.)
    UpdateStats(DeltaTime);

    // Update memory and perception
    UpdateMemory(DeltaTime);

    // Process day/night cycle effects
    ProcessDayNightCycle();

    // Update pack behavior if applicable
    if (bIsPackAnimal)
    {
        UpdatePackBehavior();
    }

    // Process current behavior state
    switch (CurrentBehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Idle:
            ProcessIdleBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Patrolling:
            ProcessPatrolBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Hunting:
            ProcessHuntingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Fleeing:
            ProcessFleeingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Feeding:
            ProcessFeedingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Sleeping:
            ProcessSleepingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Territorial:
            ProcessTerritorialBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::PackHunting:
            ProcessPackHuntingBehavior(DeltaTime);
            break;
    }

    // Check if we should change behavior state
    if (ShouldChangeState())
    {
        ENPC_DinosaurBehaviorState NextState = DetermineNextState();
        if (NextState != CurrentBehaviorState)
        {
            SetBehaviorState(NextState);
        }
    }
}

void UNPC_DinosaurAI::SetBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    if (NewState == CurrentBehaviorState)
    {
        return;
    }

    ENPC_DinosaurBehaviorState PreviousState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    StateChangeTimer = 0.0f;

    // Update blackboard if available
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), (uint8)NewState);
    }

    UE_LOG(LogTemp, Log, TEXT("%s: Behavior changed from %d to %d"), 
           *OwnerPawn->GetName(), (int32)PreviousState, (int32)NewState);
}

bool UNPC_DinosaurAI::CanSeePlayer() const
{
    if (!PlayerCharacter || !OwnerPawn)
    {
        return false;
    }

    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    float Distance = FVector::Dist(OwnerLocation, PlayerLocation);

    // Check if player is within detection range
    if (Distance > DinosaurStats.DetectionRange)
    {
        return false;
    }

    // Perform line trace to check for obstacles
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(PlayerCharacter);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        OwnerLocation,
        PlayerLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return !bHit; // Can see if no obstacles
}

AActor* UNPC_DinosaurAI::FindNearestThreat() const
{
    if (!OwnerPawn)
    {
        return nullptr;
    }

    AActor* NearestThreat = nullptr;
    float NearestDistance = DinosaurStats.DetectionRange;
    FVector OwnerLocation = OwnerPawn->GetActorLocation();

    // Check player as potential threat
    if (PlayerCharacter && CanSeePlayer())
    {
        float PlayerDistance = FVector::Dist(OwnerLocation, PlayerCharacter->GetActorLocation());
        if (PlayerDistance < NearestDistance)
        {
            NearestThreat = PlayerCharacter;
            NearestDistance = PlayerDistance;
        }
    }

    // Check other known threats
    for (AActor* Threat : DinosaurMemory.KnownThreats)
    {
        if (IsValid(Threat))
        {
            float Distance = FVector::Dist(OwnerLocation, Threat->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestThreat = Threat;
                NearestDistance = Distance;
            }
        }
    }

    return NearestThreat;
}

void UNPC_DinosaurAI::UpdatePatrolRoute()
{
    if (!OwnerPawn)
    {
        return;
    }

    DinosaurMemory.PatrolPoints.Empty();
    FVector HomeLocation = DinosaurMemory.HomeLocation;
    float PatrolRadius = DinosaurMemory.TerritoryRadius * 0.5f;

    // Create 4-6 patrol points in a circle around home
    int32 NumPoints = FMath::RandRange(4, 6);
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = (2.0f * PI * i) / NumPoints;
        FVector PatrolPoint = HomeLocation + FVector(
            FMath::Cos(Angle) * PatrolRadius,
            FMath::Sin(Angle) * PatrolRadius,
            0.0f
        );
        DinosaurMemory.PatrolPoints.Add(PatrolPoint);
    }

    CurrentPatrolIndex = 0;
}

void UNPC_DinosaurAI::StartHunting(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    CurrentTarget = Target;
    bHasValidTarget = true;
    SetBehaviorState(ENPC_DinosaurBehaviorState::Hunting);

    // Update memory
    DinosaurMemory.LastKnownPlayerLocation = Target->GetActorLocation();
    DinosaurMemory.LastPlayerSightingTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Log, TEXT("%s: Started hunting %s"), 
           *OwnerPawn->GetName(), *Target->GetName());
}

void UNPC_DinosaurAI::FleeFromThreat(AActor* Threat)
{
    if (!Threat)
    {
        return;
    }

    CurrentTarget = Threat;
    SetBehaviorState(ENPC_DinosaurBehaviorState::Fleeing);

    // Increase fear
    DinosaurStats.Fear = FMath::Min(100.0f, DinosaurStats.Fear + 20.0f);

    UE_LOG(LogTemp, Log, TEXT("%s: Fleeing from %s"), 
           *OwnerPawn->GetName(), *Threat->GetName());
}

void UNPC_DinosaurAI::InitializeSpeciesTraits()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            DinosaurStats.MaxHealth = 200.0f;
            DinosaurStats.Health = 200.0f;
            DinosaurStats.Aggression = 80.0f;
            DinosaurStats.MovementSpeed = 500.0f;
            DinosaurStats.AttackDamage = 75.0f;
            DinosaurStats.DetectionRange = 4000.0f;
            DinosaurStats.AttackRange = 400.0f;
            bIsPackAnimal = false;
            DinosaurMemory.TerritoryRadius = 8000.0f;
            break;

        case ENPC_DinosaurSpecies::Raptor:
            DinosaurStats.MaxHealth = 80.0f;
            DinosaurStats.Health = 80.0f;
            DinosaurStats.Aggression = 90.0f;
            DinosaurStats.MovementSpeed = 700.0f;
            DinosaurStats.AttackDamage = 35.0f;
            DinosaurStats.DetectionRange = 3500.0f;
            DinosaurStats.AttackRange = 200.0f;
            bIsPackAnimal = true;
            DinosaurMemory.TerritoryRadius = 5000.0f;
            break;

        case ENPC_DinosaurSpecies::Triceratops:
            DinosaurStats.MaxHealth = 150.0f;
            DinosaurStats.Health = 150.0f;
            DinosaurStats.Aggression = 40.0f;
            DinosaurStats.MovementSpeed = 300.0f;
            DinosaurStats.AttackDamage = 45.0f;
            DinosaurStats.DetectionRange = 2500.0f;
            DinosaurStats.AttackRange = 300.0f;
            bIsPackAnimal = false;
            DinosaurMemory.TerritoryRadius = 3000.0f;
            break;

        case ENPC_DinosaurSpecies::Brachiosaurus:
            DinosaurStats.MaxHealth = 300.0f;
            DinosaurStats.Health = 300.0f;
            DinosaurStats.Aggression = 10.0f;
            DinosaurStats.MovementSpeed = 200.0f;
            DinosaurStats.AttackDamage = 20.0f;
            DinosaurStats.DetectionRange = 2000.0f;
            DinosaurStats.AttackRange = 500.0f;
            bIsPackAnimal = false;
            DinosaurMemory.TerritoryRadius = 6000.0f;
            break;

        case ENPC_DinosaurSpecies::Stegosaurus:
            DinosaurStats.MaxHealth = 120.0f;
            DinosaurStats.Health = 120.0f;
            DinosaurStats.Aggression = 30.0f;
            DinosaurStats.MovementSpeed = 250.0f;
            DinosaurStats.AttackDamage = 40.0f;
            DinosaurStats.DetectionRange = 2200.0f;
            DinosaurStats.AttackRange = 350.0f;
            bIsPackAnimal = false;
            DinosaurMemory.TerritoryRadius = 4000.0f;
            break;
    }

    // Apply species stats to character movement if available
    if (ACharacter* CharacterOwner = Cast<ACharacter>(OwnerPawn))
    {
        if (UCharacterMovementComponent* Movement = CharacterOwner->GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = DinosaurStats.MovementSpeed;
        }
    }
}

void UNPC_DinosaurAI::UpdatePackBehavior()
{
    // Implementation for pack coordination
    // This would involve communication between pack members
    // For now, just update pack member list
    if (bIsPackAnimal && DinosaurMemory.PackMembers.Num() == 0)
    {
        // Find other pack members nearby
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), OwnerPawn->GetClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor != OwnerPawn)
            {
                float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), Actor->GetActorLocation());
                if (Distance < DinosaurMemory.TerritoryRadius)
                {
                    DinosaurMemory.PackMembers.Add(Actor);
                }
            }
        }
    }
}

bool UNPC_DinosaurAI::IsInTerritory(const FVector& Location) const
{
    float Distance = FVector::Dist(DinosaurMemory.HomeLocation, Location);
    return Distance <= DinosaurMemory.TerritoryRadius;
}

void UNPC_DinosaurAI::ProcessDayNightCycle()
{
    // Get current time of day (simplified)
    float TimeOfDay = GetWorld()->GetTimeSeconds();
    float DayLength = 1200.0f; // 20 minutes
    float NormalizedTime = FMath::Fmod(TimeOfDay, DayLength) / DayLength;
    
    bool bIsNight = (NormalizedTime > 0.75f || NormalizedTime < 0.25f);
    
    if (bIsNocturnal && bIsNight)
    {
        // Nocturnal creatures are more active at night
        DinosaurStats.Aggression *= 1.2f;
        DinosaurStats.DetectionRange *= 1.1f;
    }
    else if (!bIsNocturnal && !bIsNight)
    {
        // Diurnal creatures are more active during day
        DinosaurStats.MovementSpeed *= 1.1f;
    }
}

// Private behavior processing methods
void UNPC_DinosaurAI::ProcessIdleBehavior(float DeltaTime)
{
    // Look for threats or prey
    AActor* NearestThreat = FindNearestThreat();
    if (NearestThreat)
    {
        float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), NearestThreat->GetActorLocation());
        
        if (DinosaurStats.Aggression > 50.0f && Distance < DinosaurStats.DetectionRange * 0.8f)
        {
            StartHunting(NearestThreat);
        }
        else if (DinosaurStats.Fear > DinosaurStats.Aggression)
        {
            FleeFromThreat(NearestThreat);
        }
    }
}

void UNPC_DinosaurAI::ProcessPatrolBehavior(float DeltaTime)
{
    if (DinosaurMemory.PatrolPoints.Num() == 0)
    {
        UpdatePatrolRoute();
        return;
    }

    // Move towards current patrol point
    FVector CurrentTarget = DinosaurMemory.PatrolPoints[CurrentPatrolIndex];
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, CurrentTarget);

    if (Distance < 200.0f) // Reached patrol point
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % DinosaurMemory.PatrolPoints.Num();
    }

    // Check for threats while patrolling
    AActor* NearestThreat = FindNearestThreat();
    if (NearestThreat)
    {
        float ThreatDistance = FVector::Dist(CurrentLocation, NearestThreat->GetActorLocation());
        if (ThreatDistance < DinosaurStats.DetectionRange * 0.6f)
        {
            if (DinosaurStats.Aggression > 60.0f)
            {
                StartHunting(NearestThreat);
            }
            else
            {
                FleeFromThreat(NearestThreat);
            }
        }
    }
}

void UNPC_DinosaurAI::ProcessHuntingBehavior(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        bHasValidTarget = false;
        SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
        return;
    }

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);

    // Update last known location
    DinosaurMemory.LastKnownPlayerLocation = TargetLocation;
    DinosaurMemory.LastPlayerSightingTime = GetWorld()->GetTimeSeconds();

    // Check if within attack range
    if (Distance <= DinosaurStats.AttackRange)
    {
        // Perform attack (would trigger animation and damage)
        UE_LOG(LogTemp, Log, TEXT("%s: Attacking %s!"), 
               *OwnerPawn->GetName(), *CurrentTarget->GetName());
        
        // Reduce stamina from attacking
        DinosaurStats.Stamina = FMath::Max(0.0f, DinosaurStats.Stamina - 15.0f);
    }

    // Give up chase if too far or out of territory
    if (Distance > DinosaurStats.DetectionRange * 1.5f || !IsInTerritory(CurrentLocation))
    {
        bHasValidTarget = false;
        CurrentTarget = nullptr;
        SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
    }
}

void UNPC_DinosaurAI::ProcessFleeingBehavior(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
        return;
    }

    FVector ThreatLocation = CurrentTarget->GetActorLocation();
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, ThreatLocation);

    // If far enough away, stop fleeing
    if (Distance > DinosaurStats.DetectionRange * 1.2f)
    {
        CurrentTarget = nullptr;
        DinosaurStats.Fear = FMath::Max(0.0f, DinosaurStats.Fear - 10.0f);
        SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
    }

    // Reduce stamina from running
    DinosaurStats.Stamina = FMath::Max(0.0f, DinosaurStats.Stamina - 5.0f * DeltaTime);
}

void UNPC_DinosaurAI::ProcessFeedingBehavior(float DeltaTime)
{
    // Restore hunger while feeding
    DinosaurStats.Hunger = FMath::Min(100.0f, DinosaurStats.Hunger + 20.0f * DeltaTime);
    
    // Feeding makes dinosaur less aggressive temporarily
    DinosaurStats.Aggression = FMath::Max(0.0f, DinosaurStats.Aggression - 5.0f * DeltaTime);

    // Finish feeding after some time
    if (StateChangeTimer > 10.0f)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
    }
}

void UNPC_DinosaurAI::ProcessSleepingBehavior(float DeltaTime)
{
    // Restore stamina while sleeping
    DinosaurStats.Stamina = FMath::Min(DinosaurStats.MaxStamina, DinosaurStats.Stamina + 15.0f * DeltaTime);
    
    // Reduce detection range while sleeping
    float SleepDetectionRange = DinosaurStats.DetectionRange * 0.3f;
    
    // Wake up if threat is very close
    AActor* NearestThreat = FindNearestThreat();
    if (NearestThreat)
    {
        float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), NearestThreat->GetActorLocation());
        if (Distance < SleepDetectionRange)
        {
            SetBehaviorState(ENPC_DinosaurBehaviorState::Fleeing);
            FleeFromThreat(NearestThreat);
        }
    }

    // Wake up after sufficient rest
    if (DinosaurStats.Stamina >= DinosaurStats.MaxStamina * 0.9f)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
    }
}

void UNPC_DinosaurAI::ProcessTerritorialBehavior(float DeltaTime)
{
    // Patrol territory borders aggressively
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    
    // Check for intruders in territory
    AActor* NearestThreat = FindNearestThreat();
    if (NearestThreat && IsInTerritory(NearestThreat->GetActorLocation()))
    {
        StartHunting(NearestThreat);
    }
}

void UNPC_DinosaurAI::ProcessPackHuntingBehavior(float DeltaTime)
{
    // Coordinate with pack members for hunting
    if (!bIsPackAnimal || DinosaurMemory.PackMembers.Num() == 0)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Hunting);
        return;
    }

    // Enhanced hunting behavior with pack coordination
    ProcessHuntingBehavior(DeltaTime);
}

void UNPC_DinosaurAI::UpdateStats(float DeltaTime)
{
    // Decay hunger over time
    DinosaurStats.Hunger = FMath::Max(0.0f, DinosaurStats.Hunger - HungerDecayRate * DeltaTime);
    
    // Regenerate stamina when not in combat
    if (CurrentBehaviorState != ENPC_DinosaurBehaviorState::Hunting && 
        CurrentBehaviorState != ENPC_DinosaurBehaviorState::Fleeing)
    {
        DinosaurStats.Stamina = FMath::Min(DinosaurStats.MaxStamina, 
                                          DinosaurStats.Stamina + StaminaRegenRate * DeltaTime);
    }
    
    // Reduce fear over time
    DinosaurStats.Fear = FMath::Max(0.0f, DinosaurStats.Fear - 2.0f * DeltaTime);
    
    // Increase aggression when hungry
    if (DinosaurStats.Hunger < 20.0f)
    {
        DinosaurStats.Aggression = FMath::Min(100.0f, DinosaurStats.Aggression + 5.0f * DeltaTime);
    }
}

void UNPC_DinosaurAI::UpdateMemory(float DeltaTime)
{
    // Clean up invalid threats
    DinosaurMemory.KnownThreats.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });

    // Clean up invalid pack members
    DinosaurMemory.PackMembers.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });

    // Forget old player sightings
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (DinosaurMemory.LastPlayerSightingTime > 0.0f && 
        CurrentTime - DinosaurMemory.LastPlayerSightingTime > 30.0f)
    {
        DinosaurMemory.LastKnownPlayerLocation = FVector::ZeroVector;
        DinosaurMemory.LastPlayerSightingTime = -1.0f;
    }
}

bool UNPC_DinosaurAI::ShouldChangeState() const
{
    // Don't change state too frequently
    if (StateChangeTimer < MinStateTime)
    {
        return false;
    }

    // State-specific change conditions
    switch (CurrentBehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Idle:
            return StateChangeTimer > 5.0f; // Change from idle after 5 seconds
            
        case ENPC_DinosaurBehaviorState::Sleeping:
            return DinosaurStats.Stamina >= DinosaurStats.MaxStamina * 0.8f;
            
        case ENPC_DinosaurBehaviorState::Feeding:
            return DinosaurStats.Hunger >= 80.0f;
            
        default:
            return StateChangeTimer > 15.0f; // General state timeout
    }
}

ENPC_DinosaurBehaviorState UNPC_DinosaurAI::DetermineNextState() const
{
    // Priority-based state selection
    
    // Sleep if very tired
    if (DinosaurStats.Stamina < 20.0f)
    {
        return ENPC_DinosaurBehaviorState::Sleeping;
    }
    
    // Feed if very hungry
    if (DinosaurStats.Hunger < 15.0f)
    {
        return ENPC_DinosaurBehaviorState::Feeding;
    }
    
    // Check for threats
    AActor* NearestThreat = FindNearestThreat();
    if (NearestThreat)
    {
        float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), NearestThreat->GetActorLocation());
        
        if (Distance < DinosaurStats.AttackRange * 2.0f)
        {
            if (DinosaurStats.Aggression > DinosaurStats.Fear)
            {
                return bIsPackAnimal ? ENPC_DinosaurBehaviorState::PackHunting : ENPC_DinosaurBehaviorState::Hunting;
            }
            else
            {
                return ENPC_DinosaurBehaviorState::Fleeing;
            }
        }
    }
    
    // Default to patrolling
    return ENPC_DinosaurBehaviorState::Patrolling;
}