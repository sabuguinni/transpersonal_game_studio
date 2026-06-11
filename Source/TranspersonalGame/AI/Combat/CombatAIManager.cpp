#include "CombatAIManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCombatAIManager::UCombatAIManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    CurrentThreatLevel = ECombat_ThreatLevel::None;
    CurrentAIState = ECombat_AIState::Idle;
    CurrentTarget = nullptr;
    OwnerAIController = nullptr;
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
    
    ThreatScanInterval = 0.5f;
    LastThreatScanTime = 0.0f;
    TacticalUpdateInterval = 1.0f;
    LastTacticalUpdateTime = 0.0f;
}

void UCombatAIManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize AI components
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        OwnerAIController = Cast<AAIController>(OwnerPawn->GetController());
        if (OwnerAIController)
        {
            BehaviorTreeComponent = OwnerAIController->GetBehaviorTreeComponent();
            BlackboardComponent = OwnerAIController->GetBlackboardComponent();
        }
    }

    // Set initial state
    SetAIState(ECombat_AIState::Patrol);
}

void UCombatAIManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetWorld())
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Periodic threat scanning
    if (CurrentTime - LastThreatScanTime >= ThreatScanInterval)
    {
        ScanForThreats();
        LastThreatScanTime = CurrentTime;
    }

    // Periodic tactical updates
    if (CurrentTime - LastTacticalUpdateTime >= TacticalUpdateInterval)
    {
        UpdateTacticalSituation();
        LastTacticalUpdateTime = CurrentTime;
    }

    // Clean up invalid threats
    CleanupInvalidThreats();
}

void UCombatAIManager::InitializeCombatAI(AAIController* AIController)
{
    OwnerAIController = AIController;
    if (OwnerAIController)
    {
        BehaviorTreeComponent = OwnerAIController->GetBehaviorTreeComponent();
        BlackboardComponent = OwnerAIController->GetBlackboardComponent();
        
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Initialized for %s"), 
               *GetOwner()->GetName());
    }
}

void UCombatAIManager::SetThreatLevel(ECombat_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        UpdateBlackboardValues();
        
        // React to threat level changes
        switch (NewThreatLevel)
        {
            case ECombat_ThreatLevel::None:
                SetAIState(ECombat_AIState::Patrol);
                break;
            case ECombat_ThreatLevel::Low:
                SetAIState(ECombat_AIState::Alert);
                break;
            case ECombat_ThreatLevel::Medium:
            case ECombat_ThreatLevel::High:
                SetAIState(ECombat_AIState::Combat);
                break;
            case ECombat_ThreatLevel::Critical:
                if (TacticalData.FleeThreshold > 0.7f)
                {
                    SetAIState(ECombat_AIState::Flee);
                }
                else
                {
                    SetAIState(ECombat_AIState::Combat);
                    if (TacticalData.bCanCallForHelp)
                    {
                        CallForHelp(GetOwner()->GetActorLocation(), NewThreatLevel);
                    }
                }
                break;
        }
    }
}

void UCombatAIManager::SetAIState(ECombat_AIState NewState)
{
    if (CurrentAIState != NewState)
    {
        CurrentAIState = NewState;
        UpdateBlackboardValues();
        
        UE_LOG(LogTemp, Log, TEXT("CombatAIManager: %s changed state to %d"), 
               *GetOwner()->GetName(), (int32)NewState);
    }
}

void UCombatAIManager::UpdateTacticalSituation()
{
    if (!GetWorld() || !GetOwner())
        return;

    // Assess current threats
    ECombat_ThreatLevel HighestThreat = ECombat_ThreatLevel::None;
    AActor* PrimaryThreat = nullptr;

    for (AActor* Threat : KnownThreats)
    {
        if (!IsValid(Threat))
            continue;

        ECombat_ThreatLevel ThreatLevel = AssessThreat(Threat);
        if (ThreatLevel > HighestThreat)
        {
            HighestThreat = ThreatLevel;
            PrimaryThreat = Threat;
        }
    }

    // Update current target and threat level
    CurrentTarget = PrimaryThreat;
    SetThreatLevel(HighestThreat);

    // Execute tactical decision based on current situation
    ExecuteTacticalDecision();
}

