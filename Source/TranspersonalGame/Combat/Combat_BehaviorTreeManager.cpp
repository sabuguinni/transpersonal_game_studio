#include "Combat_BehaviorTreeManager.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UCombat_BehaviorTreeManager::UCombat_BehaviorTreeManager()
{
    RegisteredBehaviorTrees.Empty();
    AIMemoryMap.Empty();
    PackMembersMap.Empty();
}

void UCombat_BehaviorTreeManager::RegisterBehaviorTree(UBehaviorTree* BehaviorTree, EDinosaurSpecies Species, ECombatState CombatState, float Priority)
{
    if (!BehaviorTree)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCombat_BehaviorTreeManager::RegisterBehaviorTree - Invalid behavior tree"));
        return;
    }

    FCombat_BehaviorTreeConfig Config;
    Config.BehaviorTree = BehaviorTree;
    Config.TargetSpecies = Species;
    Config.RequiredCombatState = CombatState;
    Config.Priority = Priority;

    RegisteredBehaviorTrees.Add(Config);
    
    UE_LOG(LogTemp, Log, TEXT("Registered behavior tree for species %d, combat state %d"), 
           (int32)Species, (int32)CombatState);
}

UBehaviorTree* UCombat_BehaviorTreeManager::GetBehaviorTreeForState(EDinosaurSpecies Species, ECombatState CombatState)
{
    return FindBestBehaviorTree(Species, CombatState);
}

void UCombat_BehaviorTreeManager::SwitchBehaviorTree(AAIController* AIController, UBehaviorTree* NewBehaviorTree)
{
    if (!IsValidAIController(AIController) || !NewBehaviorTree)
    {
        return;
    }

    UBehaviorTreeComponent* BehaviorComp = AIController->GetBehaviorTreeComponent();
    if (BehaviorComp)
    {
        BehaviorComp->StopTree(EBTStopMode::Safe);
        BehaviorComp->StartTree(*NewBehaviorTree);
        
        UE_LOG(LogTemp, Log, TEXT("Switched behavior tree for AI controller %s"), 
               *AIController->GetName());
    }
}

void UCombat_BehaviorTreeManager::UpdateAIMemory(AAIController* AIController, const FCombat_AIMemory& NewMemory)
{
    if (!IsValidAIController(AIController))
    {
        return;
    }

    AIMemoryMap.Add(AIController, NewMemory);
    
    // Update blackboard with memory data
    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsVector(TEXT("LastKnownPlayerLocation"), NewMemory.LastKnownPlayerLocation);
        BlackboardComp->SetValueAsFloat(TEXT("TimeSincePlayerSeen"), NewMemory.TimeSincePlayerSeen);
        BlackboardComp->SetValueAsInt(TEXT("CurrentWaypointIndex"), NewMemory.CurrentWaypointIndex);
        BlackboardComp->SetValueAsBool(TEXT("PlayerIsHostile"), NewMemory.bPlayerIsHostile);
        BlackboardComp->SetValueAsFloat(TEXT("ThreatLevel"), NewMemory.ThreatLevel);
    }
}

FCombat_AIMemory UCombat_BehaviorTreeManager::GetAIMemory(AAIController* AIController)
{
    if (AIMemoryMap.Contains(AIController))
    {
        return AIMemoryMap[AIController];
    }
    
    return FCombat_AIMemory();
}

void UCombat_BehaviorTreeManager::SetBlackboardValue(AAIController* AIController, const FString& KeyName, const FString& Value)
{
    if (!IsValidAIController(AIController))
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsString(*KeyName, Value);
    }
}

void UCombat_BehaviorTreeManager::SetBlackboardVector(AAIController* AIController, const FString& KeyName, FVector Value)
{
    if (!IsValidAIController(AIController))
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsVector(*KeyName, Value);
    }
}

void UCombat_BehaviorTreeManager::SetBlackboardFloat(AAIController* AIController, const FString& KeyName, float Value)
{
    if (!IsValidAIController(AIController))
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsFloat(*KeyName, Value);
    }
}

void UCombat_BehaviorTreeManager::SetBlackboardBool(AAIController* AIController, const FString& KeyName, bool Value)
{
    if (!IsValidAIController(AIController))
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsBool(*KeyName, Value);
    }
}

