#include "NPC_InteractionBehaviorSystem.h"
#include "NPC_SocialMemoryComponent.h"
#include "NPC_EmotionalStateComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UNPC_InteractionBehaviorSystem::UNPC_InteractionBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    InteractionUpdateInterval = 0.5f;
    MaxInteractionRange = 2000.0f;
    MaxConcurrentInteractions = 3;
    bEnableInteractionLogging = true;
    
    LastInteractionUpdate = 0.0f;
    InteractionCounter = 0;
}

void UNPC_InteractionBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get component references
    SocialMemoryComponent = GetOwner()->FindComponentByClass<UNPC_SocialMemoryComponent>();
    EmotionalStateComponent = GetOwner()->FindComponentByClass<UNPC_EmotionalStateComponent>();
    
    if (bEnableInteractionLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("NPC_InteractionBehaviorSystem initialized for %s"), *GetOwner()->GetName());
    }
}

void UNPC_InteractionBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update interactions at specified intervals
    if (CurrentTime - LastInteractionUpdate >= InteractionUpdateInterval)
    {
        ProcessPendingInteractions(DeltaTime);
        UpdateInteractionStates(DeltaTime);
        CleanupExpiredInteractions();
        
        LastInteractionUpdate = CurrentTime;
    }
}

bool UNPC_InteractionBehaviorSystem::InitiateInteraction(AActor* TargetActor, ENPC_InteractionType InteractionType)
{
    if (!IsValidInteractionTarget(TargetActor))
    {
        return false;
    }
    
    // Check if we're already at max concurrent interactions
    if (ActiveInteractions.Num() >= MaxConcurrentInteractions)
    {
        if (bEnableInteractionLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Max concurrent interactions reached for %s"), *GetOwner()->GetName());
        }
        return false;
    }
    
    // Check cooldown
    if (InteractionCooldowns.Contains(TargetActor))
    {
        float CooldownTime = InteractionCooldowns[TargetActor];
        if (GetWorld()->GetTimeSeconds() < CooldownTime)
        {
            return false;
        }
    }
    
    // Create interaction data
    FNPC_InteractionData NewInteraction;
    NewInteraction.InteractionType = InteractionType;
    NewInteraction.TargetActor = TargetActor;
    NewInteraction.InteractionLocation = TargetActor->GetActorLocation();
    NewInteraction.Timestamp = GetWorld()->GetTimeSeconds();
    NewInteraction.InteractionStrength = CalculateInteractionPriority(TargetActor, InteractionType);
    
    // Determine interaction parameters based on type
    switch (InteractionType)
    {
        case ENPC_InteractionType::Aggressive:
            NewInteraction.InteractionRange = 500.0f;
            NewInteraction.InteractionDuration = 3.0f;
            break;
        case ENPC_InteractionType::Territorial:
            NewInteraction.InteractionRange = 1500.0f;
            NewInteraction.InteractionDuration = 10.0f;
            break;
        case ENPC_InteractionType::Curious:
            NewInteraction.InteractionRange = 800.0f;
            NewInteraction.InteractionDuration = 5.0f;
            break;
        case ENPC_InteractionType::Social:
            NewInteraction.InteractionRange = 300.0f;
            NewInteraction.InteractionDuration = 8.0f;
            break;
        default:
            NewInteraction.InteractionRange = 1000.0f;
            NewInteraction.InteractionDuration = 5.0f;
            break;
    }
    
    PendingInteractions.Add(NewInteraction);
    
    if (bEnableInteractionLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("%s initiating %s interaction with %s"), 
            *GetOwner()->GetName(), 
            *UEnum::GetValueAsString(InteractionType),
            *TargetActor->GetName());
    }
    
    return true;
}

void UNPC_InteractionBehaviorSystem::ProcessInteraction(const FNPC_InteractionData& InteractionData)
{
    if (!InteractionData.TargetActor.IsValid())
    {
        return;
    }
    
    // Analyze current context
    FNPC_InteractionContext Context = AnalyzeInteractionContext(InteractionData.TargetActor.Get());
    
    // Validate interaction conditions
    if (!ValidateInteractionConditions(InteractionData))
    {
        return;
    }
    
    // Execute the interaction behavior
    ExecuteInteractionBehavior(InteractionData);
    
    // Evaluate outcome
    ENPC_InteractionResult Result = EvaluateInteractionOutcome(InteractionData, Context);
    
    // Update behavior and memory based on result
    AdaptBehaviorBasedOnInteraction(InteractionData, Result);
    UpdateInteractionMemory(InteractionData.TargetActor.Get(), InteractionData.InteractionType, Result);
    
    // Set cooldown
    float CooldownDuration = 5.0f; // Base cooldown
    if (Result == ENPC_InteractionResult::Failed || Result == ENPC_InteractionResult::Escalated)
    {
        CooldownDuration *= 2.0f; // Longer cooldown for negative outcomes
    }
    
    InteractionCooldowns.Add(InteractionData.TargetActor, GetWorld()->GetTimeSeconds() + CooldownDuration);
}

