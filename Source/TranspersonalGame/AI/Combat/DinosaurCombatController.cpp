#include "DinosaurCombatController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ADinosaurCombatController::ADinosaurCombatController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure AI Perception
    if (AIPerceptionComponent)
    {
        // Sight configuration
        UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
        if (SightConfig)
        {
            SightConfig->SightRadius = 2000.0f;
            SightConfig->LoseSightRadius = 2500.0f;
            SightConfig->PeripheralVisionAngleDegrees = 120.0f;
            SightConfig->SetMaxAge(5.0f);
            SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
            SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
            SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
            SightConfig->DetectionByAffiliation.bDetectEnemies = true;

            AIPerceptionComponent->ConfigureSense(*SightConfig);
        }

        // Hearing configuration
        UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
        if (HearingConfig)
        {
            HearingConfig->HearingRange = 1500.0f;
            HearingConfig->SetMaxAge(3.0f);
            HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
            HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
            HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

            AIPerceptionComponent->ConfigureSense(*HearingConfig);
        }

        // Damage configuration
        UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
        if (DamageConfig)
        {
            DamageConfig->SetMaxAge(10.0f);
            AIPerceptionComponent->ConfigureSense(*DamageConfig);
        }

        // Set sight as dominant sense
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // Initialize state
    CurrentCombatState = ECombatState::Idle;
    CurrentAttackPattern = EAttackPattern::Cautious;
    TimeSinceLastDecision = 0.0f;
    AttackCooldown = 0.0f;
    LastPatrolLocation = FVector::ZeroVector;

    // Initialize combat memory
    CombatMemory.AlertLevel = 0.0f;
    CombatMemory.LastKnownPlayerLocation = FVector::ZeroVector;
    CombatMemory.TimeSinceLastPlayerSighting = 0.0f;
    CombatMemory.DamageTaken = 0.0f;
    CombatMemory.DamageDealtToPlayer = 0.0f;
    CombatMemory.AttacksReceived = 0;
    CombatMemory.AttacksLanded = 0;
    CombatMemory.TimeInCombat = 0.0f;
    CombatMemory.PreferredAttackRange = 300.0f;
    CombatMemory.RetreatThreshold = 0.3f;
}

void ADinosaurCombatController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatController::OnTargetPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ADinosaurCombatController::OnTargetPerceptionForgotten);
    }

    // Generate individual personality
    GenerateIndividualPersonality();

    // Set up alert decay timer
    GetWorld()->GetTimerManager().SetTimer(AlertDecayTimer, this, &ADinosaurCombatController::DecayAlertLevel, 1.0f, true);
}

void ADinosaurCombatController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatMemory(DeltaTime);
    UpdateBehaviorTreeKeys();
    ProcessPerceptionData();

    // Update decision timing
    TimeSinceLastDecision += DeltaTime;
    
    // Update attack cooldown
    if (AttackCooldown > 0.0f)
    {
        AttackCooldown -= DeltaTime;
    }

    // Debug drawing
    if (CVarAIDebugDraw.GetValueOnGameThread())
    {
        UWorld* World = GetWorld();
        APawn* ControlledPawn = GetPawn();
        if (World && ControlledPawn)
        {
            FVector PawnLocation = ControlledPawn->GetActorLocation();
            
            // Draw combat state
            FString StateText = FString::Printf(TEXT("State: %s\nAlert: %.1f\nPattern: %s"), 
                *UEnum::GetValueAsString(CurrentCombatState),
                CombatMemory.AlertLevel,
                *UEnum::GetValueAsString(CurrentAttackPattern));
            
            DrawDebugString(World, PawnLocation + FVector(0, 0, 150), StateText, nullptr, FColor::Yellow, 0.0f);
            
            // Draw perception radius
            DrawDebugCircle(World, PawnLocation, 2000.0f, 32, FColor::Blue, false, 0.1f, 0, 2.0f, FVector(0, 0, 1));
        }
    }
}

void ADinosaurCombatController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Start behavior tree if available
    if (CombatBehaviorTree && BehaviorTreeComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }

    // Initialize last patrol location
    if (InPawn)
    {
        LastPatrolLocation = InPawn->GetActorLocation();
    }
}

void ADinosaurCombatController::InitializeCombatAI(UCombatSpeciesData* InSpeciesData)
{
    SpeciesData = InSpeciesData;
    
    if (SpeciesData)
    {
        // Apply species-specific settings
        CombatMemory.PreferredAttackRange = SpeciesData->BaseAttackRange;
        CombatMemory.RetreatThreshold = SpeciesData->BaseRetreatThreshold;
        
        UE_LOG(LogTemp, Log, TEXT("Combat AI initialized for species: %s"), 
            SpeciesData ? *SpeciesData->GetName() : TEXT("Unknown"));
    }
}

