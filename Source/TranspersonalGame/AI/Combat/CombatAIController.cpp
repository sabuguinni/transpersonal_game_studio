#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Set up AI Perception
    SetupPerception();

    // Initialize default values
    CurrentCombatState = ECombatState::Passive;
    CombatArchetype = ECombatArchetype::SolitaryHerbivore;
    CurrentThreatLevel = EThreatLevel::None;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Start periodic updates
    GetWorldTimerManager().SetTimer(ThreatAssessmentTimer, this, &ACombatAIController::UpdateThreatAssessment, 0.5f, true);
    GetWorldTimerManager().SetTimer(MemoryUpdateTimer, this, &ACombatAIController::ProcessThreatMemory, 2.0f, true);
    GetWorldTimerManager().SetTimer(StateUpdateTimer, this, &ACombatAIController::UpdateBlackboardValues, 0.1f, true);
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update patrol behavior if in passive state
    if (CurrentCombatState == ECombatState::Passive)
    {
        UpdatePatrolBehavior();
    }
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (CombatBlackboard)
    {
        UseBlackboard(CombatBlackboard);
    }

    // Start appropriate behavior tree based on archetype
    if (ArchetypeBehaviorTrees.Contains(CombatArchetype))
    {
        RunBehaviorTree(ArchetypeBehaviorTrees[CombatArchetype]);
    }

    // Initialize blackboard values
    UpdateBlackboardValues();
}

void ACombatAIController::SetupPerception()
{
    // Configure Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = SightAngle;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Damage
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);

    // Add configs to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState OldState = CurrentCombatState;
        CurrentCombatState = NewState;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }

        // Broadcast state change
        if (OnCombatStateChanged.IsBound())
        {
            OnCombatStateChanged.Broadcast(Cast<ADinosaurPawn>(GetPawn()), NewState);
        }

        // Handle state-specific logic
        switch (NewState)
        {
        case ECombatState::Hunting:
            AggressionLevel = FMath::Min(AggressionLevel + 0.2f, 1.0f);
            break;
        case ECombatState::Fleeing:
            AggressionLevel = FMath::Max(AggressionLevel - 0.3f, 0.0f);
            break;
        case ECombatState::Attacking:
            AggressionLevel = FMath::Min(AggressionLevel + 0.1f, 1.0f);
            break;
        case ECombatState::Passive:
            AggressionLevel = FMath::Max(AggressionLevel - 0.1f, 0.1f);
            break;
        }
    }
}

void ACombatAIController::SetCombatArchetype(ECombatArchetype NewArchetype)
{
    if (CombatArchetype != NewArchetype)
    {
        CombatArchetype = NewArchetype;

        // Update behavior tree
        if (ArchetypeBehaviorTrees.Contains(NewArchetype))
        {
            RunBehaviorTree(ArchetypeBehaviorTrees[NewArchetype]);
        }

        // Update archetype-specific parameters
        switch (NewArchetype)
        {
        case ECombatArchetype::ApexPredator:
            AggressionLevel = 0.8f;
            TerritorialRadius = 1500.0f;
            bUsesAmbushTactics = false;
            bUsesPackCoordination = false;
            break;
        case ECombatArchetype::PackHunter:
            AggressionLevel = 0.7f;
            bUsesPackCoordination = true;
            bCanCallForHelp = true;
            break;
        case ECombatArchetype::AmbushPredator:
            AggressionLevel = 0.6f;
            bUsesAmbushTactics = true;
            AmbushWaitTime = 8.0f;
            break;
        case ECombatArchetype::TerritorialHerbivore:
            AggressionLevel = 0.4f;
            TerritorialRadius = 800.0f;
            break;
        case ECombatArchetype::FlockHerbivore:
            AggressionLevel = 0.2f;
            bCanCallForHelp = true;
            break;
        case ECombatArchetype::SolitaryHerbivore:
            AggressionLevel = 0.1f;
            FleeHealthThreshold = 0.8f;
            break;
        }

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatArchetype"), static_cast<uint8>(NewArchetype));
        }
    }
}

