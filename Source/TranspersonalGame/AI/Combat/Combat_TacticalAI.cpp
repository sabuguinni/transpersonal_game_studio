#include "Combat_TacticalAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombat_TacticalAI::ACombat_TacticalAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Set up AI Perception - Sight
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 2000.0f;
        SightConfig->LoseSightRadius = 2200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Set up AI Perception - Hearing
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1500.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize tactical data
    TacticalData = FCombat_TacticalData();
    CurrentTacticalState = ECombat_TacticalState::Patrol;
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    
    // Initialize timing variables
    LastStateUpdateTime = 0.0f;
    StateUpdateInterval = 0.5f; // Update tactical state every 0.5 seconds
    bIsInCombat = false;
    CombatStartTime = 0.0f;
}

void ACombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAI::OnPerceptionUpdated);
    }

    // Start behavior tree if assigned
    if (BehaviorTree && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTree);
    }

    // Initialize blackboard values
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(CurrentTacticalState));
        BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), TacticalData.AggressionLevel);
        BlackboardComponent->SetValueAsFloat(TEXT("SightRange"), TacticalData.SightRange);
    }
}

void ACombat_TacticalAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update tactical state periodically
    if (CurrentTime - LastStateUpdateTime >= StateUpdateInterval)
    {
        UpdateTacticalState();
        ProcessCombatLogic();
        LastStateUpdateTime = CurrentTime;
    }

    // Debug visualization in development builds
    #if WITH_EDITOR
    if (CurrentTarget)
    {
        DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation(), 
                     FColor::Red, false, 0.1f, 0, 2.0f);
    }
    #endif
}

void ACombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentTacticalState != NewState)
    {
        ECombat_TacticalState PreviousState = CurrentTacticalState;
        CurrentTacticalState = NewState;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(CurrentTacticalState));
        }

        // Handle state transitions
        switch (NewState)
        {
            case ECombat_TacticalState::Engage:
                bIsInCombat = true;
                CombatStartTime = GetWorld()->GetTimeSeconds();
                CommunicateWithAllies();
                break;
            case ECombat_TacticalState::Retreat:
                bIsInCombat = false;
                break;
            case ECombat_TacticalState::Patrol:
                bIsInCombat = false;
                CurrentTarget = nullptr;
                break;
        }

        UE_LOG(LogTemp, Warning, TEXT("TacticalAI %s: State changed from %d to %d"), 
               *GetName(), static_cast<int32>(PreviousState), static_cast<int32>(NewState));
    }
}

void ACombat_TacticalAI::SetTarget(AActor* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        CurrentTarget = NewTarget;
        
        if (CurrentTarget)
        {
            LastKnownTargetLocation = CurrentTarget->GetActorLocation();
            
            // Update blackboard
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
                BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
            }
        }
        else
        {
            // Clear target from blackboard
            if (BlackboardComponent)
            {
                BlackboardComponent->ClearValue(TEXT("TargetActor"));
                BlackboardComponent->ClearValue(TEXT("TargetLocation"));
            }
        }
    }
}

void ACombat_TacticalAI::AddAlliedUnit(AActor* AllyActor)
{
    if (AllyActor && !AlliedUnits.Contains(AllyActor))
    {
        AlliedUnits.Add(AllyActor);
        UE_LOG(LogTemp, Log, TEXT("TacticalAI %s: Added ally %s"), *GetName(), *AllyActor->GetName());
    }
}

void ACombat_TacticalAI::RemoveAlliedUnit(AActor* AllyActor)
{
    if (AlliedUnits.Contains(AllyActor))
    {
        AlliedUnits.Remove(AllyActor);
        UE_LOG(LogTemp, Log, TEXT("TacticalAI %s: Removed ally %s"), *GetName(), *AllyActor->GetName());
    }
}

FVector ACombat_TacticalAI::GetFlankingPosition(AActor* Target, float FlankDistance)
{
    if (!Target || !GetPawn())
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate perpendicular vector for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector).GetSafeNormal();
    
    // Randomize left or right flanking
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    FVector FlankPosition = TargetLocation + (FlankDirection * FlankDistance);
    
    // Ensure the position is on the ground (basic height adjustment)
    FHitResult HitResult;
    FVector TraceStart = FlankPosition + FVector(0, 0, 500);
    FVector TraceEnd = FlankPosition - FVector(0, 0, 500);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        FlankPosition.Z = HitResult.Location.Z + 100.0f; // Add small offset above ground
    }
    
    return FlankPosition;
}

bool ACombat_TacticalAI::ShouldRetreat() const
{
    if (!GetPawn())
    {
        return false;
    }

    // Check health percentage (assuming Character has health)
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        // Basic retreat logic - retreat if outnumbered significantly
        int32 NearbyEnemies = 0;
        int32 NearbyAllies = 0;
        
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
        
        FVector MyLocation = GetPawn()->GetActorLocation();
        float ThreatRadius = 1000.0f;
        
        for (AActor* Actor : NearbyActors)
        {
            if (Actor == GetPawn()) continue;
            
            float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
            if (Distance <= ThreatRadius)
            {
                if (AlliedUnits.Contains(Actor))
                {
                    NearbyAllies++;
                }
                else
                {
                    NearbyEnemies++;
                }
            }
        }
        
        // Retreat if outnumbered 2:1 or more
        return (NearbyEnemies >= 2 && NearbyAllies == 0) || 
               (NearbyEnemies > NearbyAllies * 2);
    }
    
    return false;
}

