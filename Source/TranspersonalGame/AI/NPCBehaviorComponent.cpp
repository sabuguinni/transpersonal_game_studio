#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    NPCName = TEXT("Unnamed");
    NPCRole = ENPC_Role::Gatherer;
    Age = 25;
    Experience = 50.0f;
    
    MaxMemories = 20;
    MemoryDecayRate = 1.0f;
    EmotionDecayRate = 0.5f;
    
    CurrentActivity = ENPC_Activity::Idle;
    ActivityTimer = 0.0f;
    HomeLocation = FVector::ZeroVector;
    PatrolRadius = 1000.0f;
    
    SocialInteractionRange = 300.0f;
    DangerThreshold = 70.0f;
    FleeDistance = 2000.0f;
    
    LastEmotionUpdate = 0.0f;
    LastMemoryCleanup = 0.0f;
    
    CurrentTarget = nullptr;
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Set home location to current position
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }
    
    // Initialize emotional state based on role
    switch (NPCRole)
    {
        case ENPC_Role::Guard:
            CurrentEmotions.Alertness = 80.0f;
            CurrentEmotions.Fear = 30.0f;
            break;
        case ENPC_Role::Hunter:
            CurrentEmotions.Alertness = 70.0f;
            CurrentEmotions.Anger = 40.0f;
            break;
        case ENPC_Role::Elder:
            CurrentEmotions.Fear = 40.0f;
            CurrentEmotions.Curiosity = 60.0f;
            break;
        case ENPC_Role::Child:
            CurrentEmotions.Curiosity = 80.0f;
            CurrentEmotions.Fear = 60.0f;
            break;
        default:
            // Keep default values
            break;
    }
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ActivityTimer += DeltaTime;
    
    // Update emotional state
    UpdateEmotionalState(DeltaTime);
    
    // Clean up old memories periodically
    if (GetWorld()->GetTimeSeconds() - LastMemoryCleanup > 30.0f)
    {
        ForgetOldMemories();
        LastMemoryCleanup = GetWorld()->GetTimeSeconds();
    }
    
    // Check for threats and update behavior
    if (ShouldFlee())
    {
        if (CurrentActivity != ENPC_Activity::Fleeing)
        {
            SetActivity(ENPC_Activity::Fleeing);
            ModifyEmotion(TEXT("Fear"), 20.0f);
        }
    }
    
    // Update AI blackboard values
    UpdateBlackboardValues();
}

void UNPC_BehaviorComponent::RememberActor(AActor* Actor, float RelationshipValue, bool bIsHostile)
{
    if (!Actor) return;
    
    // Check if we already have a memory of this actor
    FNPC_Memory* ExistingMemory = FindMemory(Actor);
    
    if (ExistingMemory)
    {
        // Update existing memory
        ExistingMemory->Relationship = FMath::Clamp(RelationshipValue, -100.0f, 100.0f);
        ExistingMemory->LastSeenLocation = Actor->GetActorLocation();
        ExistingMemory->LastSeenTime = GetWorld()->GetTimeSeconds();
        ExistingMemory->bIsHostile = bIsHostile;
    }
    else
    {
        // Create new memory
        FNPC_Memory NewMemory;
        NewMemory.RememberedActor = Actor;
        NewMemory.Relationship = FMath::Clamp(RelationshipValue, -100.0f, 100.0f);
        NewMemory.LastSeenLocation = Actor->GetActorLocation();
        NewMemory.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewMemory.bIsHostile = bIsHostile;
        
        Memories.Add(NewMemory);
        
        // Remove oldest memory if we exceed max
        if (Memories.Num() > MaxMemories)
        {
            Memories.RemoveAt(0);
        }
    }
}

FNPC_Memory* UNPC_BehaviorComponent::FindMemory(AActor* Actor)
{
    if (!Actor) return nullptr;
    
    for (FNPC_Memory& Memory : Memories)
    {
        if (Memory.RememberedActor == Actor)
        {
            return &Memory;
        }
    }
    
    return nullptr;
}

void UNPC_BehaviorComponent::UpdateMemory(AActor* Actor, float RelationshipChange)
{
    FNPC_Memory* Memory = FindMemory(Actor);
    if (Memory)
    {
        Memory->Relationship = FMath::Clamp(Memory->Relationship + RelationshipChange, -100.0f, 100.0f);
        Memory->LastSeenTime = GetWorld()->GetTimeSeconds();
        
        // Update emotional response based on relationship change
        if (RelationshipChange > 0)
        {
            ModifyEmotion(TEXT("SocialNeed"), -5.0f); // Satisfied by positive interaction
        }
        else if (RelationshipChange < 0)
        {
            ModifyEmotion(TEXT("Anger"), 10.0f);
            ModifyEmotion(TEXT("Fear"), 5.0f);
        }
    }
}

