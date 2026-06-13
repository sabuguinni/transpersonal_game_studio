#include "NPCBehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"

UNPC_BehaviorTreeComponent::UNPC_BehaviorTreeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default values
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    PatrolRadius = 1500.0f;
    WorkDuration = 300.0f; // 5 minutes
    SocialDistance = 200.0f;
    FleeDistance = 1200.0f;
    ThreatMemoryDuration = 600.0f; // 10 minutes
    ThreatCheckInterval = 2.0f;
    LastThreatCheck = 0.0f;

    // Create AI components
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
}

void UNPC_BehaviorTreeComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBehaviorTree();
}

void UNPC_BehaviorTreeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update behavior based on time of day
    UpdateBehaviorBasedOnTime();

    // Handle threat detection
    if (GetWorld()->GetTimeSeconds() - LastThreatCheck > ThreatCheckInterval)
    {
        HandleThreatDetection();
        LastThreatCheck = GetWorld()->GetTimeSeconds();
    }

    // Clear old threats from memory
    ClearOldThreats();
}

void UNPC_BehaviorTreeComponent::InitializeBehaviorTree()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Get AI Controller
    if (APawn* Pawn = Cast<APawn>(Owner))
    {
        if (AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
        {
            BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
            BlackboardComponent = AIController->GetBlackboardComponent();

            // Initialize blackboard if we have one
            if (NPCBlackboard && BlackboardComponent)
            {
                BlackboardComponent->InitializeBlackboard(*NPCBlackboard);
                
                // Set initial blackboard values
                UpdateBlackboardValue("HomeLocation", Owner->GetActorLocation());
                UpdateBlackboardValue("PatrolRadius", FVector(PatrolRadius, 0.0f, 0.0f));
            }

            // Start with idle behavior
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}

void UNPC_BehaviorTreeComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState == NewState)
    {
        return;
    }

    CurrentBehaviorState = NewState;

    // Select appropriate behavior tree
    UBehaviorTree* TreeToStart = nullptr;
    switch (NewState)
    {
        case ENPC_BehaviorState::Idle:
            TreeToStart = IdleBehaviorTree;
            break;
        case ENPC_BehaviorState::Patrolling:
            TreeToStart = PatrolBehaviorTree;
            break;
        case ENPC_BehaviorState::Working:
            TreeToStart = WorkBehaviorTree;
            break;
        case ENPC_BehaviorState::Socializing:
            TreeToStart = SocializeBehaviorTree;
            break;
        case ENPC_BehaviorState::Fleeing:
            TreeToStart = FleeBehaviorTree;
            break;
        default:
            TreeToStart = IdleBehaviorTree;
            break;
    }

    if (TreeToStart)
    {
        StartBehaviorTree(TreeToStart);
    }
}

void UNPC_BehaviorTreeComponent::StartBehaviorTree(UBehaviorTree* TreeToStart)
{
    if (!TreeToStart || !BehaviorTreeComponent)
    {
        return;
    }

    // Stop current tree if running
    if (BehaviorTreeComponent->GetCurrentTree())
    {
        BehaviorTreeComponent->StopTree();
    }

    // Start new tree
    BehaviorTreeComponent->StartTree(*TreeToStart);
}

void UNPC_BehaviorTreeComponent::StopBehaviorTree()
{
    if (BehaviorTreeComponent && BehaviorTreeComponent->GetCurrentTree())
    {
        BehaviorTreeComponent->StopTree();
    }
}

void UNPC_BehaviorTreeComponent::UpdateBlackboardValue(const FString& KeyName, const FVector& Value)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(FName(*KeyName), Value);
    }
}

void UNPC_BehaviorTreeComponent::UpdateBlackboardActor(const FString& KeyName, AActor* Actor)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(FName(*KeyName), Actor);
    }
}

void UNPC_BehaviorTreeComponent::AddThreatLocation(const FVector& Location)
{
    ThreatLocations.Add(Location);
    
    // Update blackboard with nearest threat
    if (BlackboardComponent)
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        FVector NearestThreat = Location;
        float NearestDistance = FVector::Dist(OwnerLocation, Location);

        for (const FVector& ThreatLoc : ThreatLocations)
        {
            float Distance = FVector::Dist(OwnerLocation, ThreatLoc);
            if (Distance < NearestDistance)
            {
                NearestThreat = ThreatLoc;
                NearestDistance = Distance;
            }
        }

        UpdateBlackboardValue("NearestThreat", NearestThreat);
    }
}

void UNPC_BehaviorTreeComponent::AddKnownThreat(AActor* Threat)
{
    if (Threat && !KnownThreats.Contains(Threat))
    {
        KnownThreats.Add(Threat);
        UpdateBlackboardActor("CurrentThreat", Threat);
    }
}