ECombatState UCombat_BehaviorTreeManager::DetermineOptimalCombatState(AAIController* AIController, EDinosaurSpecies Species)
{
    if (!IsValidAIController(AIController))
    {
        return ECombatState::Neutral;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return ECombatState::Neutral;
    }

    // Get player reference
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIController->GetWorld(), 0);
    if (!PlayerPawn)
    {
        return ECombatState::Neutral;
    }

    float DistanceToPlayer = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
    FCombat_AIMemory Memory = GetAIMemory(AIController);

    // Determine combat state based on species behavior and distance
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            if (DistanceToPlayer < 2000.0f) // 20 meters
            {
                return Memory.bPlayerIsHostile ? ECombatState::Attacking : ECombatState::Aggressive;
            }
            else if (DistanceToPlayer < 5000.0f) // 50 meters
            {
                return ECombatState::Alert;
            }
            break;

        case EDinosaurSpecies::Raptor:
            if (IsInPackFormation(AIController))
            {
                if (DistanceToPlayer < 1500.0f) // 15 meters
                {
                    return ECombatState::Attacking;
                }
                else if (DistanceToPlayer < 3000.0f) // 30 meters
                {
                    return ECombatState::Hunting;
                }
            }
            else
            {
                if (DistanceToPlayer < 1000.0f) // 10 meters
                {
                    return ECombatState::Fleeing;
                }
            }
            break;

        case EDinosaurSpecies::Brachiosaurus:
            if (DistanceToPlayer < 1000.0f && Memory.bPlayerIsHostile)
            {
                return ECombatState::Defending;
            }
            else if (DistanceToPlayer < 2000.0f)
            {
                return ECombatState::Alert;
            }
            break;

        default:
            break;
    }

    return ECombatState::Neutral;
}

bool UCombat_BehaviorTreeManager::ShouldEngageInCombat(AAIController* AIController, AActor* Target)
{
    if (!IsValidAIController(AIController) || !Target)
    {
        return false;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return false;
    }

    float ThreatLevel = CalculateThreatLevel(AIController, Target);
    FCombat_AIMemory Memory = GetAIMemory(AIController);

    // Decision based on threat level and AI memory
    if (ThreatLevel > 0.7f || Memory.bPlayerIsHostile)
    {
        return true;
    }

    // Pack animals are more likely to engage
    if (IsInPackFormation(AIController) && ThreatLevel > 0.4f)
    {
        return true;
    }

    return false;
}

FVector UCombat_BehaviorTreeManager::CalculateOptimalAttackPosition(AAIController* AIController, AActor* Target)
{
    if (!IsValidAIController(AIController) || !Target)
    {
        return FVector::ZeroVector;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector PawnLocation = ControlledPawn->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal();

    // Calculate optimal attack distance based on species
    float OptimalDistance = 200.0f; // Default 2 meters

    // Position slightly to the side for flanking
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    FVector FlankingOffset = RightVector * FMath::RandRange(-300.0f, 300.0f);

    return TargetLocation - (DirectionToTarget * OptimalDistance) + FlankingOffset;
}

bool UCombat_BehaviorTreeManager::IsInPackFormation(AAIController* AIController)
{
    if (!IsValidAIController(AIController))
    {
        return false;
    }

    // Check if this AI is a pack leader
    if (PackMembersMap.Contains(AIController))
    {
        return PackMembersMap[AIController].Num() > 0;
    }

    // Check if this AI is a pack member
    for (auto& PackPair : PackMembersMap)
    {
        if (PackPair.Value.Contains(AIController))
        {
            return true;
        }
    }

    return false;
}

void UCombat_BehaviorTreeManager::RegisterPackMember(AAIController* PackLeader, AAIController* PackMember)
{
    if (!IsValidAIController(PackLeader) || !IsValidAIController(PackMember))
    {
        return;
    }

    if (!PackMembersMap.Contains(PackLeader))
    {
        PackMembersMap.Add(PackLeader, TArray<AAIController*>());
    }

    PackMembersMap[PackLeader].AddUnique(PackMember);
    
    UE_LOG(LogTemp, Log, TEXT("Registered pack member %s under leader %s"), 
           *PackMember->GetName(), *PackLeader->GetName());
}

void UCombat_BehaviorTreeManager::RemovePackMember(AAIController* PackLeader, AAIController* PackMember)
{
    if (PackMembersMap.Contains(PackLeader))
    {
        PackMembersMap[PackLeader].Remove(PackMember);
        
        if (PackMembersMap[PackLeader].Num() == 0)
        {
            PackMembersMap.Remove(PackLeader);
        }
    }
}

TArray<AAIController*> UCombat_BehaviorTreeManager::GetPackMembers(AAIController* PackLeader)
{
    if (PackMembersMap.Contains(PackLeader))
    {
        return PackMembersMap[PackLeader];
    }
    
    return TArray<AAIController*>();
}

void UCombat_BehaviorTreeManager::CoordinatePackAttack(AAIController* PackLeader, AActor* Target)
{
    if (!IsValidAIController(PackLeader) || !Target)
    {
        return;
    }

    TArray<AAIController*> PackMembers = GetPackMembers(PackLeader);
    if (PackMembers.Num() == 0)
    {
        return;
    }

    FVector TargetLocation = Target->GetActorLocation();
    
    // Assign flanking positions to pack members
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        AAIController* Member = PackMembers[i];
        if (!Member)
        {
            continue;
        }

        float Angle = (360.0f / PackMembers.Num()) * i;
        FVector FlankingDirection = FVector(FMath::Cos(FMath::DegreesToRadians(Angle)), 
                                          FMath::Sin(FMath::DegreesToRadians(Angle)), 0.0f);
        FVector FlankingPosition = TargetLocation + (FlankingDirection * 500.0f);

        SetBlackboardVector(Member, TEXT("AttackPosition"), FlankingPosition);
        SetBlackboardBool(Member, TEXT("IsCoordinatedAttack"), true);
    }

    UE_LOG(LogTemp, Log, TEXT("Coordinated pack attack with %d members"), PackMembers.Num());
}

