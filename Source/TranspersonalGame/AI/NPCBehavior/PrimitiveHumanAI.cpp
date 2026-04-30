#include "PrimitiveHumanAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ANPC_PrimitiveHumanAI::ANPC_PrimitiveHumanAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize behavior state
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    EmotionalState = ENPC_EmotionalState::Calm;

    // Initialize survival stats
    Hunger = 0.3f;
    Thirst = 0.2f;
    Fear = 0.1f;
    Energy = 0.8f;

    // Initialize social behavior
    bIsGroupLeader = false;
    CurrentLeader = nullptr;
    ThreatLevel = 0.0f;
    PrimaryThreat = nullptr;

    // Initialize timers
    SurvivalUpdateTimer = 0.0f;
    ThreatEvaluationTimer = 0.0f;
    SocialUpdateTimer = 0.0f;

    // Initialize gathering
    TargetResource = nullptr;
    LastGatherLocation = FVector::ZeroVector;
}

void ANPC_PrimitiveHumanAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_PrimitiveHumanAI::OnPerceptionUpdated);
    }

    // Start behavior tree if available
    if (BehaviorTree && GetBlackboardComponent())
    {
        RunBehaviorTree(BehaviorTree);
        UpdateBlackboardKeys();
    }
}

void ANPC_PrimitiveHumanAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update survival stats periodically
    SurvivalUpdateTimer += DeltaTime;
    if (SurvivalUpdateTimer >= SURVIVAL_UPDATE_INTERVAL)
    {
        UpdateSurvivalStats(DeltaTime);
        SurvivalUpdateTimer = 0.0f;
    }

    // Evaluate threats frequently
    ThreatEvaluationTimer += DeltaTime;
    if (ThreatEvaluationTimer >= THREAT_EVALUATION_INTERVAL)
    {
        EvaluateThreats();
        ThreatEvaluationTimer = 0.0f;
    }

    // Update social behavior less frequently
    SocialUpdateTimer += DeltaTime;
    if (SocialUpdateTimer >= SOCIAL_UPDATE_INTERVAL)
    {
        UpdateSocialBehavior();
        SocialUpdateTimer = 0.0f;
    }

    // Update blackboard with current state
    UpdateBlackboardKeys();
}

void ANPC_PrimitiveHumanAI::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardAsset)
    {
        UseBlackboard(BlackboardAsset);
    }

    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ANPC_PrimitiveHumanAI::OnUnPossess()
{
    Super::OnUnPossess();
    
    if (GetBehaviorTreeComponent())
    {
        GetBehaviorTreeComponent()->StopTree();
    }
}

void ANPC_PrimitiveHumanAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        FString ActorName = Actor->GetName();
        
        // Detect dinosaurs as threats
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || 
            ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Brachio")))
        {
            if (!DetectedThreats.Contains(Actor))
            {
                DetectedThreats.Add(Actor);
                UE_LOG(LogTemp, Warning, TEXT("PrimitiveHuman detected threat: %s"), *ActorName);
            }
        }
        
        // Detect other humans for social behavior
        else if (ActorName.Contains(TEXT("Human")) || ActorName.Contains(TEXT("Primitive")))
        {
            if (!NearbyHumans.Contains(Actor) && Actor != GetPawn())
            {
                NearbyHumans.Add(Actor);
                UE_LOG(LogTemp, Log, TEXT("PrimitiveHuman detected ally: %s"), *ActorName);
            }
        }
        
        // Detect resources (trees, rocks for now)
        else if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) || 
                 ActorName.Contains(TEXT("Berry")) || ActorName.Contains(TEXT("Water")))
        {
            if (!NearbyResources.Contains(Actor))
            {
                NearbyResources.Add(Actor);
            }
        }
    }
}

void ANPC_PrimitiveHumanAI::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        UE_LOG(LogTemp, Log, TEXT("PrimitiveHuman behavior state changed to: %d"), (int32)NewState);
    }
}

void ANPC_PrimitiveHumanAI::UpdateSurvivalStats(float DeltaTime)
{
    // Gradually increase hunger and thirst
    Hunger = FMath::Clamp(Hunger + (DeltaTime * 0.01f), 0.0f, 1.0f);
    Thirst = FMath::Clamp(Thirst + (DeltaTime * 0.015f), 0.0f, 1.0f);
    
    // Energy decreases based on activity
    float EnergyDrain = 0.005f;
    if (CurrentBehaviorState == ENPC_BehaviorState::Fleeing)
    {
        EnergyDrain = 0.02f;
    }
    else if (CurrentBehaviorState == ENPC_BehaviorState::Working)
    {
        EnergyDrain = 0.01f;
    }
    
    Energy = FMath::Clamp(Energy - (DeltaTime * EnergyDrain), 0.0f, 1.0f);
    
    // Fear decreases over time if no threats
    if (DetectedThreats.Num() == 0)
    {
        Fear = FMath::Clamp(Fear - (DeltaTime * 0.1f), 0.0f, 1.0f);
    }
}

