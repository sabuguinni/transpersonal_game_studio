#include "TribalNPCController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ATribalNPCController::ATribalNPCController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Initialize tribal properties
    TribalRole = ENPC_TribalRole::Gatherer;
    FearLevel = 0.3f;
    CourageLevel = 0.5f;
    SocialBondStrength = 0.7f;

    // Initialize survival stats
    HungerLevel = 0.2f;
    ThirstLevel = 0.1f;
    EnergyLevel = 0.8f;
    HealthLevel = 1.0f;

    // Initialize territory settings
    TerritoryRadius = 2000.0f;
    HomeLocation = FVector::ZeroVector;

    // Initialize threat detection
    DinosaurDetectionRange = 1500.0f;
    PlayerDetectionRange = 800.0f;
    ThreatResponseTime = 2.0f;

    // Initialize state
    CurrentPatrolIndex = 0;
    LastThreatTime = 0.0f;
    CurrentThreat = nullptr;
    bIsAlarmed = false;
    bIsReturningHome = false;

    InitializeAIPerception();
}

void ATribalNPCController::BeginPlay()
{
    Super::BeginPlay();

    // Set home location to spawn location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Start behavior tree after a short delay
    FTimerHandle StartBehaviorTimer;
    GetWorld()->GetTimerManager().SetTimer(StartBehaviorTimer, this, &ATribalNPCController::StartBehaviorTree, 1.0f, false);
}

void ATribalNPCController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateSurvivalNeeds(DeltaTime);
    UpdateBlackboardValues();
    ProcessThreatDetection();
    HandleRoleSpecificBehavior();
}

void ATribalNPCController::InitializeAIPerception()
{
    // Configure sight sense
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = FMath::Max(DinosaurDetectionRange, PlayerDetectionRange);
    SightConfig->LoseSightRadius = SightConfig->SightRadius + 200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure hearing sense
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1000.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ATribalNPCController::OnPerceptionUpdated);
}

void ATribalNPCController::StartBehaviorTree()
{
    if (BehaviorTree && BlackboardAsset)
    {
        if (BlackboardComponent)
        {
            BlackboardComponent->InitializeBlackboard(*BlackboardAsset);
        }

        if (BehaviorTreeComponent)
        {
            BehaviorTreeComponent->StartTree(*BehaviorTree);
        }
    }
}

void ATribalNPCController::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ATribalNPCController::SetTribalRole(ENPC_TribalRole NewRole)
{
    TribalRole = NewRole;

    // Adjust stats based on role
    switch (TribalRole)
    {
    case ENPC_TribalRole::Hunter:
        CourageLevel = 0.8f;
        FearLevel = 0.2f;
        DinosaurDetectionRange = 2000.0f;
        break;
    case ENPC_TribalRole::Gatherer:
        CourageLevel = 0.4f;
        FearLevel = 0.6f;
        PlayerDetectionRange = 1200.0f;
        break;
    case ENPC_TribalRole::Scout:
        CourageLevel = 0.6f;
        FearLevel = 0.4f;
        DinosaurDetectionRange = 2500.0f;
        PlayerDetectionRange = 1500.0f;
        break;
    case ENPC_TribalRole::Elder:
        CourageLevel = 0.9f;
        FearLevel = 0.1f;
        SocialBondStrength = 1.0f;
        break;
    case ENPC_TribalRole::Child:
        CourageLevel = 0.1f;
        FearLevel = 0.9f;
        DinosaurDetectionRange = 800.0f;
        break;
    }
}

