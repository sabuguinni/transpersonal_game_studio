#include "NPC_BehaviorTreeManager.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNPC_BehaviorTreeManager::UNPC_BehaviorTreeManager()
{
    GlobalThreatLevel = 0.0f;
}

void UNPC_BehaviorTreeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultConfigs();
    
    // Start behavior update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            BehaviorUpdateTimer,
            this,
            &UNPC_BehaviorTreeManager::UpdateAllBehaviors,
            1.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NPC Behavior Tree Manager initialized"));
}

void UNPC_BehaviorTreeManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BehaviorUpdateTimer);
    }
    
    RegisteredControllers.Empty();
    Super::Deinitialize();
}

void UNPC_BehaviorTreeManager::InitializeDefaultConfigs()
{
    // T-Rex configuration
    FNPC_BehaviorTreeConfig TRexConfig;
    TRexConfig.UpdateInterval = 0.3f;
    TRexConfig.DetectionRadius = 3000.0f;
    TRexConfig.FleeRadius = 0.0f; // T-Rex doesn't flee
    BehaviorConfigs.Add(ENPCSpecies::TRex, TRexConfig);
    
    // Raptor configuration
    FNPC_BehaviorTreeConfig RaptorConfig;
    RaptorConfig.UpdateInterval = 0.2f;
    RaptorConfig.DetectionRadius = 2000.0f;
    RaptorConfig.FleeRadius = 4000.0f; // Raptors flee from T-Rex
    BehaviorConfigs.Add(ENPCSpecies::Raptor, RaptorConfig);
    
    // Herbivore configuration
    FNPC_BehaviorTreeConfig HerbivoreConfig;
    HerbivoreConfig.UpdateInterval = 0.5f;
    HerbivoreConfig.DetectionRadius = 1500.0f;
    HerbivoreConfig.FleeRadius = 2500.0f;
    BehaviorConfigs.Add(ENPCSpecies::Triceratops, HerbivoreConfig);
    BehaviorConfigs.Add(ENPCSpecies::Brachiosaurus, HerbivoreConfig);
}

void UNPC_BehaviorTreeManager::RegisterAIController(AAIController* Controller, ENPCSpecies Species)
{
    if (!Controller)
    {
        return;
    }
    
    RegisteredControllers.AddUnique(Controller);
    
    // Set up blackboard with species-specific values
    if (UBlackboardComponent* BlackboardComp = Controller->GetBlackboardComponent())
    {
        FNPC_BehaviorTreeConfig Config = GetBehaviorConfig(Species);
        BlackboardComp->SetValueAsFloat(TEXT("DetectionRadius"), Config.DetectionRadius);
        BlackboardComp->SetValueAsFloat(TEXT("FleeRadius"), Config.FleeRadius);
        BlackboardComp->SetValueAsEnum(TEXT("Species"), static_cast<uint8>(Species));
        BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(ENPCBehaviorState::Idle));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Registered AI Controller for species: %d"), static_cast<int32>(Species));
}

void UNPC_BehaviorTreeManager::UnregisterAIController(AAIController* Controller)
{
    RegisteredControllers.RemoveAll([Controller](const TWeakObjectPtr<AAIController>& WeakPtr)
    {
        return WeakPtr.Get() == Controller;
    });
}

void UNPC_BehaviorTreeManager::UpdateAllBehaviors()
{
    CleanupInvalidControllers();
    
    for (const TWeakObjectPtr<AAIController>& WeakController : RegisteredControllers)
    {
        if (AAIController* Controller = WeakController.Get())
        {
            UpdateControllerBehavior(Controller);
        }
    }
}

void UNPC_BehaviorTreeManager::UpdateControllerBehavior(AAIController* Controller)
{
    if (!Controller || !Controller->GetBlackboardComponent())
    {
        return;
    }
    
    UBlackboardComponent* BlackboardComp = Controller->GetBlackboardComponent();
    
    // Update global threat level
    BlackboardComp->SetValueAsFloat(TEXT("GlobalThreatLevel"), GlobalThreatLevel);
    
    // Find nearest player
    if (UWorld* World = GetWorld())
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn && Controller->GetPawn())
        {
            float Distance = FVector::Dist(Controller->GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
            BlackboardComp->SetValueAsFloat(TEXT("PlayerDistance"), Distance);
            BlackboardComp->SetValueAsVector(TEXT("PlayerLocation"), PlayerPawn->GetActorLocation());
            
            // Update behavior state based on distance
            float DetectionRadius = BlackboardComp->GetValueAsFloat(TEXT("DetectionRadius"));
            float FleeRadius = BlackboardComp->GetValueAsFloat(TEXT("FleeRadius"));
            
            ENPCBehaviorState CurrentState = static_cast<ENPCBehaviorState>(
                BlackboardComp->GetValueAsEnum(TEXT("BehaviorState"))
            );
            
            if (Distance <= DetectionRadius && CurrentState == ENPCBehaviorState::Idle)
            {
                BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(ENPCBehaviorState::Alert));
            }
            else if (Distance <= FleeRadius && FleeRadius > 0.0f)
            {
                BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(ENPCBehaviorState::Fleeing));
            }
            else if (Distance > DetectionRadius * 1.5f)
            {
                BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(ENPCBehaviorState::Idle));
            }
        }
    }
}

void UNPC_BehaviorTreeManager::SetGlobalThreatLevel(float ThreatLevel)
{
    GlobalThreatLevel = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
    
    // Immediately update all controllers with new threat level
    for (const TWeakObjectPtr<AAIController>& WeakController : RegisteredControllers)
    {
        if (AAIController* Controller = WeakController.Get())
        {
            if (UBlackboardComponent* BlackboardComp = Controller->GetBlackboardComponent())
            {
                BlackboardComp->SetValueAsFloat(TEXT("GlobalThreatLevel"), GlobalThreatLevel);
            }
        }
    }
}

FNPC_BehaviorTreeConfig UNPC_BehaviorTreeManager::GetBehaviorConfig(ENPCSpecies Species) const
{
    if (const FNPC_BehaviorTreeConfig* Config = BehaviorConfigs.Find(Species))
    {
        return *Config;
    }
    
    // Return default config if species not found
    return FNPC_BehaviorTreeConfig();
}

void UNPC_BehaviorTreeManager::TriggerPackBehavior(ENPCSpecies Species, const FVector& Location, ENPCBehaviorState NewState)
{
    // Find all controllers of the specified species within pack range
    const float PackRange = 5000.0f;
    
    for (const TWeakObjectPtr<AAIController>& WeakController : RegisteredControllers)
    {
        if (AAIController* Controller = WeakController.Get())
        {
            if (UBlackboardComponent* BlackboardComp = Controller->GetBlackboardComponent())
            {
                ENPCSpecies ControllerSpecies = static_cast<ENPCSpecies>(
                    BlackboardComp->GetValueAsEnum(TEXT("Species"))
                );
                
                if (ControllerSpecies == Species && Controller->GetPawn())
                {
                    float Distance = FVector::Dist(Controller->GetPawn()->GetActorLocation(), Location);
                    if (Distance <= PackRange)
                    {
                        BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
                        BlackboardComp->SetValueAsVector(TEXT("PackTargetLocation"), Location);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Triggered pack behavior for species %d at location %s"), 
           static_cast<int32>(Species), *Location.ToString());
}

void UNPC_BehaviorTreeManager::CleanupInvalidControllers()
{
    RegisteredControllers.RemoveAll([](const TWeakObjectPtr<AAIController>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });
}