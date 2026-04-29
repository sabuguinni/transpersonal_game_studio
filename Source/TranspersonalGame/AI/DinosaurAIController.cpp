#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Set blackboard component
    SetBlackboardComponent(BlackboardComponent);

    // Initialize default values
    DinosaurSpecies = ENPC_DinosaurSpecies::TRex;
    CurrentState = ENPC_DinosaurState::Idle;
    TerritoryCenter = FVector::ZeroVector;
    CurrentPatrolIndex = 0;
    CurrentTarget = nullptr;
    CurrentThreat = nullptr;
    LastPlayerSightTime = 0.0f;
    LastStateChangeTime = 0.0f;
    StateChangeCooldown = 2.0f;

    // Configure default stats for T-Rex
    DinosaurStats.Health = 150.0f;
    DinosaurStats.MaxHealth = 150.0f;
    DinosaurStats.Damage = 50.0f;
    DinosaurStats.Speed = 600.0f;
    DinosaurStats.SightRange = 3000.0f;
    DinosaurStats.HearingRange = 2000.0f;
    DinosaurStats.TerritoryRadius = 1500.0f;
    DinosaurStats.AggressionLevel = 0.8f;
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    SetupPerception();
    
    // Set territory center to current location
    if (GetPawn())
    {
        TerritoryCenter = GetPawn()->GetActorLocation();
        GeneratePatrolPoints(4);
    }

    // Start behavior tree after a short delay
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &ADinosaurAIController::StartBehaviorTree,
        1.0f,
        false
    );
}

void ADinosaurAIController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);

    if (InPawn)
    {
        TerritoryCenter = InPawn->GetActorLocation();
        GeneratePatrolPoints(4);
        UpdateBlackboard();
    }
}

void ADinosaurAIController::SetupPerception()
{
    if (!PerceptionComponent)
        return;

    // Configure sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = DinosaurStats.SightRange;
        SightConfig->LoseSightRadius = DinosaurStats.SightRange + 500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        PerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = DinosaurStats.HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        PerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set sight as dominant sense
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnTargetPerceptionUpdated);
}

void ADinosaurAIController::StartBehaviorTree()
{
    if (!BehaviorTreeComponent || !BlackboardComponent)
        return;

    UBehaviorTree* TreeToRun = nullptr;

    // Select behavior tree based on species
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            TreeToRun = TRexBehaviorTree;
            break;
        case ENPC_DinosaurSpecies::Raptor:
            TreeToRun = RaptorBehaviorTree;
            break;
        case ENPC_DinosaurSpecies::Triceratops:
        case ENPC_DinosaurSpecies::Brachiosaurus:
        case ENPC_DinosaurSpecies::Stegosaurus:
            TreeToRun = HerbivoreBehaviorTree;
            break;
    }

    if (TreeToRun)
    {
        RunBehaviorTree(TreeToRun);
        UpdateBlackboard();
    }
}

void ADinosaurAIController::UpdateBlackboard()
{
    if (!BlackboardComponent)
        return;

    // Update basic state information
    BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), static_cast<uint8>(CurrentState));
    BlackboardComponent->SetValueAsEnum(TEXT("DinosaurSpecies"), static_cast<uint8>(DinosaurSpecies));
    BlackboardComponent->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
    BlackboardComponent->SetValueAsFloat(TEXT("TerritoryRadius"), DinosaurStats.TerritoryRadius);
    BlackboardComponent->SetValueAsFloat(TEXT("Health"), DinosaurStats.Health);
    BlackboardComponent->SetValueAsFloat(TEXT("MaxHealth"), DinosaurStats.MaxHealth);

    // Update targets
    BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
    BlackboardComponent->SetValueAsObject(TEXT("CurrentThreat"), CurrentThreat);

    // Update patrol information
    if (PatrolPoints.Num() > 0)
    {
        FVector NextPatrol = GetNextPatrolPoint();
        BlackboardComponent->SetValueAsVector(TEXT("NextPatrolPoint"), NextPatrol);
    }

    // Update player information
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        BlackboardComponent->SetValueAsObject(TEXT("PlayerPawn"), PlayerPawn);
        float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("DistanceToPlayer"), DistanceToPlayer);
    }
}

