#include "CombatAIArchitecture.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombatAIComponent::UCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default values
    CurrentStamina = Capabilities.StaminaMax;
}

void UCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize combat state
    SetCombatState(ECombatState::Passive);
    CurrentThreatLevel = EThreatLevel::None;
    
    UE_LOG(LogTemp, Log, TEXT("CombatAI Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
    {
        return;
    }
    
    // Update combat logic
    UpdateCombatLogic(DeltaTime);
    
    // Regenerate stamina
    if (CurrentStamina < Capabilities.StaminaMax)
    {
        CurrentStamina = FMath::Min(CurrentStamina + (Capabilities.StaminaRegenRate * DeltaTime), 
                                   Capabilities.StaminaMax);
    }
}

void UCombatAIComponent::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState == NewState)
    {
        return;
    }
    
    ECombatState OldState = CurrentCombatState;
    CurrentCombatState = NewState;
    StateChangeTime = GetWorld()->GetTimeSeconds();
    
    // Update gameplay tags based on combat state
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag("Combat.State.Passive"));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag("Combat.State.Investigating"));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag("Combat.State.Stalking"));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag("Combat.State.Hunting"));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag("Combat.State.Attacking"));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag("Combat.State.Retreating"));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag("Combat.State.Defending"));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag("Combat.State.Fleeing"));
    
    switch (NewState)
    {
        case ECombatState::Passive:
            CombatTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.State.Passive"));
            break;
        case ECombatState::Investigating:
            CombatTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.State.Investigating"));
            break;
        case ECombatState::Stalking:
            CombatTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.State.Stalking"));
            break;
        case ECombatState::Hunting:
            CombatTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.State.Hunting"));
            break;
        case ECombatState::Attacking:
            CombatTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.State.Attacking"));
            break;
        case ECombatState::Retreating:
            CombatTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.State.Retreating"));
            break;
        case ECombatState::Defending:
            CombatTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.State.Defending"));
            break;
        case ECombatState::Fleeing:
            CombatTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.State.Fleeing"));
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s: Combat state changed from %d to %d"), 
           *GetOwner()->GetName(), (int32)OldState, (int32)NewState);
    
    // Fire Blueprint event
    OnCombatStateChanged(OldState, NewState);
}

void UCombatAIComponent::AddThreat(AActor* Threat, EThreatLevel ThreatLevel)
{
    if (!Threat || KnownThreats.Contains(Threat))
    {
        return;
    }
    
    KnownThreats.AddUnique(Threat);
    
    // Update current threat level if this is more severe
    if (ThreatLevel > CurrentThreatLevel)
    {
        CurrentThreatLevel = ThreatLevel;
    }
    
    // Set as current target if we don't have one or this is a higher priority threat
    if (!CurrentTarget || ThreatLevel > EvaluateThreatLevel(CurrentTarget))
    {
        CurrentTarget = Threat;
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s: Added threat %s with level %d"), 
           *GetOwner()->GetName(), *Threat->GetName(), (int32)ThreatLevel);
    
    // Fire Blueprint event
    OnThreatDetected(Threat, ThreatLevel);
    
    // Transition to appropriate combat state
    if (CurrentCombatState == ECombatState::Passive)
    {
        if (Personality.Curiosity > 0.5f)
        {
            SetCombatState(ECombatState::Investigating);
        }
        else if (Personality.Aggression > 0.7f)
        {
            SetCombatState(ECombatState::Stalking);
        }
    }
}