void ATribalNPCController::UpdateSurvivalNeeds(float DeltaTime)
{
    // Gradually increase hunger and thirst
    HungerLevel += DeltaTime * 0.01f; // Hunger increases slowly
    ThirstLevel += DeltaTime * 0.015f; // Thirst increases slightly faster

    // Energy decreases with activity
    if (GetPawn() && GetPawn()->GetVelocity().Size() > 100.0f)
    {
        EnergyLevel -= DeltaTime * 0.02f; // Moving drains energy
    }
    else
    {
        EnergyLevel += DeltaTime * 0.005f; // Resting restores energy slowly
    }

    // Health decreases if basic needs aren't met
    if (HungerLevel > 0.8f || ThirstLevel > 0.8f)
    {
        HealthLevel -= DeltaTime * 0.005f;
    }
    else if (HungerLevel < 0.3f && ThirstLevel < 0.3f && EnergyLevel > 0.5f)
    {
        HealthLevel += DeltaTime * 0.002f; // Slow health recovery
    }

    // Clamp values
    HungerLevel = FMath::Clamp(HungerLevel, 0.0f, 1.0f);
    ThirstLevel = FMath::Clamp(ThirstLevel, 0.0f, 1.0f);
    EnergyLevel = FMath::Clamp(EnergyLevel, 0.0f, 1.0f);
    HealthLevel = FMath::Clamp(HealthLevel, 0.0f, 1.0f);
}

bool ATribalNPCController::IsInDanger() const
{
    return CurrentThreat != nullptr || bIsAlarmed || HealthLevel < 0.3f;
}

void ATribalNPCController::ReactToThreat(AActor* ThreatActor)
{
    if (!ThreatActor)
    {
        return;
    }

    CurrentThreat = ThreatActor;
    bIsAlarmed = true;
    LastThreatTime = GetWorld()->GetTimeSeconds();

    // Increase fear based on threat type and role
    float ThreatFear = 0.3f;
    if (ThreatActor->GetClass()->GetName().Contains("Dinosaur"))
    {
        ThreatFear = 0.7f;
    }

    FearLevel = FMath::Min(FearLevel + ThreatFear, 1.0f);

    // Role-specific threat responses
    switch (TribalRole)
    {
    case ENPC_TribalRole::Hunter:
        // Hunters may stand their ground or try to distract the threat
        if (CourageLevel > FearLevel)
        {
            // Attempt to face the threat
            if (GetPawn())
            {
                FVector Direction = (ThreatActor->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
                GetPawn()->SetActorRotation(Direction.Rotation());
            }
        }
        break;
    case ENPC_TribalRole::Scout:
        // Scouts call for help and try to lead threats away
        CallForHelp();
        break;
    case ENPC_TribalRole::Elder:
        // Elders organize group response
        CallForHelp();
        break;
    default:
        // Others flee to safety
        ReturnToHome();
        break;
    }
}

void ATribalNPCController::CallForHelp()
{
    // Find nearby tribal NPCs and alert them
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATribalNPCController::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        ATribalNPCController* OtherNPC = Cast<ATribalNPCController>(Actor);
        if (OtherNPC && OtherNPC != this)
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), OtherNPC->GetPawn()->GetActorLocation());
            if (Distance < 1500.0f) // Help call range
            {
                OtherNPC->RespondToDistressCall(this);
            }
        }
    }
}

void ATribalNPCController::RespondToDistressCall(ATribalNPCController* CallingNPC)
{
    if (!CallingNPC || !CallingNPC->GetPawn())
    {
        return;
    }

    // Increase alertness
    bIsAlarmed = true;
    FearLevel = FMath::Min(FearLevel + 0.2f, 1.0f);

    // Role-specific responses to distress calls
    switch (TribalRole)
    {
    case ENPC_TribalRole::Hunter:
        // Hunters may move to help
        if (CourageLevel > 0.6f)
        {
            // Move toward the calling NPC
            MoveToLocation(CallingNPC->GetPawn()->GetActorLocation());
        }
        break;
    case ENPC_TribalRole::Elder:
        // Elders coordinate group response
        CallForHelp();
        break;
    default:
        // Others flee to safety
        ReturnToHome();
        break;
    }
}

void ATribalNPCController::SetPatrolPoints(const TArray<FVector>& NewPatrolPoints)
{
    PatrolPoints = NewPatrolPoints;
    CurrentPatrolIndex = 0;
}

