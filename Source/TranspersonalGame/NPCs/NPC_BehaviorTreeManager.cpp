#include "NPC_BehaviorTreeManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNPC_BehaviorTreeManager::UNPC_BehaviorTreeManager()
{
    InitializeSpeciesTemplates();
}

void UNPC_BehaviorTreeManager::InitializeDinosaurBehavior(AAIController* AIController, EDinosaurSpecies Species)
{
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC_BehaviorTreeManager: Invalid AI Controller"));
        return;
    }

    // Get behavior template for species
    FNPC_DinosaurBehaviorData* BehaviorTemplate = SpeciesBehaviorTemplates.Find(Species);
    if (!BehaviorTemplate)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC_BehaviorTreeManager: No behavior template for species %d"), (int32)Species);
        return;
    }

    // Configure perception
    ConfigurePerception(AIController, *BehaviorTemplate);

    // Initialize blackboard with default values
    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        InitializeBlackboardKeys(BlackboardComp);
        
        // Set species-specific values
        SetBlackboardFloat(BlackboardComp, "TerritoryRadius", BehaviorTemplate->TerritoryRadius);
        SetBlackboardFloat(BlackboardComp, "AggressionLevel", BehaviorTemplate->AggressionLevel);
        SetBlackboardFloat(BlackboardComp, "PatrolSpeed", BehaviorTemplate->PatrolSpeed);
        SetBlackboardFloat(BlackboardComp, "ChaseSpeed", BehaviorTemplate->ChaseSpeed);
        SetBlackboardBool(BlackboardComp, "IsPackHunter", BehaviorTemplate->bIsPackHunter);
        SetBlackboardFloat(BlackboardComp, "RestDuration", BehaviorTemplate->RestDuration);
    }

    // Set initial territory around spawn location
    if (APawn* ControlledPawn = AIController->GetPawn())
    {
        FVector SpawnLocation = ControlledPawn->GetActorLocation();
        SetTerritoryCenter(AIController, SpawnLocation, BehaviorTemplate->TerritoryRadius);
    }

    UE_LOG(LogTemp, Log, TEXT("NPC_BehaviorTreeManager: Initialized behavior for species %d"), (int32)Species);
}

void UNPC_BehaviorTreeManager::SetBehaviorState(AAIController* AIController, ENPCBehaviorState NewState)
{
    if (!AIController)
        return;

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        SetBlackboardFloat(BlackboardComp, "BehaviorState", (float)NewState);
        
        // Log state change
        FString StateName;
        switch (NewState)
        {
        case ENPCBehaviorState::Idle:
            StateName = "Idle";
            break;
        case ENPCBehaviorState::Patrolling:
            StateName = "Patrolling";
            break;
        case ENPCBehaviorState::Investigating:
            StateName = "Investigating";
            break;
        case ENPCBehaviorState::Chasing:
            StateName = "Chasing";
            break;
        case ENPCBehaviorState::Attacking:
            StateName = "Attacking";
            break;
        case ENPCBehaviorState::Fleeing:
            StateName = "Fleeing";
            break;
        case ENPCBehaviorState::Resting:
            StateName = "Resting";
            break;
        default:
            StateName = "Unknown";
        }
        
        UE_LOG(LogTemp, Log, TEXT("NPC Behavior State Changed: %s"), *StateName);
    }
}

void UNPC_BehaviorTreeManager::UpdatePatrolRoute(AAIController* AIController, const TArray<FNPC_PatrolPoint>& PatrolPoints)
{
    if (!AIController || PatrolPoints.Num() == 0)
        return;

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (BlackboardComp)
    {
        // Set first patrol point as current target
        SetBlackboardVector(BlackboardComp, "PatrolTarget", PatrolPoints[0].Location);
        SetBlackboardFloat(BlackboardComp, "PatrolWaitTime", PatrolPoints[0].WaitTime);
        SetBlackboardFloat(BlackboardComp, "PatrolIndex", 0.0f);
        
        UE_LOG(LogTemp, Log, TEXT("NPC_BehaviorTreeManager: Updated patrol route with %d points"), PatrolPoints.Num());
    }
}

