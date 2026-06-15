#include "NPCBehaviorTreeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Math/UnrealMathUtility.h"

UNPCBehaviorTreeManager::UNPCBehaviorTreeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 times per second

    // Initialize default values
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    SocialRole = ENPC_SocialRole::Gatherer;
    
    Energy = 100.0f;
    Hunger = 0.0f;
    Fear = 0.0f;
    Alertness = 50.0f;
    
    MaxShortTermMemories = 20;
    MaxLongTermMemories = 100;
    
    SocialRadius = 1500.0f;
    CommunicationCooldown = 0.0f;
    DailyRoutineTimer = 0.0f;
    
    HomeLocation = FVector::ZeroVector;
    WorkLocation = FVector::ZeroVector;
}

void UNPCBehaviorTreeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize home location to current position
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
        WorkLocation = HomeLocation + FVector(FMath::RandRange(-2000.0f, 2000.0f), FMath::RandRange(-2000.0f, 2000.0f), 0.0f);
    }
    
    // Setup initial patrol points
    PatrolPoints.Empty();
    for (int32 i = 0; i < 4; i++)
    {
        FVector PatrolPoint = HomeLocation + FVector(
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(-1000.0f, 1000.0f),
            0.0f
        );
        PatrolPoints.Add(PatrolPoint);
    }
    
    // Initialize blackboard if we have an AI controller
    if (AActor* Owner = GetOwner())
    {
        if (APawn* PawnOwner = Cast<APawn>(Owner))
        {
            if (AAIController* AIController = Cast<AAIController>(PawnOwner->GetController()))
            {
                NPCBlackboard = AIController->GetBlackboardComponent();
                if (NPCBlackboard && DefaultBehaviorTree)
                {
                    AIController->RunBehaviorTree(DefaultBehaviorTree);
                }
            }
        }
    }
}

void UNPCBehaviorTreeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateNeeds(DeltaTime);
    ProcessSocialInteractions(DeltaTime);
    ProcessMemoryDecay(DeltaTime);
    
    // Update daily routine timer
    DailyRoutineTimer += DeltaTime;
    if (DailyRoutineTimer >= 86400.0f) // 24 hours in seconds
    {
        DailyRoutineTimer = 0.0f;
    }
    
    // Execute daily routine based on time
    float TimeOfDay = FMath::Fmod(DailyRoutineTimer / 86400.0f, 1.0f);
    ExecuteDailyRoutine(TimeOfDay);
    
    // Update communication cooldown
    if (CommunicationCooldown > 0.0f)
    {
        CommunicationCooldown -= DeltaTime;
    }
}

void UNPCBehaviorTreeManager::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        
        // Update blackboard
        if (NPCBlackboard)
        {
            NPCBlackboard->SetValueAsEnum(TEXT("BehaviorState"), (uint8)NewState);
        }
        
        // Add memory entry for state change
        AddMemoryEntry(
            GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector,
            GetOwner(),
            0.5f,
            FString::Printf(TEXT("State changed to %d"), (int32)NewState)
        );
    }
}

void UNPCBehaviorTreeManager::SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree)
{
    if (NewBehaviorTree && GetOwner())
    {
        if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
        {
            if (AAIController* AIController = Cast<AAIController>(PawnOwner->GetController()))
            {
                AIController->RunBehaviorTree(NewBehaviorTree);
            }
        }
    }
}

void UNPCBehaviorTreeManager::AddMemoryEntry(FVector Location, AActor* Actor, float Importance, const FString& Description)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.RelatedActor = Actor;
    NewMemory.Importance = Importance;
    NewMemory.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewMemory.EventDescription = Description;
    
    // Add to short-term memory
    ShortTermMemory.Add(NewMemory);
    
    // If importance is high enough, also add to long-term memory
    if (Importance >= 0.7f)
    {
        LongTermMemory.Add(NewMemory);
    }
    
    ManageMemoryCapacity();
}

void UNPCBehaviorTreeManager::ProcessMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Decay short-term memories (remove after 300 seconds)
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - ShortTermMemory[i].TimeStamp > 300.0f)
        {
            ShortTermMemory.RemoveAt(i);
        }
    }
    
    // Decay long-term memories (reduce importance over time)
    for (FNPC_MemoryEntry& Memory : LongTermMemory)
    {
        float TimeSinceEvent = CurrentTime - Memory.TimeStamp;
        float DecayRate = 0.0001f; // Very slow decay for long-term memories
        Memory.Importance = FMath::Max(0.0f, Memory.Importance - (DecayRate * TimeSinceEvent));
    }
    
    // Remove long-term memories with very low importance
    for (int32 i = LongTermMemory.Num() - 1; i >= 0; i--)
    {
        if (LongTermMemory[i].Importance < 0.1f)
        {
            LongTermMemory.RemoveAt(i);
        }
    }
}

