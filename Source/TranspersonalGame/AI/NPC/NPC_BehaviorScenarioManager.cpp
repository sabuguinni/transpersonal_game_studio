#include "NPC_BehaviorScenarioManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UNPC_BehaviorScenarioManager::UNPC_BehaviorScenarioManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second
    
    ScenarioUpdateInterval = 1.0f;
    MaxConcurrentScenarios = 5.0f;
}

void UNPC_BehaviorScenarioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeScenarioManager();
    
    // Start scenario update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ScenarioUpdateTimer,
            this,
            &UNPC_BehaviorScenarioManager::UpdateActiveScenarios,
            ScenarioUpdateInterval,
            true
        );
    }
}

void UNPC_BehaviorScenarioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessScenarioTriggers();
}

void UNPC_BehaviorScenarioManager::InitializeScenarioManager()
{
    UE_LOG(LogTemp, Log, TEXT("NPC Behavior Scenario Manager initialized"));
    
    // Clear existing data
    RegisteredScenarios.Empty();
    ActiveScenarios.Empty();
    TerritorialCenters.Empty();
    TerritorialRadii.Empty();
    
    // Register default scenarios
    FNPC_BehaviorScenario TRexTerritorialScenario;
    TRexTerritorialScenario.ScenarioName = TEXT("TRex_Territorial_Defense");
    TRexTerritorialScenario.ScenarioType = ENPC_BehaviorScenarioType::TerritorialConflict;
    TRexTerritorialScenario.TriggerLocation = FVector(2000, 2000, 300);
    TRexTerritorialScenario.TriggerRadius = 3000.0f;
    TRexTerritorialScenario.ScenarioDuration = 120.0f;
    TRexTerritorialScenario.Priority = 5;
    RegisterBehaviorScenario(TRexTerritorialScenario);
    
    FNPC_BehaviorScenario RaptorPackHunt;
    RaptorPackHunt.ScenarioName = TEXT("Raptor_Pack_Hunt");
    RaptorPackHunt.ScenarioType = ENPC_BehaviorScenarioType::PackHunting;
    RaptorPackHunt.TriggerLocation = FVector(-2000, 2000, 300);
    RaptorPackHunt.TriggerRadius = 2500.0f;
    RaptorPackHunt.ScenarioDuration = 90.0f;
    RaptorPackHunt.Priority = 4;
    RegisterBehaviorScenario(RaptorPackHunt);
    
    FNPC_BehaviorScenario WateringHoleSocial;
    WateringHoleSocial.ScenarioName = TEXT("WateringHole_Social");
    WateringHoleSocial.ScenarioType = ENPC_BehaviorScenarioType::SocialInteraction;
    WateringHoleSocial.TriggerLocation = FVector(0, 3000, 200);
    WateringHoleSocial.TriggerRadius = 1500.0f;
    WateringHoleSocial.ScenarioDuration = 180.0f;
    WateringHoleSocial.Priority = 2;
    RegisterBehaviorScenario(WateringHoleSocial);
}

void UNPC_BehaviorScenarioManager::RegisterBehaviorScenario(const FNPC_BehaviorScenario& NewScenario)
{
    RegisteredScenarios.Add(NewScenario);
    UE_LOG(LogTemp, Log, TEXT("Registered behavior scenario: %s"), *NewScenario.ScenarioName);
}

