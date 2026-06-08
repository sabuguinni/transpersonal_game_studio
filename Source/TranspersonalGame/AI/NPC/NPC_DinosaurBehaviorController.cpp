#include "NPC_DinosaurBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ANPC_DinosaurBehaviorController::ANPC_DinosaurBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Default Configuration
    DinosaurSpecies = ENPC_DinosaurSpecies::TRex;
    SightRadius = 3000.0f;
    HearingRadius = 1500.0f;
    ChaseRadius = 3000.0f;
    AttackRadius = 300.0f;
    MovementSpeed = 400.0f;
    ChaseSpeed = 600.0f;

    // Initial State
    CurrentState = ENPC_DinosaurState::Idle;
    TargetPlayer = nullptr;
    StateTimer = 0.0f;
    bIsPackLeader = false;
    PackLeader = nullptr;

    // Initialize Memory
    DinosaurMemory = FNPC_DinosaurMemory();
}

void ANPC_DinosaurBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    // Configure perception based on species
    ConfigurePerceptionForSpecies();

    // Bind perception events
    if (PerceptionComponent)
    {
        PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurBehaviorController::OnPerceptionUpdated);
    }

    // Initialize species-specific behavior
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            InitializeTRexBehavior();
            break;
        case ENPC_DinosaurSpecies::Raptor:
            InitializeRaptorBehavior();
            break;
        case ENPC_DinosaurSpecies::Brachiosaurus:
        case ENPC_DinosaurSpecies::Triceratops:
        case ENPC_DinosaurSpecies::Stegosaurus:
        case ENPC_DinosaurSpecies::Parasaurolophus:
            InitializeHerbivoreBehavior();
            break;
    }

    // Start behavior tree if available
    if (BehaviorTreeAsset && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }
}

void ANPC_DinosaurBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateBehaviorState(DeltaTime);
    UpdateMemory();
    StateTimer += DeltaTime;
}

void ANPC_DinosaurBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Set territory center to current location
    if (InPawn)
    {
        DinosaurMemory.TerritoryCenter = InPawn->GetActorLocation();
        
        // Generate patrol points around territory
        for (int32 i = 0; i < 4; i++)
        {
            float Angle = (i * 90.0f) * PI / 180.0f;
            FVector PatrolPoint = DinosaurMemory.TerritoryCenter + FVector(
                FMath::Cos(Angle) * DinosaurMemory.TerritoryRadius * 0.7f,
                FMath::Sin(Angle) * DinosaurMemory.TerritoryRadius * 0.7f,
                0.0f
            );
            DinosaurMemory.PatrolPoints.Add(PatrolPoint);
        }
    }
}

void ANPC_DinosaurBehaviorController::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), static_cast<uint8>(NewState));
        }
    }
}

void ANPC_DinosaurBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            // Check if this is a player character
            if (Character->IsPlayerControlled())
            {
                OnPlayerSighted(Character);
                return;
            }
        }
    }
}

void ANPC_DinosaurBehaviorController::OnPlayerSighted(ACharacter* Player)
{
    TargetPlayer = Player;
    DinosaurMemory.LastPlayerLocation = Player->GetActorLocation();
    DinosaurMemory.LastPlayerSightTime = GetWorld()->GetTimeSeconds();

    float DistanceToPlayer = GetDistanceToPlayer();

    // State transition based on distance and species
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            if (DistanceToPlayer <= AttackRadius)
            {
                SetDinosaurState(ENPC_DinosaurState::Attacking);
            }
            else if (DistanceToPlayer <= ChaseRadius)
            {
                SetDinosaurState(ENPC_DinosaurState::Chasing);
                DinosaurMemory.AggressionLevel = FMath::Clamp(DinosaurMemory.AggressionLevel + 0.2f, 0.0f, 1.0f);
            }
            else
            {
                SetDinosaurState(ENPC_DinosaurState::Investigating);
            }
            break;

        case ENPC_DinosaurSpecies::Raptor:
            if (bIsPackLeader)
            {
                CoordinatePackAttack(Player->GetActorLocation());
            }
            if (DistanceToPlayer <= ChaseRadius)
            {
                SetDinosaurState(ENPC_DinosaurState::PackHunting);
            }
            break;

        default: // Herbivores
            DinosaurMemory.FearLevel = FMath::Clamp(DinosaurMemory.FearLevel + 0.3f, 0.0f, 1.0f);
            if (DistanceToPlayer <= ChaseRadius * 0.5f)
            {
                SetDinosaurState(ENPC_DinosaurState::Fleeing);
            }
            else
            {
                SetDinosaurState(ENPC_DinosaurState::Investigating);
            }
            break;
    }

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetPlayer"), Player);
        BlackboardComponent->SetValueAsVector(TEXT("LastPlayerLocation"), DinosaurMemory.LastPlayerLocation);
    }
}