void UNPCBehaviorTreeManager::UpdateSocialRelationship(AActor* TargetActor, float RelationshipChange, float TrustChange, float FearChange)
{
    if (!TargetActor) return;
    
    // Find existing relationship or create new one
    FNPC_SocialRelationship* ExistingRelationship = nullptr;
    for (FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.TargetActor == TargetActor)
        {
            ExistingRelationship = &Relationship;
            break;
        }
    }
    
    if (!ExistingRelationship)
    {
        FNPC_SocialRelationship NewRelationship;
        NewRelationship.TargetActor = TargetActor;
        SocialRelationships.Add(NewRelationship);
        ExistingRelationship = &SocialRelationships.Last();
    }
    
    // Update relationship values
    ExistingRelationship->RelationshipValue = FMath::Clamp(ExistingRelationship->RelationshipValue + RelationshipChange, -100.0f, 100.0f);
    ExistingRelationship->Trust = FMath::Clamp(ExistingRelationship->Trust + TrustChange, 0.0f, 100.0f);
    ExistingRelationship->Fear = FMath::Clamp(ExistingRelationship->Fear + FearChange, 0.0f, 100.0f);
    ExistingRelationship->LastInteractionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Add memory entry for social interaction
    AddMemoryEntry(
        TargetActor->GetActorLocation(),
        TargetActor,
        FMath::Abs(RelationshipChange) * 0.1f,
        FString::Printf(TEXT("Social interaction with %s"), *TargetActor->GetName())
    );
}

FNPC_SocialRelationship UNPCBehaviorTreeManager::GetSocialRelationship(AActor* TargetActor)
{
    for (const FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.TargetActor == TargetActor)
        {
            return Relationship;
        }
    }
    
    // Return default relationship if not found
    FNPC_SocialRelationship DefaultRelationship;
    DefaultRelationship.TargetActor = TargetActor;
    return DefaultRelationship;
}

void UNPCBehaviorTreeManager::ExecuteDailyRoutine(float TimeOfDay)
{
    ENPC_BehaviorState NewState = CurrentBehaviorState;
    
    // Simple daily routine based on time of day
    if (TimeOfDay >= 0.0f && TimeOfDay < 0.25f) // Night (0-6 AM)
    {
        NewState = ENPC_BehaviorState::Sleeping;
    }
    else if (TimeOfDay >= 0.25f && TimeOfDay < 0.5f) // Morning (6 AM - 12 PM)
    {
        switch (SocialRole)
        {
            case ENPC_SocialRole::Hunter:
                NewState = ENPC_BehaviorState::Hunting;
                break;
            case ENPC_SocialRole::Gatherer:
                NewState = ENPC_BehaviorState::Gathering;
                break;
            case ENPC_SocialRole::Guard:
                NewState = ENPC_BehaviorState::Patrolling;
                break;
            default:
                NewState = ENPC_BehaviorState::Working;
                break;
        }
    }
    else if (TimeOfDay >= 0.5f && TimeOfDay < 0.75f) // Afternoon (12 PM - 6 PM)
    {
        NewState = ENPC_BehaviorState::Socializing;
    }
    else // Evening (6 PM - 12 AM)
    {
        NewState = ENPC_BehaviorState::Idle;
    }
    
    // Override routine if fear is high
    if (Fear > 70.0f)
    {
        NewState = ENPC_BehaviorState::Fleeing;
    }
    
    SetBehaviorState(NewState);
}

void UNPCBehaviorTreeManager::OnPerceptionUpdated(AActor* Actor, float Stimulus)
{
    if (!Actor) return;
    
    // Increase alertness based on stimulus
    Alertness = FMath::Clamp(Alertness + Stimulus * 0.1f, 0.0f, 100.0f);
    
    // Add memory entry for perceived actor
    AddMemoryEntry(
        Actor->GetActorLocation(),
        Actor,
        Stimulus * 0.01f,
        FString::Printf(TEXT("Perceived %s"), *Actor->GetName())
    );
    
    // Update blackboard with perceived actor
    if (NPCBlackboard)
    {
        NPCBlackboard->SetValueAsObject(TEXT("PerceivedActor"), Actor);
        NPCBlackboard->SetValueAsVector(TEXT("PerceivedLocation"), Actor->GetActorLocation());
    }
}

