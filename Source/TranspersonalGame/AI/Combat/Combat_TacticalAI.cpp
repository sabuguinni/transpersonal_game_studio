#include "Combat_TacticalAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

ACombat_TacticalAI::ACombat_TacticalAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure sight perception
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = SightRadius * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = SightAngle;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing perception
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRadius;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize tactical state
    CurrentTacticalState = ECombat_TacticalState::Idle;
    TerritorialCenter = FVector::ZeroVector;
}

void ACombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAI::OnPerceptionUpdated);
    }

    // Set territorial center to spawn location
    if (GetPawn())
    {
        TerritorialCenter = GetPawn()->GetActorLocation();
    }

    // Start behavior tree if available
    if (CombatBehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
}

void ACombat_TacticalAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateTacticalBehavior(DeltaTime);
}

void ACombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentTacticalState != NewState)
    {
        CurrentTacticalState = NewState;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(NewState));
        }
    }
}

void ACombat_TacticalAI::InitializePackBehavior(const TArray<AActor*>& PackMembers, bool bAsLeader)
{
    TacticalData.PackMembers = PackMembers;
    TacticalData.bIsPackLeader = bAsLeader;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsPackLeader"), bAsLeader);
        BlackboardComponent->SetValueAsInt(TEXT("PackSize"), PackMembers.Num());
    }
}

void ACombat_TacticalAI::CoordinatePackAttack(AActor* Target)
{
    if (!Target || !TacticalData.bIsPackLeader)
        return;

    // Coordinate pack members for attack
    for (AActor* PackMember : TacticalData.PackMembers)
    {
        if (!PackMember || PackMember == GetPawn())
            continue;

        // Try to get the AI controller of pack member
        if (APawn* PackPawn = Cast<APawn>(PackMember))
        {
            if (ACombat_TacticalAI* PackAI = Cast<ACombat_TacticalAI>(PackPawn->GetController()))
            {
                PackAI->SetTacticalState(ECombat_TacticalState::Attack);
                if (PackAI->BlackboardComponent)
                {
                    PackAI->BlackboardComponent->SetValueAsObject(TEXT("AttackTarget"), Target);
                }
            }
        }
    }
}

void ACombat_TacticalAI::CallForPackHelp(AActor* Threat)
{
    if (!Threat || !TacticalData.bCanCallForHelp)
        return;

    // Alert nearby pack members
    for (AActor* PackMember : TacticalData.PackMembers)
    {
        if (!PackMember || PackMember == GetPawn())
            continue;

        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PackMember->GetActorLocation());
        if (Distance <= TacticalData.PackCoordinationRange)
        {
            if (APawn* PackPawn = Cast<APawn>(PackMember))
            {
                if (ACombat_TacticalAI* PackAI = Cast<ACombat_TacticalAI>(PackPawn->GetController()))
                {
                    PackAI->SetTacticalState(ECombat_TacticalState::Defend);
                    if (PackAI->BlackboardComponent)
                    {
                        PackAI->BlackboardComponent->SetValueAsObject(TEXT("ThreatTarget"), Threat);
                    }
                }
            }
        }
    }
}

bool ACombat_TacticalAI::IsInTerritorialRange(const FVector& Location) const
{
    float Distance = FVector::Dist(TerritorialCenter, Location);
    return Distance <= TacticalData.TerritorialRadius;
}

void ACombat_TacticalAI::DefendTerritory(AActor* Intruder)
{
    if (!Intruder)
        return;

    SetTacticalState(ECombat_TacticalState::Territorial);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TerritorialIntruder"), Intruder);
        BlackboardComponent->SetValueAsVector(TEXT("TerritorialCenter"), TerritorialCenter);
    }
}

float ACombat_TacticalAI::CalculateThreatLevel(AActor* Target) const
{
    if (!Target)
        return 0.0f;

    float ThreatLevel = 0.5f; // Base threat level
    
    // Distance factor - closer = more threatening
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.1f, 1.0f);
    ThreatLevel += DistanceFactor * 0.3f;
    
    // Size factor (rough estimation based on actor bounds)
    FVector ActorBounds = Target->GetActorBounds(false).BoxExtent;
    float SizeFactor = FMath::Clamp(ActorBounds.Size() / 1000.0f, 0.1f, 1.0f);
    ThreatLevel += SizeFactor * 0.2f;
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

FVector ACombat_TacticalAI::GetOptimalAttackPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Calculate position behind target for ambush
    FVector TargetForward = Target->GetActorForwardVector();
    FVector OptimalPosition = TargetLocation - (TargetForward * 300.0f);
    
    // Add some randomization for pack coordination
    FVector RandomOffset = FVector(
        FMath::RandRange(-200.0f, 200.0f),
        FMath::RandRange(-200.0f, 200.0f),
        0.0f
    );
    
    return OptimalPosition + RandomOffset;
}