void UCombatAIComponent::RemoveThreat(AActor* Threat)
{
    if (!Threat)
    {
        return;
    }
    
    KnownThreats.Remove(Threat);
    
    if (CurrentTarget == Threat)
    {
        CurrentTarget = nullptr;
        OnTargetLost();
        
        // Find new target from remaining threats
        if (KnownThreats.Num() > 0)
        {
            EThreatLevel HighestThreat = EThreatLevel::None;
            for (AActor* RemainingThreat : KnownThreats)
            {
                EThreatLevel ThreatLevel = EvaluateThreatLevel(RemainingThreat);
                if (ThreatLevel > HighestThreat)
                {
                    HighestThreat = ThreatLevel;
                    CurrentTarget = RemainingThreat;
                }
            }
            CurrentThreatLevel = HighestThreat;
        }
        else
        {
            CurrentThreatLevel = EThreatLevel::None;
            SetCombatState(ECombatState::Passive);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s: Removed threat %s"), 
           *GetOwner()->GetName(), *Threat->GetName());
}

EThreatLevel UCombatAIComponent::EvaluateThreatLevel(AActor* PotentialThreat)
{
    if (!PotentialThreat)
    {
        return EThreatLevel::None;
    }
    
    float Distance = CalculateDistance(PotentialThreat);
    bool bIsPlayer = PotentialThreat->IsA<APawn>() && 
                     Cast<APawn>(PotentialThreat)->GetController() && 
                     Cast<APawn>(PotentialThreat)->GetController()->IsA<APlayerController>();
    
    // Base threat level calculation
    EThreatLevel ThreatLevel = EThreatLevel::Minimal;
    
    // Players are always considered higher threat
    if (bIsPlayer)
    {
        ThreatLevel = EThreatLevel::Medium;
    }
    
    // Distance modifies threat level
    if (Distance < Capabilities.AttackRange)
    {
        ThreatLevel = static_cast<EThreatLevel>(FMath::Min((int32)ThreatLevel + 2, (int32)EThreatLevel::Lethal));
    }
    else if (Distance < Capabilities.DetectionRange * 0.5f)
    {
        ThreatLevel = static_cast<EThreatLevel>(FMath::Min((int32)ThreatLevel + 1, (int32)EThreatLevel::Critical));
    }
    
    // Personality affects threat evaluation
    if (Personality.Courage < 0.3f)
    {
        // Cowardly creatures see everything as more threatening
        ThreatLevel = static_cast<EThreatLevel>(FMath::Min((int32)ThreatLevel + 1, (int32)EThreatLevel::Lethal));
    }
    else if (Personality.Aggression > 0.8f)
    {
        // Aggressive creatures are less concerned about threats
        ThreatLevel = static_cast<EThreatLevel>(FMath::Max((int32)ThreatLevel - 1, (int32)EThreatLevel::None));
    }
    
    return ThreatLevel;
}

bool UCombatAIComponent::ShouldEngageTarget(AActor* Target)
{
    if (!Target)
    {
        return false;
    }
    
    // Check if we have enough stamina for combat
    if (CurrentStamina < Capabilities.StaminaMax * 0.3f)
    {
        return false;
    }
    
    float Distance = CalculateDistance(Target);
    EThreatLevel ThreatLevel = EvaluateThreatLevel(Target);
    
    // Base engagement decision on personality and threat level
    float EngagementThreshold = 0.5f;
    
    // Modify threshold based on personality
    EngagementThreshold += (Personality.Aggression - 0.5f) * 0.4f;
    EngagementThreshold -= (Personality.Courage - 0.5f) * 0.2f;
    
    // Modify based on combat role
    switch (CombatRole)
    {
        case EDinosaurCombatRole::Predator:
            EngagementThreshold -= 0.2f; // More likely to engage
            break;
        case EDinosaurCombatRole::PackHunter:
            if (PackMembers.Num() > 0)
            {
                EngagementThreshold -= 0.3f; // Much more likely with pack support
            }
            break;
        case EDinosaurCombatRole::Territorial:
            if (Distance < Capabilities.DetectionRange * 0.5f) // In territory
            {
                EngagementThreshold -= 0.4f; // Very likely to defend territory
            }
            break;
        case EDinosaurCombatRole::Herbivore:
            EngagementThreshold += 0.3f; // Less likely to engage
            break;
        case EDinosaurCombatRole::Scavenger:
            EngagementThreshold += 0.1f; // Slightly less likely
            break;
    }
    
    // Random factor for unpredictability
    float RandomFactor = FMath::RandRange(-0.1f, 0.1f);
    
    return (Personality.Aggression + RandomFactor) > EngagementThreshold;
}

bool UCombatAIComponent::ShouldRetreat()
{
    // Check health/stamina status
    if (CurrentStamina < Capabilities.StaminaMax * 0.2f)
    {
        return true;
    }
    
    // Check if outnumbered (for pack hunters)
    if (CombatRole == EDinosaurCombatRole::PackHunter && PackMembers.Num() == 0 && KnownThreats.Num() > 1)
    {
        return true;
    }
    
    // Personality-based retreat decision
    float RetreatThreshold = 1.0f - Personality.Courage;
    
    // Modify based on current threat level
    switch (CurrentThreatLevel)
    {
        case EThreatLevel::Critical:
            RetreatThreshold += 0.3f;
            break;
        case EThreatLevel::Lethal:
            RetreatThreshold += 0.5f;
            break;
        default:
            break;
    }
    
    return FMath::RandRange(0.0f, 1.0f) < RetreatThreshold;
}

FVector UCombatAIComponent::GetOptimalAttackPosition(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return GetOwner()->GetActorLocation();
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate direction to target
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Base position at attack range
    FVector BasePosition = TargetLocation - (DirectionToTarget * Capabilities.AttackRange * 0.8f);
    
    // Modify based on combat role and personality
    switch (CombatRole)
    {
        case EDinosaurCombatRole::Ambusher:
            // Try to find cover or concealment
            // For now, add some randomness to simulate finding hiding spots
            BasePosition += FVector(FMath::RandRange(-300.0f, 300.0f), 
                                   FMath::RandRange(-300.0f, 300.0f), 0.0f);
            break;
            
        case EDinosaurCombatRole::PackHunter:
            // Position to flank if pack members are present
            if (PackMembers.Num() > 0)
            {
                FVector FlankDirection = FVector(-DirectionToTarget.Y, DirectionToTarget.X, 0.0f);
                BasePosition += FlankDirection * 200.0f;
            }
            break;
            
        case EDinosaurCombatRole::Territorial:
            // Stay closer to territory center (for now, stay closer to current position)
            BasePosition = FMath::Lerp(BasePosition, OwnerLocation, 0.3f);
            break;
            
        default:
            // Direct approach for predators
            break;
    }
    
    return BasePosition;
}

void UCombatAIComponent::CallForPackSupport()
{
    if (!Capabilities.bCanCallForHelp)
    {
        return;
    }
    
    // Find nearby pack members within hearing range
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> NearbyActors;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Simple sphere overlap to find nearby actors
    // In a real implementation, this would use proper collision detection
    for (TActorIterator<APawn> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        APawn* Pawn = *ActorIterator;
        if (Pawn && Pawn != GetOwner())
        {
            float Distance = FVector::Dist(Pawn->GetActorLocation(), OwnerLocation);
            if (Distance <= Capabilities.HearingRange)
            {
                // Check if this pawn has the same combat AI component and is of same species
                UCombatAIComponent* OtherCombatAI = Pawn->FindComponentByClass<UCombatAIComponent>();
                if (OtherCombatAI && OtherCombatAI->CombatRole == CombatRole)
                {
                    // Add current target as threat to pack member
                    if (CurrentTarget)
                    {
                        OtherCombatAI->AddThreat(CurrentTarget, CurrentThreatLevel);
                    }
                    
                    PackMembers.AddUnique(Pawn);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s: Called for pack support, %d members responded"), 
           *GetOwner()->GetName(), PackMembers.Num());
}

void UCombatAIComponent::UpdateCombatLogic(float DeltaTime)
{
    if (!GetOwner())
    {
        return;
    }
    
    UpdateThreatAssessment();
    UpdatePackCoordination();
    
    // State-specific logic
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    
    switch (CurrentCombatState)
    {
        case ECombatState::Passive:
            // Look for threats
            if (KnownThreats.Num() > 0)
            {
                SetCombatState(ECombatState::Investigating);
            }
            break;
            
        case ECombatState::Investigating:
            if (CurrentTarget && TimeSinceStateChange > 2.0f)
            {
                if (ShouldEngageTarget(CurrentTarget))
                {
                    SetCombatState(ECombatState::Stalking);
                }
                else if (Personality.Courage < 0.4f)
                {
                    SetCombatState(ECombatState::Fleeing);
                }
                else
                {
                    SetCombatState(ECombatState::Passive);
                }
            }
            break;
            
        case ECombatState::Stalking:
            if (CurrentTarget)
            {
                float Distance = CalculateDistance(CurrentTarget);
                if (Distance < Capabilities.AttackRange * 1.2f)
                {
                    SetCombatState(ECombatState::Attacking);
                }
                else if (TimeSinceStateChange > 10.0f && Personality.Persistence < 0.5f)
                {
                    SetCombatState(ECombatState::Passive);
                }
            }
            break;
            
        case ECombatState::Attacking:
            if (ShouldRetreat())
            {
                SetCombatState(ECombatState::Retreating);
            }
            else if (!CurrentTarget || CalculateDistance(CurrentTarget) > Capabilities.DetectionRange)
            {
                SetCombatState(ECombatState::Passive);
            }
            break;
            
        case ECombatState::Retreating:
            if (TimeSinceStateChange > 5.0f && CurrentThreatLevel <= EThreatLevel::Low)
            {
                SetCombatState(ECombatState::Passive);
            }
            break;
            
        case ECombatState::Fleeing:
            if (TimeSinceStateChange > 8.0f && KnownThreats.Num() == 0)
            {
                SetCombatState(ECombatState::Passive);
            }
            break;
    }
}

void UCombatAIComponent::UpdateThreatAssessment()
{
    // Remove invalid or distant threats
    for (int32 i = KnownThreats.Num() - 1; i >= 0; i--)
    {
        AActor* Threat = KnownThreats[i];
        if (!IsValid(Threat))
        {
            KnownThreats.RemoveAt(i);
            continue;
        }
        
        float Distance = CalculateDistance(Threat);
        if (Distance > Capabilities.DetectionRange * 1.5f) // Give some buffer
        {
            RemoveThreat(Threat);
        }
    }
    
    // Update current threat level based on remaining threats
    CurrentThreatLevel = EThreatLevel::None;
    for (AActor* Threat : KnownThreats)
    {
        EThreatLevel ThreatLevel = EvaluateThreatLevel(Threat);
        if (ThreatLevel > CurrentThreatLevel)
        {
            CurrentThreatLevel = ThreatLevel;
        }
    }
}

void UCombatAIComponent::UpdatePackCoordination()
{
    if (!Capabilities.bCanPackHunt)
    {
        return;
    }
    
    // Remove invalid pack members
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(PackMembers[i]) || CalculateDistance(PackMembers[i]) > Capabilities.HearingRange)
        {
            PackMembers.RemoveAt(i);
        }
    }
    
    // Share threat information with pack
    for (AActor* PackMember : PackMembers)
    {
        UCombatAIComponent* PackMemberAI = PackMember->FindComponentByClass<UCombatAIComponent>();
        if (PackMemberAI && CurrentTarget && !PackMemberAI->KnownThreats.Contains(CurrentTarget))
        {
            PackMemberAI->AddThreat(CurrentTarget, CurrentThreatLevel);
        }
    }
}

float UCombatAIComponent::CalculateDistance(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return MAX_FLT;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
}

bool UCombatAIComponent::HasLineOfSight(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    
    return !bHit; // No hit means clear line of sight
}

bool UCombatAIComponent::IsInAttackRange(AActor* Target)
{
    if (!Target)
    {
        return false;
    }
    
    float Distance = CalculateDistance(Target);
    return Distance <= Capabilities.AttackRange;
}