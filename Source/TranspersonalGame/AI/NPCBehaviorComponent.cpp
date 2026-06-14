#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize default values
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentEmotionalState = ENPC_EmotionalState::Calm;
    AlertnessLevel = 0.3f;
    EnergyLevel = 1.0f;
    SocialNeed = 0.5f;

    // Memory system defaults
    MaxShortTermMemories = 10;
    MaxLongTermMemories = 50;
    LastMemoryConsolidationTime = 0.0f;

    // Social system defaults
    SocialInteractionRadius = 500.0f;
    LastSocialUpdate = 0.0f;

    // Patrol system defaults
    CurrentPatrolIndex = 0;
    PatrolSpeed = 200.0f;
    PatrolWaitTime = 3.0f;
    PatrolTimer = 0.0f;

    // AI components
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize AI components
    InitializeBehaviorTree();

    // Set initial patrol points if none provided
    if (PatrolPoints.Num() == 0)
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        PatrolPoints.Add(OwnerLocation + FVector(500, 0, 0));
        PatrolPoints.Add(OwnerLocation + FVector(0, 500, 0));
        PatrolPoints.Add(OwnerLocation + FVector(-500, 0, 0));
        PatrolPoints.Add(OwnerLocation + FVector(0, -500, 0));
    }

    // Add initial memory of spawn location
    AddMemory(GetOwner()->GetActorLocation(), ENPC_EmotionalState::Calm, TEXT("Spawn location"));
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update core systems
    UpdateEnergyAndNeeds(DeltaTime);
    ProcessSocialInteractions(DeltaTime);
    UpdatePatrolBehavior(DeltaTime);
    HandleEmotionalDecay(DeltaTime);

    // Periodic memory consolidation
    if (GetWorld()->GetTimeSeconds() - LastMemoryConsolidationTime > 30.0f)
    {
        ConsolidateMemories();
        LastMemoryConsolidationTime = GetWorld()->GetTimeSeconds();
    }

    // Update AI blackboard
    UpdateBlackboardValues();
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        ENPC_BehaviorState OldState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;

        // Add memory of state change
        FString StateChangeDesc = FString::Printf(TEXT("Changed from %d to %d"), (int32)OldState, (int32)NewState);
        AddMemory(GetOwner()->GetActorLocation(), CurrentEmotionalState, StateChangeDesc);

        // Update blackboard
        UpdateBlackboardValues();
    }
}

void UNPCBehaviorComponent::SetEmotionalState(ENPC_EmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        CurrentEmotionalState = NewState;
        
        // Emotional states affect alertness
        switch (NewState)
        {
            case ENPC_EmotionalState::Afraid:
                AlertnessLevel = FMath::Clamp(AlertnessLevel + 0.3f, 0.0f, 1.0f);
                break;
            case ENPC_EmotionalState::Angry:
                AlertnessLevel = FMath::Clamp(AlertnessLevel + 0.2f, 0.0f, 1.0f);
                break;
            case ENPC_EmotionalState::Happy:
                AlertnessLevel = FMath::Clamp(AlertnessLevel - 0.1f, 0.0f, 1.0f);
                break;
            case ENPC_EmotionalState::Tired:
                EnergyLevel = FMath::Clamp(EnergyLevel - 0.2f, 0.0f, 1.0f);
                break;
        }
    }
}

void UNPCBehaviorComponent::UpdateAlertnessLevel(float DeltaAlertness)
{
    AlertnessLevel = FMath::Clamp(AlertnessLevel + DeltaAlertness, 0.0f, 1.0f);
    
    // High alertness triggers alert behavior
    if (AlertnessLevel > 0.8f && CurrentBehaviorState != ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
        SetEmotionalState(ENPC_EmotionalState::Afraid);
    }
}