void ACombat_TacticalAI::ExecuteFormation(ECombat_Formation Formation)
{
    if (AlliedUnits.Num() == 0)
    {
        return;
    }

    // Basic formation implementation
    switch (Formation)
    {
        case ECombat_Formation::Line:
            // Arrange allies in a line formation
            for (int32 i = 0; i < AlliedUnits.Num(); i++)
            {
                if (ACombat_TacticalAI* AllyAI = Cast<ACombat_TacticalAI>(AlliedUnits[i]))
                {
                    FVector FormationPos = GetPawn()->GetActorLocation() + 
                                         FVector(0, (i - AlliedUnits.Num()/2) * 200.0f, 0);
                    // Send formation position to ally (would need custom implementation)
                }
            }
            break;
        case ECombat_Formation::Circle:
            // Arrange allies in a circle around the leader
            for (int32 i = 0; i < AlliedUnits.Num(); i++)
            {
                float Angle = (2.0f * PI * i) / AlliedUnits.Num();
                FVector FormationPos = GetPawn()->GetActorLocation() + 
                                     FVector(FMath::Cos(Angle) * 300.0f, FMath::Sin(Angle) * 300.0f, 0);
                // Send formation position to ally
            }
            break;
    }
}

float ACombat_TacticalAI::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return -1.0f;
    }
    
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool ACombat_TacticalAI::HasLineOfSightToTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    
    FHitResult HitResult;
    FVector Start = GetPawn()->GetActorLocation() + FVector(0, 0, 50); // Eye level
    FVector End = CurrentTarget->GetActorLocation() + FVector(0, 0, 50);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
    
    return !bHit || HitResult.GetActor() == CurrentTarget;
}

void ACombat_TacticalAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor || Actor == GetPawn())
        {
            continue;
        }
        
        // Check if this is a potential target (player or enemy)
        if (Actor->IsA<ACharacter>() && !AlliedUnits.Contains(Actor))
        {
            FActorPerceptionBlueprintInfo PerceptionInfo;
            AIPerceptionComponent->GetActorsPerception(Actor, PerceptionInfo);
            
            if (PerceptionInfo.bIsHostile || CurrentTacticalState == ECombat_TacticalState::Patrol)
            {
                SetTarget(Actor);
                SetTacticalState(ECombat_TacticalState::Investigate);
            }
        }
    }
}

void ACombat_TacticalAI::UpdateTacticalState()
{
    if (!GetPawn())
    {
        return;
    }

    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Patrol:
            // Look for targets
            if (CurrentTarget)
            {
                SetTacticalState(ECombat_TacticalState::Investigate);
            }
            break;
            
        case ECombat_TacticalState::Investigate:
            if (CurrentTarget)
            {
                float DistanceToTarget = GetDistanceToTarget();
                if (DistanceToTarget <= 500.0f && HasLineOfSightToTarget())
                {
                    SetTacticalState(ECombat_TacticalState::Engage);
                }
                else if (DistanceToTarget > TacticalData.SightRange * 1.5f)
                {
                    SetTacticalState(ECombat_TacticalState::Patrol);
                }
            }
            break;
            
        case ECombat_TacticalState::Engage:
            if (ShouldRetreat())
            {
                SetTacticalState(ECombat_TacticalState::Retreat);
            }
            else if (TacticalData.FlankingTendency > 0.5f && FMath::RandFloat() < 0.3f)
            {
                SetTacticalState(ECombat_TacticalState::Flank);
            }
            break;
            
        case ECombat_TacticalState::Flank:
            // Return to engage after flanking maneuver
            if (bIsInCombat && GetWorld()->GetTimeSeconds() - CombatStartTime > 5.0f)
            {
                SetTacticalState(ECombat_TacticalState::Engage);
            }
            break;
            
        case ECombat_TacticalState::Retreat:
            // Check if it's safe to re-engage
            if (!ShouldRetreat() && CurrentTarget && GetDistanceToTarget() > 1000.0f)
            {
                SetTacticalState(ECombat_TacticalState::Investigate);
            }
            break;
    }
}

void ACombat_TacticalAI::ProcessCombatLogic()
{
    if (!CurrentTarget)
    {
        return;
    }

    // Update last known target location
    if (HasLineOfSightToTarget())
    {
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
        }
    }

    // Execute formation if in combat and has allies
    if (bIsInCombat && AlliedUnits.Num() > 0 && TacticalData.PreferredFormation != ECombat_Formation::None)
    {
        ExecuteFormation(TacticalData.PreferredFormation);
    }
}

FVector ACombat_TacticalAI::CalculateOptimalPosition()
{
    if (!CurrentTarget || !GetPawn())
    {
        return GetPawn()->GetActorLocation();
    }

    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    
    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Flank:
            return GetFlankingPosition(CurrentTarget, 400.0f);
            
        case ECombat_TacticalState::Retreat:
            // Move away from target
            return MyLocation + ((MyLocation - TargetLocation).GetSafeNormal() * 1000.0f);
            
        case ECombat_TacticalState::Engage:
            // Move closer to optimal combat range
            return TargetLocation + ((MyLocation - TargetLocation).GetSafeNormal() * 300.0f);
            
        default:
            return MyLocation;
    }
}

void ACombat_TacticalAI::CommunicateWithAllies()
{
    // Basic ally communication system
    for (AActor* Ally : AlliedUnits)
    {
        if (ACombat_TacticalAI* AllyAI = Cast<ACombat_TacticalAI>(Ally))
        {
            if (CurrentTarget && AllyAI->CurrentTarget != CurrentTarget)
            {
                AllyAI->SetTarget(CurrentTarget);
                AllyAI->SetTacticalState(ECombat_TacticalState::Investigate);
            }
        }
    }
}