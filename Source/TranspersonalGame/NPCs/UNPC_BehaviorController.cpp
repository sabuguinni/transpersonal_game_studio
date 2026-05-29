#include "UNPC_BehaviorController.h"
#include "UNPC_MemoryComponent.h"
#include "ANPC_Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNPC_BehaviorController::UNPC_BehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create behavior tree component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    
    // Create blackboard component
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Create AI perception component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Create memory component
    MemoryComponent = CreateDefaultSubobject<UNPC_MemoryComponent>(TEXT("MemoryComponent"));

    // Initialize default values
    LastRoutineUpdateTime = 0.0f;
    CurrentPatrolIndex = 0;
    bIsInCombat = false;
    bIsInteracting = false;

    // Set default personality traits
    Aggression = 0.5f;
    Curiosity = 0.5f;
    Sociability = 0.5f;
    Fearfulness = 0.5f;
    Intelligence = 0.5f;
    Loyalty = 0.5f;
}

void UNPC_BehaviorController::BeginPlay()
{
    Super::BeginPlay();

    SetupPerception();
    SetupBlackboard();
    InitializePersonality();

    // Start behavior tree if assigned
    if (BehaviorTree)
    {
        StartBehaviorTree();
    }

    // Start daily routine
    if (bFollowsDailyRoutine)
    {
        StartDailyRoutine();
    }
}

void UNPC_BehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update daily routine based on time
    if (bFollowsDailyRoutine)
    {
        UpdateRoutineBasedOnTime();
    }

    // Update blackboard with current state
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), bIsInCombat);
        BlackboardComponent->SetValueAsBool(TEXT("IsInteracting"), bIsInteracting);
        BlackboardComponent->SetValueAsBool(TEXT("PlayerNearby"), IsPlayerNearby());
        
        // Update personality values
        BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), Aggression);
        BlackboardComponent->SetValueAsFloat(TEXT("Curiosity"), Curiosity);
        BlackboardComponent->SetValueAsFloat(TEXT("Sociability"), Sociability);
        BlackboardComponent->SetValueAsFloat(TEXT("Fearfulness"), Fearfulness);
    }
}

void UNPC_BehaviorController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);

    if (ANPC_Character* NPCCharacter = Cast<ANPC_Character>(InPawn))
    {
        // Link NPC character to memory component
        if (MemoryComponent)
        {
            MemoryComponent->SetOwnerNPC(NPCCharacter);
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// BEHAVIOR TREE SYSTEM
// ═══════════════════════════════════════════════════════════════

void UNPC_BehaviorController::StartBehaviorTree()
{
    if (BehaviorTree && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTree);
        UE_LOG(LogTemp, Log, TEXT("NPC Behavior Controller: Started behavior tree"));
    }
}

void UNPC_BehaviorController::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
        UE_LOG(LogTemp, Log, TEXT("NPC Behavior Controller: Stopped behavior tree"));
    }
}

void UNPC_BehaviorController::SetBehaviorTree(UBehaviorTree* NewBehaviorTree)
{
    if (NewBehaviorTree)
    {
        BehaviorTree = NewBehaviorTree;
        
        // Restart with new behavior tree
        if (BehaviorTreeComponent && BehaviorTreeComponent->IsRunning())
        {
            StopBehaviorTree();
            StartBehaviorTree();
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// PERCEPTION CALLBACKS
// ═══════════════════════════════════════════════════════════════

void UNPC_BehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor)
        {
            // Check if it's a player
            if (Actor->IsA<ACharacter>() && Actor->Tags.Contains("Player"))
            {
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsObject(TEXT("PlayerActor"), Actor);
                }
            }
            
            // Check if it's another NPC
            if (ANPC_Character* OtherNPC = Cast<ANPC_Character>(Actor))
            {
                if (!KnownNPCs.Contains(OtherNPC))
                {
                    KnownNPCs.Add(OtherNPC);
                    RelationshipValues.Add(OtherNPC, 0.0f); // Neutral relationship
                }
            }

            // Store in memory
            if (MemoryComponent)
            {
                MemoryComponent->RememberActor(Actor, GetPawn()->GetActorLocation());
            }
        }
    }
}

