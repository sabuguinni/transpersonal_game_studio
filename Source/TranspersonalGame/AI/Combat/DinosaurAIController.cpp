#include "DinosaurAIController.h"
#include "CombatAITypes.h"
#include "CombatAIArchitecture.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurAIController::ADinosaurAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Create AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Create Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

    // Create Blackboard Component
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Configure AI Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    // Configure AI Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    // Configure AI Damage Sense
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    AIPerceptionComponent->ConfigureSense(*DamageConfig);

    // Set dominant sense to sight
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Enable ticking
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for AI logic

    // Initialize state
    CurrentCombatState = ECombatState::Passive;
    PreviousCombatState = ECombatState::Passive;
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnTargetPerceptionUpdated);
    }

    // Initialize blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        BlackboardComponent->SetValueAsFloat(TEXT("AlertLevel"), 0.0f);
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), FVector::ZeroVector);
        BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), nullptr);
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurAIController initialized for %s"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
}

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update threat assessment periodically
    if (CurrentTime - LastThreatAssessmentTime >= ThreatAssessmentInterval)
    {
        UpdateThreatAssessment();
        LastThreatAssessmentTime = CurrentTime;
    }

    // Clean up old memories
    CleanupMemory();

    // Update blackboard values
    UpdateBlackboardValues();

    // Handle state timeouts
    float TimeSinceStateChange = CurrentTime - StateChangeTime;
    
    switch (CurrentCombatState)
    {
        case ECombatState::Investigating:
            if (TimeSinceStateChange > AlertStateTimeout && KnownThreats.Num() == 0)
            {
                SetCombatState(ECombatState::Passive);
            }
            break;
            
        case ECombatState::Stalking:
            if (TimeSinceStateChange > AlertStateTimeout * 2.0f && !PrimaryThreat.ThreatActor)
            {
                SetCombatState(ECombatState::Passive);
            }
            break;
            
        case ECombatState::Fleeing:
            if (TimeSinceStateChange > AlertStateTimeout && KnownThreats.Num() == 0)
            {
                SetCombatState(ECombatState::Passive);
            }
            break;
    }
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            AssessThreat(Actor);
            
            // Update memory
            ActorMemory.Add(Actor, GetWorld()->GetTimeSeconds());
            
            // If this is a player, remember their location
            if (Actor->IsA<ACharacter>() && Cast<ACharacter>(Actor)->GetController() && 
                Cast<ACharacter>(Actor)->GetController()->IsA<APlayerController>())
            {
                RememberLocation(Actor->GetActorLocation(), TEXT("LastPlayerLocation"));
                
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), Actor->GetActorLocation());
                    BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), Actor);
                }
            }
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
        // Actor was detected
        AssessThreat(Actor);
        ActorMemory.Add(Actor, GetWorld()->GetTimeSeconds());
        
        UE_LOG(LogTemp, Log, TEXT("%s detected %s"), 
               GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), 
               *Actor->GetName());
    }
    else
    {
        // Actor was lost
        // Remove from known threats if it was there
        for (int32 i = KnownThreats.Num() - 1; i >= 0; i--)
        {
            if (KnownThreats[i].ThreatActor == Actor)
            {
                KnownThreats.RemoveAt(i);
                break;
            }
        }
        
        // If this was our primary threat, find a new one
        if (PrimaryThreat.ThreatActor == Actor)
        {
            PrimaryThreat.ThreatActor = nullptr;
            PrimaryThreat.ThreatLevel = EThreatLevel::None;
            
            // Find new primary threat
            for (const FThreatAssessment& Threat : KnownThreats)
            {
                if (Threat.ThreatLevel > PrimaryThreat.ThreatLevel)
                {
                    PrimaryThreat = Threat;
                }
            }
            
            // If no threats remain, return to passive state
            if (PrimaryThreat.ThreatLevel == EThreatLevel::None)
            {
                SetCombatState(ECombatState::Passive);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("%s lost sight of %s"), 
               GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), 
               *Actor->GetName());
    }
}

void ADinosaurAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState == NewState)
    {
        return;
    }

    PreviousCombatState = CurrentCombatState;
    CurrentCombatState = NewState;
    StateChangeTime = GetWorld()->GetTimeSeconds();

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    }

    // Fire event
    OnCombatStateChanged.Broadcast(NewState);

    UE_LOG(LogTemp, Log, TEXT("%s: Combat state changed from %d to %d"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), 
           (int32)PreviousCombatState, (int32)CurrentCombatState);

    // State-specific logic
    switch (NewState)
    {
        case ECombatState::Passive:
            // Clear current target
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), nullptr);
                BlackboardComponent->SetValueAsFloat(TEXT("AlertLevel"), 0.0f);
            }
            break;
            
        case ECombatState::Investigating:
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsFloat(TEXT("AlertLevel"), 0.3f);
            }
            break;
            
        case ECombatState::Stalking:
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsFloat(TEXT("AlertLevel"), 0.6f);
            }
            break;
            
        case ECombatState::Hunting:
        case ECombatState::Attacking:
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsFloat(TEXT("AlertLevel"), 1.0f);
            }
            StartCombatBehavior();
            break;
            
        case ECombatState::Fleeing:
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsFloat(TEXT("AlertLevel"), 0.8f);
            }
            break;
    }
}

void ADinosaurAIController::AssessThreat(AActor* PotentialThreat)
{
    if (!PotentialThreat || PotentialThreat == GetPawn())
    {
        return;
    }

    EThreatLevel ThreatLevel = CalculateThreatLevel(PotentialThreat);
    
    if (ThreatLevel > EThreatLevel::None)
    {
        // Add or update threat in our list
        bool bFoundExisting = false;
        for (FThreatAssessment& Threat : KnownThreats)
        {
            if (Threat.ThreatActor == PotentialThreat)
            {
                Threat.ThreatLevel = ThreatLevel;
                Threat.LastSeenTime = GetWorld()->GetTimeSeconds();
                bFoundExisting = true;
                break;
            }
        }
        
        if (!bFoundExisting)
        {
            FThreatAssessment NewThreat;
            NewThreat.ThreatActor = PotentialThreat;
            NewThreat.ThreatLevel = ThreatLevel;
            NewThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
            NewThreat.FirstDetectedTime = NewThreat.LastSeenTime;
            KnownThreats.Add(NewThreat);
            
            // Fire threat detected event
            OnThreatDetected.Broadcast(PotentialThreat, ThreatLevel);
        }
        
        // Update primary threat if this is more dangerous
        if (ThreatLevel > PrimaryThreat.ThreatLevel)
        {
            PrimaryThreat.ThreatActor = PotentialThreat;
            PrimaryThreat.ThreatLevel = ThreatLevel;
            PrimaryThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
            
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), PotentialThreat);
            }
        }
        
        // Determine appropriate response based on threat level and current state
        if (CurrentCombatState == ECombatState::Passive)
        {
            if (ThreatLevel >= EThreatLevel::Medium)
            {
                if (ShouldEngageTarget(PotentialThreat))
                {
                    SetCombatState(ECombatState::Stalking);
                }
                else if (ShouldFleeFromTarget(PotentialThreat))
                {
                    SetCombatState(ECombatState::Fleeing);
                }
                else
                {
                    SetCombatState(ECombatState::Investigating);
                }
            }
            else if (ThreatLevel >= EThreatLevel::Low)
            {
                SetCombatState(ECombatState::Investigating);
            }
        }
    }
}

EThreatLevel ADinosaurAIController::CalculateThreatLevel(AActor* Actor) const
{
    if (!Actor || !GetPawn())
    {
        return EThreatLevel::None;
    }

    float ThreatScore = CalculateActorThreatScore(Actor);
    
    // Convert threat score to threat level
    if (ThreatScore >= 0.8f)
        return EThreatLevel::Lethal;
    else if (ThreatScore >= 0.6f)
        return EThreatLevel::Critical;
    else if (ThreatScore >= 0.4f)
        return EThreatLevel::High;
    else if (ThreatScore >= 0.3f)
        return EThreatLevel::Medium;
    else if (ThreatScore >= 0.2f)
        return EThreatLevel::Low;
    else if (ThreatScore >= 0.1f)
        return EThreatLevel::Minimal;
    else
        return EThreatLevel::None;
}