void UNPC_BehaviorScenarioManager::TriggerScenario(const FString& ScenarioName)
{
    // Find scenario in registered list
    for (FNPC_BehaviorScenario& Scenario : RegisteredScenarios)
    {
        if (Scenario.ScenarioName == ScenarioName && !Scenario.bIsActive)
        {
            if (CanActivateScenario(Scenario))
            {
                Scenario.bIsActive = true;
                ActiveScenarios.Add(Scenario);
                
                // Set timer for scenario duration
                if (UWorld* World = GetWorld())
                {
                    FTimerHandle& ScenarioTimer = ScenarioTimers.FindOrAdd(ScenarioName);
                    World->GetTimerManager().SetTimer(
                        ScenarioTimer,
                        [this, ScenarioName]() { EndScenario(ScenarioName); },
                        Scenario.ScenarioDuration,
                        false
                    );
                }
                
                ExecuteScenarioBehavior(Scenario);
                UE_LOG(LogTemp, Warning, TEXT("Triggered scenario: %s"), *ScenarioName);
                return;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Could not trigger scenario: %s"), *ScenarioName);
}

void UNPC_BehaviorScenarioManager::EndScenario(const FString& ScenarioName)
{
    // Remove from active scenarios
    ActiveScenarios.RemoveAll([&ScenarioName](const FNPC_BehaviorScenario& Scenario) {
        return Scenario.ScenarioName == ScenarioName;
    });
    
    // Mark as inactive in registered scenarios
    for (FNPC_BehaviorScenario& Scenario : RegisteredScenarios)
    {
        if (Scenario.ScenarioName == ScenarioName)
        {
            Scenario.bIsActive = false;
            break;
        }
    }
    
    // Clear timer
    if (FTimerHandle* Timer = ScenarioTimers.Find(ScenarioName))
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(*Timer);
        }
        ScenarioTimers.Remove(ScenarioName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ended scenario: %s"), *ScenarioName);
}

bool UNPC_BehaviorScenarioManager::IsScenarioActive(const FString& ScenarioName) const
{
    return ActiveScenarios.ContainsByPredicate([&ScenarioName](const FNPC_BehaviorScenario& Scenario) {
        return Scenario.ScenarioName == ScenarioName;
    });
}

void UNPC_BehaviorScenarioManager::CoordinatePackHunt(const TArray<AActor*>& PackMembers, AActor* Target)
{
    if (!Target || PackMembers.Num() == 0)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Coordinating pack hunt with %d members targeting %s"), 
           PackMembers.Num(), *Target->GetName());
    
    // Implement pack hunting coordination logic
    FVector TargetLocation = Target->GetActorLocation();
    
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (AActor* PackMember = PackMembers[i])
        {
            // Calculate flanking positions around target
            float AngleStep = 360.0f / PackMembers.Num();
            float Angle = i * AngleStep;
            float RadianAngle = FMath::DegreesToRadians(Angle);
            
            FVector FlankPosition = TargetLocation + FVector(
                FMath::Cos(RadianAngle) * 800.0f,
                FMath::Sin(RadianAngle) * 800.0f,
                0.0f
            );
            
            // Move pack member to flanking position
            PackMember->SetActorLocation(FlankPosition);
        }
    }
}

void UNPC_BehaviorScenarioManager::FormDefensiveCircle(const TArray<AActor*>& PackMembers, const FVector& ThreatLocation)
{
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    // Calculate center point of pack
    FVector CenterPoint = FVector::ZeroVector;
    for (AActor* Member : PackMembers)
    {
        if (Member)
        {
            CenterPoint += Member->GetActorLocation();
        }
    }
    CenterPoint /= PackMembers.Num();
    
    // Position members in defensive circle facing threat
    float CircleRadius = 600.0f;
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (AActor* Member = PackMembers[i])
        {
            float AngleStep = 360.0f / PackMembers.Num();
            float Angle = i * AngleStep;
            float RadianAngle = FMath::DegreesToRadians(Angle);
            
            FVector Position = CenterPoint + FVector(
                FMath::Cos(RadianAngle) * CircleRadius,
                FMath::Sin(RadianAngle) * CircleRadius,
                0.0f
            );
            
            Member->SetActorLocation(Position);
            
            // Face toward threat
            FVector LookDirection = (ThreatLocation - Position).GetSafeNormal();
            FRotator LookRotation = LookDirection.Rotation();
            Member->SetActorRotation(LookRotation);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Formed defensive circle with %d members"), PackMembers.Num());
}

void UNPC_BehaviorScenarioManager::ExecuteMigrationPattern(const TArray<AActor*>& HerdMembers, const FVector& Destination)
{
    if (HerdMembers.Num() == 0)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Executing migration pattern for %d herd members to %s"), 
           HerdMembers.Num(), *Destination.ToString());
    
    // Lead animal moves directly toward destination
    if (HerdMembers.Num() > 0 && HerdMembers[0])
    {
        HerdMembers[0]->SetActorLocation(Destination);
    }
    
    // Other animals follow in formation
    for (int32 i = 1; i < HerdMembers.Num(); i++)
    {
        if (AActor* Member = HerdMembers[i])
        {
            FVector FollowPosition = Destination + FVector(
                FMath::RandRange(-500.0f, 500.0f),
                FMath::RandRange(-500.0f, 500.0f),
                0.0f
            );
            Member->SetActorLocation(FollowPosition);
        }
    }
}

void UNPC_BehaviorScenarioManager::EstablishTerritory(AActor* TerritorialActor, const FVector& CenterPoint, float Radius)
{
    if (!TerritorialActor)
    {
        return;
    }
    
    TerritorialCenters.Add(TerritorialActor, CenterPoint);
    TerritorialRadii.Add(TerritorialActor, Radius);
    
    UE_LOG(LogTemp, Warning, TEXT("Established territory for %s at %s with radius %.1f"), 
           *TerritorialActor->GetName(), *CenterPoint.ToString(), Radius);
}

void UNPC_BehaviorScenarioManager::HandleTerritorialIntrusion(AActor* TerritorialOwner, AActor* Intruder)
{
    if (!TerritorialOwner || !Intruder)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Territorial intrusion: %s invaded %s's territory"), 
           *Intruder->GetName(), *TerritorialOwner->GetName());
    
    // Trigger aggressive response from territorial owner
    FVector IntruderLocation = Intruder->GetActorLocation();
    FVector OwnerLocation = TerritorialOwner->GetActorLocation();
    
    // Move owner toward intruder aggressively
    FVector Direction = (IntruderLocation - OwnerLocation).GetSafeNormal();
    FVector ChasePosition = IntruderLocation - (Direction * 300.0f);
    TerritorialOwner->SetActorLocation(ChasePosition);
    
    // Face the intruder
    FRotator LookRotation = Direction.Rotation();
    TerritorialOwner->SetActorRotation(LookRotation);
}