ENPC_InteractionResult UNPC_InteractionBehaviorSystem::EvaluateInteractionOutcome(const FNPC_InteractionData& InteractionData, const FNPC_InteractionContext& Context)
{
    if (!InteractionData.TargetActor.IsValid())
    {
        return ENPC_InteractionResult::Failed;
    }
    
    // Calculate success probability based on various factors
    float SuccessProbability = 0.5f; // Base 50% success rate
    
    // Adjust based on interaction type
    switch (InteractionData.InteractionType)
    {
        case ENPC_InteractionType::Aggressive:
            SuccessProbability += Context.bHasPackSupport ? 0.3f : -0.2f;
            SuccessProbability -= Context.ThreatLevel * 0.1f;
            break;
        case ENPC_InteractionType::Social:
            SuccessProbability += Context.bIsInTerritory ? 0.2f : -0.1f;
            break;
        case ENPC_InteractionType::Territorial:
            SuccessProbability += Context.bIsInTerritory ? 0.4f : -0.3f;
            break;
        case ENPC_InteractionType::Curious:
            SuccessProbability += 0.1f; // Generally more likely to succeed
            break;
    }
    
    // Adjust based on environmental factors
    SuccessProbability -= Context.EnvironmentalStress * 0.2f;
    
    // Random factor
    float RandomFactor = FMath::RandRange(0.0f, 1.0f);
    
    if (RandomFactor <= SuccessProbability)
    {
        return ENPC_InteractionResult::Success;
    }
    else if (RandomFactor <= SuccessProbability + 0.2f)
    {
        return ENPC_InteractionResult::Interrupted;
    }
    else if (Context.ThreatLevel > 2 && InteractionData.InteractionType == ENPC_InteractionType::Aggressive)
    {
        return ENPC_InteractionResult::Escalated;
    }
    else
    {
        return ENPC_InteractionResult::Failed;
    }
}

FNPC_InteractionContext UNPC_InteractionBehaviorSystem::AnalyzeInteractionContext(AActor* TargetActor)
{
    FNPC_InteractionContext Context;
    
    if (!TargetActor || !GetOwner())
    {
        return Context;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find nearby actors
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != GetOwner() && Actor != TargetActor)
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance <= MaxInteractionRange)
                {
                    Context.NearbyActors.Add(Actor);
                    
                    // Check if it's a potential threat or ally
                    if (Actor->IsA<APawn>())
                    {
                        Context.ThreatLevel++;
                        
                        // Check if it's part of our pack/group
                        if (Actor->GetName().Contains(GetOwner()->GetName().Left(4))) // Simple pack detection
                        {
                            Context.bHasPackSupport = true;
                        }
                    }
                }
            }
        }
    }
    
    // Calculate environmental stress (simplified)
    Context.EnvironmentalStress = FMath::Clamp(Context.NearbyActors.Num() * 0.1f, 0.0f, 1.0f);
    
    // Time of day (simplified - could be enhanced with actual day/night cycle)
    Context.TimeOfDay = FMath::Fmod(GetWorld()->GetTimeSeconds() / 60.0f, 24.0f);
    
    // Territory check (simplified - within certain range of spawn location)
    Context.bIsInTerritory = FVector::Dist(OwnerLocation, FVector::ZeroVector) < 3000.0f;
    
    return Context;
}