void UNPC_BehaviorTreeComponent::ClearOldThreats()
{
    // Remove invalid actors
    KnownThreats.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });

    // Clear threat locations that are too old (simplified - in real implementation would track timestamps)
    if (ThreatLocations.Num() > 10)
    {
        ThreatLocations.RemoveAt(0, ThreatLocations.Num() - 10);
    }
}

bool UNPC_BehaviorTreeComponent::IsLocationThreatening(const FVector& Location, float Radius)
{
    for (const FVector& ThreatLoc : ThreatLocations)
    {
        if (FVector::Dist(Location, ThreatLoc) < Radius)
        {
            return true;
        }
    }
    return false;
}

FVector UNPC_BehaviorTreeComponent::GetRandomPatrolPoint()
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector HomeLocation = GetOwner()->GetActorLocation();
    
    // Generate random point within patrol radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );

    return HomeLocation + RandomOffset;
}

AActor* UNPC_BehaviorTreeComponent::FindNearestNPC(float MaxDistance)
{
    if (!GetOwner())
    {
        return nullptr;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    AActor* NearestNPC = nullptr;
    float NearestDistance = MaxDistance;

    // Find all actors with NPC behavior components
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor != GetOwner())
            {
                if (Actor->FindComponentByClass<UNPC_BehaviorTreeComponent>())
                {
                    float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                    if (Distance < NearestDistance)
                    {
                        NearestNPC = Actor;
                        NearestDistance = Distance;
                    }
                }
            }
        }
    }

    return NearestNPC;
}

bool UNPC_BehaviorTreeComponent::IsPlayerNearby(float DetectionRadius)
{
    if (!GetOwner())
    {
        return false;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
        return Distance <= DetectionRadius;
    }

    return false;
}

void UNPC_BehaviorTreeComponent::UpdateBehaviorBasedOnTime()
{
    // Simple time-based behavior switching
    // In a full implementation, this would use a proper day/night cycle system
    
    float GameTime = GetWorld()->GetTimeSeconds();
    int32 TimeOfDay = ((int32)GameTime / 60) % 24; // Simplified hour system

    // Early morning (6-8): Work
    if (TimeOfDay >= 6 && TimeOfDay < 8)
    {
        if (CurrentBehaviorState != ENPC_BehaviorState::Working && CurrentBehaviorState != ENPC_BehaviorState::Fleeing)
        {
            SetBehaviorState(ENPC_BehaviorState::Working);
        }
    }
    // Day (8-18): Patrol or socialize
    else if (TimeOfDay >= 8 && TimeOfDay < 18)
    {
        if (CurrentBehaviorState == ENPC_BehaviorState::Idle)
        {
            // Randomly choose between patrolling and socializing
            if (FMath::RandBool())
            {
                SetBehaviorState(ENPC_BehaviorState::Patrolling);
            }
            else if (FindNearestNPC(SocialDistance))
            {
                SetBehaviorState(ENPC_BehaviorState::Socializing);
            }
        }
    }
    // Evening/Night (18-6): Idle
    else
    {
        if (CurrentBehaviorState != ENPC_BehaviorState::Fleeing)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}

void UNPC_BehaviorTreeComponent::HandleThreatDetection()
{
    if (!GetOwner())
    {
        return;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    // Check for player proximity as potential threat
    if (IsPlayerNearby(FleeDistance))
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            AddKnownThreat(PlayerPawn);
            AddThreatLocation(PlayerPawn->GetActorLocation());
            
            // Switch to flee behavior if close enough
            float Distance = FVector::Dist(OwnerLocation, PlayerPawn->GetActorLocation());
            if (Distance < FleeDistance * 0.7f)
            {
                SetBehaviorState(ENPC_BehaviorState::Fleeing);
            }
        }
    }

    // Check for dinosaur threats (simplified - would use proper dinosaur detection)
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<APawn> PawnIterator(World); PawnIterator; ++PawnIterator)
        {
            APawn* Pawn = *PawnIterator;
            if (Pawn && Pawn != GetOwner())
            {
                // Check if it's a dinosaur (simplified check by name)
                FString ActorName = Pawn->GetName().ToLower();
                if (ActorName.Contains("trex") || ActorName.Contains("raptor") || ActorName.Contains("dino"))
                {
                    float Distance = FVector::Dist(OwnerLocation, Pawn->GetActorLocation());
                    if (Distance < FleeDistance)
                    {
                        AddKnownThreat(Pawn);
                        AddThreatLocation(Pawn->GetActorLocation());
                        SetBehaviorState(ENPC_BehaviorState::Fleeing);
                        break;
                    }
                }
            }
        }
    }
}