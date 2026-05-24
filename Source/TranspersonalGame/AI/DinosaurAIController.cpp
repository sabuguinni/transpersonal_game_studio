#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Set default values
    DinosaurSpecies = ENPC_DinosaurSpecies::TRex;
    TerritorialRadius = 5000.0f;
    AggressionLevel = 0.7f;
    PackLoyalty = 0.8f;
    bIsPackLeader = false;
    bIsAlphaSpecimen = false;

    // Perception defaults
    SightRadius = 3000.0f;
    HearingRadius = 2000.0f;
    PeripheralVisionAngle = 90.0f;

    // Internal state
    CurrentTarget = nullptr;
    HomeLocation = FVector::ZeroVector;
    LastThreatAssessmentTime = 0.0f;
    bInCombatMode = false;
    bInFlockingMode = false;
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    ConfigurePerception();
    InitializeBehaviorTree();

    // Set home location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Start memory cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MemoryCleanupTimer,
            this,
            &ADinosaurAIController::CleanupMemories,
            30.0f, // Every 30 seconds
            true
        );
    }
}

void ADinosaurAIController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);

    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
        
        // Configure species-specific behavior
        switch (DinosaurSpecies)
        {
        case ENPC_DinosaurSpecies::TRex:
            ConfigureTRexBehavior();
            break;
        case ENPC_DinosaurSpecies::Raptor:
            ConfigureRaptorBehavior();
            break;
        case ENPC_DinosaurSpecies::Triceratops:
        case ENPC_DinosaurSpecies::Brachiosaurus:
            ConfigureHerbivoreBehavior();
            break;
        default:
            break;
        }

        SetupBlackboardValues();
        
        if (DefaultBehaviorTree)
        {
            RunBehaviorTree(DefaultBehaviorTree);
        }
    }
}

void ADinosaurAIController::UnPossess()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }

    Super::UnPossess();
}

void ADinosaurAIController::ConfigurePerception()
{
    if (!PerceptionComponent)
    {
        return;
    }

    // Configure sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = SightRadius * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
        SightConfig->SetMaxAge(10.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        PerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRadius;
        HearingConfig->SetMaxAge(5.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        PerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set dominant sense
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnTargetPerceptionUpdated);
}

void ADinosaurAIController::InitializeBehaviorTree()
{
    if (BlackboardComponent && DinosaurBlackboard)
    {
        BlackboardComponent->InitializeBlackboard(*DinosaurBlackboard);
    }
}

void ADinosaurAIController::SetupBlackboardValues()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Set initial blackboard values
    BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
    BlackboardComponent->SetValueAsFloat(TEXT("TerritorialRadius"), TerritorialRadius);
    BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
    BlackboardComponent->SetValueAsBool(TEXT("IsPackLeader"), bIsPackLeader);
    BlackboardComponent->SetValueAsBool(TEXT("IsAlphaSpecimen"), bIsAlphaSpecimen);
    BlackboardComponent->SetValueAsBool(TEXT("InCombatMode"), false);
    BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), false);
}

void ADinosaurAIController::ConfigureTRexBehavior()
{
    // T-Rex: Apex predator, highly territorial, solitary
    TerritorialRadius = 8000.0f;
    AggressionLevel = 0.9f;
    PackLoyalty = 0.1f;
    SightRadius = 4000.0f;
    HearingRadius = 3000.0f;
    bIsAlphaSpecimen = true;
}

void ADinosaurAIController::ConfigureRaptorBehavior()
{
    // Raptor: Pack hunter, intelligent, coordinated
    TerritorialRadius = 3000.0f;
    AggressionLevel = 0.8f;
    PackLoyalty = 0.9f;
    SightRadius = 2500.0f;
    HearingRadius = 2000.0f;
}

void ADinosaurAIController::ConfigureHerbivoreBehavior()
{
    // Herbivore: Defensive, herd-oriented, flee-focused
    TerritorialRadius = 2000.0f;
    AggressionLevel = 0.3f;
    PackLoyalty = 0.7f;
    SightRadius = 2000.0f;
    HearingRadius = 1500.0f;
}