void UNPC_BehaviorTreeManager::FormPack(const TArray<AAIController*>& PackMembers, AAIController* PackLeader)
{
    if (!PackLeader || PackMembers.Num() == 0)
        return;

    // Register pack formation
    ActivePacks.Add(PackLeader, PackMembers);

    // Set pack data in blackboards
    for (AAIController* Member : PackMembers)
    {
        if (Member && Member->GetBlackboardComponent())
        {
            UBlackboardComponent* BlackboardComp = Member->GetBlackboardComponent();
            SetBlackboardObject(BlackboardComp, "PackLeader", PackLeader);
            SetBlackboardBool(BlackboardComp, "InPack", true);
            SetBlackboardFloat(BlackboardComp, "PackSize", PackMembers.Num());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("NPC_BehaviorTreeManager: Formed pack with %d members"), PackMembers.Num());
}

void UNPC_BehaviorTreeManager::DisbandPack(AAIController* PackMember)
{
    if (!PackMember)
        return;

    // Find and remove from active packs
    for (auto& PackPair : ActivePacks)
    {
        TArray<AAIController*>& Members = PackPair.Value;
        if (Members.Contains(PackMember))
        {
            Members.Remove(PackMember);
            
            // Clear pack data from blackboard
            if (UBlackboardComponent* BlackboardComp = PackMember->GetBlackboardComponent())
            {
                SetBlackboardObject(BlackboardComp, "PackLeader", nullptr);
                SetBlackboardBool(BlackboardComp, "InPack", false);
                SetBlackboardFloat(BlackboardComp, "PackSize", 1.0f);
            }
            
            UE_LOG(LogTemp, Log, TEXT("NPC_BehaviorTreeManager: Removed member from pack"));
            break;
        }
    }
}

void UNPC_BehaviorTreeManager::SetTerritoryCenter(AAIController* AIController, const FVector& Center, float Radius)
{
    if (!AIController)
        return;

    TerritoryCenters.Add(AIController, Center);
    TerritoryRadii.Add(AIController, Radius);

    // Update blackboard
    if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
    {
        SetBlackboardVector(BlackboardComp, "TerritoryCenter", Center);
        SetBlackboardFloat(BlackboardComp, "TerritoryRadius", Radius);
    }

    UE_LOG(LogTemp, Log, TEXT("NPC_BehaviorTreeManager: Set territory at %s with radius %f"), *Center.ToString(), Radius);
}

bool UNPC_BehaviorTreeManager::IsInTerritory(AAIController* AIController, const FVector& Location)
{
    if (!AIController)
        return false;

    FVector* Center = TerritoryCenters.Find(AIController);
    float* Radius = TerritoryRadii.Find(AIController);

    if (Center && Radius)
    {
        float Distance = FVector::Dist(*Center, Location);
        return Distance <= *Radius;
    }

    return false;
}

void UNPC_BehaviorTreeManager::ConfigurePerception(AAIController* AIController, const FNPC_DinosaurBehaviorData& BehaviorData)
{
    if (!AIController)
        return;

    SetupPerceptionComponent(AIController, BehaviorData);
}

float UNPC_BehaviorTreeManager::CalculateThreatLevel(AAIController* AIController, AActor* Target)
{
    if (!AIController || !Target)
        return 0.0f;

    float ThreatLevel = 0.0f;
    APawn* ControlledPawn = AIController->GetPawn();
    
    if (ControlledPawn)
    {
        float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
        
        // Base threat based on proximity (closer = more threatening)
        ThreatLevel = FMath::Clamp(1.0f - (Distance / 5000.0f), 0.0f, 1.0f);
        
        // Increase threat if target is player character
        if (Target->IsA<ACharacter>())
        {
            ThreatLevel += 0.3f;
        }
        
        // Modify by aggression level
        UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
        if (BlackboardComp)
        {
            float AggressionLevel = BlackboardComp->GetValueAsFloat("AggressionLevel");
            ThreatLevel *= AggressionLevel;
        }
    }

    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

void UNPC_BehaviorTreeManager::SetBlackboardVector(UBlackboardComponent* Blackboard, const FName& KeyName, const FVector& Value)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsVector(KeyName, Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardObject(UBlackboardComponent* Blackboard, const FName& KeyName, UObject* Value)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsObject(KeyName, Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardBool(UBlackboardComponent* Blackboard, const FName& KeyName, bool Value)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsBool(KeyName, Value);
    }
}

void UNPC_BehaviorTreeManager::SetBlackboardFloat(UBlackboardComponent* Blackboard, const FName& KeyName, float Value)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsFloat(KeyName, Value);
    }
}