bool UNPC_InteractionBehaviorSystem::ShouldInitiateInteraction(AActor* TargetActor, ENPC_InteractionType& OutInteractionType)
{
    if (!IsValidInteractionTarget(TargetActor))
    {
        return false;
    }
    
    FNPC_InteractionContext Context = AnalyzeInteractionContext(TargetActor);
    OutInteractionType = DetermineOptimalInteractionType(TargetActor, Context);
    
    // Calculate interaction probability based on context
    float InteractionProbability = 0.3f; // Base probability
    
    // Adjust based on various factors
    if (Context.bIsInTerritory)
    {
        InteractionProbability += 0.2f;
    }
    
    if (Context.ThreatLevel > 1)
    {
        InteractionProbability += 0.3f;
    }
    
    if (Context.EnvironmentalStress > 0.5f)
    {
        InteractionProbability -= 0.2f;
    }
    
    return FMath::RandRange(0.0f, 1.0f) <= InteractionProbability;
}

void UNPC_InteractionBehaviorSystem::AdaptBehaviorBasedOnInteraction(const FNPC_InteractionData& InteractionData, ENPC_InteractionResult Result)
{
    if (!EmotionalStateComponent)
    {
        return;
    }
    
    // Adjust emotional state based on interaction result
    switch (Result)
    {
        case ENPC_InteractionResult::Success:
            // Positive outcome - reduce stress, increase confidence
            break;
        case ENPC_InteractionResult::Failed:
            // Negative outcome - increase caution
            break;
        case ENPC_InteractionResult::Escalated:
            // Dangerous outcome - increase fear/aggression
            break;
        case ENPC_InteractionResult::Interrupted:
            // Neutral outcome - slight stress increase
            break;
    }
    
    if (bEnableInteractionLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Behavior adapted for %s based on %s interaction result"), 
            *GetOwner()->GetName(), 
            *UEnum::GetValueAsString(Result));
    }
}

void UNPC_InteractionBehaviorSystem::UpdateInteractionMemory(AActor* TargetActor, ENPC_InteractionType InteractionType, ENPC_InteractionResult Result)
{
    if (!SocialMemoryComponent || !TargetActor)
    {
        return;
    }
    
    // Update social memory with interaction outcome
    // This would integrate with the social memory system to remember past interactions
    
    if (bEnableInteractionLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Updated interaction memory: %s -> %s (%s result)"), 
            *GetOwner()->GetName(), 
            *TargetActor->GetName(),
            *UEnum::GetValueAsString(Result));
    }
}

float UNPC_InteractionBehaviorSystem::CalculateInteractionPriority(AActor* TargetActor, ENPC_InteractionType InteractionType)
{
    if (!TargetActor)
    {
        return 0.0f;
    }
    
    float Priority = 1.0f;
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
    
    // Distance factor (closer = higher priority)
    Priority *= FMath::Clamp(2000.0f / FMath::Max(Distance, 100.0f), 0.1f, 2.0f);
    
    // Interaction type factor
    switch (InteractionType)
    {
        case ENPC_InteractionType::Aggressive:
        case ENPC_InteractionType::Territorial:
            Priority *= 1.5f; // High priority for defensive behaviors
            break;
        case ENPC_InteractionType::Fearful:
            Priority *= 2.0f; // Highest priority for survival
            break;
        case ENPC_InteractionType::Social:
            Priority *= 0.8f; // Lower priority for social interactions
            break;
        case ENPC_InteractionType::Curious:
            Priority *= 0.6f; // Lowest priority for curiosity
            break;
    }
    
    return FMath::Clamp(Priority, 0.1f, 3.0f);
}

bool UNPC_InteractionBehaviorSystem::IsValidInteractionTarget(AActor* TargetActor)
{
    if (!TargetActor || TargetActor == GetOwner())
    {
        return false;
    }
    
    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
    if (Distance > MaxInteractionRange)
    {
        return false;
    }
    
    // Check if target is valid (not destroyed, etc.)
    if (TargetActor->IsPendingKill() || !IsValid(TargetActor))
    {
        return false;
    }
    
    return true;
}

void UNPC_InteractionBehaviorSystem::RegisterInteractionCallback(AActor* TargetActor)
{
    // This could be used to register callbacks for when interactions complete
    // Implementation would depend on the specific callback system used
}

void UNPC_InteractionBehaviorSystem::ProcessPendingInteractions(float DeltaTime)
{
    // Move pending interactions to active if we have capacity
    while (PendingInteractions.Num() > 0 && ActiveInteractions.Num() < MaxConcurrentInteractions)
    {
        FNPC_InteractionData Interaction = PendingInteractions[0];
        PendingInteractions.RemoveAt(0);
        
        if (IsValidInteractionTarget(Interaction.TargetActor.Get()))
        {
            ActiveInteractions.Add(Interaction);
            ProcessInteraction(Interaction);
        }
    }
}