void ADinosaurAIController::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState == NewState)
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastStateChangeTime < StateChangeCooldown)
        return;

    ENPC_DinosaurState PreviousState = CurrentState;
    CurrentState = NewState;
    LastStateChangeTime = CurrentTime;

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), static_cast<uint8>(CurrentState));
    }

    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state from %d to %d"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
           static_cast<int32>(PreviousState),
           static_cast<int32>(CurrentState));
}

void ADinosaurAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
    }

    if (NewTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s acquired target: %s"), 
               GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
               *NewTarget->GetName());
    }
}

void ADinosaurAIController::SetTerritoryCenter(FVector NewCenter)
{
    TerritoryCenter = NewCenter;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
    }

    // Regenerate patrol points around new center
    GeneratePatrolPoints(4);
}

void ADinosaurAIController::GeneratePatrolPoints(int32 NumPoints)
{
    PatrolPoints.Empty();

    if (NumPoints <= 0)
        return;

    float AngleStep = 360.0f / NumPoints;
    float Radius = DinosaurStats.TerritoryRadius * 0.7f; // Stay within territory

    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = FMath::DegreesToRadians(AngleStep * i);
        FVector Offset = FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        
        FVector PatrolPoint = TerritoryCenter + Offset;
        
        // Try to find a valid navigation point
        UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
        if (NavSystem)
        {
            FNavLocation NavLocation;
            if (NavSystem->ProjectPointToNavigation(PatrolPoint, NavLocation, FVector(500.0f)))
            {
                PatrolPoint = NavLocation.Location;
            }
        }
        
        PatrolPoints.Add(PatrolPoint);
    }

    CurrentPatrolIndex = 0;
}

FVector ADinosaurAIController::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return TerritoryCenter;
    }

    FVector NextPoint = PatrolPoints[CurrentPatrolIndex];
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    
    return NextPoint;
}

bool ADinosaurAIController::IsInTerritory(FVector Location) const
{
    float Distance = FVector::Dist2D(Location, TerritoryCenter);
    return Distance <= DinosaurStats.TerritoryRadius;
}

void ADinosaurAIController::AttackTarget()
{
    if (!CurrentTarget || !GetPawn())
        return;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    float AttackRange = 200.0f; // Melee range

    if (Distance <= AttackRange)
    {
        // Perform attack logic here
        SetDinosaurState(ENPC_DinosaurState::Fighting);
        
        // Apply damage if target is a character
        if (ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
        {
            // Damage logic would go here
            UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacks %s for %.1f damage!"),
                   *GetPawn()->GetName(),
                   *CurrentTarget->GetName(),
                   DinosaurStats.Damage);
        }
    }
}

void ADinosaurAIController::StartHunting(AActor* Prey)
{
    if (!Prey)
        return;

    SetTarget(Prey);
    SetDinosaurState(ENPC_DinosaurState::Hunting);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s started hunting %s"),
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
           *Prey->GetName());
}

void ADinosaurAIController::StartFleeing(AActor* Threat)
{
    if (!Threat)
        return;

    CurrentThreat = Threat;
    SetDinosaurState(ENPC_DinosaurState::Fleeing);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("CurrentThreat"), CurrentThreat);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s started fleeing from %s"),
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
           *Threat->GetName());
}

bool ADinosaurAIController::ShouldAttackPlayer() const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !GetPawn())
        return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
    
    // Attack based on species behavior and distance
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            return Distance < 1500.0f && DinosaurStats.AggressionLevel > 0.6f;
        case ENPC_DinosaurSpecies::Raptor:
            return Distance < 1000.0f && DinosaurStats.AggressionLevel > 0.4f;
        default:
            return false; // Herbivores don't attack unless threatened
    }
}