void UNPC_BehaviorComponent::ForgetOldMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MemoryLifetime = 300.0f; // 5 minutes
    
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - Memories[i].LastSeenTime > MemoryLifetime)
        {
            Memories.RemoveAt(i);
        }
    }
}

void UNPC_BehaviorComponent::ModifyEmotion(const FString& EmotionName, float Change)
{
    if (EmotionName == TEXT("Fear"))
    {
        CurrentEmotions.Fear = FMath::Clamp(CurrentEmotions.Fear + Change, 0.0f, 100.0f);
    }
    else if (EmotionName == TEXT("Anger"))
    {
        CurrentEmotions.Anger = FMath::Clamp(CurrentEmotions.Anger + Change, 0.0f, 100.0f);
    }
    else if (EmotionName == TEXT("Curiosity"))
    {
        CurrentEmotions.Curiosity = FMath::Clamp(CurrentEmotions.Curiosity + Change, 0.0f, 100.0f);
    }
    else if (EmotionName == TEXT("Alertness"))
    {
        CurrentEmotions.Alertness = FMath::Clamp(CurrentEmotions.Alertness + Change, 0.0f, 100.0f);
    }
    else if (EmotionName == TEXT("SocialNeed"))
    {
        CurrentEmotions.SocialNeed = FMath::Clamp(CurrentEmotions.SocialNeed + Change, 0.0f, 100.0f);
    }
}

void UNPC_BehaviorComponent::UpdateEmotionalState(float DeltaTime)
{
    // Decay emotions over time toward baseline
    DecayEmotions(DeltaTime);
    
    // Emotional reactions based on current situation
    AActor* NearestThreat = FindNearestThreat();
    if (NearestThreat)
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), NearestThreat->GetActorLocation());
        if (Distance < 1500.0f)
        {
            ModifyEmotion(TEXT("Fear"), 2.0f * DeltaTime);
            ModifyEmotion(TEXT("Alertness"), 3.0f * DeltaTime);
        }
    }
    
    // Social needs increase over time
    if (CurrentEmotions.SocialNeed < 80.0f)
    {
        ModifyEmotion(TEXT("SocialNeed"), 0.5f * DeltaTime);
    }
}

void UNPC_BehaviorComponent::SetActivity(ENPC_Activity NewActivity)
{
    if (CurrentActivity != NewActivity)
    {
        CurrentActivity = NewActivity;
        ActivityTimer = 0.0f;
        
        // Emotional responses to activity changes
        switch (NewActivity)
        {
            case ENPC_Activity::Fleeing:
                ModifyEmotion(TEXT("Fear"), 15.0f);
                ModifyEmotion(TEXT("Alertness"), 20.0f);
                break;
            case ENPC_Activity::Socializing:
                ModifyEmotion(TEXT("SocialNeed"), -10.0f);
                break;
            case ENPC_Activity::Resting:
                ModifyEmotion(TEXT("Fear"), -5.0f);
                break;
            default:
                break;
        }
    }
}

bool UNPC_BehaviorComponent::ShouldFlee()
{
    AActor* NearestThreat = FindNearestThreat();
    if (!NearestThreat) return false;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), NearestThreat->GetActorLocation());
    float FleeThreshold = FleeDistance * (CurrentEmotions.Fear / 100.0f);
    
    return Distance < FleeThreshold;
}

AActor* UNPC_BehaviorComponent::FindNearestThreat()
{
    if (!GetOwner()) return nullptr;
    
    AActor* NearestThreat = nullptr;
    float NearestDistance = FLT_MAX;
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    // Check for dinosaurs and other threats
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (IsActorThreat(Actor))
        {
            float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestThreat = Actor;
            }
        }
    }
    
    return NearestThreat;
}

AActor* UNPC_BehaviorComponent::FindNearestTribeMate()
{
    if (!GetOwner()) return nullptr;
    
    AActor* NearestMate = nullptr;
    float NearestDistance = FLT_MAX;
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (AActor* TribeMate : TribeMates)
    {
        if (TribeMate && TribeMate != GetOwner())
        {
            float Distance = FVector::Dist(MyLocation, TribeMate->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestMate = TribeMate;
            }
        }
    }
    
    return NearestMate;
}

void UNPC_BehaviorComponent::InitiateSocialInteraction(AActor* Target)
{
    if (!Target || !IsInSocialRange(Target)) return;
    
    CurrentTarget = Target;
    SetActivity(ENPC_Activity::Socializing);
    
    // Improve relationship through interaction
    UpdateMemory(Target, 5.0f);
    
    // Reduce social need
    ModifyEmotion(TEXT("SocialNeed"), -15.0f);
}

