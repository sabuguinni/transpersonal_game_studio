#include "NPC_AdvancedBehaviorSystem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_AdvancedBehaviorSystem::UNPC_AdvancedBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    BehaviorComplexity = ENPC_BehaviorComplexity::Intermediate;
    SocialRole = ENPC_SocialRole::Follower;
    BehaviorUpdateInterval = 0.5f;
    DecisionMakingSpeed = 1.0f;
    MemoryRetentionTime = 300.0f; // 5 minutes
    MaxMemoryEntries = 50;
    SocialInteractionRange = 1500.0f;
    RelationshipDecayRate = 0.1f;
    bUseAdvancedAnimations = true;
    AnimationBlendSpeed = 2.0f;
    EmotionalExpressionIntensity = 1.0f;
    
    LastBehaviorUpdate = 0.0f;
    LastMemoryCleanup = 0.0f;
    LastSocialUpdate = 0.0f;
    CachedAIController = nullptr;
    CachedBlackboard = nullptr;
}

void UNPC_AdvancedBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAIReferences();
    
    // Initialize behavior memory
    BehaviorMemory = FNPC_BehaviorMemory();
    
    // Set initial behavior tree
    if (PrimaryBehaviorTree && CachedAIController)
    {
        SwitchBehaviorTree(PrimaryBehaviorTree);
    }
}

void UNPC_AdvancedBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update behavior state at intervals
    if (CurrentTime - LastBehaviorUpdate >= BehaviorUpdateInterval)
    {
        UpdateBehaviorState();
        LastBehaviorUpdate = CurrentTime;
    }
    
    // Clean up old memories periodically
    if (CurrentTime - LastMemoryCleanup >= 60.0f) // Every minute
    {
        CleanupOldMemories();
        LastMemoryCleanup = CurrentTime;
    }
    
    // Update social relationships
    if (CurrentTime - LastSocialUpdate >= 2.0f) // Every 2 seconds
    {
        UpdateSocialRelationships();
        LastSocialUpdate = CurrentTime;
    }
    
    // Process complex behavior based on complexity level
    if (BehaviorComplexity == ENPC_BehaviorComplexity::Advanced || 
        BehaviorComplexity == ENPC_BehaviorComplexity::Expert)
    {
        ProcessComplexBehavior();
    }
}

void UNPC_AdvancedBehaviorSystem::InitializeAIReferences()
{
    if (AActor* Owner = GetOwner())
    {
        if (APawn* PawnOwner = Cast<APawn>(Owner))
        {
            CachedAIController = Cast<AAIController>(PawnOwner->GetController());
            if (CachedAIController)
            {
                CachedBlackboard = CachedAIController->GetBlackboardComponent();
            }
        }
    }
}

void UNPC_AdvancedBehaviorSystem::UpdateBehaviorState()
{
    ProcessEnvironmentalAwareness();
    MakeDecision();
    UpdateBlackboardValues();
    
    if (bUseAdvancedAnimations)
    {
        UpdateAnimationParameters();
    }
}

void UNPC_AdvancedBehaviorSystem::ProcessEnvironmentalAwareness()
{
    if (!GetOwner()) return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Scan for nearby actors
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner()) continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance <= SocialInteractionRange)
        {
            // Check if this is a player
            if (Actor->IsA<APawn>() && Cast<APawn>(Actor)->IsPlayerControlled())
            {
                UpdatePlayerMemory(Actor->GetActorLocation(), Distance < 800.0f);
            }
            else
            {
                // Process as potential social interaction
                ProcessSocialInteraction(Actor);
            }
        }
    }
}

void UNPC_AdvancedBehaviorSystem::MakeDecision()
{
    if (!CachedAIController || !CachedBlackboard) return;
    
    // Determine if we should switch behavior trees
    if (ShouldEnterCombatMode() && CombatBehaviorTree)
    {
        SwitchBehaviorTree(CombatBehaviorTree);
    }
    else if (ShouldEnterSocialMode() && SocialBehaviorTree)
    {
        SwitchBehaviorTree(SocialBehaviorTree);
    }
    else if (PrimaryBehaviorTree)
    {
        SwitchBehaviorTree(PrimaryBehaviorTree);
    }
    
    // Handle social role responsibilities
    HandleSocialRoleResponsibilities();
}

