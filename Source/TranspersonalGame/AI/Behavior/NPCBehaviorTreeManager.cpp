#include "NPCBehaviorTreeManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorTreeManager::UNPCBehaviorTreeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    MemoryDuration = 30.0f;
    ThreatDetectionRange = 1500.0f;
    SocialInteractionRange = 800.0f;
    AlertDecayRate = 0.5f;
    MaxAlertLevel = 10.0f;
    
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
}

void UNPCBehaviorTreeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get AI Controller components
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
        {
            BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
            BlackboardComponent = AIController->GetBlackboardComponent();
        }
    }
    
    // Initialize behavior state
    BehaviorState.CurrentState = ENPC_NPCState::Idle;
    BehaviorState.StateTimer = 0.0f;
    BehaviorState.AlertLevel = 0.0f;
}

void UNPCBehaviorTreeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update behavior state timer
    BehaviorState.StateTimer += DeltaTime;
    
    // Update threat assessment
    UpdateThreatAssessment();
    
    // Process memory decay
    ProcessMemoryDecay(DeltaTime);
    
    // Update alert level
    UpdateAlertLevel(DeltaTime);
    
    // Update social relations
    UpdateSocialRelations();
    
    // Update blackboard with current state
    UpdateBehaviorTreeBlackboard();
}

void UNPCBehaviorTreeManager::InitializeBehaviorTree(UBehaviorTree* BehaviorTreeAsset)
{
    if (!BehaviorTreeAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCBehaviorTreeManager: No BehaviorTree asset provided"));
        return;
    }
    
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
        {
            AIController->RunBehaviorTree(BehaviorTreeAsset);
            BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
            BlackboardComponent = AIController->GetBlackboardComponent();
            
            UE_LOG(LogTemp, Log, TEXT("NPCBehaviorTreeManager: Behavior tree initialized for %s"), *GetOwner()->GetName());
        }
    }
}

void UNPCBehaviorTreeManager::SetBehaviorState(ENPC_NPCState NewState)
{
    if (BehaviorState.CurrentState != NewState)
    {
        ENPC_NPCState PreviousState = BehaviorState.CurrentState;
        BehaviorState.CurrentState = NewState;
        BehaviorState.StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("NPCBehaviorTreeManager: State changed from %d to %d for %s"), 
               (int32)PreviousState, (int32)NewState, *GetOwner()->GetName());
    }
}

ENPC_NPCState UNPCBehaviorTreeManager::GetCurrentBehaviorState() const
{
    return BehaviorState.CurrentState;
}

void UNPCBehaviorTreeManager::AddMemoryEntry(AActor* Target, FVector Location, float ThreatLevel, bool bHostile)
{
    if (!Target)
    {
        return;
    }
    
    // Check if we already have a memory entry for this target
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.TargetActor == Target)
        {
            // Update existing entry
            Entry.LastSeenLocation = Location;
            Entry.LastSeenTime = GetWorld()->GetTimeSeconds();
            Entry.ThreatLevel = ThreatLevel;
            Entry.bIsHostile = bHostile;
            return;
        }
    }
    
    // Create new memory entry
    FNPC_MemoryEntry NewEntry;
    NewEntry.TargetActor = Target;
    NewEntry.LastSeenLocation = Location;
    NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.bIsHostile = bHostile;
    
    MemoryEntries.Add(NewEntry);
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorTreeManager: Added memory entry for %s (Threat: %.1f)"), 
           *Target->GetName(), ThreatLevel);
}

void UNPCBehaviorTreeManager::UpdateMemoryEntry(AActor* Target, FVector NewLocation)
{
    if (!Target)
    {
        return;
    }
    
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.TargetActor == Target)
        {
            Entry.LastSeenLocation = NewLocation;
            Entry.LastSeenTime = GetWorld()->GetTimeSeconds();
            return;
        }
    }
}

FNPC_MemoryEntry UNPCBehaviorTreeManager::GetMemoryEntry(AActor* Target)
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.TargetActor == Target)
        {
            return Entry;
        }
    }
    
    return FNPC_MemoryEntry();
}

void UNPCBehaviorTreeManager::ForgetTarget(AActor* Target)
{
    MemoryEntries.RemoveAll([Target](const FNPC_MemoryEntry& Entry)
    {
        return Entry.TargetActor == Target;
    });
}

void UNPCBehaviorTreeManager::ClearOldMemories(float MaxAge)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    MemoryEntries.RemoveAll([CurrentTime, MaxAge](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.LastSeenTime) > MaxAge;
    });
}

float UNPCBehaviorTreeManager::CalculateThreatLevel(AActor* Target)
{
    if (!Target)
    {
        return 0.0f;
    }
    
    float ThreatLevel = 0.0f;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    float Distance = FVector::Dist(OwnerLocation, TargetLocation);
    
    // Base threat based on target type
    if (Target->IsA<APawn>())
    {
        ThreatLevel = 5.0f;
        
        // Higher threat for player characters
        if (Cast<APawn>(Target)->IsPlayerControlled())
        {
            ThreatLevel = 8.0f;
        }
    }
    
    // Distance modifier (closer = more threatening)
    if (Distance < ThreatDetectionRange)
    {
        float DistanceModifier = 1.0f - (Distance / ThreatDetectionRange);
        ThreatLevel *= (1.0f + DistanceModifier);
    }
    
    // Check if target is moving towards us
    if (APawn* TargetPawn = Cast<APawn>(Target))
    {
        FVector TargetVelocity = TargetPawn->GetVelocity();
        FVector DirectionToUs = (OwnerLocation - TargetLocation).GetSafeNormal();
        
        if (FVector::DotProduct(TargetVelocity.GetSafeNormal(), DirectionToUs) > 0.5f)
        {
            ThreatLevel *= 1.5f; // 50% increase if moving towards us
        }
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 10.0f);
}