void UNPCBehaviorTreeManager::SendSocialSignal(const FString& Message, float Range)
{
    if (CommunicationCooldown > 0.0f) return;
    
    if (!GetOwner()) return;
    
    // Find nearby NPCs to send signal to
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetOwner() && IsInSocialRange(Actor))
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= Range)
            {
                // Try to find NPC behavior component on target
                if (UNPCBehaviorTreeManager* TargetNPC = Actor->FindComponentByClass<UNPCBehaviorTreeManager>())
                {
                    TargetNPC->ReceiveSocialSignal(GetOwner(), Message);
                }
            }
        }
    }
    
    CommunicationCooldown = 2.0f; // 2 second cooldown
}

void UNPCBehaviorTreeManager::ReceiveSocialSignal(AActor* Sender, const FString& Message)
{
    if (!Sender) return;
    
    // Process the social signal
    AddMemoryEntry(
        Sender->GetActorLocation(),
        Sender,
        0.3f,
        FString::Printf(TEXT("Received signal: %s"), *Message)
    );
    
    // Update social relationship
    UpdateSocialRelationship(Sender, 1.0f, 0.5f, 0.0f);
    
    // Update blackboard with signal information
    if (NPCBlackboard)
    {
        NPCBlackboard->SetValueAsString(TEXT("LastSocialMessage"), Message);
        NPCBlackboard->SetValueAsObject(TEXT("LastSocialSender"), Sender);
    }
}

void UNPCBehaviorTreeManager::UpdateNeeds(float DeltaTime)
{
    // Gradually increase hunger
    Hunger = FMath::Clamp(Hunger + (DeltaTime * 0.5f), 0.0f, 100.0f);
    
    // Gradually decrease energy
    Energy = FMath::Clamp(Energy - (DeltaTime * 0.3f), 0.0f, 100.0f);
    
    // Gradually decrease fear if no threats
    Fear = FMath::Clamp(Fear - (DeltaTime * 2.0f), 0.0f, 100.0f);
    
    // Gradually normalize alertness
    float TargetAlertness = 50.0f;
    Alertness = FMath::FInterpTo(Alertness, TargetAlertness, DeltaTime, 1.0f);
    
    // Update blackboard with current needs
    if (NPCBlackboard)
    {
        NPCBlackboard->SetValueAsFloat(TEXT("Energy"), Energy);
        NPCBlackboard->SetValueAsFloat(TEXT("Hunger"), Hunger);
        NPCBlackboard->SetValueAsFloat(TEXT("Fear"), Fear);
        NPCBlackboard->SetValueAsFloat(TEXT("Alertness"), Alertness);
    }
}

void UNPCBehaviorTreeManager::ProcessSocialInteractions(float DeltaTime)
{
    if (!GetOwner()) return;
    
    // Find nearby NPCs for social interactions
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetOwner() && IsInSocialRange(Actor))
        {
            // Check if this is an NPC we can interact with
            if (UNPCBehaviorTreeManager* OtherNPC = Actor->FindComponentByClass<UNPCBehaviorTreeManager>())
            {
                // Simple social interaction based on proximity
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                if (Distance < 500.0f) // Very close proximity
                {
                    // Positive social interaction
                    UpdateSocialRelationship(Actor, 0.1f * DeltaTime, 0.05f * DeltaTime, 0.0f);
                }
            }
        }
    }
}

void UNPCBehaviorTreeManager::ManageMemoryCapacity()
{
    // Remove oldest short-term memories if over capacity
    while (ShortTermMemory.Num() > MaxShortTermMemories)
    {
        ShortTermMemory.RemoveAt(0);
    }
    
    // Remove least important long-term memories if over capacity
    while (LongTermMemory.Num() > MaxLongTermMemories)
    {
        int32 LeastImportantIndex = 0;
        float LowestImportance = LongTermMemory[0].Importance;
        
        for (int32 i = 1; i < LongTermMemory.Num(); i++)
        {
            if (LongTermMemory[i].Importance < LowestImportance)
            {
                LowestImportance = LongTermMemory[i].Importance;
                LeastImportantIndex = i;
            }
        }
        
        LongTermMemory.RemoveAt(LeastImportantIndex);
    }
}

AActor* UNPCBehaviorTreeManager::FindNearestActorOfType(TSubclassOf<AActor> ActorClass, float MaxDistance)
{
    if (!GetOwner() || !ActorClass) return nullptr;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, FoundActors);
    
    AActor* NearestActor = nullptr;
    float NearestDistance = MaxDistance;
    
    for (AActor* Actor : FoundActors)
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestActor = Actor;
        }
    }
    
    return NearestActor;
}

bool UNPCBehaviorTreeManager::IsInSocialRange(AActor* TargetActor)
{
    if (!GetOwner() || !TargetActor) return false;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
    return Distance <= SocialRadius;
}