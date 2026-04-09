// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "DinosaurBehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

// ============================================================================
// UBTTask_FindFoodSource Implementation
// ============================================================================

UBTTask_FindFoodSource::UBTTask_FindFoodSource()
{
    NodeName = TEXT("Find Food Source");
    SearchRadius = 1500.0f;
    FoodTags.Add(TEXT("Food"));
    FoodTags.Add(TEXT("Vegetation"));
    FoodTags.Add(TEXT("Fruit"));
}

EBTNodeResult::Type UBTTask_FindFoodSource::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    UWorld* World = ControlledPawn->GetWorld();
    FVector PawnLocation = ControlledPawn->GetActorLocation();

    // Find all actors within search radius with food tags
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

    AActor* BestFoodSource = nullptr;
    float BestDistance = SearchRadius + 1.0f;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor) continue;

        // Check if actor has any food tags
        bool bHasFoodTag = false;
        for (const FString& FoodTag : FoodTags)
        {
            if (Actor->Tags.Contains(FName(*FoodTag)))
            {
                bHasFoodTag = true;
                break;
            }
        }

        if (!bHasFoodTag) continue;

        float Distance = FVector::Dist(PawnLocation, Actor->GetActorLocation());
        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            BestFoodSource = Actor;
        }
    }

    // Set the food location in blackboard
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp && BestFoodSource)
    {
        BlackboardComp->SetValueAsVector(FoodLocationKey.SelectedKeyName, BestFoodSource->GetActorLocation());
        BlackboardComp->SetValueAsObject(TEXT("TargetFoodActor"), BestFoodSource);

        UE_LOG(LogTemp, Log, TEXT("Found food source at distance: %f"), BestDistance);
        return EBTNodeResult::Succeeded;
    }

    // If no food found, generate random foraging location
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
    if (NavSystem)
    {
        FNavLocation RandomLocation;
        if (NavSystem->GetRandomReachablePointInRadius(PawnLocation, SearchRadius * 0.5f, RandomLocation))
        {
            BlackboardComp->SetValueAsVector(FoodLocationKey.SelectedKeyName, RandomLocation.Location);
            UE_LOG(LogTemp, Log, TEXT("Generated random foraging location"));
            return EBTNodeResult::Succeeded;
        }
    }

    return EBTNodeResult::Failed;
}

FString UBTTask_FindFoodSource::GetStaticDescription() const
{
    return FString::Printf(TEXT("Find food source within %f units"), SearchRadius);
}

// ============================================================================
// UBTTask_PatrolTerritory Implementation
// ============================================================================

UBTTask_PatrolTerritory::UBTTask_PatrolTerritory()
{
    NodeName = TEXT("Patrol Territory");
    PatrolRadius = 800.0f;
    MinPatrolPoints = 3;
    MaxPatrolPoints = 6;
}

EBTNodeResult::Type UBTTask_PatrolTerritory::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    UNPCBehaviorCore* BehaviorCore = ControlledPawn->FindComponentByClass<UNPCBehaviorCore>();
    
    if (!BehaviorCore)
    {
        return EBTNodeResult::Failed;
    }

    FVector TerritoryCenter = BehaviorCore->TerritoryCenter;
    float TerritoryRadius = FMath::Min(BehaviorCore->TerritoryRadius, PatrolRadius);

    // Generate random patrol point within territory
    UWorld* World = ControlledPawn->GetWorld();
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
    
    if (NavSystem)
    {
        FNavLocation RandomLocation;
        if (NavSystem->GetRandomReachablePointInRadius(TerritoryCenter, TerritoryRadius, RandomLocation))
        {
            UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsVector(PatrolPointKey.SelectedKeyName, RandomLocation.Location);
                UE_LOG(LogTemp, Log, TEXT("Generated patrol point at: %s"), *RandomLocation.Location.ToString());
                return EBTNodeResult::Succeeded;
            }
        }
    }

    return EBTNodeResult::Failed;
}

FString UBTTask_PatrolTerritory::GetStaticDescription() const
{
    return FString::Printf(TEXT("Generate patrol point within %f units of territory"), PatrolRadius);
}

// ============================================================================
// UBTTask_SocialInteraction Implementation
// ============================================================================

UBTTask_SocialInteraction::UBTTask_SocialInteraction()
{
    NodeName = TEXT("Social Interaction");
    InteractionRadius = 300.0f;
    InteractionDuration = 5.0f;
    bRequiresSameSpecies = true;
}