EThreatLevel ACombatAIController::CalculateThreatLevel(AActor* Target)
{
    if (!Target)
        return EThreatLevel::None;

    float ThreatScore = 0.0f;

    // Distance factor
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / SightRadius), 0.0f, 1.0f);
    ThreatScore += DistanceFactor * 0.3f;

    // Player factor
    if (Target == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        ThreatScore += 0.4f;
    }

    // Health factor (lower health = higher threat perception)
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        // Assuming health component exists
        float HealthPercentage = 1.0f; // Would get from health component
        ThreatScore += (1.0f - HealthPercentage) * 0.2f;
    }

    // Archetype-specific threat calculation
    switch (CombatArchetype)
    {
    case ECombatArchetype::ApexPredator:
        ThreatScore *= 0.7f; // Less threatened
        break;
    case ECombatArchetype::FlockHerbivore:
        ThreatScore *= 1.5f; // More easily threatened
        break;
    }

    // Convert to enum
    if (ThreatScore < 0.2f) return EThreatLevel::None;
    if (ThreatScore < 0.4f) return EThreatLevel::Low;
    if (ThreatScore < 0.6f) return EThreatLevel::Medium;
    if (ThreatScore < 0.8f) return EThreatLevel::High;
    return EThreatLevel::Extreme;
}

void ACombatAIController::UpdateThreatAssessment()
{
    EThreatLevel HighestThreat = EThreatLevel::None;
    AActor* PrimaryThreat = nullptr;

    // Assess all known threats
    for (AActor* Threat : KnownThreats)
    {
        if (IsValid(Threat))
        {
            EThreatLevel ThreatLevel = CalculateThreatLevel(Threat);
            if (ThreatLevel > HighestThreat)
            {
                HighestThreat = ThreatLevel;
                PrimaryThreat = Threat;
            }
        }
    }

    CurrentThreatLevel = HighestThreat;

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(HighestThreat));
        BlackboardComponent->SetValueAsObject(TEXT("PrimaryThreat"), PrimaryThreat);
    }

    // Update combat state based on threat level
    switch (HighestThreat)
    {
    case EThreatLevel::None:
        if (CurrentCombatState != ECombatState::Passive)
            SetCombatState(ECombatState::Passive);
        break;
    case EThreatLevel::Low:
        if (CurrentCombatState == ECombatState::Passive)
            SetCombatState(ECombatState::Investigating);
        break;
    case EThreatLevel::Medium:
        if (ShouldRetreat())
            SetCombatState(ECombatState::Fleeing);
        else
            SetCombatState(ECombatState::Hunting);
        break;
    case EThreatLevel::High:
    case EThreatLevel::Extreme:
        if (ShouldRetreat())
            SetCombatState(ECombatState::Fleeing);
        else
            SetCombatState(ECombatState::Attacking);
        break;
    }
}

FVector ACombatAIController::FindAmbushPosition(AActor* Target)
{
    if (!Target || !bUsesAmbushTactics)
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Find position behind cover near target
    TArray<FVector> PotentialPositions;
    
    for (int32 i = 0; i < 8; ++i)
    {
        float Angle = i * 45.0f;
        FVector Direction = FVector(FMath::Cos(FMath::DegreesToRadians(Angle)), 
                                   FMath::Sin(FMath::DegreesToRadians(Angle)), 0);
        FVector TestPosition = TargetLocation + Direction * 500.0f;
        
        // Check if position has cover
        FHitResult HitResult;
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TestPosition, TargetLocation, ECC_Visibility))
        {
            PotentialPositions.Add(TestPosition);
        }
    }
    
    if (PotentialPositions.Num() > 0)
    {
        return PotentialPositions[FMath::RandRange(0, PotentialPositions.Num() - 1)];
    }
    
    return MyLocation;
}

FVector ACombatAIController::FindFlankingPosition(AActor* Target)
{
    if (!Target)
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    
    // Find position to the side of target
    FVector RightVector = FVector::CrossProduct(TargetForward, FVector::UpVector);
    float Side = FMath::RandBool() ? 1.0f : -1.0f;
    
    FVector FlankPosition = TargetLocation + (RightVector * Side * 300.0f) + (TargetForward * -200.0f);
    
    // Ensure position is navigable
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(FlankPosition, NavLocation))
        {
            return NavLocation.Location;
        }
    }
    
    return GetPawn()->GetActorLocation();
}

