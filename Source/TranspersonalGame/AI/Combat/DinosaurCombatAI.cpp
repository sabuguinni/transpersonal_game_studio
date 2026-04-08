#include "DinosaurCombatAI.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure Sight Sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Hearing Sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 3000.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configure Damage Sense
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Set default combat profile
    CombatProfile.ThreatLevel = EDinosaurThreatLevel::Cautious;
    CombatProfile.AggressionLevel = 0.5f;
    CombatProfile.FearThreshold = 0.3f;
    CombatProfile.TerritorialRadius = 1000.0f;
    CombatProfile.HuntingRange = 2000.0f;
    CombatProfile.PackCoordinationRange = 500.0f;
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnTargetPerceptionUpdated);

    // Start behavior tree if assigned
    if (CombatBehaviorTree && CombatBlackboard)
    {
        UseBlackboard(CombatBlackboard);
        RunBehaviorTree(CombatBehaviorTree);
        
        // Initialize blackboard values
        UpdateBlackboardValues();
    }

    // Initialize memory
    DinosaurMemory.PlayerTrustLevel = 0.0f;
    DinosaurMemory.LastThreatTime = 0.0f;
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatBehavior(DeltaTime);
    ProcessMemoryDecay(DeltaTime);
    
    if (GetWorld()->GetTimeSeconds() - DinosaurMemory.LastThreatTime > 1.0f)
    {
        EvaluateTacticalOptions();
    }
}

void ADinosaurCombatAI::SetCombatState(EDinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        UpdateBlackboardValues();
        
        // Notify pack members of state change if we're the leader
        if (bIsPackLeader)
        {
            for (ADinosaurCombatAI* PackMember : PackMembers)
            {
                if (PackMember && PackMember != this)
                {
                    // Pack members should coordinate their states
                    if (NewState == EDinosaurCombatState::Attack || NewState == EDinosaurCombatState::Hunt)
                    {
                        PackMember->SetCombatState(EDinosaurCombatState::Pack);
                    }
                }
            }
        }
    }
}

bool ADinosaurCombatAI::CanAttack() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void ADinosaurCombatAI::ExecuteAttack(AActor* Target)
{
    if (!CanAttack() || !Target)
        return;

    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Update memory
    DinosaurMemory.LastAttacker = Target;
    DinosaurMemory.LastThreatTime = LastAttackTime;

    // Dynamic difficulty adjustment
    if (Target == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        AdjustDifficultyBasedOnPlayerPerformance();
    }

    // Coordinate pack attack if applicable
    if (bIsPackLeader && PackMembers.Num() > 1)
    {
        CoordinatePackAttack(Target);
    }

    // Set blackboard target for behavior tree
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsObject(TEXT("AttackTarget"), Target);
        BlackboardComp->SetValueAsBool(TEXT("ShouldAttack"), true);
    }
}

void ADinosaurCombatAI::FleeFromThreat(AActor* Threat)
{
    if (!Threat)
        return;

    SetCombatState(EDinosaurCombatState::Flee);
    
    // Find safe location away from threat
    FVector ThreatLocation = Threat->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector FleeDirection = (MyLocation - ThreatLocation).GetSafeNormal();
    
    FVector SafeLocation = MyLocation + (FleeDirection * 2000.0f);
    
    // Use navigation system to find valid flee location
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(SafeLocation, NavLocation))
        {
            DinosaurMemory.SafeLocation = NavLocation.Location;
            
            if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
            {
                BlackboardComp->SetValueAsVector(TEXT("FleeLocation"), NavLocation.Location);
                BlackboardComp->SetValueAsBool(TEXT("ShouldFlee"), true);
            }
        }
    }

    // Alert pack members
    if (PackMembers.Num() > 0)
    {
        SendPackAlert(Threat, ThreatLocation);
    }
}

FVector ADinosaurCombatAI::FindBestAttackPosition(AActor* Target)
{
    if (!Target)
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Consider flanking if we're in a pack
    if (PackMembers.Num() > 1)
    {
        return GetFlankingPosition(Target);
    }
    
    // For solo attacks, find position with good escape route
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    FVector AttackPosition = TargetLocation - (DirectionToTarget * 300.0f); // Stay at medium range
    
    return AttackPosition;
}