void ADinosaurAIController::StartCombatBehavior()
{
    // This would start the appropriate behavior tree for combat
    // For now, we'll just log the action
    UE_LOG(LogTemp, Log, TEXT("%s: Starting combat behavior"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
    
    // In a full implementation, this would:
    // 1. Load the appropriate behavior tree based on dinosaur type
    // 2. Set up blackboard values for combat
    // 3. Start the behavior tree execution
}

void ADinosaurAIController::StopCombatBehavior()
{
    // Stop combat behavior tree and return to normal behavior
    UE_LOG(LogTemp, Log, TEXT("%s: Stopping combat behavior"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
    
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ADinosaurAIController::RememberLocation(const FVector& Location, const FString& LocationTag)
{
    RememberedLocations.Add(LocationTag, Location);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("%s: Remembered location %s at %s"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), 
           *LocationTag, *Location.ToString());
}

FVector ADinosaurAIController::GetRememberedLocation(const FString& LocationTag) const
{
    if (const FVector* Location = RememberedLocations.Find(LocationTag))
    {
        return *Location;
    }
    return FVector::ZeroVector;
}

bool ADinosaurAIController::ShouldEngageTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }

    // Get combat AI component for personality-based decisions
    UCombatAIComponent* CombatAI = GetPawn()->FindComponentByClass<UCombatAIComponent>();
    if (CombatAI)
    {
        return CombatAI->ShouldEngageTarget(Target);
    }

    // Fallback logic if no combat AI component
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    EThreatLevel ThreatLevel = CalculateThreatLevel(Target);
    
    // Basic engagement logic
    return (ThreatLevel >= EThreatLevel::Medium && Distance < 800.0f);
}

bool ADinosaurAIController::ShouldFleeFromTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }

    // Get combat AI component for personality-based decisions
    UCombatAIComponent* CombatAI = GetPawn()->FindComponentByClass<UCombatAIComponent>();
    if (CombatAI)
    {
        return CombatAI->ShouldRetreat();
    }

    // Fallback logic
    EThreatLevel ThreatLevel = CalculateThreatLevel(Target);
    return (ThreatLevel >= EThreatLevel::Critical);
}

bool ADinosaurAIController::ShouldInvestigateSound(const FVector& SoundLocation, float SoundLoudness) const
{
    if (!GetPawn())
    {
        return false;
    }

    // Get combat AI component for personality-based decisions
    UCombatAIComponent* CombatAI = GetPawn()->FindComponentByClass<UCombatAIComponent>();
    if (CombatAI)
    {
        // Base investigation on curiosity and current state
        float InvestigationChance = CombatAI->Personality.Curiosity;
        
        // Modify based on current combat state
        switch (CurrentCombatState)
        {
            case ECombatState::Passive:
                InvestigationChance += 0.2f;
                break;
            case ECombatState::Investigating:
                InvestigationChance += 0.1f;
                break;
            case ECombatState::Attacking:
            case ECombatState::Fleeing:
                InvestigationChance -= 0.3f;
                break;
        }
        
        // Louder sounds are more likely to be investigated
        InvestigationChance += (SoundLoudness / 100.0f) * 0.3f;
        
        return FMath::RandRange(0.0f, 1.0f) < InvestigationChance;
    }

    // Fallback logic
    return (SoundLoudness > 50.0f && CurrentCombatState == ECombatState::Passive);
}