bool ADinosaurAIController::ShouldFleeFromPlayer() const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !GetPawn())
        return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
    
    // Flee based on species behavior and health
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            return DinosaurStats.Health < (DinosaurStats.MaxHealth * 0.3f) && Distance < 800.0f;
        case ENPC_DinosaurSpecies::Raptor:
            return DinosaurStats.Health < (DinosaurStats.MaxHealth * 0.5f) && Distance < 600.0f;
        default:
            return Distance < 1200.0f; // Herbivores flee more readily
    }
}

void ADinosaurAIController::ConfigureForSpecies(ENPC_DinosaurSpecies Species)
{
    DinosaurSpecies = Species;
    
    // Configure stats based on species
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            DinosaurStats.Health = 200.0f;
            DinosaurStats.MaxHealth = 200.0f;
            DinosaurStats.Damage = 75.0f;
            DinosaurStats.Speed = 600.0f;
            DinosaurStats.SightRange = 3000.0f;
            DinosaurStats.TerritoryRadius = 2000.0f;
            DinosaurStats.AggressionLevel = 0.8f;
            break;
            
        case ENPC_DinosaurSpecies::Raptor:
            DinosaurStats.Health = 80.0f;
            DinosaurStats.MaxHealth = 80.0f;
            DinosaurStats.Damage = 35.0f;
            DinosaurStats.Speed = 800.0f;
            DinosaurStats.SightRange = 2500.0f;
            DinosaurStats.TerritoryRadius = 1500.0f;
            DinosaurStats.AggressionLevel = 0.7f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            DinosaurStats.Health = 150.0f;
            DinosaurStats.MaxHealth = 150.0f;
            DinosaurStats.Damage = 40.0f;
            DinosaurStats.Speed = 400.0f;
            DinosaurStats.SightRange = 2000.0f;
            DinosaurStats.TerritoryRadius = 1200.0f;
            DinosaurStats.AggressionLevel = 0.3f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            DinosaurStats.Health = 300.0f;
            DinosaurStats.MaxHealth = 300.0f;
            DinosaurStats.Damage = 20.0f;
            DinosaurStats.Speed = 200.0f;
            DinosaurStats.SightRange = 1500.0f;
            DinosaurStats.TerritoryRadius = 2500.0f;
            DinosaurStats.AggressionLevel = 0.1f;
            break;
            
        case ENPC_DinosaurSpecies::Stegosaurus:
            DinosaurStats.Health = 120.0f;
            DinosaurStats.MaxHealth = 120.0f;
            DinosaurStats.Damage = 30.0f;
            DinosaurStats.Speed = 300.0f;
            DinosaurStats.SightRange = 1800.0f;
            DinosaurStats.TerritoryRadius = 1000.0f;
            DinosaurStats.AggressionLevel = 0.4f;
            break;
    }
    
    UpdateBlackboard();
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
            continue;

        // Check if it's the player
        if (Actor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            LastPlayerSightTime = GetWorld()->GetTimeSeconds();
            
            if (ShouldAttackPlayer())
            {
                StartHunting(Actor);
            }
            else if (ShouldFleeFromPlayer())
            {
                StartFleeing(Actor);
            }
        }
    }
}

void ADinosaurAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !GetPawn())
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Actor was detected
        if (Actor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            LastPlayerSightTime = GetWorld()->GetTimeSeconds();
            
            if (CurrentState == ENPC_DinosaurState::Idle || CurrentState == ENPC_DinosaurState::Patrolling)
            {
                if (ShouldAttackPlayer())
                {
                    StartHunting(Actor);
                }
                else if (ShouldFleeFromPlayer())
                {
                    StartFleeing(Actor);
                }
                else
                {
                    SetDinosaurState(ENPC_DinosaurState::Territorial);
                }
            }
        }
    }
    else
    {
        // Actor was lost
        if (Actor == CurrentTarget)
        {
            SetTarget(nullptr);
            SetDinosaurState(ENPC_DinosaurState::Patrolling);
        }
    }
}