EBTNodeResult::Type UBTTask_SocialInteraction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    UNPCBehaviorCore* BehaviorCore = ControlledPawn->FindComponentByClass<UNPCBehaviorCore>();
    
    if (!BehaviorCore)
    {
        return EBTNodeResult::Failed;
    }

    UWorld* World = ControlledPawn->GetWorld();
    FVector PawnLocation = ControlledPawn->GetActorLocation();

    // Find nearby NPCs for social interaction
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    AActor* BestSocialTarget = nullptr;
    float BestRelationship = -2.0f; // Start below minimum relationship

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == ControlledPawn) continue;

        APawn* OtherPawn = Cast<APawn>(Actor);
        if (!OtherPawn) continue;

        UNPCBehaviorCore* OtherBehaviorCore = OtherPawn->FindComponentByClass<UNPCBehaviorCore>();
        if (!OtherBehaviorCore) continue;

        float Distance = FVector::Dist(PawnLocation, Actor->GetActorLocation());
        if (Distance > InteractionRadius) continue;

        // Check species compatibility if required
        if (bRequiresSameSpecies)
        {
            if (ControlledPawn->GetClass() != OtherPawn->GetClass()) continue;
        }

        // Check relationship
        float Relationship = BehaviorCore->GetRelationshipWith(Actor);
        if (Relationship > BestRelationship)
        {
            BestRelationship = Relationship;
            BestSocialTarget = Actor;
        }
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp && BestSocialTarget)
    {
        BlackboardComp->SetValueAsObject(TargetActorKey.SelectedKeyName, BestSocialTarget);
        
        // Improve relationship through interaction
        BehaviorCore->UpdateSocialRelationship(BestSocialTarget, 0.05f);
        
        // Reduce social need
        BehaviorCore->SocialNeed = FMath::Max(0.0f, BehaviorCore->SocialNeed - 0.2f);
        
        UE_LOG(LogTemp, Log, TEXT("Starting social interaction with %s"), *BestSocialTarget->GetName());
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}

FString UBTTask_SocialInteraction::GetStaticDescription() const
{
    return FString::Printf(TEXT("Interact socially within %f units for %f seconds"), InteractionRadius, InteractionDuration);
}

// ============================================================================
// UBTTask_FleeFromThreat Implementation
// ============================================================================

UBTTask_FleeFromThreat::UBTTask_FleeFromThreat()
{
    NodeName = TEXT("Flee From Threat");
    MinFleeDistance = 800.0f;
    MaxFleeDistance = 1500.0f;
}

EBTNodeResult::Type UBTTask_FleeFromThreat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !AIController->GetPawn() || !BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    AActor* ThreatActor = Cast<AActor>(BlackboardComp->GetValueAsObject(ThreatActorKey.SelectedKeyName));
    
    if (!ThreatActor)
    {
        return EBTNodeResult::Failed;
    }

    FVector PawnLocation = ControlledPawn->GetActorLocation();
    FVector ThreatLocation = ThreatActor->GetActorLocation();
    
    // Calculate flee direction (opposite to threat)
    FVector FleeDirection = (PawnLocation - ThreatLocation).GetSafeNormal();
    float FleeDistance = FMath::RandRange(MinFleeDistance, MaxFleeDistance);
    FVector FleeLocation = PawnLocation + (FleeDirection * FleeDistance);

    // Find navigable location
    UWorld* World = ControlledPawn->GetWorld();
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
    
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(FleeLocation, NavLocation, FVector(500.0f)))
        {
            BlackboardComp->SetValueAsVector(FleeLocationKey.SelectedKeyName, NavLocation.Location);
            
            // Update NPC behavior
            UNPCBehaviorCore* BehaviorCore = ControlledPawn->FindComponentByClass<UNPCBehaviorCore>();
            if (BehaviorCore)
            {
                BehaviorCore->AddMemoryEntry(ThreatLocation, TEXT("Fled from dangerous threat"), 0.8f, false);
                BehaviorCore->StressLevel = FMath::Min(1.0f, BehaviorCore->StressLevel + 0.3f);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Fleeing from threat to location: %s"), *NavLocation.Location.ToString());
            return EBTNodeResult::Succeeded;
        }
    }

    return EBTNodeResult::Failed;
}

FString UBTTask_FleeFromThreat::GetStaticDescription() const
{
    return FString::Printf(TEXT("Flee %f-%f units from threat"), MinFleeDistance, MaxFleeDistance);
}

// ============================================================================
// UBTService_UpdateNeeds Implementation
// ============================================================================

UBTService_UpdateNeeds::UBTService_UpdateNeeds()
{
    NodeName = TEXT("Update Needs");
    Interval = 1.0f; // Update every second
    RandomDeviation = 0.2f;
}