void UCombatAIManager::ExecuteTacticalDecision()
{
    if (!CurrentTarget || CurrentAIState == ECombat_AIState::Idle)
        return;

    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    switch (CurrentAIState)
    {
        case ECombat_AIState::Combat:
            if (DistanceToTarget <= TacticalData.AttackRange)
            {
                ExecuteAttack(CurrentTarget);
            }
            else if (TacticalData.PackCoordination > 0.5f && PackMembers.Num() > 0)
            {
                ExecuteFlank(CurrentTarget);
            }
            break;

        case ECombat_AIState::Flee:
            ExecuteFlee(CurrentTarget->GetActorLocation());
            break;

        case ECombat_AIState::Hunt:
            if (DistanceToTarget > TacticalData.AttackRange * 2.0f)
            {
                // Move closer to target
                if (OwnerAIController && OwnerAIController->GetPawn())
                {
                    OwnerAIController->MoveToLocation(CurrentTarget->GetActorLocation());
                }
            }
            break;
    }
}

ECombat_ThreatLevel UCombatAIManager::AssessThreat(AActor* Target)
{
    if (!IsValid(Target) || !GetOwner())
        return ECombat_ThreatLevel::None;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    // Distance-based threat assessment
    if (Distance > 2000.0f)
        return ECombat_ThreatLevel::None;
    else if (Distance > 1000.0f)
        return ECombat_ThreatLevel::Low;
    else if (Distance > 500.0f)
        return ECombat_ThreatLevel::Medium;
    else if (Distance > 200.0f)
        return ECombat_ThreatLevel::High;
    else
        return ECombat_ThreatLevel::Critical;
}

void UCombatAIManager::ScanForThreats()
{
    if (!GetWorld() || !GetOwner())
        return;

    // Find all pawns within scan radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!IsValidThreat(Actor))
            continue;

        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= 1500.0f) // Scan radius
        {
            RegisterThreat(Actor, AssessThreat(Actor));
        }
    }
}

void UCombatAIManager::RegisterThreat(AActor* ThreatActor, ECombat_ThreatLevel ThreatLevel)
{
    if (!IsValid(ThreatActor) || ThreatLevel == ECombat_ThreatLevel::None)
        return;

    KnownThreats.AddUnique(ThreatActor);
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: %s registered threat %s with level %d"), 
           *GetOwner()->GetName(), *ThreatActor->GetName(), (int32)ThreatLevel);
}

void UCombatAIManager::CallForHelp(const FVector& Location, ECombat_ThreatLevel ThreatLevel)
{
    if (!TacticalData.bCanCallForHelp)
        return;

    TArray<AActor*> NearbyAllies = FindNearbyAllies(TacticalData.HelpRadius);
    
    for (AActor* Ally : NearbyAllies)
    {
        if (UCombatAIManager* AllyCombatAI = Ally->FindComponentByClass<UCombatAIManager>())
        {
            AllyCombatAI->RespondToHelpCall(Location, GetOwner());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: %s called for help, %d allies responded"), 
           *GetOwner()->GetName(), NearbyAllies.Num());
}

void UCombatAIManager::RespondToHelpCall(const FVector& Location, AActor* Caller)
{
    if (CurrentAIState == ECombat_AIState::Combat || CurrentAIState == ECombat_AIState::Flee)
        return; // Already engaged

    // Move towards the help location
    if (OwnerAIController && OwnerAIController->GetPawn())
    {
        OwnerAIController->MoveToLocation(Location);
        SetAIState(ECombat_AIState::Alert);
    }

    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: %s responding to help call from %s"), 
           *GetOwner()->GetName(), Caller ? *Caller->GetName() : TEXT("Unknown"));
}