void ADinosaurAIController::UpdateThreatAssessment()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old or invalid threats
    for (int32 i = KnownThreats.Num() - 1; i >= 0; i--)
    {
        FThreatAssessment& Threat = KnownThreats[i];
        
        // Remove if actor is invalid or too old
        if (!IsValid(Threat.ThreatActor) || 
            (CurrentTime - Threat.LastSeenTime) > MemoryDecayTime)
        {
            if (PrimaryThreat.ThreatActor == Threat.ThreatActor)
            {
                PrimaryThreat.ThreatActor = nullptr;
                PrimaryThreat.ThreatLevel = EThreatLevel::None;
            }
            KnownThreats.RemoveAt(i);
        }
        else
        {
            // Update threat level for existing threats
            EThreatLevel NewThreatLevel = CalculateThreatLevel(Threat.ThreatActor);
            Threat.ThreatLevel = NewThreatLevel;
        }
    }
    
    // Find new primary threat
    PrimaryThreat.ThreatLevel = EThreatLevel::None;
    PrimaryThreat.ThreatActor = nullptr;
    
    for (const FThreatAssessment& Threat : KnownThreats)
    {
        if (Threat.ThreatLevel > PrimaryThreat.ThreatLevel)
        {
            PrimaryThreat = Threat;
        }
    }
}

void ADinosaurAIController::CleanupMemory()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Clean up actor memory
    for (auto It = ActorMemory.CreateIterator(); It; ++It)
    {
        if (!IsValid(It.Key()) || (CurrentTime - It.Value()) > MemoryDecayTime)
        {
            It.RemoveCurrent();
        }
    }
}

void ADinosaurAIController::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Update primary threat
    BlackboardComponent->SetValueAsObject(TEXT("PrimaryThreat"), PrimaryThreat.ThreatActor);
    BlackboardComponent->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(PrimaryThreat.ThreatLevel));
    
    // Update threat count
    BlackboardComponent->SetValueAsInt(TEXT("ThreatCount"), KnownThreats.Num());
    
    // Update last known player location if we have one
    FVector LastPlayerLocation = GetRememberedLocation(TEXT("LastPlayerLocation"));
    if (!LastPlayerLocation.IsZero())
    {
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), LastPlayerLocation);
    }
}

float ADinosaurAIController::CalculateActorThreatScore(AActor* Actor) const
{
    if (!Actor || !GetPawn())
    {
        return 0.0f;
    }

    float ThreatScore = 0.0f;
    
    // Base threat for different actor types
    if (Actor->IsA<ACharacter>())
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character->GetController() && Character->GetController()->IsA<APlayerController>())
        {
            // Players are inherently threatening
            ThreatScore += 0.4f;
        }
        else
        {
            // Other NPCs are less threatening
            ThreatScore += 0.2f;
        }
    }
    
    // Distance factor (closer = more threatening)
    float Distance = FVector::Dist(Actor->GetActorLocation(), GetPawn()->GetActorLocation());
    float MaxThreatDistance = 1500.0f; // Same as sight radius
    float DistanceFactor = 1.0f - FMath::Clamp(Distance / MaxThreatDistance, 0.0f, 1.0f);
    ThreatScore += DistanceFactor * 0.3f;
    
    // Line of sight factor (visible = more threatening)
    if (IsActorInLineOfSight(Actor))
    {
        ThreatScore += 0.2f;
    }
    
    // Movement factor (moving actors are more threatening)
    if (Actor->GetVelocity().Size() > 100.0f)
    {
        ThreatScore += 0.1f;
    }
    
    return FMath::Clamp(ThreatScore, 0.0f, 1.0f);
}

bool ADinosaurAIController::IsActorInLineOfSight(AActor* Actor) const
{
    if (!Actor || !GetPawn() || !AIPerceptionComponent)
    {
        return false;
    }

    FActorPerceptionBlueprintInfo PerceptionInfo;
    bool bHasInfo = AIPerceptionComponent->GetActorsPerception(Actor, PerceptionInfo);
    
    if (bHasInfo)
    {
        for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
        {
            if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && 
                Stimulus.WasSuccessfullySensed())
            {
                return true;
            }
        }
    }
    
    return false;
}