void UBTService_UpdateNeeds::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !AIController->GetPawn() || !BlackboardComp)
    {
        return;
    }

    UNPCBehaviorCore* BehaviorCore = AIController->GetPawn()->FindComponentByClass<UNPCBehaviorCore>();
    if (!BehaviorCore)
    {
        return;
    }

    // Update blackboard with current need levels
    BlackboardComp->SetValueAsFloat(HungerLevelKey.SelectedKeyName, BehaviorCore->HungerLevel);
    BlackboardComp->SetValueAsFloat(EnergyLevelKey.SelectedKeyName, BehaviorCore->EnergyLevel);
    BlackboardComp->SetValueAsFloat(StressLevelKey.SelectedKeyName, BehaviorCore->StressLevel);
    BlackboardComp->SetValueAsFloat(SocialNeedKey.SelectedKeyName, BehaviorCore->SocialNeed);

    // Update current mood and activity
    BlackboardComp->SetValueAsEnum(TEXT("CurrentMood"), static_cast<uint8>(BehaviorCore->CurrentMood));
    BlackboardComp->SetValueAsEnum(TEXT("CurrentActivity"), static_cast<uint8>(BehaviorCore->CurrentActivity));
    BlackboardComp->SetValueAsEnum(TEXT("Personality"), static_cast<uint8>(BehaviorCore->Personality));
}

// ============================================================================
// UBTService_ScanEnvironment Implementation
// ============================================================================

UBTService_ScanEnvironment::UBTService_ScanEnvironment()
{
    NodeName = TEXT("Scan Environment");
    Interval = 2.0f; // Scan every 2 seconds
    RandomDeviation = 0.5f;
    ScanRadius = 1000.0f;
    
    ThreatTags.Add(TEXT("Predator"));
    ThreatTags.Add(TEXT("Threat"));
    ThreatTags.Add(TEXT("Player"));
    
    FoodTags.Add(TEXT("Food"));
    FoodTags.Add(TEXT("Vegetation"));
    
    AllyTags.Add(TEXT("Herbivore"));
    AllyTags.Add(TEXT("Ally"));
}

void UBTService_ScanEnvironment::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !AIController->GetPawn() || !BlackboardComp)
    {
        return;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    UWorld* World = ControlledPawn->GetWorld();
    FVector PawnLocation = ControlledPawn->GetActorLocation();

    // Scan for different types of actors
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

    AActor* NearestThreat = nullptr;
    AActor* NearestFood = nullptr;
    AActor* NearestAlly = nullptr;
    
    float NearestThreatDistance = ScanRadius + 1.0f;
    float NearestFoodDistance = ScanRadius + 1.0f;
    float NearestAllyDistance = ScanRadius + 1.0f;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == ControlledPawn) continue;

        float Distance = FVector::Dist(PawnLocation, Actor->GetActorLocation());
        if (Distance > ScanRadius) continue;

        // Check for threats
        for (const FString& ThreatTag : ThreatTags)
        {
            if (Actor->Tags.Contains(FName(*ThreatTag)) && Distance < NearestThreatDistance)
            {
                NearestThreatDistance = Distance;
                NearestThreat = Actor;
                break;
            }
        }

        // Check for food
        for (const FString& FoodTag : FoodTags)
        {
            if (Actor->Tags.Contains(FName(*FoodTag)) && Distance < NearestFoodDistance)
            {
                NearestFoodDistance = Distance;
                NearestFood = Actor;
                break;
            }
        }

        // Check for allies
        for (const FString& AllyTag : AllyTags)
        {
            if (Actor->Tags.Contains(FName(*AllyTag)) && Distance < NearestAllyDistance)
            {
                NearestAllyDistance = Distance;
                NearestAlly = Actor;
                break;
            }
        }
    }

    // Update blackboard with scan results
    BlackboardComp->SetValueAsObject(NearbyThreatsKey.SelectedKeyName, NearestThreat);
    BlackboardComp->SetValueAsObject(NearbyFoodKey.SelectedKeyName, NearestFood);
    BlackboardComp->SetValueAsObject(NearbyAlliesKey.SelectedKeyName, NearestAlly);

    // React to threats
    if (NearestThreat)
    {
        UNPCBehaviorCore* BehaviorCore = ControlledPawn->FindComponentByClass<UNPCBehaviorCore>();
        if (BehaviorCore)
        {
            BehaviorCore->ReactToPlayerPresence(NearestThreat, NearestThreatDistance);
        }
    }
}