bool UNPC_BehaviorScenarioManager::IsInTerritory(AActor* TerritorialOwner, const FVector& Location) const
{
    if (!TerritorialOwner)
    {
        return false;
    }
    
    const FVector* Center = TerritorialCenters.Find(TerritorialOwner);
    const float* Radius = TerritorialRadii.Find(TerritorialOwner);
    
    if (Center && Radius)
    {
        float Distance = FVector::Dist(Location, *Center);
        return Distance <= *Radius;
    }
    
    return false;
}

void UNPC_BehaviorScenarioManager::InitiateSocialInteraction(AActor* Initiator, AActor* Target, const FString& InteractionType)
{
    if (!Initiator || !Target)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Social interaction: %s -> %s (%s)"), 
           *Initiator->GetName(), *Target->GetName(), *InteractionType);
    
    // Move actors closer for interaction
    FVector InitiatorPos = Initiator->GetActorLocation();
    FVector TargetPos = Target->GetActorLocation();
    FVector MidPoint = (InitiatorPos + TargetPos) * 0.5f;
    
    FVector Direction = (TargetPos - InitiatorPos).GetSafeNormal();
    
    // Position actors facing each other
    Initiator->SetActorLocation(MidPoint - (Direction * 200.0f));
    Target->SetActorLocation(MidPoint + (Direction * 200.0f));
    
    Initiator->SetActorRotation(Direction.Rotation());
    Target->SetActorRotation((-Direction).Rotation());
}