void ADinosaurCombatController::GenerateIndividualPersonality()
{
    // Generate procedural personality traits
    IndividualPersonality.Aggression = FMath::RandRange(0.2f, 1.0f);
    IndividualPersonality.Caution = FMath::RandRange(0.1f, 0.9f);
    IndividualPersonality.Curiosity = FMath::RandRange(0.3f, 0.8f);
    IndividualPersonality.SocialTendency = FMath::RandRange(0.4f, 1.0f);
    IndividualPersonality.TerritorialBehavior = FMath::RandRange(0.2f, 0.9f);
    IndividualPersonality.FearResponse = FMath::RandRange(0.1f, 0.7f);
    IndividualPersonality.LearningRate = FMath::RandRange(0.5f, 1.0f);
    IndividualPersonality.AdaptabilityFactor = FMath::RandRange(0.6f, 1.0f);

    // Ensure some balance in personality
    if (IndividualPersonality.Aggression > 0.8f && IndividualPersonality.Caution > 0.8f)
    {
        IndividualPersonality.Caution *= 0.6f; // Reduce caution if very aggressive
    }

    UE_LOG(LogTemp, Log, TEXT("Generated personality - Aggression: %.2f, Caution: %.2f, Social: %.2f"), 
        IndividualPersonality.Aggression, IndividualPersonality.Caution, IndividualPersonality.SocialTendency);
}

void ADinosaurCombatController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    // Add to perceived targets if not already present
    PerceivedTargets.AddUnique(Actor);

    // Update alert level based on stimulus
    if (Stimulus.WasSuccessfullySensed())
    {
        float AlertIncrease = 0.3f;
        
        // Increase alert more for closer targets
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
        if (Distance < 500.0f)
        {
            AlertIncrease *= 2.0f;
        }

        UpdateAlertLevel(AlertIncrease);

        // Update last known player location if this is the player
        if (Actor->IsA<ACharacter>() && Cast<ACharacter>(Actor)->IsPlayerControlled())
        {
            CombatMemory.LastKnownPlayerLocation = Actor->GetActorLocation();
            CombatMemory.TimeSinceLastPlayerSighting = 0.0f;

            // Transition to alert or combat state
            if (CurrentCombatState == ECombatState::Idle)
            {
                SetCombatState(ECombatState::Alert);
            }
        }

        UE_LOG(LogTemp, Log, TEXT("Target perceived: %s, Alert level: %.2f"), 
            *Actor->GetName(), CombatMemory.AlertLevel);
    }
}

void ADinosaurCombatController::OnTargetPerceptionForgotten(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    PerceivedTargets.Remove(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("Target forgotten: %s"), *Actor->GetName());
}

void ADinosaurCombatController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;

        // Handle state transitions
        switch (NewState)
        {
            case ECombatState::Alert:
                UE_LOG(LogTemp, Log, TEXT("Entering Alert state"));
                break;
                
            case ECombatState::Hunting:
                UE_LOG(LogTemp, Log, TEXT("Entering Hunting state"));
                CombatMemory.TimeInCombat = 0.0f;
                break;
                
            case ECombatState::Combat:
                UE_LOG(LogTemp, Log, TEXT("Entering Combat state"));
                CombatMemory.TimeInCombat = 0.0f;
                break;
                
            case ECombatState::Fleeing:
                UE_LOG(LogTemp, Log, TEXT("Entering Fleeing state"));
                break;
                
            case ECombatState::Idle:
                UE_LOG(LogTemp, Log, TEXT("Returning to Idle state"));
                CombatMemory.AlertLevel *= 0.5f; // Reduce alert when returning to idle
                break;
        }

        // Notify nearby allies of state change
        if (NewState == ECombatState::Combat || NewState == ECombatState::Alert)
        {
            NotifyNearbyAllies(CombatMemory.LastKnownPlayerLocation);
        }
    }
}

void ADinosaurCombatController::UpdateAlertLevel(float DeltaAlert)
{
    CombatMemory.AlertLevel = FMath::Clamp(CombatMemory.AlertLevel + DeltaAlert, 0.0f, 1.0f);
    
    // Update combat state based on alert level
    if (CombatMemory.AlertLevel > 0.8f && CurrentCombatState != ECombatState::Combat)
    {
        SetCombatState(ECombatState::Combat);
    }
    else if (CombatMemory.AlertLevel > 0.4f && CurrentCombatState == ECombatState::Idle)
    {
        SetCombatState(ECombatState::Alert);
    }
    else if (CombatMemory.AlertLevel < 0.2f && CurrentCombatState != ECombatState::Idle)
    {
        SetCombatState(ECombatState::Idle);
    }
}