void UNPC_AdvancedBehaviorSystem::ProcessComplexBehavior()
{
    // Advanced behavior processing for complex NPCs
    UpdateEmotionalState();
    
    // Process long-term goals and planning
    if (BehaviorComplexity == ENPC_BehaviorComplexity::Expert)
    {
        // Expert-level NPCs can plan multiple steps ahead
        // and adapt their behavior based on past experiences
        
        // Analyze past threat encounters
        float RecentThreatTime = GetWorld()->GetTimeSeconds() - BehaviorMemory.LastThreatTime;
        if (RecentThreatTime < 120.0f) // Within last 2 minutes
        {
            // Increase caution and defensive behavior
            if (CachedBlackboard)
            {
                CachedBlackboard->SetValueAsFloat("CautionLevel", 0.8f);
                CachedBlackboard->SetValueAsBool("RecentThreat", true);
            }
        }
    }
}

void UNPC_AdvancedBehaviorSystem::HandleSocialRoleResponsibilities()
{
    switch (SocialRole)
    {
        case ENPC_SocialRole::Leader:
            // Leaders coordinate group behavior
            if (CachedBlackboard)
            {
                CachedBlackboard->SetValueAsBool("IsLeader", true);
                CachedBlackboard->SetValueAsFloat("LeadershipRadius", SocialInteractionRange * 1.5f);
            }
            break;
            
        case ENPC_SocialRole::Scout:
            // Scouts explore and report back
            if (CachedBlackboard)
            {
                CachedBlackboard->SetValueAsBool("IsScout", true);
                CachedBlackboard->SetValueAsFloat("ExplorationRadius", SocialInteractionRange * 2.0f);
            }
            break;
            
        case ENPC_SocialRole::Guardian:
            // Guardians protect the group
            if (CachedBlackboard)
            {
                CachedBlackboard->SetValueAsBool("IsGuardian", true);
                CachedBlackboard->SetValueAsFloat("ProtectionRadius", SocialInteractionRange);
            }
            break;
            
        default:
            break;
    }
}

void UNPC_AdvancedBehaviorSystem::UpdateEmotionalState()
{
    if (!bUseAdvancedAnimations) return;
    
    // Calculate emotional state based on recent experiences
    float StressLevel = 0.0f;
    float SocialSatisfaction = 0.0f;
    
    // Stress from recent threats
    float TimeSinceLastThreat = GetWorld()->GetTimeSeconds() - BehaviorMemory.LastThreatTime;
    if (TimeSinceLastThreat < 60.0f)
    {
        StressLevel = FMath::Clamp(1.0f - (TimeSinceLastThreat / 60.0f), 0.0f, 1.0f);
    }
    
    // Social satisfaction from relationships
    for (const FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.bIsAlly && Relationship.RelationshipStrength > 0.5f)
        {
            SocialSatisfaction += 0.1f;
        }
    }
    SocialSatisfaction = FMath::Clamp(SocialSatisfaction, 0.0f, 1.0f);
    
    // Update blackboard with emotional state
    if (CachedBlackboard)
    {
        CachedBlackboard->SetValueAsFloat("StressLevel", StressLevel);
        CachedBlackboard->SetValueAsFloat("SocialSatisfaction", SocialSatisfaction);
    }
    
    // Trigger appropriate emotional animations
    if (StressLevel > 0.7f)
    {
        TriggerEmotionalAnimation("Stressed");
    }
    else if (SocialSatisfaction > 0.7f)
    {
        TriggerEmotionalAnimation("Content");
    }
}

void UNPC_AdvancedBehaviorSystem::AddMemoryLocation(FVector Location)
{
    BehaviorMemory.ImportantLocations.Add(Location);
    
    // Limit memory entries
    if (BehaviorMemory.ImportantLocations.Num() > MaxMemoryEntries)
    {
        BehaviorMemory.ImportantLocations.RemoveAt(0);
    }
}

void UNPC_AdvancedBehaviorSystem::AddThreatToMemory(AActor* Threat)
{
    if (!Threat) return;
    
    BehaviorMemory.KnownThreats.AddUnique(Threat);
    BehaviorMemory.LastThreatTime = GetWorld()->GetTimeSeconds();
    
    // Limit memory entries
    if (BehaviorMemory.KnownThreats.Num() > MaxMemoryEntries)
    {
        BehaviorMemory.KnownThreats.RemoveAt(0);
    }
}

void UNPC_AdvancedBehaviorSystem::AddAllyToMemory(AActor* Ally)
{
    if (!Ally) return;
    
    BehaviorMemory.KnownAllies.AddUnique(Ally);
    
    // Limit memory entries
    if (BehaviorMemory.KnownAllies.Num() > MaxMemoryEntries)
    {
        BehaviorMemory.KnownAllies.RemoveAt(0);
    }
}