void ADinosaurCombatAI::UpdateThreatAssessment(AActor* PotentialThreat)
{
    if (!PotentialThreat)
        return;

    float ThreatLevel = CalculateThreatLevel(PotentialThreat);
    
    // Update memory
    if (!DinosaurMemory.KnownThreats.Contains(PotentialThreat))
    {
        DinosaurMemory.KnownThreats.Add(PotentialThreat);
    }
    
    // Decide on response based on threat level and our combat profile
    if (ThreatLevel > CombatProfile.FearThreshold && 
        CombatProfile.ThreatLevel != EDinosaurThreatLevel::Apex)
    {
        FleeFromThreat(PotentialThreat);
    }
    else if (ThreatLevel > 0.3f && CombatProfile.AggressionLevel > 0.6f)
    {
        SetCombatState(EDinosaurCombatState::Attack);
        ExecuteAttack(PotentialThreat);
    }
    else
    {
        SetCombatState(EDinosaurCombatState::Investigate);
    }
}

void ADinosaurCombatAI::JoinPack(ADinosaurCombatAI* Leader)
{
    if (!Leader || Leader == this)
        return;

    PackLeader = Leader;
    bIsPackLeader = false;
    
    // Add ourselves to the leader's pack
    if (!Leader->PackMembers.Contains(this))
    {
        Leader->PackMembers.Add(this);
    }
}

void ADinosaurCombatAI::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    // If we were the leader, disband the pack
    if (bIsPackLeader)
    {
        for (ADinosaurCombatAI* Member : PackMembers)
        {
            if (Member)
            {
                Member->PackLeader = nullptr;
                Member->bIsPackLeader = false;
            }
        }
        PackMembers.Empty();
        bIsPackLeader = false;
    }
}

void ADinosaurCombatAI::CoordinatePackAttack(AActor* Target)
{
    if (!bIsPackLeader || !Target)
        return;

    // Assign different roles to pack members
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (PackMembers[i] && PackMembers[i] != this)
        {
            // Stagger attacks for maximum pressure
            float AttackDelay = i * 0.5f;
            
            FTimerHandle TimerHandle;
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUFunction(PackMembers[i], FName("ExecuteAttack"), Target);
            
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, AttackDelay, false);
        }
    }
}

void ADinosaurCombatAI::SendPackAlert(AActor* Threat, FVector ThreatLocation)
{
    for (ADinosaurCombatAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->UpdateThreatAssessment(Threat);
            Member->DinosaurMemory.DangerousLocations.Add(ThreatLocation);
        }
    }
}

void ADinosaurCombatAI::ProcessPlayerInteraction(float InteractionQuality)
{
    if (!CombatProfile.bCanBeDomesticated)
        return;

    // Positive interactions increase trust
    DinosaurMemory.PlayerTrustLevel = FMath::Clamp(
        DinosaurMemory.PlayerTrustLevel + (InteractionQuality * 0.1f),
        0.0f, 1.0f
    );

    // Update aggression based on trust level
    if (DinosaurMemory.PlayerTrustLevel > 0.5f)
    {
        CombatProfile.AggressionLevel *= 0.9f; // Become less aggressive
    }
}

bool ADinosaurCombatAI::CanBeDomesticated() const
{
    return CombatProfile.bCanBeDomesticated && 
           CombatProfile.ThreatLevel <= EDinosaurThreatLevel::Cautious;
}

float ADinosaurCombatAI::GetDomesticationProgress() const
{
    if (!CanBeDomesticated())
        return 0.0f;
        
    return DinosaurMemory.PlayerTrustLevel / CombatProfile.DomesticationDifficulty;
}

void ADinosaurCombatAI::AdjustDifficultyBasedOnPlayerPerformance()
{
    // If player is struggling, make AI slightly less aggressive
    if (PlayerDeathCount > 2)
    {
        CombatProfile.AggressionLevel *= 0.95f;
        AttackCooldown *= 1.1f; // Slightly longer cooldowns
    }
    // If player is doing well, increase challenge
    else if (PlayerDeathCount == 0 && PlayerSkillAssessment > 0.7f)
    {
        CombatProfile.AggressionLevel *= 1.05f;
        AttackCooldown *= 0.95f; // Slightly shorter cooldowns
    }
}

void ADinosaurCombatAI::OnPlayerDeath()
{
    PlayerDeathCount++;
    PlayerSkillAssessment = FMath::Max(0.1f, PlayerSkillAssessment - 0.1f);
}

void ADinosaurCombatAI::OnPlayerSuccessfulEscape()
{
    PlayerSkillAssessment = FMath::Min(1.0f, PlayerSkillAssessment + 0.05f);
}

void ADinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor)
        {
            UpdateThreatAssessment(Actor);
        }
    }
}

void ADinosaurCombatAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Actor detected
        UpdateThreatAssessment(Actor);
    }
    else
    {
        // Lost sight of actor - enter investigate mode
        if (CurrentCombatState == EDinosaurCombatState::Attack)
        {
            SetCombatState(EDinosaurCombatState::Investigate);
        }
    }
}

void ADinosaurCombatAI::UpdateCombatBehavior(float DeltaTime)
{
    // Update blackboard values for behavior tree
    UpdateBlackboardValues();
    
    // Handle state-specific logic
    switch (CurrentCombatState)
    {
        case EDinosaurCombatState::Patrol:
            // Patrol behavior is handled by behavior tree
            break;
            
        case EDinosaurCombatState::Hunt:
            // Increase perception range when hunting
            if (AIPerceptionComponent)
            {
                // Temporarily boost sight range
            }
            break;
            
        case EDinosaurCombatState::Territorial:
            // Check if anyone is in our territory
            // This is handled by EQS queries in behavior tree
            break;
    }
}

void ADinosaurCombatAI::ProcessMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old threats from memory
    DinosaurMemory.KnownThreats.RemoveAll([CurrentTime](AActor* Threat) {
        return !IsValid(Threat);
    });
    
    // Decay dangerous locations over time
    if (CurrentTime - DinosaurMemory.LastThreatTime > 300.0f) // 5 minutes
    {
        DinosaurMemory.DangerousLocations.Empty();
    }
}

void ADinosaurCombatAI::EvaluateTacticalOptions()
{
    // This function runs periodically to reassess the situation
    // and potentially change tactics
    
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));
        
        if (CurrentTarget)
        {
            float DistanceToTarget = FVector::Dist(
                GetPawn()->GetActorLocation(), 
                CurrentTarget->GetActorLocation()
            );
            
            // Decide on best approach based on distance and pack status
            if (DistanceToTarget > CombatProfile.HuntingRange)
            {
                SetCombatState(EDinosaurCombatState::Patrol);
            }
            else if (DistanceToTarget < 500.0f && CanAttack())
            {
                SetCombatState(EDinosaurCombatState::Attack);
            }
            else
            {
                SetCombatState(EDinosaurCombatState::Hunt);
            }
        }
    }
}

float ADinosaurCombatAI::CalculateThreatLevel(AActor* PotentialThreat) const
{
    if (!PotentialThreat)
        return 0.0f;

    float ThreatLevel = 0.0f;
    
    // Base threat for players
    if (PotentialThreat == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        ThreatLevel = 0.6f;
        
        // Adjust based on player's current state/equipment
        // This would be expanded based on player's actual threat level
    }
    
    // Distance factor - closer threats are more dangerous
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PotentialThreat->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / 1000.0f), 0.1f, 1.0f);
    
    return ThreatLevel * DistanceFactor;
}

bool ADinosaurCombatAI::IsInPackFormation() const
{
    if (PackMembers.Num() <= 1)
        return false;
    
    // Check if pack members are within coordination range
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    for (const ADinosaurCombatAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            float Distance = FVector::Dist(MyLocation, Member->GetPawn()->GetActorLocation());
            if (Distance <= CombatProfile.PackCoordinationRange)
            {
                return true;
            }
        }
    }
    
    return false;
}

FVector ADinosaurCombatAI::GetFlankingPosition(AActor* Target) const
{
    if (!Target)
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Find a position to the side of the target
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    
    // Alternate between left and right flanking based on pack member index
    float FlankDirection = (PackMembers.Find(const_cast<ADinosaurCombatAI*>(this)) % 2 == 0) ? 1.0f : -1.0f;
    
    return TargetLocation + (RightVector * FlankDirection * 400.0f);
}

void ADinosaurCombatAI::UpdateBlackboardValues()
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        // Update combat state
        BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        
        // Update threat level
        BlackboardComp->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CombatProfile.ThreatLevel));
        
        // Update pack status
        BlackboardComp->SetValueAsBool(TEXT("IsInPack"), PackMembers.Num() > 1);
        BlackboardComp->SetValueAsBool(TEXT("IsPackLeader"), bIsPackLeader);
        
        // Update memory data
        BlackboardComp->SetValueAsFloat(TEXT("PlayerTrustLevel"), DinosaurMemory.PlayerTrustLevel);
        BlackboardComp->SetValueAsObject(TEXT("LastAttacker"), DinosaurMemory.LastAttacker);
        
        if (DinosaurMemory.SafeLocation != FVector::ZeroVector)
        {
            BlackboardComp->SetValueAsVector(TEXT("SafeLocation"), DinosaurMemory.SafeLocation);
        }
    }
}