void UNPC_InteractionBehaviorSystem::UpdateInteractionStates(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update active interactions
    for (int32 i = ActiveInteractions.Num() - 1; i >= 0; i--)
    {
        FNPC_InteractionData& Interaction = ActiveInteractions[i];
        
        // Check if interaction has expired
        if (CurrentTime - Interaction.Timestamp > Interaction.InteractionDuration)
        {
            if (bEnableInteractionLogging)
            {
                UE_LOG(LogTemp, Log, TEXT("Interaction expired: %s"), 
                    *UEnum::GetValueAsString(Interaction.InteractionType));
            }
            ActiveInteractions.RemoveAt(i);
        }
        // Check if target is still valid
        else if (!IsValidInteractionTarget(Interaction.TargetActor.Get()))
        {
            ActiveInteractions.RemoveAt(i);
        }
    }
}

void UNPC_InteractionBehaviorSystem::CleanupExpiredInteractions()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Clean up expired cooldowns
    TArray<TWeakObjectPtr<AActor>> ExpiredCooldowns;
    for (auto& Cooldown : InteractionCooldowns)
    {
        if (CurrentTime >= Cooldown.Value || !Cooldown.Key.IsValid())
        {
            ExpiredCooldowns.Add(Cooldown.Key);
        }
    }
    
    for (const auto& ExpiredActor : ExpiredCooldowns)
    {
        InteractionCooldowns.Remove(ExpiredActor);
    }
}

ENPC_InteractionType UNPC_InteractionBehaviorSystem::DetermineOptimalInteractionType(AActor* TargetActor, const FNPC_InteractionContext& Context)
{
    if (!TargetActor)
    {
        return ENPC_InteractionType::Neutral;
    }
    
    // Simple decision tree based on context
    if (Context.ThreatLevel > 2)
    {
        return Context.bHasPackSupport ? ENPC_InteractionType::Aggressive : ENPC_InteractionType::Fearful;
    }
    
    if (Context.bIsInTerritory && TargetActor->IsA<APawn>())
    {
        return ENPC_InteractionType::Territorial;
    }
    
    if (TargetActor->GetName().Contains("Player"))
    {
        return Context.EnvironmentalStress > 0.5f ? ENPC_InteractionType::Defensive : ENPC_InteractionType::Curious;
    }
    
    // Default to neutral for unknown targets
    return ENPC_InteractionType::Neutral;
}

bool UNPC_InteractionBehaviorSystem::ValidateInteractionConditions(const FNPC_InteractionData& InteractionData)
{
    if (!InteractionData.TargetActor.IsValid())
    {
        return false;
    }
    
    // Check distance
    float CurrentDistance = FVector::Dist(GetOwner()->GetActorLocation(), InteractionData.TargetActor->GetActorLocation());
    if (CurrentDistance > InteractionData.InteractionRange)
    {
        return false;
    }
    
    // Check line of sight if required
    if (InteractionData.bRequiresLineOfSight)
    {
        FHitResult HitResult;
        FVector Start = GetOwner()->GetActorLocation();
        FVector End = InteractionData.TargetActor->GetActorLocation();
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
        if (bHit && HitResult.GetActor() != InteractionData.TargetActor.Get())
        {
            return false; // Line of sight blocked
        }
    }
    
    return true;
}

void UNPC_InteractionBehaviorSystem::ExecuteInteractionBehavior(const FNPC_InteractionData& InteractionData)
{
    if (!InteractionData.TargetActor.IsValid())
    {
        return;
    }
    
    // Execute behavior based on interaction type
    switch (InteractionData.InteractionType)
    {
        case ENPC_InteractionType::Aggressive:
            // Move towards target, play aggressive animation
            break;
        case ENPC_InteractionType::Territorial:
            // Display territorial behavior, possibly move to block target
            break;
        case ENPC_InteractionType::Curious:
            // Move closer to investigate, play curious animation
            break;
        case ENPC_InteractionType::Fearful:
            // Move away from target, play fear animation
            break;
        case ENPC_InteractionType::Social:
            // Approach target in a non-threatening manner
            break;
        default:
            // Default neutral behavior
            break;
    }
    
    if (bEnableInteractionLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Executing %s interaction behavior"), 
            *UEnum::GetValueAsString(InteractionData.InteractionType));
    }
}