void UNPC_AdvancedBehaviorSystem::UpdatePlayerMemory(FVector PlayerLocation, bool bIsThreat)
{
    BehaviorMemory.LastKnownPlayerLocation = PlayerLocation;
    BehaviorMemory.bPlayerIsKnownThreat = bIsThreat;
    
    if (CachedBlackboard)
    {
        CachedBlackboard->SetValueAsVector("LastKnownPlayerLocation", PlayerLocation);
        CachedBlackboard->SetValueAsBool("PlayerIsKnownThreat", bIsThreat);
    }
}

void UNPC_AdvancedBehaviorSystem::CleanupOldMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old threat memories
    BehaviorMemory.KnownThreats.RemoveAll([CurrentTime, this](AActor* Threat) {
        return !IsValid(Threat) || (CurrentTime - BehaviorMemory.LastThreatTime) > MemoryRetentionTime;
    });
    
    // Remove old ally memories
    BehaviorMemory.KnownAllies.RemoveAll([](AActor* Ally) {
        return !IsValid(Ally);
    });
}

void UNPC_AdvancedBehaviorSystem::UpdateSocialRelationships()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay relationships over time
    for (FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (IsValid(Relationship.RelatedActor))
        {
            float TimeSinceInteraction = CurrentTime - Relationship.LastInteractionTime;
            if (TimeSinceInteraction > 30.0f) // 30 seconds
            {
                Relationship.RelationshipStrength -= RelationshipDecayRate * (TimeSinceInteraction / 30.0f);
                Relationship.RelationshipStrength = FMath::Clamp(Relationship.RelationshipStrength, -1.0f, 1.0f);
            }
        }
    }
    
    // Remove invalid relationships
    SocialRelationships.RemoveAll([](const FNPC_SocialRelationship& Relationship) {
        return !IsValid(Relationship.RelatedActor) || FMath::Abs(Relationship.RelationshipStrength) < 0.1f;
    });
}

void UNPC_AdvancedBehaviorSystem::ProcessSocialInteraction(AActor* OtherActor)
{
    if (!OtherActor) return;
    
    // Find existing relationship or create new one
    FNPC_SocialRelationship* ExistingRelationship = SocialRelationships.FindByPredicate(
        [OtherActor](const FNPC_SocialRelationship& Rel) {
            return Rel.RelatedActor == OtherActor;
        });
    
    if (ExistingRelationship)
    {
        ExistingRelationship->LastInteractionTime = GetWorld()->GetTimeSeconds();
        // Positive interaction strengthens relationship
        ExistingRelationship->RelationshipStrength += 0.1f;
        ExistingRelationship->RelationshipStrength = FMath::Clamp(ExistingRelationship->RelationshipStrength, -1.0f, 1.0f);
    }
    else
    {
        // Create new relationship
        FNPC_SocialRelationship NewRelationship;
        NewRelationship.RelatedActor = OtherActor;
        NewRelationship.RelationshipStrength = 0.1f;
        NewRelationship.LastInteractionTime = GetWorld()->GetTimeSeconds();
        NewRelationship.bIsAlly = false;
        NewRelationship.bIsRival = false;
        
        SocialRelationships.Add(NewRelationship);
    }
}

float UNPC_AdvancedBehaviorSystem::GetRelationshipStrength(AActor* OtherActor)
{
    if (!OtherActor) return 0.0f;
    
    const FNPC_SocialRelationship* Relationship = SocialRelationships.FindByPredicate(
        [OtherActor](const FNPC_SocialRelationship& Rel) {
            return Rel.RelatedActor == OtherActor;
        });
    
    return Relationship ? Relationship->RelationshipStrength : 0.0f;
}

void UNPC_AdvancedBehaviorSystem::ModifyRelationship(AActor* OtherActor, float StrengthChange)
{
    if (!OtherActor) return;
    
    FNPC_SocialRelationship* Relationship = SocialRelationships.FindByPredicate(
        [OtherActor](const FNPC_SocialRelationship& Rel) {
            return Rel.RelatedActor == OtherActor;
        });
    
    if (Relationship)
    {
        Relationship->RelationshipStrength += StrengthChange;
        Relationship->RelationshipStrength = FMath::Clamp(Relationship->RelationshipStrength, -1.0f, 1.0f);
        Relationship->LastInteractionTime = GetWorld()->GetTimeSeconds();
        
        // Update ally/rival status
        if (Relationship->RelationshipStrength > 0.6f)
        {
            Relationship->bIsAlly = true;
            Relationship->bIsRival = false;
        }
        else if (Relationship->RelationshipStrength < -0.6f)
        {
            Relationship->bIsAlly = false;
            Relationship->bIsRival = true;
        }
    }
}