void UNPCBehaviorComponent::AddMemory(FVector Location, ENPC_EmotionalState Emotion, const FString& Description)
{
    FNPC_Memory NewMemory;
    NewMemory.Location = Location;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    NewMemory.EmotionalContext = Emotion;
    NewMemory.EventDescription = Description;

    ShortTermMemory.Add(NewMemory);

    // Limit short-term memory size
    if (ShortTermMemory.Num() > MaxShortTermMemories)
    {
        ShortTermMemory.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::ConsolidateMemories()
{
    // Move important short-term memories to long-term
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        const FNPC_Memory& Memory = ShortTermMemory[i];
        
        // Consolidate memories with high emotional impact
        if (Memory.EmotionalContext == ENPC_EmotionalState::Afraid || 
            Memory.EmotionalContext == ENPC_EmotionalState::Angry ||
            Memory.EmotionalContext == ENPC_EmotionalState::Happy)
        {
            LongTermMemory.Add(Memory);
            ShortTermMemory.RemoveAt(i);
        }
    }

    // Limit long-term memory size
    if (LongTermMemory.Num() > MaxLongTermMemories)
    {
        LongTermMemory.RemoveAt(0);
    }
}

TArray<FNPC_Memory> UNPCBehaviorComponent::GetMemoriesAtLocation(FVector Location, float Radius)
{
    TArray<FNPC_Memory> NearbyMemories;
    
    // Check both short-term and long-term memories
    for (const FNPC_Memory& Memory : ShortTermMemory)
    {
        if (FVector::Dist(Memory.Location, Location) <= Radius)
        {
            NearbyMemories.Add(Memory);
        }
    }
    
    for (const FNPC_Memory& Memory : LongTermMemory)
    {
        if (FVector::Dist(Memory.Location, Location) <= Radius)
        {
            NearbyMemories.Add(Memory);
        }
    }
    
    return NearbyMemories;
}

void UNPCBehaviorComponent::UpdateSocialRelationship(APawn* TargetPawn, float TrustDelta, float FearDelta)
{
    if (!TargetPawn) return;

    // Find existing relationship
    FNPC_SocialRelationship* ExistingRelationship = nullptr;
    for (FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.TargetPawn == TargetPawn)
        {
            ExistingRelationship = &Relationship;
            break;
        }
    }

    // Create new relationship if none exists
    if (!ExistingRelationship)
    {
        FNPC_SocialRelationship NewRelationship;
        NewRelationship.TargetPawn = TargetPawn;
        NewRelationship.TrustLevel = 0.5f;
        NewRelationship.FearLevel = 0.0f;
        NewRelationship.LastInteractionTime = GetWorld()->GetTimeSeconds();
        SocialRelationships.Add(NewRelationship);
        ExistingRelationship = &SocialRelationships.Last();
    }

    // Update relationship values
    ExistingRelationship->TrustLevel = FMath::Clamp(ExistingRelationship->TrustLevel + TrustDelta, 0.0f, 1.0f);
    ExistingRelationship->FearLevel = FMath::Clamp(ExistingRelationship->FearLevel + FearDelta, 0.0f, 1.0f);
    ExistingRelationship->LastInteractionTime = GetWorld()->GetTimeSeconds();

    // Add memory of social interaction
    FString InteractionDesc = FString::Printf(TEXT("Interacted with %s"), *TargetPawn->GetName());
    AddMemory(TargetPawn->GetActorLocation(), CurrentEmotionalState, InteractionDesc);
}

FNPC_SocialRelationship UNPCBehaviorComponent::GetSocialRelationship(APawn* TargetPawn)
{
    for (const FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.TargetPawn == TargetPawn)
        {
            return Relationship;
        }
    }
    
    // Return default relationship
    FNPC_SocialRelationship DefaultRelationship;
    DefaultRelationship.TargetPawn = TargetPawn;
    return DefaultRelationship;
}

TArray<APawn*> UNPCBehaviorComponent::GetNearbyNPCs()
{
    TArray<APawn*> NearbyNPCs;
    TArray<AActor*> FoundActors;
    
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    for (AActor* Actor : FoundActors)
    {
        APawn* Pawn = Cast<APawn>(Actor);
        if (Pawn && Pawn != GetOwner() && FVector::Dist(Pawn->GetActorLocation(), MyLocation) <= SocialInteractionRadius)
        {
            NearbyNPCs.Add(Pawn);
        }
    }
    
    return NearbyNPCs;
}