void ADinosaurCombatController::RegisterDamage(float Damage, AActor* DamageSource)
{
    CombatMemory.DamageTaken += Damage;
    CombatMemory.AttacksReceived++;
    
    // Increase alert significantly when taking damage
    UpdateAlertLevel(0.5f);
    
    // Enter combat state immediately
    SetCombatState(ECombatState::Combat);
    
    // Update last known player location if damage came from player
    if (DamageSource && DamageSource->IsA<ACharacter>() && Cast<ACharacter>(DamageSource)->IsPlayerControlled())
    {
        CombatMemory.LastKnownPlayerLocation = DamageSource->GetActorLocation();
        CombatMemory.TimeSinceLastPlayerSighting = 0.0f;
    }

    UE_LOG(LogTemp, Log, TEXT("Damage registered: %.1f from %s. Total damage: %.1f"), 
        Damage, DamageSource ? *DamageSource->GetName() : TEXT("Unknown"), CombatMemory.DamageTaken);
}

EAttackPattern ADinosaurCombatController::SelectAttackPattern()
{
    // Consider personality and current situation
    float AggressionFactor = IndividualPersonality.Aggression;
    float CautionFactor = IndividualPersonality.Caution;
    float HealthPercentage = 1.0f; // Would get from health component in real implementation
    
    // Modify based on damage taken
    if (CombatMemory.DamageTaken > 50.0f)
    {
        CautionFactor += 0.3f;
        AggressionFactor -= 0.2f;
    }
    
    // Select pattern based on factors
    if (AggressionFactor > 0.8f && CautionFactor < 0.4f)
    {
        CurrentAttackPattern = EAttackPattern::Aggressive;
    }
    else if (CautionFactor > 0.7f)
    {
        CurrentAttackPattern = EAttackPattern::Cautious;
    }
    else if (IndividualPersonality.SocialTendency > 0.6f)
    {
        CurrentAttackPattern = EAttackPattern::Coordinated;
    }
    else
    {
        CurrentAttackPattern = EAttackPattern::Opportunistic;
    }
    
    TimeSinceLastDecision = 0.0f;
    return CurrentAttackPattern;
}

bool ADinosaurCombatController::ShouldEngageTarget(AActor* Target)
{
    if (!Target)
    {
        return false;
    }

    // Check distance
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    if (Distance > 3000.0f) // Too far
    {
        return false;
    }

    // Check health and damage taken
    float DamageRatio = CombatMemory.DamageTaken / 100.0f; // Normalize to expected max health
    if (DamageRatio > CombatMemory.RetreatThreshold && IndividualPersonality.FearResponse > 0.5f)
    {
        return false; // Too damaged and fearful
    }

    // Check personality factors
    float EngagementScore = IndividualPersonality.Aggression - (IndividualPersonality.Caution * DamageRatio);
    
    return EngagementScore > 0.3f;
}

bool ADinosaurCombatController::ShouldRetreat()
{
    // Check damage threshold
    float DamageRatio = CombatMemory.DamageTaken / 100.0f;
    if (DamageRatio > CombatMemory.RetreatThreshold)
    {
        return true;
    }

    // Check fear response
    if (IndividualPersonality.FearResponse > 0.6f && CombatMemory.AlertLevel > 0.8f && CombatMemory.AttacksReceived > 3)
    {
        return true;
    }

    // Check if outnumbered (simplified)
    if (PerceivedTargets.Num() > 2 && IndividualPersonality.Caution > 0.7f)
    {
        return true;
    }

    return false;
}

FVector ADinosaurCombatController::FindOptimalCombatPosition()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return FVector::ZeroVector;
    }

    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FVector OptimalPosition = CurrentLocation;

    // If we have a target, position relative to it
    if (!CombatMemory.LastKnownPlayerLocation.IsZero())
    {
        FVector ToTarget = CombatMemory.LastKnownPlayerLocation - CurrentLocation;
        ToTarget.Normalize();

        // Position at preferred attack range
        OptimalPosition = CombatMemory.LastKnownPlayerLocation - (ToTarget * CombatMemory.PreferredAttackRange);

        // Adjust based on personality
        if (IndividualPersonality.Caution > 0.6f)
        {
            // More cautious - stay further back
            OptimalPosition = CombatMemory.LastKnownPlayerLocation - (ToTarget * CombatMemory.PreferredAttackRange * 1.5f);
        }
        else if (IndividualPersonality.Aggression > 0.8f)
        {
            // More aggressive - get closer
            OptimalPosition = CombatMemory.LastKnownPlayerLocation - (ToTarget * CombatMemory.PreferredAttackRange * 0.7f);
        }
    }

    return OptimalPosition;
}