void UCombat_BehaviorTreeManager::InitializeDefaultBehaviorTrees()
{
    // This would typically load behavior trees from assets
    // For now, we'll just log that initialization is complete
    UE_LOG(LogTemp, Log, TEXT("UCombat_BehaviorTreeManager::InitializeDefaultBehaviorTrees - Default trees initialized"));
}

void UCombat_BehaviorTreeManager::CleanupAIMemory()
{
    // Remove invalid AI controllers from memory
    TArray<AAIController*> ControllersToRemove;
    
    for (auto& MemoryPair : AIMemoryMap)
    {
        if (!IsValidAIController(MemoryPair.Key))
        {
            ControllersToRemove.Add(MemoryPair.Key);
        }
    }

    for (AAIController* Controller : ControllersToRemove)
    {
        AIMemoryMap.Remove(Controller);
        PackMembersMap.Remove(Controller);
    }

    UE_LOG(LogTemp, Log, TEXT("Cleaned up %d invalid AI controllers from memory"), ControllersToRemove.Num());
}

UBehaviorTree* UCombat_BehaviorTreeManager::FindBestBehaviorTree(EDinosaurSpecies Species, ECombatState CombatState)
{
    UBehaviorTree* BestTree = nullptr;
    float BestPriority = -1.0f;

    for (const FCombat_BehaviorTreeConfig& Config : RegisteredBehaviorTrees)
    {
        if (Config.TargetSpecies == Species && Config.RequiredCombatState == CombatState)
        {
            if (Config.Priority > BestPriority)
            {
                BestTree = Config.BehaviorTree;
                BestPriority = Config.Priority;
            }
        }
    }

    return BestTree;
}

void UCombat_BehaviorTreeManager::UpdateBlackboardForCombatState(AAIController* AIController, ECombatState NewState)
{
    if (!IsValidAIController(AIController))
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum(TEXT("CombatState"), (uint8)NewState);
    }
}

float UCombat_BehaviorTreeManager::CalculateThreatLevel(AAIController* AIController, AActor* Target)
{
    if (!IsValidAIController(AIController) || !Target)
    {
        return 0.0f;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
    float ThreatLevel = FMath::Clamp(1.0f - (Distance / 5000.0f), 0.0f, 1.0f); // Max threat at 0 distance, min at 50m

    FCombat_AIMemory Memory = GetAIMemory(AIController);
    if (Memory.bPlayerIsHostile)
    {
        ThreatLevel *= 1.5f; // Increase threat if player is known to be hostile
    }

    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

bool UCombat_BehaviorTreeManager::IsValidAIController(AAIController* AIController)
{
    return AIController && IsValid(AIController) && !AIController->IsPendingKill();
}