void UNPC_BehaviorScenarioManager::TriggerFleeResponse(const TArray<AActor*>& FleeingActors, const FVector& DangerSource)
{
    for (AActor* Actor : FleeingActors)
    {
        if (Actor)
        {
            FVector ActorLocation = Actor->GetActorLocation();
            FVector FleeDirection = (ActorLocation - DangerSource).GetSafeNormal();
            FVector FleePosition = ActorLocation + (FleeDirection * 1500.0f);
            
            Actor->SetActorLocation(FleePosition);
            Actor->SetActorRotation(FleeDirection.Rotation());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Triggered flee response for %d actors"), FleeingActors.Num());
}

void UNPC_BehaviorScenarioManager::UpdateActiveScenarios()
{
    CleanupExpiredScenarios();
    
    for (const FNPC_BehaviorScenario& Scenario : ActiveScenarios)
    {
        ExecuteScenarioBehavior(Scenario);
    }
}

void UNPC_BehaviorScenarioManager::ProcessScenarioTriggers()
{
    // Check for scenario trigger conditions
    // This would typically involve checking player proximity, time of day, etc.
}

void UNPC_BehaviorScenarioManager::CleanupExpiredScenarios()
{
    ActiveScenarios.RemoveAll([](const FNPC_BehaviorScenario& Scenario) {
        return !Scenario.bIsActive;
    });
}

bool UNPC_BehaviorScenarioManager::CanActivateScenario(const FNPC_BehaviorScenario& Scenario) const
{
    // Check if we're under the concurrent scenario limit
    return ActiveScenarios.Num() < MaxConcurrentScenarios;
}

void UNPC_BehaviorScenarioManager::ExecuteScenarioBehavior(const FNPC_BehaviorScenario& Scenario)
{
    // Execute scenario-specific behavior based on type
    switch (Scenario.ScenarioType)
    {
        case ENPC_BehaviorScenarioType::TerritorialConflict:
            // Handle territorial behavior
            break;
        case ENPC_BehaviorScenarioType::PackHunting:
            // Coordinate pack hunting
            break;
        case ENPC_BehaviorScenarioType::SocialInteraction:
            // Manage social behaviors
            break;
        default:
            break;
    }
}

void UNPC_BehaviorScenarioManager::DebugDrawActiveScenarios() const
{
    if (UWorld* World = GetWorld())
    {
        for (const FNPC_BehaviorScenario& Scenario : ActiveScenarios)
        {
            FColor DebugColor = FColor::Green;
            switch (Scenario.ScenarioType)
            {
                case ENPC_BehaviorScenarioType::TerritorialConflict:
                    DebugColor = FColor::Red;
                    break;
                case ENPC_BehaviorScenarioType::PackHunting:
                    DebugColor = FColor::Orange;
                    break;
                case ENPC_BehaviorScenarioType::SocialInteraction:
                    DebugColor = FColor::Blue;
                    break;
                default:
                    DebugColor = FColor::Green;
                    break;
            }
            
            DrawDebugSphere(World, Scenario.TriggerLocation, Scenario.TriggerRadius, 16, DebugColor, false, 1.0f);
        }
    }
}

TArray<FString> UNPC_BehaviorScenarioManager::GetActiveScenarioNames() const
{
    TArray<FString> Names;
    for (const FNPC_BehaviorScenario& Scenario : ActiveScenarios)
    {
        Names.Add(Scenario.ScenarioName);
    }
    return Names;
}

void UNPC_BehaviorScenarioManager::LogScenarioStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Behavior Scenario Manager Status ==="));
    UE_LOG(LogTemp, Log, TEXT("Registered Scenarios: %d"), RegisteredScenarios.Num());
    UE_LOG(LogTemp, Log, TEXT("Active Scenarios: %d"), ActiveScenarios.Num());
    
    for (const FNPC_BehaviorScenario& Scenario : ActiveScenarios)
    {
        UE_LOG(LogTemp, Log, TEXT("  - %s (Priority: %d)"), *Scenario.ScenarioName, Scenario.Priority);
    }
}