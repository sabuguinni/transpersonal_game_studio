#include "NPCBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UNPC_BehaviorController::UNPC_BehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Memory System Defaults
    MaxShortTermMemories = 20;
    MaxLongTermMemories = 50;
    MemoryDecayRate = 0.1f;

    // Perception Defaults
    SightRadius = 2000.0f;
    HearingRadius = 1500.0f;
    PeripheralVisionAngle = 90.0f;

    // Routine Defaults
    RoutineStartTime = 6.0f; // 6 AM
    RoutineEndTime = 22.0f;  // 10 PM
    CurrentPatrolIndex = 0;

    // Internal State
    LastMemoryUpdate = 0.0f;
    bInEmergencyMode = false;
    CurrentThreat = nullptr;
}

void UNPC_BehaviorController::BeginPlay()
{
    Super::BeginPlay();

    InitializePerception();

    if (DefaultBlackboard)
    {
        UseBlackboard(DefaultBlackboard);
    }

    if (DefaultBehaviorTree)
    {
        RunBehaviorTree(DefaultBehaviorTree);
    }

    // Initialize basic patrol route if none set
    if (PatrolPoints.Num() == 0 && GetPawn())
    {
        FVector PawnLocation = GetPawn()->GetActorLocation();
        PatrolPoints.Add(PawnLocation + FVector(500, 0, 0));
        PatrolPoints.Add(PawnLocation + FVector(0, 500, 0));
        PatrolPoints.Add(PawnLocation + FVector(-500, 0, 0));
        PatrolPoints.Add(PawnLocation + FVector(0, -500, 0));
    }
}

void UNPC_BehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateMemoryDecay(DeltaTime);
    UpdateRoutineBehavior();

    // Process memory consolidation every 5 seconds
    if (GetWorld()->GetTimeSeconds() - LastMemoryUpdate > 5.0f)
    {
        ProcessMemoryConsolidation();
        LastMemoryUpdate = GetWorld()->GetTimeSeconds();
    }
}

void UNPC_BehaviorController::InitializePerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configure Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius + 200.0f;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
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

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &UNPC_BehaviorController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &UNPC_BehaviorController::OnTargetPerceptionUpdated);
}

void UNPC_BehaviorController::AddMemory(FVector Location, ENPC_MemoryType Type, float Importance)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.MemoryType = Type;
    NewMemory.Importance = Importance;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();

    // Add to short-term memory
    ShortTermMemory.Add(NewMemory);

    // Manage memory limits
    if (ShortTermMemory.Num() > MaxShortTermMemories)
    {
        // Move oldest important memories to long-term
        for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
        {
            if (ShortTermMemory[i].Importance > 0.7f && LongTermMemory.Num() < MaxLongTermMemories)
            {
                LongTermMemory.Add(ShortTermMemory[i]);
                ShortTermMemory.RemoveAt(i);
                break;
            }
        }

        // Remove oldest if still over limit
        if (ShortTermMemory.Num() > MaxShortTermMemories)
        {
            ShortTermMemory.RemoveAt(0);
        }
    }
}

void UNPC_BehaviorController::UpdateMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Decay short-term memories
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        float Age = CurrentTime - ShortTermMemory[i].Timestamp;
        ShortTermMemory[i].Importance -= MemoryDecayRate * DeltaTime * Age;

        if (ShortTermMemory[i].Importance <= 0.0f)
        {
            ShortTermMemory.RemoveAt(i);
        }
    }

    // Decay long-term memories more slowly
    for (int32 i = LongTermMemory.Num() - 1; i >= 0; i--)
    {
        float Age = CurrentTime - LongTermMemory[i].Timestamp;
        LongTermMemory[i].Importance -= (MemoryDecayRate * 0.1f) * DeltaTime * Age;

        if (LongTermMemory[i].Importance <= 0.0f)
        {
            LongTermMemory.RemoveAt(i);
        }
    }
}

TArray<FNPC_MemoryEntry> UNPC_BehaviorController::GetMemoriesOfType(ENPC_MemoryType Type)
{
    TArray<FNPC_MemoryEntry> FilteredMemories;

    for (const FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        if (Memory.MemoryType == Type)
        {
            FilteredMemories.Add(Memory);
        }
    }

    for (const FNPC_MemoryEntry& Memory : LongTermMemory)
    {
        if (Memory.MemoryType == Type)
        {
            FilteredMemories.Add(Memory);
        }
    }

    return FilteredMemories;
}

FNPC_MemoryEntry UNPC_BehaviorController::GetNearestMemory(FVector Location, ENPC_MemoryType Type)
{
    FNPC_MemoryEntry NearestMemory;
    float NearestDistance = FLT_MAX;

    TArray<FNPC_MemoryEntry> TypeMemories = GetMemoriesOfType(Type);

    for (const FNPC_MemoryEntry& Memory : TypeMemories)
    {
        float Distance = FVector::Dist(Location, Memory.Location);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestMemory = Memory;
        }
    }

    return NearestMemory;
}