bool UNPC_BehaviorComponent::IsInSocialRange(AActor* Target)
{
    if (!Target || !GetOwner()) return false;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= SocialInteractionRange;
}

void UNPC_BehaviorComponent::UpdateBlackboardValues()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    ACharacter* Character = Cast<ACharacter>(Owner);
    if (!Character) return;
    
    AAIController* AIController = Cast<AAIController>(Character->GetController());
    if (!AIController) return;
    
    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (!BlackboardComp) return;
    
    // Update blackboard with current state
    BlackboardComp->SetValueAsFloat(TEXT("Fear"), CurrentEmotions.Fear);
    BlackboardComp->SetValueAsFloat(TEXT("Alertness"), CurrentEmotions.Alertness);
    BlackboardComp->SetValueAsFloat(TEXT("SocialNeed"), CurrentEmotions.SocialNeed);
    
    BlackboardComp->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
    BlackboardComp->SetValueAsFloat(TEXT("PatrolRadius"), PatrolRadius);
    
    if (CurrentTarget)
    {
        BlackboardComp->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
    }
    
    AActor* NearestThreat = FindNearestThreat();
    if (NearestThreat)
    {
        BlackboardComp->SetValueAsObject(TEXT("NearestThreat"), NearestThreat);
        BlackboardComp->SetValueAsVector(TEXT("ThreatLocation"), NearestThreat->GetActorLocation());
    }
    
    AActor* NearestMate = FindNearestTribeMate();
    if (NearestMate)
    {
        BlackboardComp->SetValueAsObject(TEXT("NearestTribeMate"), NearestMate);
    }
}

void UNPC_BehaviorComponent::ReactToPerception(AActor* PerceivedActor, bool bCanSee)
{
    if (!PerceivedActor) return;
    
    if (bCanSee)
    {
        // Update memory of seen actor
        float RelationshipValue = 0.0f;
        bool bIsHostile = IsActorThreat(PerceivedActor);
        
        if (bIsHostile)
        {
            RelationshipValue = -20.0f;
            ModifyEmotion(TEXT("Fear"), 10.0f);
            ModifyEmotion(TEXT("Alertness"), 15.0f);
        }
        else if (TribeMates.Contains(PerceivedActor))
        {
            RelationshipValue = 10.0f;
            ModifyEmotion(TEXT("SocialNeed"), -5.0f);
        }
        
        RememberActor(PerceivedActor, RelationshipValue, bIsHostile);
    }
}

void UNPC_BehaviorComponent::DecayEmotions(float DeltaTime)
{
    float DecayAmount = EmotionDecayRate * DeltaTime;
    
    // Decay toward baseline values
    float FearBaseline = (NPCRole == ENPC_Role::Child) ? 40.0f : 20.0f;
    float AlertnessBaseline = (NPCRole == ENPC_Role::Guard) ? 60.0f : 40.0f;
    
    CurrentEmotions.Fear = FMath::FInterpTo(CurrentEmotions.Fear, FearBaseline, DeltaTime, DecayAmount);
    CurrentEmotions.Anger = FMath::FInterpTo(CurrentEmotions.Anger, 10.0f, DeltaTime, DecayAmount);
    CurrentEmotions.Curiosity = FMath::FInterpTo(CurrentEmotions.Curiosity, 30.0f, DeltaTime, DecayAmount);
    CurrentEmotions.Alertness = FMath::FInterpTo(CurrentEmotions.Alertness, AlertnessBaseline, DeltaTime, DecayAmount);
}

void UNPC_BehaviorComponent::CleanupMemories()
{
    // Remove memories of destroyed actors
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(Memories[i].RememberedActor))
        {
            Memories.RemoveAt(i);
        }
    }
}

bool UNPC_BehaviorComponent::IsActorThreat(AActor* Actor)
{
    if (!Actor) return false;
    
    // Check if actor is in threat classes
    for (TSubclassOf<AActor> ThreatClass : ThreatClasses)
    {
        if (Actor->IsA(ThreatClass))
        {
            return true;
        }
    }
    
    // Check for dinosaur actors by name
    FString ActorName = Actor->GetName().ToLower();
    if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
        ActorName.Contains(TEXT("carno")) || ActorName.Contains(TEXT("dilo")))
    {
        return true;
    }
    
    return false;
}

float UNPC_BehaviorComponent::CalculateRelationshipChange(AActor* Actor)
{
    if (!Actor) return 0.0f;
    
    // Positive interactions with tribe mates
    if (TribeMates.Contains(Actor))
    {
        return FMath::RandRange(1.0f, 5.0f);
    }
    
    // Negative interactions with threats
    if (IsActorThreat(Actor))
    {
        return FMath::RandRange(-10.0f, -5.0f);
    }
    
    // Neutral for unknown actors
    return FMath::RandRange(-1.0f, 1.0f);
}