void UNPC_BehaviorController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Actor was detected
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Actor);
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), Actor->GetActorLocation());
        }

        // React based on personality
        if (Actor->IsA<ACharacter>() && Actor->Tags.Contains("Player"))
        {
            float ReactionIntensity = Curiosity + (1.0f - Fearfulness);
            if (ReactionIntensity > 1.0f)
            {
                // Approach player
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsBool(TEXT("ShouldApproachPlayer"), true);
                }
            }
        }
    }
    else
    {
        // Actor was lost
        if (BlackboardComponent && BlackboardComponent->GetValueAsObject(TEXT("TargetActor")) == Actor)
        {
            BlackboardComponent->ClearValue(TEXT("TargetActor"));
            BlackboardComponent->ClearValue(TEXT("TargetLocation"));
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// SOCIAL INTERACTION FUNCTIONS
// ═══════════════════════════════════════════════════════════════

void UNPC_BehaviorController::InteractWithNPC(ANPC_Character* OtherNPC)
{
    if (!OtherNPC) return;

    bIsInteracting = true;

    // Get current relationship
    float CurrentRelationship = GetRelationshipValue(OtherNPC);
    
    // Modify relationship based on personality compatibility
    if (UNPC_BehaviorController* OtherController = Cast<UNPC_BehaviorController>(OtherNPC->GetController()))
    {
        float CompatibilityScore = 0.0f;
        
        // Calculate compatibility based on personality traits
        CompatibilityScore += (1.0f - FMath::Abs(Sociability - OtherController->Sociability)) * 0.3f;
        CompatibilityScore += (1.0f - FMath::Abs(Aggression - OtherController->Aggression)) * 0.2f;
        CompatibilityScore += (1.0f - FMath::Abs(Intelligence - OtherController->Intelligence)) * 0.2f;
        
        // Modify relationship
        float RelationshipDelta = (CompatibilityScore - 0.5f) * 0.1f;
        ModifyRelationship(OtherNPC, RelationshipDelta);
    }

    // Store interaction in memory
    if (MemoryComponent)
    {
        MemoryComponent->RememberInteraction(OtherNPC, "social_interaction");
    }

    // Reset interaction flag after delay
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { bIsInteracting = false; },
        2.0f,
        false
    );
}

float UNPC_BehaviorController::GetRelationshipValue(ANPC_Character* OtherNPC)
{
    if (RelationshipValues.Contains(OtherNPC))
    {
        return RelationshipValues[OtherNPC];
    }
    return 0.0f; // Neutral
}

void UNPC_BehaviorController::ModifyRelationship(ANPC_Character* OtherNPC, float Delta)
{
    if (!OtherNPC) return;

    float CurrentValue = GetRelationshipValue(OtherNPC);
    float NewValue = FMath::Clamp(CurrentValue + Delta, -1.0f, 1.0f);
    
    RelationshipValues.Add(OtherNPC, NewValue);

    // Update blackboard if this NPC is currently targeted
    if (BlackboardComponent && BlackboardComponent->GetValueAsObject(TEXT("TargetActor")) == OtherNPC)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("TargetRelationship"), NewValue);
    }
}

// ═══════════════════════════════════════════════════════════════
// DAILY ROUTINE FUNCTIONS
// ═══════════════════════════════════════════════════════════════

void UNPC_BehaviorController::StartDailyRoutine()
{
    if (PatrolPoints.Num() > 0)
    {
        CurrentPatrolIndex = 0;
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolPoints[CurrentPatrolIndex]);
        }
    }
}