void UNPCBehaviorTreeManager::UpdateThreatAssessment()
{
    if (!GetWorld())
    {
        return;
    }
    
    BehaviorState.NearbyThreats.Empty();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find all pawns within threat detection range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= ThreatDetectionRange)
        {
            float ThreatLevel = CalculateThreatLevel(Actor);
            if (ThreatLevel > 2.0f)
            {
                BehaviorState.NearbyThreats.Add(Actor);
                AddMemoryEntry(Actor, Actor->GetActorLocation(), ThreatLevel, ThreatLevel > 5.0f);
            }
        }
    }
    
    // Update alert level based on nearby threats
    if (BehaviorState.NearbyThreats.Num() > 0)
    {
        BehaviorState.AlertLevel = FMath::Min(BehaviorState.AlertLevel + 2.0f, MaxAlertLevel);
        
        // Update behavior state based on threat level
        AActor* HighestThreat = GetHighestThreat();
        if (HighestThreat)
        {
            float ThreatLevel = CalculateThreatLevel(HighestThreat);
            if (ThreatLevel > 7.0f)
            {
                SetBehaviorState(ENPC_NPCState::Combat);
            }
            else if (ThreatLevel > 4.0f)
            {
                SetBehaviorState(ENPC_NPCState::Alert);
            }
        }
    }
}

AActor* UNPCBehaviorTreeManager::GetHighestThreat()
{
    AActor* HighestThreat = nullptr;
    float HighestThreatLevel = 0.0f;
    
    for (AActor* Threat : BehaviorState.NearbyThreats)
    {
        if (!Threat)
        {
            continue;
        }
        
        float ThreatLevel = CalculateThreatLevel(Threat);
        if (ThreatLevel > HighestThreatLevel)
        {
            HighestThreatLevel = ThreatLevel;
            HighestThreat = Threat;
        }
    }
    
    return HighestThreat;
}

void UNPCBehaviorTreeManager::UpdateSocialRelations()
{
    // Find nearby NPCs for social interaction
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner() || !Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= SocialInteractionRange)
        {
            // Check if this NPC has behavior manager
            if (UNPCBehaviorTreeManager* OtherBehavior = Actor->FindComponentByClass<UNPCBehaviorTreeManager>())
            {
                // Share threat information
                if (BehaviorState.NearbyThreats.Num() > 0)
                {
                    for (AActor* Threat : BehaviorState.NearbyThreats)
                    {
                        OtherBehavior->AddMemoryEntry(Threat, Threat->GetActorLocation(), 
                                                    CalculateThreatLevel(Threat), true);
                    }
                }
            }
        }
    }
}

void UNPCBehaviorTreeManager::ReactToNearbyNPCs()
{
    // Implementation for NPC-to-NPC reactions
    // This can include flocking behavior, formation keeping, etc.
}

void UNPCBehaviorTreeManager::ShareThreatInformation(AActor* Threat, float Range)
{
    if (!Threat)
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner() || !Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= Range)
        {
            if (UNPCBehaviorTreeManager* OtherBehavior = Actor->FindComponentByClass<UNPCBehaviorTreeManager>())
            {
                float ThreatLevel = CalculateThreatLevel(Threat);
                OtherBehavior->AddMemoryEntry(Threat, Threat->GetActorLocation(), ThreatLevel, true);
            }
        }
    }
}

void UNPCBehaviorTreeManager::UpdateBehaviorTreeBlackboard()
{
    if (!BlackboardComponent)
    {
        return;
    }
    
    // Update blackboard with current behavior state
    BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), (uint8)BehaviorState.CurrentState);
    BlackboardComponent->SetValueAsFloat(TEXT("AlertLevel"), BehaviorState.AlertLevel);
    
    // Set highest threat as target
    AActor* HighestThreat = GetHighestThreat();
    if (HighestThreat)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), HighestThreat);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), HighestThreat->GetActorLocation());
        BehaviorState.LastKnownPlayerLocation = HighestThreat->GetActorLocation();
    }
    else
    {
        BlackboardComponent->ClearValue(TEXT("TargetActor"));
    }
    
    // Update last known player location
    BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), BehaviorState.LastKnownPlayerLocation);
}

void UNPCBehaviorTreeManager::ProcessMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove memories older than MemoryDuration
    MemoryEntries.RemoveAll([CurrentTime, this](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.LastSeenTime) > MemoryDuration;
    });
    
    // Decay threat levels over time
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        float TimeSinceLastSeen = CurrentTime - Entry.LastSeenTime;
        if (TimeSinceLastSeen > 5.0f) // Start decaying after 5 seconds
        {
            float DecayAmount = DeltaTime * 0.5f; // Decay rate
            Entry.ThreatLevel = FMath::Max(0.0f, Entry.ThreatLevel - DecayAmount);
        }
    }
}

void UNPCBehaviorTreeManager::UpdateAlertLevel(float DeltaTime)
{
    if (BehaviorState.NearbyThreats.Num() == 0)
    {
        // Decay alert level when no threats present
        BehaviorState.AlertLevel = FMath::Max(0.0f, BehaviorState.AlertLevel - (AlertDecayRate * DeltaTime));
        
        // Return to idle state when alert level is low
        if (BehaviorState.AlertLevel < 1.0f && BehaviorState.CurrentState != ENPC_NPCState::Idle)
        {
            SetBehaviorState(ENPC_NPCState::Idle);
        }
    }
}