void UNPC_AdvancedBehaviorSystem::SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree)
{
    if (!CachedAIController || !NewBehaviorTree) return;
    
    if (UBehaviorTreeComponent* BTComponent = CachedAIController->GetBehaviorTreeComponent())
    {
        if (BTComponent->GetCurrentTree() != NewBehaviorTree)
        {
            CachedAIController->RunBehaviorTree(NewBehaviorTree);
        }
    }
}

void UNPC_AdvancedBehaviorSystem::UpdateBlackboardValues()
{
    if (!CachedBlackboard) return;
    
    // Update basic behavior values
    CachedBlackboard->SetValueAsFloat("BehaviorComplexity", static_cast<float>(BehaviorComplexity));
    CachedBlackboard->SetValueAsFloat("SocialRole", static_cast<float>(SocialRole));
    CachedBlackboard->SetValueAsFloat("SocialInteractionRange", SocialInteractionRange);
    
    // Update memory-based values
    if (BehaviorMemory.ImportantLocations.Num() > 0)
    {
        CachedBlackboard->SetValueAsVector("LastImportantLocation", BehaviorMemory.ImportantLocations.Last());
    }
    
    CachedBlackboard->SetValueAsInt("KnownThreatCount", BehaviorMemory.KnownThreats.Num());
    CachedBlackboard->SetValueAsInt("KnownAllyCount", BehaviorMemory.KnownAllies.Num());
    CachedBlackboard->SetValueAsInt("SocialRelationshipCount", SocialRelationships.Num());
}

bool UNPC_AdvancedBehaviorSystem::ShouldEnterCombatMode()
{
    // Enter combat mode if there are recent threats or player is hostile
    float TimeSinceLastThreat = GetWorld()->GetTimeSeconds() - BehaviorMemory.LastThreatTime;
    return (TimeSinceLastThreat < 30.0f) || (BehaviorMemory.bPlayerIsKnownThreat && 
           FVector::Dist(GetOwner()->GetActorLocation(), BehaviorMemory.LastKnownPlayerLocation) < 1000.0f);
}

bool UNPC_AdvancedBehaviorSystem::ShouldEnterSocialMode()
{
    // Enter social mode if there are nearby allies
    int32 NearbyAllies = 0;
    for (const FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.bIsAlly && IsValid(Relationship.RelatedActor))
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Relationship.RelatedActor->GetActorLocation());
            if (Distance <= SocialInteractionRange)
            {
                NearbyAllies++;
            }
        }
    }
    
    return NearbyAllies >= 2;
}

void UNPC_AdvancedBehaviorSystem::TriggerEmotionalAnimation(const FString& EmotionType)
{
    if (!bUseAdvancedAnimations) return;
    
    // This would integrate with the animation system
    // For now, we update blackboard values that animations can read
    if (CachedBlackboard)
    {
        CachedBlackboard->SetValueAsString("CurrentEmotion", EmotionType);
        CachedBlackboard->SetValueAsFloat("EmotionIntensity", EmotionalExpressionIntensity);
    }
}

void UNPC_AdvancedBehaviorSystem::UpdateAnimationParameters()
{
    if (!CachedBlackboard) return;
    
    // Update animation parameters based on current state
    float MovementSpeed = 0.0f;
    if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
    {
        MovementSpeed = PawnOwner->GetVelocity().Size();
    }
    
    CachedBlackboard->SetValueAsFloat("MovementSpeed", MovementSpeed);
    CachedBlackboard->SetValueAsFloat("AnimationBlendSpeed", AnimationBlendSpeed);
    
    // Set behavioral animation flags
    CachedBlackboard->SetValueAsBool("IsAlert", BehaviorMemory.bPlayerIsKnownThreat);
    CachedBlackboard->SetValueAsBool("IsSocial", ShouldEnterSocialMode());
    CachedBlackboard->SetValueAsBool("IsAggressive", ShouldEnterCombatMode());
}

void UNPC_AdvancedBehaviorSystem::BlendToNewAnimationState(const FString& StateName)
{
    if (!CachedBlackboard) return;
    
    CachedBlackboard->SetValueAsString("TargetAnimationState", StateName);
    CachedBlackboard->SetValueAsBool("ShouldBlendAnimation", true);
}