void ADinosaurCombatController::NotifyNearbyAllies(const FVector& ThreatLocation)
{
    TArray<ADinosaurCombatController*> NearbyAllies = GetNearbyAllies(1500.0f);
    
    for (ADinosaurCombatController* Ally : NearbyAllies)
    {
        if (Ally && Ally != this)
        {
            // Simple notification - in real implementation this would be more sophisticated
            Ally->UpdateAlertLevel(0.2f);
            Ally->CombatMemory.LastKnownPlayerLocation = ThreatLocation;
            Ally->CombatMemory.TimeSinceLastPlayerSighting = 0.0f;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Notified %d allies of threat at location %s"), 
        NearbyAllies.Num(), *ThreatLocation.ToString());
}

TArray<ADinosaurCombatController*> ADinosaurCombatController::GetNearbyAllies(float Radius)
{
    TArray<ADinosaurCombatController*> NearbyAllies;
    
    UWorld* World = GetWorld();
    APawn* ControlledPawn = GetPawn();
    if (!World || !ControlledPawn)
    {
        return NearbyAllies;
    }

    FVector MyLocation = ControlledPawn->GetActorLocation();
    
    // Find all AI controllers in range
    for (TActorIterator<ADinosaurCombatController> ActorItr(World); ActorItr; ++ActorItr)
    {
        ADinosaurCombatController* OtherController = *ActorItr;
        if (OtherController && OtherController != this && OtherController->GetPawn())
        {
            float Distance = FVector::Dist(MyLocation, OtherController->GetPawn()->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyAllies.Add(OtherController);
            }
        }
    }

    return NearbyAllies;
}

bool ADinosaurCombatController::IsHostileToPlayer() const
{
    // Determine hostility based on species data and current state
    if (SpeciesData)
    {
        // Would check species-specific hostility settings
        return true; // Placeholder - most dinosaurs are hostile
    }

    // Default based on combat state and alert level
    return CurrentCombatState == ECombatState::Combat || 
           (CurrentCombatState == ECombatState::Alert && CombatMemory.AlertLevel > 0.6f);
}

void ADinosaurCombatController::UpdateCombatMemory(float DeltaTime)
{
    // Update timers
    CombatMemory.TimeSinceLastPlayerSighting += DeltaTime;
    
    if (CurrentCombatState == ECombatState::Combat)
    {
        CombatMemory.TimeInCombat += DeltaTime;
    }

    // Decay some memory values over time
    if (CombatMemory.TimeSinceLastPlayerSighting > 10.0f)
    {
        // Gradually forget player location
        CombatMemory.LastKnownPlayerLocation = FVector::ZeroVector;
    }
}

void ADinosaurCombatController::UpdateBehaviorTreeKeys()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Update blackboard with current state
    BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    BlackboardComponent->SetValueAsEnum(TEXT("AttackPattern"), static_cast<uint8>(CurrentAttackPattern));
    BlackboardComponent->SetValueAsFloat(TEXT("AlertLevel"), CombatMemory.AlertLevel);
    
    if (!CombatMemory.LastKnownPlayerLocation.IsZero())
    {
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), CombatMemory.LastKnownPlayerLocation);
    }

    // Set target if we have perceived targets
    if (PerceivedTargets.Num() > 0)
    {
        BlackboardComponent->SetValueAsObject(TEXT("Target"), PerceivedTargets[0]);
    }
    else
    {
        BlackboardComponent->ClearValue(TEXT("Target"));
    }
}

void ADinosaurCombatController::ProcessPerceptionData()
{
    // Remove invalid targets
    PerceivedTargets.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });

    // Sort targets by priority (distance, threat level, etc.)
    if (PerceivedTargets.Num() > 1)
    {
        APawn* ControlledPawn = GetPawn();
        if (ControlledPawn)
        {
            FVector MyLocation = ControlledPawn->GetActorLocation();
            
            PerceivedTargets.Sort([MyLocation](const AActor& A, const AActor& B) {
                float DistA = FVector::DistSquared(MyLocation, A.GetActorLocation());
                float DistB = FVector::DistSquared(MyLocation, B.GetActorLocation());
                return DistA < DistB; // Closer targets first
            });
        }
    }
}

void ADinosaurCombatController::DecayAlertLevel()
{
    // Gradually reduce alert level when not in active combat
    if (CurrentCombatState != ECombatState::Combat)
    {
        float DecayRate = 0.1f;
        
        // Decay faster if no targets are perceived
        if (PerceivedTargets.Num() == 0)
        {
            DecayRate = 0.2f;
        }

        UpdateAlertLevel(-DecayRate);
    }
}