void UNPC_BehaviorController::UpdateRoutineBasedOnTime()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update routine every 5 seconds
    if (CurrentTime - LastRoutineUpdateTime > 5.0f)
    {
        LastRoutineUpdateTime = CurrentTime;
        
        // Get time of day (simplified - would normally use day/night cycle system)
        float TimeOfDay = FMath::Fmod(CurrentTime / 60.0f, 24.0f); // Simplified time calculation
        
        // Check if within active routine hours
        bool bInActiveHours = (TimeOfDay >= RoutineStartTime && TimeOfDay <= RoutineEndTime);
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsBool(TEXT("InActiveHours"), bInActiveHours);
            BlackboardComponent->SetValueAsFloat(TEXT("TimeOfDay"), TimeOfDay);
        }
        
        // Update patrol target if needed
        if (bInActiveHours && PatrolPoints.Num() > 0)
        {
            FVector CurrentTarget = GetCurrentRoutineTarget();
            if (GetPawn() && FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget) < 200.0f)
            {
                // Move to next patrol point
                CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolPoints[CurrentPatrolIndex]);
                }
            }
        }
    }
}

FVector UNPC_BehaviorController::GetCurrentRoutineTarget()
{
    if (PatrolPoints.Num() > 0 && CurrentPatrolIndex < PatrolPoints.Num())
    {
        return PatrolPoints[CurrentPatrolIndex];
    }
    return FVector::ZeroVector;
}

// ═══════════════════════════════════════════════════════════════
// UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════

bool UNPC_BehaviorController::IsPlayerNearby(float Radius)
{
    if (!GetPawn()) return false;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
        return Distance <= Radius;
    }
    return false;
}

AActor* UNPC_BehaviorController::GetNearestThreat()
{
    // This would be expanded to check for actual threats
    // For now, return player if they're hostile
    if (IsPlayerNearby(1000.0f))
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn && Aggression > 0.7f)
        {
            return PlayerPawn;
        }
    }
    return nullptr;
}

void UNPC_BehaviorController::SetPersonalityTrait(const FString& TraitName, float Value)
{
    Value = FMath::Clamp(Value, 0.0f, 1.0f);
    
    if (TraitName == "Aggression")
    {
        Aggression = Value;
    }
    else if (TraitName == "Curiosity")
    {
        Curiosity = Value;
    }
    else if (TraitName == "Sociability")
    {
        Sociability = Value;
    }
    else if (TraitName == "Fearfulness")
    {
        Fearfulness = Value;
    }
    else if (TraitName == "Intelligence")
    {
        Intelligence = Value;
    }
    else if (TraitName == "Loyalty")
    {
        Loyalty = Value;
    }
}

// ═══════════════════════════════════════════════════════════════
// PRIVATE SETUP FUNCTIONS
// ═══════════════════════════════════════════════════════════════

void UNPC_BehaviorController::SetupPerception()
{
    if (!AIPerceptionComponent) return;

    // Setup sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Setup hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set sight as dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &UNPC_BehaviorController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &UNPC_BehaviorController::OnTargetPerceptionUpdated);
}

void UNPC_BehaviorController::SetupBlackboard()
{
    if (BlackboardComponent && BehaviorTree && BehaviorTree->BlackboardAsset)
    {
        BlackboardComponent->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
    }
}

void UNPC_BehaviorController::InitializePersonality()
{
    // Randomize personality traits slightly for variety
    float Variance = 0.1f;
    
    Aggression = FMath::Clamp(Aggression + FMath::RandRange(-Variance, Variance), 0.0f, 1.0f);
    Curiosity = FMath::Clamp(Curiosity + FMath::RandRange(-Variance, Variance), 0.0f, 1.0f);
    Sociability = FMath::Clamp(Sociability + FMath::RandRange(-Variance, Variance), 0.0f, 1.0f);
    Fearfulness = FMath::Clamp(Fearfulness + FMath::RandRange(-Variance, Variance), 0.0f, 1.0f);
    Intelligence = FMath::Clamp(Intelligence + FMath::RandRange(-Variance, Variance), 0.0f, 1.0f);
    Loyalty = FMath::Clamp(Loyalty + FMath::RandRange(-Variance, Variance), 0.0f, 1.0f);
}