bool ACombat_TacticalAI::ShouldFlee() const
{
    // Check health threshold
    if (GetPawn())
    {
        // Simplified health check - in real implementation would check actual health component
        float HealthRatio = 1.0f; // Placeholder
        if (HealthRatio <= TacticalData.FleeHealthThreshold)
            return true;
    }
    
    // Check if overwhelmed by threats
    if (PerceivedThreats.Num() > 3 && !TacticalData.bIsPackLeader)
        return true;
    
    return false;
}

void ACombat_TacticalAI::UpdateTacticalBehavior(float DeltaTime)
{
    LastTacticalUpdate += DeltaTime;
    
    if (LastTacticalUpdate >= TacticalUpdateInterval)
    {
        LastTacticalUpdate = 0.0f;
        
        ProcessPerceptionUpdates();
        HandlePackCoordination();
        EvaluateThreatResponse();
    }
}

void ACombat_TacticalAI::ProcessPerceptionUpdates()
{
    if (!AIPerceptionComponent)
        return;

    PerceivedThreats.Empty();
    PerceivedAllies.Empty();
    
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);
    
    for (AActor* Actor : PerceivedActors)
    {
        if (!Actor || Actor == GetPawn())
            continue;
            
        // Simple threat classification - in real implementation would use proper faction system
        if (Actor->GetName().Contains("Player") || Actor->GetName().Contains("Enemy"))
        {
            PerceivedThreats.Add(Actor);
        }
        else if (Actor->GetName().Contains("Pack") || Actor->GetName().Contains("Ally"))
        {
            PerceivedAllies.Add(Actor);
        }
    }
}

void ACombat_TacticalAI::HandlePackCoordination()
{
    if (!TacticalData.bIsPackLeader || TacticalData.PackMembers.Num() == 0)
        return;

    // Update pack member positions and states
    for (AActor* PackMember : TacticalData.PackMembers)
    {
        if (!PackMember)
            continue;
            
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PackMember->GetActorLocation());
        
        // If pack member is too far, call them back
        if (Distance > TacticalData.PackCoordinationRange * 1.5f)
        {
            if (APawn* PackPawn = Cast<APawn>(PackMember))
            {
                if (ACombat_TacticalAI* PackAI = Cast<ACombat_TacticalAI>(PackPawn->GetController()))
                {
                    if (PackAI->BlackboardComponent)
                    {
                        PackAI->BlackboardComponent->SetValueAsVector(TEXT("PackRallyPoint"), GetPawn()->GetActorLocation());
                    }
                }
            }
        }
    }
}

void ACombat_TacticalAI::EvaluateThreatResponse()
{
    if (PerceivedThreats.Num() == 0)
    {
        if (CurrentTacticalState == ECombat_TacticalState::Attack || 
            CurrentTacticalState == ECombat_TacticalState::Defend)
        {
            SetTacticalState(ECombat_TacticalState::Patrol);
        }
        return;
    }

    // Find highest threat
    AActor* HighestThreat = nullptr;
    float HighestThreatLevel = 0.0f;
    
    for (AActor* Threat : PerceivedThreats)
    {
        float ThreatLevel = CalculateThreatLevel(Threat);
        if (ThreatLevel > HighestThreatLevel)
        {
            HighestThreatLevel = ThreatLevel;
            HighestThreat = Threat;
        }
    }
    
    if (HighestThreat)
    {
        CurrentTarget = HighestThreat;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), HighestThreat);
            BlackboardComponent->SetValueAsFloat(TEXT("ThreatLevel"), HighestThreatLevel);
        }
        
        // Decide on tactical response
        if (ShouldFlee())
        {
            SetTacticalState(ECombat_TacticalState::Flee);
        }
        else if (IsInTerritorialRange(HighestThreat->GetActorLocation()))
        {
            DefendTerritory(HighestThreat);
        }
        else if (TacticalData.bIsPackLeader && TacticalData.PackMembers.Num() > 1)
        {
            CoordinatePackAttack(HighestThreat);
            SetTacticalState(ECombat_TacticalState::PackCoordinate);
        }
        else
        {
            SetTacticalState(ECombat_TacticalState::Attack);
        }
    }
}

void ACombat_TacticalAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // Perception update handled in ProcessPerceptionUpdates during tick
    // This callback can be used for immediate responses to new threats
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
            continue;
            
        // Immediate threat response for high-priority targets
        if (Actor->GetName().Contains("Player"))
        {
            float ThreatLevel = CalculateThreatLevel(Actor);
            if (ThreatLevel > 0.8f && TacticalData.bCanCallForHelp)
            {
                CallForPackHelp(Actor);
            }
        }
    }
}