void UNPC_BehaviorController::UpdateSocialRelation(AActor* Target, float TrustDelta, float FearDelta)
{
    if (!Target)
        return;

    // Find existing relation
    FNPC_SocialRelation* ExistingRelation = nullptr;
    for (FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.TargetActor == Target)
        {
            ExistingRelation = &Relation;
            break;
        }
    }

    // Create new relation if none exists
    if (!ExistingRelation)
    {
        FNPC_SocialRelation NewRelation;
        NewRelation.TargetActor = Target;
        NewRelation.TrustLevel = 0.0f;
        NewRelation.FearLevel = 0.0f;
        NewRelation.LastInteractionTime = GetWorld()->GetTimeSeconds();
        SocialRelations.Add(NewRelation);
        ExistingRelation = &SocialRelations.Last();
    }

    // Update relation
    ExistingRelation->TrustLevel = FMath::Clamp(ExistingRelation->TrustLevel + TrustDelta, -1.0f, 1.0f);
    ExistingRelation->FearLevel = FMath::Clamp(ExistingRelation->FearLevel + FearDelta, 0.0f, 1.0f);
    ExistingRelation->LastInteractionTime = GetWorld()->GetTimeSeconds();
}

FNPC_SocialRelation UNPC_BehaviorController::GetSocialRelation(AActor* Target)
{
    for (const FNPC_SocialRelation& Relation : SocialRelations)
    {
        if (Relation.TargetActor == Target)
        {
            return Relation;
        }
    }

    // Return neutral relation if none found
    FNPC_SocialRelation NeutralRelation;
    NeutralRelation.TargetActor = Target;
    return NeutralRelation;
}

bool UNPC_BehaviorController::IsFriendly(AActor* Target)
{
    FNPC_SocialRelation Relation = GetSocialRelation(Target);
    return Relation.TrustLevel > 0.3f && Relation.FearLevel < 0.2f;
}

bool UNPC_BehaviorController::IsHostile(AActor* Target)
{
    FNPC_SocialRelation Relation = GetSocialRelation(Target);
    return Relation.TrustLevel < -0.3f || Relation.FearLevel > 0.7f;
}

void UNPC_BehaviorController::SetPatrolRoute(const TArray<FVector>& NewPatrolPoints)
{
    PatrolPoints = NewPatrolPoints;
    CurrentPatrolIndex = 0;
}

FVector UNPC_BehaviorController::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    }

    FVector NextPoint = PatrolPoints[CurrentPatrolIndex];
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return NextPoint;
}

bool UNPC_BehaviorController::IsWithinRoutineHours()
{
    UWorld* World = GetWorld();
    if (!World)
        return true;

    // Simple time calculation based on world time
    float WorldTime = World->GetTimeSeconds();
    float HourOfDay = FMath::Fmod(WorldTime / 3600.0f, 24.0f); // Convert to hours of day

    return HourOfDay >= RoutineStartTime && HourOfDay <= RoutineEndTime;
}

void UNPC_BehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            // Add memory of seeing this actor
            AddMemory(Actor->GetActorLocation(), ENPC_MemoryType::ActorSighting, 0.5f);

            // Update social relations based on actor type
            if (Actor->IsA<APawn>())
            {
                UpdateSocialRelation(Actor, 0.0f, 0.1f); // Slight increase in awareness
            }
        }
    }
}

void UNPC_BehaviorController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Actor was detected
        AddMemory(Stimulus.StimulusLocation, ENPC_MemoryType::ActorSighting, 0.7f);

        // Check if this is a threat
        if (Actor->IsA<APawn>() && !IsFriendly(Actor))
        {
            CurrentThreat = Actor;
            bInEmergencyMode = true;
            AddMemory(Stimulus.StimulusLocation, ENPC_MemoryType::Danger, 1.0f);
        }
    }
    else
    {
        // Actor was lost
        if (CurrentThreat == Actor)
        {
            CurrentThreat = nullptr;
            bInEmergencyMode = false;
        }
    }
}

void UNPC_BehaviorController::ProcessMemoryConsolidation()
{
    // Move important short-term memories to long-term storage
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        if (ShortTermMemory[i].Importance > 0.8f && LongTermMemory.Num() < MaxLongTermMemories)
        {
            LongTermMemory.Add(ShortTermMemory[i]);
            ShortTermMemory.RemoveAt(i);
        }
    }
}

void UNPC_BehaviorController::UpdateRoutineBehavior()
{
    if (!BlackboardComponent)
        return;

    // Update blackboard with current routine state
    BlackboardComponent->SetValueAsBool(TEXT("IsWithinRoutineHours"), IsWithinRoutineHours());
    BlackboardComponent->SetValueAsBool(TEXT("InEmergencyMode"), bInEmergencyMode);
    BlackboardComponent->SetValueAsObject(TEXT("CurrentThreat"), CurrentThreat);

    if (PatrolPoints.Num() > 0)
    {
        BlackboardComponent->SetValueAsVector(TEXT("NextPatrolPoint"), PatrolPoints[CurrentPatrolIndex]);
    }
}

void UNPC_BehaviorController::HandleEmergencyBehavior()
{
    if (bInEmergencyMode && CurrentThreat)
    {
        // Emergency behavior - flee or hide
        FVector ThreatLocation = CurrentThreat->GetActorLocation();
        FVector MyLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
        FVector FleeDirection = (MyLocation - ThreatLocation).GetSafeNormal();
        FVector FleeTarget = MyLocation + FleeDirection * 2000.0f;

        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("FleeTarget"), FleeTarget);
        }

        AddMemory(ThreatLocation, ENPC_MemoryType::Danger, 1.0f);
    }
}