void ADinosaurAIController::SetBehaviorTree(UBehaviorTree* NewBehaviorTree)
{
    if (BehaviorTreeComponent && NewBehaviorTree)
    {
        BehaviorTreeComponent->StopTree();
        RunBehaviorTree(NewBehaviorTree);
    }
}

void ADinosaurAIController::SwitchToCombatMode()
{
    bInCombatMode = true;
    bInFlockingMode = false;

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InCombatMode"), true);
    }

    if (CombatBehaviorTree)
    {
        SetBehaviorTree(CombatBehaviorTree);
    }
}

void ADinosaurAIController::SwitchToFlockingMode()
{
    bInFlockingMode = true;
    bInCombatMode = false;

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InCombatMode"), false);
    }

    if (FlockingBehaviorTree)
    {
        SetBehaviorTree(FlockingBehaviorTree);
    }
}

void ADinosaurAIController::SwitchToDefaultMode()
{
    bInCombatMode = false;
    bInFlockingMode = false;

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InCombatMode"), false);
    }

    if (DefaultBehaviorTree)
    {
        SetBehaviorTree(DefaultBehaviorTree);
    }
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && IsActorThreat(Actor))
        {
            AssessThreat(Actor);
        }
    }
}

void ADinosaurAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // Actor detected
        if (IsActorThreat(Actor))
        {
            CurrentTarget = Actor;
            
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Actor);
                BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), true);
            }

            // Switch to appropriate behavior mode
            if (DinosaurSpecies == ENPC_DinosaurSpecies::Raptor && PackMembers.Num() > 0)
            {
                CallForBackup(Actor);
                SwitchToFlockingMode();
            }
            else
            {
                SwitchToCombatMode();
            }
        }
    }
    else
    {
        // Actor lost
        if (CurrentTarget == Actor)
        {
            CurrentTarget = nullptr;
            
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
                BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), false);
            }

            SwitchToDefaultMode();
        }
    }
}

void ADinosaurAIController::AssessThreat(AActor* ThreatActor)
{
    if (!ThreatActor)
    {
        return;
    }

    float ThreatLevel = CalculateThreatLevel(ThreatActor);
    RememberThreat(ThreatActor, ThreatLevel);

    LastThreatAssessmentTime = GetWorld()->GetTimeSeconds();
}

bool ADinosaurAIController::IsActorThreat(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }

    // Check if it's a player character
    if (Actor->IsA<APawn>() && Cast<APawn>(Actor)->IsPlayerControlled())
    {
        return true;
    }

    // Check if it's another dinosaur of different species
    if (ADinosaurAIController* OtherController = Cast<ADinosaurAIController>(Cast<APawn>(Actor)->GetController()))
    {
        // Different species are potential threats
        return OtherController->DinosaurSpecies != DinosaurSpecies;
    }

    return false;
}

float ADinosaurAIController::CalculateThreatLevel(AActor* Actor) const
{
    if (!Actor)
    {
        return 0.0f;
    }

    float BaseThreat = 0.5f;
    float DistanceToActor = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());

    // Distance factor (closer = more threatening)
    float DistanceFactor = FMath::Clamp(1.0f - (DistanceToActor / TerritorialRadius), 0.1f, 1.0f);

    // Size factor (larger actors are more threatening)
    float SizeFactor = 1.0f;
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        FVector ActorBounds = PrimComp->Bounds.BoxExtent;
        float ActorSize = ActorBounds.Size();
        SizeFactor = FMath::Clamp(ActorSize / 1000.0f, 0.5f, 2.0f);
    }

    return BaseThreat * DistanceFactor * SizeFactor * AggressionLevel;
}

void ADinosaurAIController::RegisterPackMember(AActor* PackMember)
{
    if (PackMember && !PackMembers.Contains(PackMember))
    {
        PackMembers.Add(PackMember);
    }
}

void ADinosaurAIController::RemovePackMember(AActor* PackMember)
{
    PackMembers.Remove(PackMember);
}