void ANPC_PrimitiveHumanAI::EvaluateThreats()
{
    // Clean up invalid threat references
    DetectedThreats.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    
    if (DetectedThreats.Num() == 0)
    {
        PrimaryThreat = nullptr;
        ThreatLevel = 0.0f;
        return;
    }

    // Find closest threat
    float ClosestDistance = FLT_MAX;
    AActor* ClosestThreat = nullptr;
    FVector MyLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;

    for (AActor* Threat : DetectedThreats)
    {
        if (!Threat) continue;
        
        float Distance = FVector::Dist(MyLocation, Threat->GetActorLocation());
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestThreat = Threat;
        }
    }

    PrimaryThreat = ClosestThreat;
    
    // Calculate threat level based on distance
    if (ClosestDistance < 500.0f)
    {
        ThreatLevel = 1.0f;
        Fear = FMath::Clamp(Fear + 0.3f, 0.0f, 1.0f);
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
    }
    else if (ClosestDistance < 1000.0f)
    {
        ThreatLevel = 0.7f;
        Fear = FMath::Clamp(Fear + 0.1f, 0.0f, 1.0f);
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
    else
    {
        ThreatLevel = 0.3f;
    }
}

void ANPC_PrimitiveHumanAI::FindNearbyResources()
{
    // Clean up invalid resource references
    NearbyResources.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    
    if (NearbyResources.Num() == 0 || !GetPawn()) return;

    // Find closest resource
    float ClosestDistance = FLT_MAX;
    AActor* ClosestResource = nullptr;
    FVector MyLocation = GetPawn()->GetActorLocation();

    for (AActor* Resource : NearbyResources)
    {
        if (!Resource) continue;
        
        float Distance = FVector::Dist(MyLocation, Resource->GetActorLocation());
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestResource = Resource;
        }
    }

    TargetResource = ClosestResource;
}

void ANPC_PrimitiveHumanAI::UpdateSocialBehavior()
{
    // Clean up invalid human references
    NearbyHumans.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    
    // Simple leadership logic - first human becomes leader
    if (NearbyHumans.Num() > 0 && !CurrentLeader)
    {
        if (!bIsGroupLeader)
        {
            CurrentLeader = NearbyHumans[0];
        }
        else
        {
            // I am the leader
            CurrentLeader = GetPawn();
        }
    }
    
    // If I'm alone, I can become a leader
    if (NearbyHumans.Num() == 0)
    {
        bIsGroupLeader = true;
        CurrentLeader = GetPawn();
    }
}

void ANPC_PrimitiveHumanAI::UpdateBlackboardKeys()
{
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp) return;

    // Update survival stats
    BlackboardComp->SetValueAsFloat(TEXT("Hunger"), Hunger);
    BlackboardComp->SetValueAsFloat(TEXT("Thirst"), Thirst);
    BlackboardComp->SetValueAsFloat(TEXT("Fear"), Fear);
    BlackboardComp->SetValueAsFloat(TEXT("Energy"), Energy);
    
    // Update behavior state
    BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), (uint8)CurrentBehaviorState);
    BlackboardComp->SetValueAsEnum(TEXT("EmotionalState"), (uint8)EmotionalState);
    
    // Update threat information
    BlackboardComp->SetValueAsObject(TEXT("PrimaryThreat"), PrimaryThreat);
    BlackboardComp->SetValueAsFloat(TEXT("ThreatLevel"), ThreatLevel);
    BlackboardComp->SetValueAsBool(TEXT("InDanger"), IsInDanger());
    
    // Update resource information
    BlackboardComp->SetValueAsObject(TEXT("TargetResource"), TargetResource);
    
    // Update social information
    BlackboardComp->SetValueAsObject(TEXT("GroupLeader"), CurrentLeader);
    BlackboardComp->SetValueAsBool(TEXT("IsGroupLeader"), bIsGroupLeader);
    BlackboardComp->SetValueAsInt(TEXT("NearbyHumansCount"), NearbyHumans.Num());
    
    // Update needs
    BlackboardComp->SetValueAsBool(TEXT("NeedsFood"), NeedsFood());
    BlackboardComp->SetValueAsBool(TEXT("NeedsWater"), NeedsWater());
    BlackboardComp->SetValueAsBool(TEXT("IsExhausted"), IsExhausted());
    
    // Update safe location
    FVector SafeLocation = GetSafeLocation();
    BlackboardComp->SetValueAsVector(TEXT("SafeLocation"), SafeLocation);
}

bool ANPC_PrimitiveHumanAI::IsInDanger() const
{
    return ThreatLevel > DANGER_THRESHOLD || Fear > 0.8f;
}

bool ANPC_PrimitiveHumanAI::NeedsFood() const
{
    return Hunger > HUNGER_THRESHOLD;
}

bool ANPC_PrimitiveHumanAI::NeedsWater() const
{
    return Thirst > THIRST_THRESHOLD;
}

bool ANPC_PrimitiveHumanAI::IsExhausted() const
{
    return Energy < ENERGY_THRESHOLD;
}

FVector ANPC_PrimitiveHumanAI::GetSafeLocation() const
{
    if (!GetPawn()) return FVector::ZeroVector;
    
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // If no threats, current location is safe
    if (!PrimaryThreat)
    {
        return MyLocation;
    }
    
    // Calculate direction away from primary threat
    FVector ThreatLocation = PrimaryThreat->GetActorLocation();
    FVector SafeDirection = (MyLocation - ThreatLocation).GetSafeNormal();
    
    // Find safe location 1000 units away from threat
    FVector SafeLocation = MyLocation + (SafeDirection * 1000.0f);
    
    return SafeLocation;
}