void ANPC_DinosaurBehaviorController::OnPlayerLost()
{
    TargetPlayer = nullptr;
    
    // Return to appropriate state
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            if (DinosaurMemory.AggressionLevel > 0.5f)
            {
                SetDinosaurState(ENPC_DinosaurState::Investigating);
            }
            else
            {
                SetDinosaurState(ENPC_DinosaurState::Patrolling);
            }
            break;

        case ENPC_DinosaurSpecies::Raptor:
            SetDinosaurState(ENPC_DinosaurState::Patrolling);
            break;

        default: // Herbivores
            SetDinosaurState(ENPC_DinosaurState::Grazing);
            break;
    }

    if (BlackboardComponent)
    {
        BlackboardComponent->ClearValue(TEXT("TargetPlayer"));
    }
}

void ANPC_DinosaurBehaviorController::SetTerritoryCenter(FVector Center, float Radius)
{
    DinosaurMemory.TerritoryCenter = Center;
    DinosaurMemory.TerritoryRadius = Radius;
}

bool ANPC_DinosaurBehaviorController::IsInTerritory(FVector Location) const
{
    float Distance = FVector::Dist(Location, DinosaurMemory.TerritoryCenter);
    return Distance <= DinosaurMemory.TerritoryRadius;
}

FVector ANPC_DinosaurBehaviorController::GetRandomPatrolPoint() const
{
    if (DinosaurMemory.PatrolPoints.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, DinosaurMemory.PatrolPoints.Num() - 1);
        return DinosaurMemory.PatrolPoints[RandomIndex];
    }
    
    // Generate random point in territory
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(DinosaurMemory.TerritoryRadius * 0.3f, DinosaurMemory.TerritoryRadius * 0.8f);
    
    return DinosaurMemory.TerritoryCenter + FVector(
        FMath::Cos(RandomAngle) * RandomRadius,
        FMath::Sin(RandomAngle) * RandomRadius,
        0.0f
    );
}

void ANPC_DinosaurBehaviorController::JoinPack(ANPC_DinosaurBehaviorController* Leader)
{
    if (Leader && Leader != this)
    {
        PackLeader = Leader;
        bIsPackLeader = false;
        Leader->PackMembers.AddUnique(this);
        DinosaurMemory.PackSize = Leader->PackMembers.Num() + 1;
    }
}

void ANPC_DinosaurBehaviorController::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    PackMembers.Empty();
    bIsPackLeader = false;
    DinosaurMemory.PackSize = 1;
}

void ANPC_DinosaurBehaviorController::CoordinatePackAttack(FVector TargetLocation)
{
    if (!bIsPackLeader) return;

    for (ANPC_DinosaurBehaviorController* PackMember : PackMembers)
    {
        if (PackMember && PackMember->GetPawn())
        {
            PackMember->SetDinosaurState(ENPC_DinosaurState::PackHunting);
            if (PackMember->BlackboardComponent)
            {
                PackMember->BlackboardComponent->SetValueAsVector(TEXT("PackTargetLocation"), TargetLocation);
            }
        }
    }
}

void ANPC_DinosaurBehaviorController::InitializeTRexBehavior()
{
    SightRadius = 3000.0f;
    HearingRadius = 2000.0f;
    ChaseRadius = 3000.0f;
    AttackRadius = 300.0f;
    MovementSpeed = 400.0f;
    ChaseSpeed = 600.0f;
    DinosaurMemory.TerritoryRadius = 5000.0f;
    DinosaurMemory.AggressionLevel = 0.8f;
    
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
}

void ANPC_DinosaurBehaviorController::InitializeRaptorBehavior()
{
    SightRadius = 2500.0f;
    HearingRadius = 1800.0f;
    ChaseRadius = 2500.0f;
    AttackRadius = 200.0f;
    MovementSpeed = 500.0f;
    ChaseSpeed = 800.0f;
    DinosaurMemory.TerritoryRadius = 3000.0f;
    DinosaurMemory.AggressionLevel = 0.9f;
    
    // Try to form pack with nearby raptors
    if (GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC_DinosaurBehaviorController::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (ANPC_DinosaurBehaviorController* OtherController = Cast<ANPC_DinosaurBehaviorController>(Actor))
            {
                if (OtherController != this && 
                    OtherController->DinosaurSpecies == ENPC_DinosaurSpecies::Raptor &&
                    !OtherController->bIsPackLeader && 
                    !OtherController->PackLeader)
                {
                    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), OtherController->GetPawn()->GetActorLocation());
                    if (Distance <= 1000.0f)
                    {
                        if (PackMembers.Num() == 0)
                        {
                            bIsPackLeader = true;
                        }
                        OtherController->JoinPack(this);
                    }
                }
            }
        }
    }
    
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
}