void ADinosaurAIController::CallForBackup(AActor* Threat)
{
    if (!Threat || PackMembers.Num() == 0)
    {
        return;
    }

    // Notify pack members of threat
    for (AActor* PackMember : PackMembers)
    {
        if (APawn* PackPawn = Cast<APawn>(PackMember))
        {
            if (ADinosaurAIController* PackController = Cast<ADinosaurAIController>(PackPawn->GetController()))
            {
                PackController->RespondToPackCall(GetPawn(), Threat);
            }
        }
    }
}

void ADinosaurAIController::RespondToPackCall(AActor* PackLeader, AActor* Threat)
{
    if (!PackLeader || !Threat)
    {
        return;
    }

    // Set threat as current target
    CurrentTarget = Threat;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Threat);
        BlackboardComponent->SetValueAsObject(TEXT("PackLeader"), PackLeader);
        BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), true);
    }

    SwitchToFlockingMode();
}

void ADinosaurAIController::RememberThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
    {
        return;
    }

    FNPC_ThreatMemory NewMemory;
    NewMemory.ThreatActor = ThreatActor;
    NewMemory.ThreatLevel = ThreatLevel;
    NewMemory.LastSeenLocation = ThreatActor->GetActorLocation();
    NewMemory.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewMemory.EncounterCount = 1;

    // Check if we already have memory of this threat
    bool bFoundExisting = false;
    for (FNPC_ThreatMemory& Memory : ThreatMemories)
    {
        if (Memory.ThreatActor == ThreatActor)
        {
            Memory.ThreatLevel = FMath::Max(Memory.ThreatLevel, ThreatLevel);
            Memory.LastSeenLocation = NewMemory.LastSeenLocation;
            Memory.LastSeenTime = NewMemory.LastSeenTime;
            Memory.EncounterCount++;
            bFoundExisting = true;
            break;
        }
    }

    if (!bFoundExisting)
    {
        ThreatMemories.Add(NewMemory);
    }
}

void ADinosaurAIController::RememberLocation(FVector Location, ENPC_LocationImportance Importance)
{
    FNPC_LocationMemory NewMemory;
    NewMemory.Location = Location;
    NewMemory.Importance = Importance;
    NewMemory.VisitedTime = GetWorld()->GetTimeSeconds();
    NewMemory.VisitCount = 1;

    // Check for duplicate locations
    bool bFoundExisting = false;
    for (FNPC_LocationMemory& Memory : LocationMemories)
    {
        if (FVector::Dist(Memory.Location, Location) < 500.0f)
        {
            Memory.VisitedTime = NewMemory.VisitedTime;
            Memory.VisitCount++;
            Memory.Importance = FMath::Max(Memory.Importance, Importance);
            bFoundExisting = true;
            break;
        }
    }

    if (!bFoundExisting)
    {
        LocationMemories.Add(NewMemory);
    }
}

void ADinosaurAIController::ForgetOldMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MemoryDuration = 300.0f; // 5 minutes

    // Clean up old threat memories
    ThreatMemories.RemoveAll([CurrentTime, MemoryDuration](const FNPC_ThreatMemory& Memory)
    {
        return (CurrentTime - Memory.LastSeenTime) > MemoryDuration;
    });

    // Clean up old location memories (keep important ones longer)
    LocationMemories.RemoveAll([CurrentTime](const FNPC_LocationMemory& Memory)
    {
        float MaxAge = (Memory.Importance == ENPC_LocationImportance::High) ? 600.0f : 300.0f;
        return (CurrentTime - Memory.VisitedTime) > MaxAge;
    });
}

void ADinosaurAIController::CleanupMemories()
{
    ForgetOldMemories();

    // Limit memory arrays to prevent excessive growth
    const int32 MaxThreatMemories = 20;
    const int32 MaxLocationMemories = 50;

    if (ThreatMemories.Num() > MaxThreatMemories)
    {
        ThreatMemories.RemoveAt(0, ThreatMemories.Num() - MaxThreatMemories);
    }

    if (LocationMemories.Num() > MaxLocationMemories)
    {
        LocationMemories.RemoveAt(0, LocationMemories.Num() - MaxLocationMemories);
    }
}