void UNPCBehaviorComponent::SetPatrolPoints(const TArray<FVector>& NewPatrolPoints)
{
    PatrolPoints = NewPatrolPoints;
    CurrentPatrolIndex = 0;
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner()->GetActorLocation();
    }
    
    return PatrolPoints[CurrentPatrolIndex];
}

void UNPCBehaviorComponent::AdvancePatrolPoint()
{
    if (PatrolPoints.Num() > 0)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    }
}

void UNPCBehaviorComponent::UpdateBlackboardValues()
{
    if (!BlackboardComponent) return;

    // Update behavior state
    BlackboardComponent->SetValueAsEnum(TEXT("NPCState"), (uint8)CurrentBehaviorState);
    
    // Update patrol location
    BlackboardComponent->SetValueAsVector(TEXT("PatrolLocation"), GetNextPatrolPoint());
    
    // Update alertness
    BlackboardComponent->SetValueAsFloat(TEXT("AlertnessLevel"), AlertnessLevel);
    
    // Update energy
    BlackboardComponent->SetValueAsFloat(TEXT("EnergyLevel"), EnergyLevel);
}

void UNPCBehaviorComponent::InitializeBehaviorTree()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    // Get AI controller
    if (AController* AIController = OwnerPawn->GetController())
    {
        BehaviorTreeComponent = AIController->FindComponentByClass<UBehaviorTreeComponent>();
        BlackboardComponent = AIController->FindComponentByClass<UBlackboardComponent>();
    }
}

void UNPCBehaviorComponent::UpdateEnergyAndNeeds(float DeltaTime)
{
    // Energy decreases over time
    EnergyLevel = FMath::Clamp(EnergyLevel - (DeltaTime * 0.01f), 0.0f, 1.0f);
    
    // Social need increases over time
    SocialNeed = FMath::Clamp(SocialNeed + (DeltaTime * 0.005f), 0.0f, 1.0f);
    
    // Low energy triggers tired state
    if (EnergyLevel < 0.3f && CurrentEmotionalState != ENPC_EmotionalState::Tired)
    {
        SetEmotionalState(ENPC_EmotionalState::Tired);
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
    
    // High social need triggers socializing behavior
    if (SocialNeed > 0.8f && GetNearbyNPCs().Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Socializing);
        SocialNeed = FMath::Clamp(SocialNeed - 0.1f, 0.0f, 1.0f);
    }
}

void UNPCBehaviorComponent::ProcessSocialInteractions(float DeltaTime)
{
    if (GetWorld()->GetTimeSeconds() - LastSocialUpdate < 1.0f) return;
    
    TArray<APawn*> NearbyNPCs = GetNearbyNPCs();
    for (APawn* NPC : NearbyNPCs)
    {
        // Positive interaction by default
        UpdateSocialRelationship(NPC, 0.01f, 0.0f);
    }
    
    LastSocialUpdate = GetWorld()->GetTimeSeconds();
}

void UNPCBehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    if (CurrentBehaviorState != ENPC_BehaviorState::Patrolling) return;
    
    PatrolTimer += DeltaTime;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = GetNextPatrolPoint();
    
    // Check if reached patrol point
    if (FVector::Dist(CurrentLocation, TargetLocation) < 100.0f)
    {
        if (PatrolTimer >= PatrolWaitTime)
        {
            AdvancePatrolPoint();
            PatrolTimer = 0.0f;
        }
    }
}

void UNPCBehaviorComponent::HandleEmotionalDecay(float DeltaTime)
{
    // Gradually return to calm state
    if (CurrentEmotionalState != ENPC_EmotionalState::Calm)
    {
        static float EmotionalDecayTimer = 0.0f;
        EmotionalDecayTimer += DeltaTime;
        
        if (EmotionalDecayTimer >= 10.0f) // 10 seconds to decay
        {
            SetEmotionalState(ENPC_EmotionalState::Calm);
            EmotionalDecayTimer = 0.0f;
        }
    }
    
    // Gradually reduce alertness
    if (AlertnessLevel > 0.3f)
    {
        AlertnessLevel = FMath::Clamp(AlertnessLevel - (DeltaTime * 0.05f), 0.3f, 1.0f);
    }
}