void ACombatAIController::CallForBackup(float Radius)
{
    if (!bCanCallForHelp)
        return;

    TArray<ACombatAIController*> NearbyAI = GetNearbyPackMembers(Radius);
    
    for (ACombatAIController* AI : NearbyAI)
    {
        if (AI && AI->CurrentThreatLevel < EThreatLevel::Medium)
        {
            // Alert nearby AI to threat
            if (AI->BlackboardComponent)
            {
                AI->BlackboardComponent->SetValueAsObject(TEXT("AlertedThreat"), 
                    BlackboardComponent->GetValueAsObject(TEXT("PrimaryThreat")));
                AI->SetCombatState(ECombatState::Investigating);
            }
        }
    }
}

bool ACombatAIController::ShouldRetreat()
{
    // Check health threshold
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        // Would check actual health component
        float HealthPercentage = 1.0f;
        if (HealthPercentage < FleeHealthThreshold)
            return true;
    }

    // Check if outnumbered (for pack hunters)
    if (bUsesPackCoordination)
    {
        int32 NearbyAllies = GetNearbyPackMembers(500.0f).Num();
        int32 NearbyThreats = 0;
        
        for (AActor* Threat : KnownThreats)
        {
            if (IsValid(Threat) && FVector::Dist(GetPawn()->GetActorLocation(), Threat->GetActorLocation()) < 500.0f)
            {
                NearbyThreats++;
            }
        }
        
        if (NearbyThreats > NearbyAllies * 2)
            return true;
    }

    // Archetype-specific retreat logic
    switch (CombatArchetype)
    {
    case ECombatArchetype::FlockHerbivore:
    case ECombatArchetype::SolitaryHerbivore:
        return CurrentThreatLevel >= EThreatLevel::Medium;
    case ECombatArchetype::ApexPredator:
        return false; // Apex predators rarely retreat
    default:
        return CurrentThreatLevel >= EThreatLevel::High;
    }
}

TArray<ACombatAIController*> ACombatAIController::GetNearbyPackMembers(float Radius)
{
    TArray<ACombatAIController*> PackMembers;
    
    for (TActorIterator<ACombatAIController> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ACombatAIController* OtherAI = *ActorItr;
        if (OtherAI && OtherAI != this && OtherAI->CombatArchetype == CombatArchetype)
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), OtherAI->GetPawn()->GetActorLocation());
            if (Distance <= Radius)
            {
                PackMembers.Add(OtherAI);
            }
        }
    }
    
    return PackMembers;
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            // Add to known threats if not already present
            if (!KnownThreats.Contains(Actor))
            {
                KnownThreats.Add(Actor);
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Update last known location
        if (Actor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            LastKnownPlayerLocation = Actor;
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), Actor->GetActorLocation());
            }
        }
    }
}

void ACombatAIController::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    // Update core values
    BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CurrentThreatLevel));
    BlackboardComponent->SetValueAsEnum(TEXT("CombatArchetype"), static_cast<uint8>(CombatArchetype));
    BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
    BlackboardComponent->SetValueAsFloat(TEXT("TerritorialRadius"), TerritorialRadius);
    
    // Update behavioral flags
    BlackboardComponent->SetValueAsBool(TEXT("UsesAmbushTactics"), bUsesAmbushTactics);
    BlackboardComponent->SetValueAsBool(TEXT("UsesPackCoordination"), bUsesPackCoordination);
    BlackboardComponent->SetValueAsBool(TEXT("CanCallForHelp"), bCanCallForHelp);
}

void ACombatAIController::ProcessThreatMemory()
{
    // Remove old threats from memory
    KnownThreats.RemoveAll([this](AActor* Threat)
    {
        if (!IsValid(Threat))
            return true;

        // Check if threat is still within perception range
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Threat->GetActorLocation());
        return Distance > SightRadius * 1.5f;
    });
}

void ACombatAIController::UpdatePatrolBehavior()
{
    if (PatrolPoints.Num() == 0)
    {
        // Generate random patrol points around territory
        FVector CenterLocation = GetPawn()->GetActorLocation();
        for (int32 i = 0; i < 4; ++i)
        {
            float Angle = i * 90.0f;
            FVector Direction = FVector(FMath::Cos(FMath::DegreesToRadians(Angle)), 
                                       FMath::Sin(FMath::DegreesToRadians(Angle)), 0);
            FVector PatrolPoint = CenterLocation + Direction * (TerritorialRadius * 0.7f);
            PatrolPoints.Add(PatrolPoint);
        }
    }
}