void UNPC_BehaviorTreeManager::InitializeSpeciesTemplates()
{
    // T-Rex - Apex predator, territorial, solitary
    FNPC_DinosaurBehaviorData TRexBehavior;
    TRexBehavior.TerritoryRadius = 8000.0f;
    TRexBehavior.AggressionLevel = 0.9f;
    TRexBehavior.PatrolSpeed = 400.0f;
    TRexBehavior.ChaseSpeed = 800.0f;
    TRexBehavior.SightRange = 4000.0f;
    TRexBehavior.HearingRange = 2000.0f;
    TRexBehavior.bIsPackHunter = false;
    TRexBehavior.PackSize = 1;
    TRexBehavior.RestDuration = 45.0f;
    TRexBehavior.HuntingCooldown = 180.0f;
    SpeciesBehaviorTemplates.Add(EDinosaurSpecies::TRex, TRexBehavior);

    // Velociraptor - Pack hunter, high aggression, coordinated
    FNPC_DinosaurBehaviorData RaptorBehavior;
    RaptorBehavior.TerritoryRadius = 6000.0f;
    RaptorBehavior.AggressionLevel = 0.8f;
    RaptorBehavior.PatrolSpeed = 500.0f;
    RaptorBehavior.ChaseSpeed = 900.0f;
    RaptorBehavior.SightRange = 3500.0f;
    RaptorBehavior.HearingRange = 1800.0f;
    RaptorBehavior.bIsPackHunter = true;
    RaptorBehavior.PackSize = 3;
    RaptorBehavior.RestDuration = 20.0f;
    RaptorBehavior.HuntingCooldown = 90.0f;
    SpeciesBehaviorTemplates.Add(EDinosaurSpecies::Velociraptor, RaptorBehavior);

    // Triceratops - Defensive herbivore, moderate aggression when threatened
    FNPC_DinosaurBehaviorData TriceratopsBehavior;
    TriceratopsBehavior.TerritoryRadius = 4000.0f;
    TriceratopsBehavior.AggressionLevel = 0.4f;
    TriceratopsBehavior.PatrolSpeed = 200.0f;
    TriceratopsBehavior.ChaseSpeed = 400.0f;
    TriceratopsBehavior.SightRange = 2500.0f;
    TriceratopsBehavior.HearingRange = 1200.0f;
    TriceratopsBehavior.bIsPackHunter = false;
    TriceratopsBehavior.PackSize = 1;
    TriceratopsBehavior.RestDuration = 60.0f;
    TriceratopsBehavior.HuntingCooldown = 0.0f; // Herbivore
    SpeciesBehaviorTemplates.Add(EDinosaurSpecies::Triceratops, TriceratopsBehavior);

    UE_LOG(LogTemp, Log, TEXT("NPC_BehaviorTreeManager: Initialized species behavior templates"));
}

void UNPC_BehaviorTreeManager::SetupPerceptionComponent(AAIController* AIController, const FNPC_DinosaurBehaviorData& BehaviorData)
{
    if (!AIController)
        return;

    UAIPerceptionComponent* PerceptionComp = AIController->GetPerceptionComponent();
    if (!PerceptionComp)
    {
        PerceptionComp = AIController->CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    }

    if (PerceptionComp)
    {
        // Configure sight sense
        UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
        if (SightConfig)
        {
            SightConfig->SightRadius = BehaviorData.SightRange;
            SightConfig->LoseSightRadius = BehaviorData.SightRange * 1.2f;
            SightConfig->PeripheralVisionAngleDegrees = 90.0f;
            SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
            SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
            SightConfig->DetectionByAffiliation.bDetectEnemies = true;
            
            PerceptionComp->ConfigureSense(*SightConfig);
        }

        // Configure hearing sense
        UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
        if (HearingConfig)
        {
            HearingConfig->HearingRange = BehaviorData.HearingRange;
            HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
            HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
            HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
            
            PerceptionComp->ConfigureSense(*HearingConfig);
        }

        // Set dominant sense
        PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    }
}

void UNPC_BehaviorTreeManager::InitializeBlackboardKeys(UBlackboardComponent* Blackboard)
{
    if (!Blackboard)
        return;

    // Initialize common blackboard keys with default values
    SetBlackboardFloat(Blackboard, "BehaviorState", (float)ENPCBehaviorState::Idle);
    SetBlackboardVector(Blackboard, "PatrolTarget", FVector::ZeroVector);
    SetBlackboardVector(Blackboard, "TerritoryCenter", FVector::ZeroVector);
    SetBlackboardObject(Blackboard, "TargetActor", nullptr);
    SetBlackboardObject(Blackboard, "PackLeader", nullptr);
    SetBlackboardBool(Blackboard, "InPack", false);
    SetBlackboardBool(Blackboard, "IsPackHunter", false);
    SetBlackboardFloat(Blackboard, "ThreatLevel", 0.0f);
    SetBlackboardFloat(Blackboard, "LastSeenTime", 0.0f);
    SetBlackboardVector(Blackboard, "LastKnownLocation", FVector::ZeroVector);
}