FVector ATribalNPCController::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return HomeLocation;
    }

    FVector NextPoint = PatrolPoints[CurrentPatrolIndex];
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return NextPoint;
}

void ATribalNPCController::ReturnToHome()
{
    bIsReturningHome = true;
    if (GetPawn())
    {
        MoveToLocation(HomeLocation);
    }
}

void ATribalNPCController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
        {
            continue;
        }

        FString ActorName = Actor->GetClass()->GetName();
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());

        // Check for dinosaur threats
        if (ActorName.Contains("Dinosaur") && Distance < DinosaurDetectionRange)
        {
            ReactToThreat(Actor);
        }
        // Check for player
        else if (ActorName.Contains("Character") && Distance < PlayerDetectionRange)
        {
            // Different reaction to player based on role and fear level
            if (FearLevel > 0.5f)
            {
                ReactToThreat(Actor);
            }
        }
    }
}

void ATribalNPCController::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Update survival stats
    BlackboardComponent->SetValueAsFloat("HungerLevel", HungerLevel);
    BlackboardComponent->SetValueAsFloat("ThirstLevel", ThirstLevel);
    BlackboardComponent->SetValueAsFloat("EnergyLevel", EnergyLevel);
    BlackboardComponent->SetValueAsFloat("HealthLevel", HealthLevel);
    BlackboardComponent->SetValueAsFloat("FearLevel", FearLevel);

    // Update state flags
    BlackboardComponent->SetValueAsBool("IsInDanger", IsInDanger());
    BlackboardComponent->SetValueAsBool("IsAlarmed", bIsAlarmed);
    BlackboardComponent->SetValueAsBool("IsReturningHome", bIsReturningHome);

    // Update locations
    BlackboardComponent->SetValueAsVector("HomeLocation", HomeLocation);
    if (PatrolPoints.Num() > 0)
    {
        BlackboardComponent->SetValueAsVector("NextPatrolPoint", GetNextPatrolPoint());
    }

    // Update threat information
    if (CurrentThreat)
    {
        BlackboardComponent->SetValueAsObject("CurrentThreat", CurrentThreat);
        BlackboardComponent->SetValueAsVector("ThreatLocation", CurrentThreat->GetActorLocation());
    }
}

void ATribalNPCController::ProcessThreatDetection()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Clear threat after some time if no longer detected
    if (CurrentThreat && (CurrentTime - LastThreatTime) > ThreatResponseTime * 2.0f)
    {
        CurrentThreat = nullptr;
        bIsAlarmed = false;
        bIsReturningHome = false;

        // Gradually reduce fear
        FearLevel = FMath::Max(FearLevel - 0.1f, 0.1f);
    }
}

void ATribalNPCController::HandleRoleSpecificBehavior()
{
    // Role-specific periodic behaviors
    switch (TribalRole)
    {
    case ENPC_TribalRole::Scout:
        // Scouts have wider patrol ranges
        if (PatrolPoints.Num() == 0)
        {
            // Generate patrol points around home
            TArray<FVector> ScoutPatrol;
            for (int32 i = 0; i < 4; i++)
            {
                float Angle = i * 90.0f * PI / 180.0f;
                FVector PatrolPoint = HomeLocation + FVector(
                    FMath::Cos(Angle) * TerritoryRadius * 0.8f,
                    FMath::Sin(Angle) * TerritoryRadius * 0.8f,
                    0.0f
                );
                ScoutPatrol.Add(PatrolPoint);
            }
            SetPatrolPoints(ScoutPatrol);
        }
        break;
    case ENPC_TribalRole::Gatherer:
        // Gatherers focus on resource areas
        if (HungerLevel > 0.6f)
        {
            // Look for food sources (simplified)
            ThirstLevel = FMath::Max(ThirstLevel - 0.1f, 0.0f);
        }
        break;
    }
}