void ANPC_DinosaurBehaviorController::InitializeHerbivoreBehavior()
{
    SightRadius = 2000.0f;
    HearingRadius = 1500.0f;
    ChaseRadius = 1500.0f;
    AttackRadius = 0.0f; // Herbivores don't attack
    MovementSpeed = 300.0f;
    ChaseSpeed = 500.0f;
    DinosaurMemory.TerritoryRadius = 2000.0f;
    DinosaurMemory.AggressionLevel = 0.1f;
    DinosaurMemory.FearLevel = 0.2f;
    
    SetDinosaurState(ENPC_DinosaurState::Grazing);
}

void ANPC_DinosaurBehaviorController::UpdateBehaviorState(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ENPC_DinosaurState::Patrolling:
            HandlePatrollingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Chasing:
            HandleChasingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Grazing:
            HandleGrazingState(DeltaTime);
            break;
    }
}

void ANPC_DinosaurBehaviorController::HandleIdleState(float DeltaTime)
{
    if (StateTimer > 3.0f)
    {
        if (DinosaurSpecies == ENPC_DinosaurSpecies::TRex || DinosaurSpecies == ENPC_DinosaurSpecies::Raptor)
        {
            SetDinosaurState(ENPC_DinosaurState::Patrolling);
        }
        else
        {
            SetDinosaurState(ENPC_DinosaurState::Grazing);
        }
    }
}

void ANPC_DinosaurBehaviorController::HandlePatrollingState(float DeltaTime)
{
    if (!CanSeePlayer() && StateTimer > 10.0f)
    {
        // Move to random patrol point
        FVector PatrolPoint = GetRandomPatrolPoint();
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolPoint);
        }
        StateTimer = 0.0f;
    }
}

void ANPC_DinosaurBehaviorController::HandleChasingState(float DeltaTime)
{
    if (!TargetPlayer)
    {
        OnPlayerLost();
        return;
    }

    float DistanceToPlayer = GetDistanceToPlayer();
    
    if (DistanceToPlayer <= AttackRadius && DinosaurSpecies == ENPC_DinosaurSpecies::TRex)
    {
        SetDinosaurState(ENPC_DinosaurState::Attacking);
    }
    else if (DistanceToPlayer > ChaseRadius * 1.5f)
    {
        OnPlayerLost();
    }
}

void ANPC_DinosaurBehaviorController::HandleAttackingState(float DeltaTime)
{
    if (!TargetPlayer)
    {
        SetDinosaurState(ENPC_DinosaurState::Patrolling);
        return;
    }

    float DistanceToPlayer = GetDistanceToPlayer();
    
    if (DistanceToPlayer > AttackRadius * 2.0f)
    {
        SetDinosaurState(ENPC_DinosaurState::Chasing);
    }
}

void ANPC_DinosaurBehaviorController::HandleGrazingState(float DeltaTime)
{
    if (StateTimer > 15.0f)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            SetDinosaurState(ENPC_DinosaurState::Patrolling);
        }
        else
        {
            StateTimer = 0.0f;
        }
    }
}

float ANPC_DinosaurBehaviorController::GetDistanceToPlayer() const
{
    if (!TargetPlayer || !GetPawn())
    {
        return MAX_FLT;
    }
    
    return FVector::Dist(GetPawn()->GetActorLocation(), TargetPlayer->GetActorLocation());
}

bool ANPC_DinosaurBehaviorController::CanSeePlayer() const
{
    return TargetPlayer != nullptr && GetDistanceToPlayer() <= SightRadius;
}

void ANPC_DinosaurBehaviorController::UpdateMemory()
{
    // Decay aggression and fear over time
    DinosaurMemory.AggressionLevel = FMath::Clamp(DinosaurMemory.AggressionLevel - 0.01f * GetWorld()->GetDeltaSeconds(), 0.0f, 1.0f);
    DinosaurMemory.FearLevel = FMath::Clamp(DinosaurMemory.FearLevel - 0.02f * GetWorld()->GetDeltaSeconds(), 0.0f, 1.0f);
}

void ANPC_DinosaurBehaviorController::ConfigurePerceptionForSpecies()
{
    if (!PerceptionComponent) return;

    // Configure sight
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = DinosaurSpecies == ENPC_DinosaurSpecies::TRex ? 60.0f : 90.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure hearing
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}