// ============================================================================
// Decorator Implementations
// ============================================================================

UBTDecorator_CheckPersonality::UBTDecorator_CheckPersonality()
{
    NodeName = TEXT("Check Personality");
    RequiredPersonality = ENPCPersonality::Peaceful;
    bInvertCondition = false;
}

bool UBTDecorator_CheckPersonality::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return false;
    }

    UNPCBehaviorCore* BehaviorCore = AIController->GetPawn()->FindComponentByClass<UNPCBehaviorCore>();
    if (!BehaviorCore)
    {
        return false;
    }

    bool bConditionMet = (BehaviorCore->Personality == RequiredPersonality);
    return bInvertCondition ? !bConditionMet : bConditionMet;
}

FString UBTDecorator_CheckPersonality::GetStaticDescription() const
{
    return FString::Printf(TEXT("Check if personality is %s%s"), 
                          bInvertCondition ? TEXT("NOT ") : TEXT(""),
                          *UEnum::GetValueAsString(RequiredPersonality));
}

UBTDecorator_CheckMoodState::UBTDecorator_CheckMoodState()
{
    NodeName = TEXT("Check Mood State");
    RequiredMood = ENPCMoodState::Calm;
    bInvertCondition = false;
}

bool UBTDecorator_CheckMoodState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return false;
    }

    UNPCBehaviorCore* BehaviorCore = AIController->GetPawn()->FindComponentByClass<UNPCBehaviorCore>();
    if (!BehaviorCore)
    {
        return false;
    }

    bool bConditionMet = (BehaviorCore->CurrentMood == RequiredMood);
    return bInvertCondition ? !bConditionMet : bConditionMet;
}

FString UBTDecorator_CheckMoodState::GetStaticDescription() const
{
    return FString::Printf(TEXT("Check if mood is %s%s"), 
                          bInvertCondition ? TEXT("NOT ") : TEXT(""),
                          *UEnum::GetValueAsString(RequiredMood));
}

UBTDecorator_CheckTerritory::UBTDecorator_CheckTerritory()
{
    NodeName = TEXT("Check Territory");
    bMustBeInTerritory = true;
}

bool UBTDecorator_CheckTerritory::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    
    if (!AIController || !AIController->GetPawn() || !BlackboardComp)
    {
        return false;
    }

    UNPCBehaviorCore* BehaviorCore = AIController->GetPawn()->FindComponentByClass<UNPCBehaviorCore>();
    if (!BehaviorCore)
    {
        return false;
    }

    FVector CheckLocation;
    if (LocationKey.SelectedKeyName != NAME_None)
    {
        CheckLocation = BlackboardComp->GetValueAsVector(LocationKey.SelectedKeyName);
    }
    else
    {
        CheckLocation = AIController->GetPawn()->GetActorLocation();
    }

    bool bInTerritory = BehaviorCore->IsInTerritory(CheckLocation);
    return bMustBeInTerritory ? bInTerritory : !bInTerritory;
}

FString UBTDecorator_CheckTerritory::GetStaticDescription() const
{
    return FString::Printf(TEXT("Check if %s territory"), bMustBeInTerritory ? TEXT("inside") : TEXT("outside"));
}

UBTDecorator_CheckTimeOfDay::UBTDecorator_CheckTimeOfDay()
{
    NodeName = TEXT("Check Time of Day");
    StartTime = 6.0f;
    EndTime = 18.0f;
    bInvertCondition = false;
}

bool UBTDecorator_CheckTimeOfDay::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return false;
    }

    UNPCBehaviorCore* BehaviorCore = AIController->GetPawn()->FindComponentByClass<UNPCBehaviorCore>();
    if (!BehaviorCore)
    {
        return false;
    }

    float CurrentTime = BehaviorCore->CurrentDayTime;
    bool bInTimeRange;
    
    if (StartTime <= EndTime)
    {
        // Normal time range (e.g., 6:00 to 18:00)
        bInTimeRange = (CurrentTime >= StartTime && CurrentTime <= EndTime);
    }
    else
    {
        // Overnight range (e.g., 22:00 to 6:00)
        bInTimeRange = (CurrentTime >= StartTime || CurrentTime <= EndTime);
    }

    return bInvertCondition ? !bInTimeRange : bInTimeRange;
}

FString UBTDecorator_CheckTimeOfDay::GetStaticDescription() const
{
    return FString::Printf(TEXT("Check if time is %s%.1f:00-%.1f:00"), 
                          bInvertCondition ? TEXT("NOT ") : TEXT(""),
                          StartTime, EndTime);
}