TArray<AActor*> UCombatAIManager::FindNearbyAllies(float SearchRadius)
{
    TArray<AActor*> Allies;
    
    if (!GetWorld() || !GetOwner())
        return Allies;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetOwner()->GetClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner() || !IsValid(Actor))
            continue;

        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= SearchRadius)
        {
            Allies.Add(Actor);
        }
    }

    return Allies;
}

void UCombatAIManager::ExecuteAttack(AActor* Target)
{
    if (!IsValid(Target) || !OwnerAIController)
        return;

    // Basic attack implementation - move to target
    OwnerAIController->MoveToLocation(Target->GetActorLocation());
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: %s attacking %s"), 
           *GetOwner()->GetName(), *Target->GetName());
}

void UCombatAIManager::ExecuteFlee(const FVector& DangerLocation)
{
    if (!OwnerAIController || !GetOwner())
        return;

    FVector FleeDirection = CalculateFleeDirection(DangerLocation);
    FVector FleeLocation = GetOwner()->GetActorLocation() + (FleeDirection * 1000.0f);
    
    OwnerAIController->MoveToLocation(FleeLocation);
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: %s fleeing from danger"), 
           *GetOwner()->GetName());
}

void UCombatAIManager::ExecuteFlank(AActor* Target)
{
    if (!IsValid(Target) || !OwnerAIController)
        return;

    FVector FlankPosition = CalculateFlankPosition(Target);
    OwnerAIController->MoveToLocation(FlankPosition);
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: %s flanking %s"), 
           *GetOwner()->GetName(), *Target->GetName());
}

void UCombatAIManager::ExecuteAmbush(const FVector& AmbushLocation)
{
    if (!OwnerAIController)
        return;

    OwnerAIController->MoveToLocation(AmbushLocation);
    SetAIState(ECombat_AIState::Hunt);
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager: %s setting up ambush"), 
           *GetOwner()->GetName());
}

void UCombatAIManager::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), (uint8)CurrentThreatLevel);
    BlackboardComponent->SetValueAsEnum(TEXT("AIState"), (uint8)CurrentAIState);
    
    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
    }
    else
    {
        BlackboardComponent->ClearValue(TEXT("CurrentTarget"));
        BlackboardComponent->ClearValue(TEXT("TargetLocation"));
    }
}

bool UCombatAIManager::IsValidThreat(AActor* PotentialThreat) const
{
    if (!IsValid(PotentialThreat) || PotentialThreat == GetOwner())
        return false;

    // Check if it's a player character or different species
    APawn* ThreatPawn = Cast<APawn>(PotentialThreat);
    if (!ThreatPawn)
        return false;

    // Simple threat validation - different class or player controlled
    return (ThreatPawn->GetClass() != GetOwner()->GetClass()) || ThreatPawn->IsPlayerControlled();
}

FVector UCombatAIManager::CalculateFlankPosition(AActor* Target)
{
    if (!IsValid(Target) || !GetOwner())
        return GetOwner()->GetActorLocation();

    FVector ToTarget = (Target->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
    
    // Choose random side for flanking
    float FlankSide = FMath::RandBool() ? 1.0f : -1.0f;
    FVector FlankDirection = RightVector * FlankSide;
    
    return Target->GetActorLocation() + (FlankDirection * TacticalData.AttackRange * 1.5f);
}

FVector UCombatAIManager::CalculateFleeDirection(const FVector& DangerLocation)
{
    if (!GetOwner())
        return FVector::ZeroVector;

    FVector FleeDirection = (GetOwner()->GetActorLocation() - DangerLocation).GetSafeNormal();
    
    // Add some randomness to avoid predictable fleeing
    FVector RandomOffset = FVector(
        FMath::RandRange(-0.3f, 0.3f),
        FMath::RandRange(-0.3f, 0.3f),
        0.0f
    );
    
    return (FleeDirection + RandomOffset).GetSafeNormal();
}

void UCombatAIManager::CleanupInvalidThreats()
{
    KnownThreats.RemoveAll([](AActor* Threat) {
        return !IsValid(Threat);
    });

    PackMembers.RemoveAll([](AActor* Member) {
        return !IsValid(Member);
    });
}