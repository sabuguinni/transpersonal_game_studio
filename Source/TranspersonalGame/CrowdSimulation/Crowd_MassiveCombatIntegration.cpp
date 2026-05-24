#include "Crowd_MassiveCombatIntegration.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "SharedTypes.h"

DEFINE_LOG_CATEGORY(LogCrowdCombatIntegration);

ACrowd_MassiveCombatIntegration::ACrowd_MassiveCombatIntegration()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create collision sphere for combat zone detection
    CombatZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatZoneSphere"));
    CombatZoneSphere->SetupAttachment(RootComponent);
    CombatZoneSphere->SetSphereRadius(2000.0f);
    CombatZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CombatZoneSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CombatZoneSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create visual mesh for debugging
    DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMesh"));
    DebugMesh->SetupAttachment(RootComponent);
    DebugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize combat integration properties
    MaxCombatEntities = 10000;
    CombatRadius = 2000.0f;
    CombatIntensity = 1.0f;
    bCombatActive = false;
    bMassiveBattleMode = false;
    
    CurrentCombatEntities = 0;
    CombatUpdateFrequency = 0.1f;
    BattleEscalationThreshold = 5000;
    
    CombatZoneType = ECrowd_CombatZoneType::MixedCombat;
    CombatBehaviorMode = ECrowd_CombatBehavior::TacticalEngagement;
    
    // Initialize entity type ratios
    HerbivoreRatio = 0.6f;
    CarnivoreRatio = 0.3f;
    NeutralRatio = 0.1f;
    
    // Initialize combat metrics
    TotalCombatEvents = 0;
    ActiveCombatGroups = 0;
    CombatEfficiencyScore = 0.0f;
}

void ACrowd_MassiveCombatIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Massive Combat Integration Actor initialized"));
    
    // Initialize mass entity system
    InitializeMassEntitySystem();
    
    // Set up combat zone
    SetupCombatZone();
    
    // Start combat monitoring
    StartCombatMonitoring();
}

void ACrowd_MassiveCombatIntegration::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bCombatActive)
    {
        UpdateCombatSystems(DeltaTime);
        ProcessCombatEvents(DeltaTime);
        UpdateCombatMetrics(DeltaTime);
    }
}

void ACrowd_MassiveCombatIntegration::InitializeMassEntitySystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogCrowdCombatIntegration, Error, TEXT("Failed to get world for mass entity system"));
        return;
    }
    
    // Get Mass Entity subsystem
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogCrowdCombatIntegration, Error, TEXT("Mass Entity subsystem not available"));
        return;
    }
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Mass Entity subsystem initialized for combat integration"));
    
    // Create entity archetypes for combat
    CreateCombatEntityArchetypes();
}

void ACrowd_MassiveCombatIntegration::CreateCombatEntityArchetypes()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create herbivore combat archetype
    HerbivoreArchetype.Reset();
    HerbivoreArchetype.AddTag<FMassTransformFragment>();
    HerbivoreArchetype.AddTag<FMassVelocityFragment>();
    
    // Create carnivore combat archetype  
    CarnivoreArchetype.Reset();
    CarnivoreArchetype.AddTag<FMassTransformFragment>();
    CarnivoreArchetype.AddTag<FMassVelocityFragment>();
    
    // Create neutral entity archetype
    NeutralArchetype.Reset();
    NeutralArchetype.AddTag<FMassTransformFragment>();
    NeutralArchetype.AddTag<FMassVelocityFragment>();
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Combat entity archetypes created"));
}

void ACrowd_MassiveCombatIntegration::SetupCombatZone()
{
    // Configure combat zone based on type
    switch (CombatZoneType)
    {
        case ECrowd_CombatZoneType::PredatorTerritory:
            CarnivoreRatio = 0.7f;
            HerbivoreRatio = 0.2f;
            NeutralRatio = 0.1f;
            CombatIntensity = 1.5f;
            break;
            
        case ECrowd_CombatZoneType::HerbivoreSafeZone:
            HerbivoreRatio = 0.8f;
            CarnivoreRatio = 0.1f;
            NeutralRatio = 0.1f;
            CombatIntensity = 0.3f;
            break;
            
        case ECrowd_CombatZoneType::MixedCombat:
            HerbivoreRatio = 0.5f;
            CarnivoreRatio = 0.4f;
            NeutralRatio = 0.1f;
            CombatIntensity = 1.0f;
            break;
            
        case ECrowd_CombatZoneType::PackHunting:
            CarnivoreRatio = 0.6f;
            HerbivoreRatio = 0.3f;
            NeutralRatio = 0.1f;
            CombatIntensity = 1.2f;
            break;
            
        case ECrowd_CombatZoneType::TerritorialDispute:
            CarnivoreRatio = 0.8f;
            HerbivoreRatio = 0.1f;
            NeutralRatio = 0.1f;
            CombatIntensity = 2.0f;
            break;
    }
    
    // Update collision sphere radius
    if (CombatZoneSphere)
    {
        CombatZoneSphere->SetSphereRadius(CombatRadius);
    }
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Combat zone setup complete - Type: %d, Intensity: %f"), 
           (int32)CombatZoneType, CombatIntensity);
}

void ACrowd_MassiveCombatIntegration::StartCombatMonitoring()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Set up timer for combat updates
    World->GetTimerManager().SetTimer(
        CombatUpdateTimer,
        this,
        &ACrowd_MassiveCombatIntegration::UpdateCombatSystems,
        CombatUpdateFrequency,
        true
    );
    
    bCombatActive = true;
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Combat monitoring started"));
}

void ACrowd_MassiveCombatIntegration::UpdateCombatSystems(float DeltaTime)
{
    if (!MassEntitySubsystem || !bCombatActive)
    {
        return;
    }
    
    // Update combat entity counts
    UpdateEntityCounts();
    
    // Check for massive battle conditions
    CheckMassiveBattleConditions();
    
    // Process combat behaviors
    ProcessCombatBehaviors(DeltaTime);
    
    // Update combat zones
    UpdateCombatZones(DeltaTime);
}

void ACrowd_MassiveCombatIntegration::UpdateEntityCounts()
{
    // Get current entity count in combat zone
    CurrentCombatEntities = GetEntitiesInCombatZone();
    
    // Calculate entity type distributions
    int32 HerbivoreCount = FMath::RoundToInt(CurrentCombatEntities * HerbivoreRatio);
    int32 CarnivoreCount = FMath::RoundToInt(CurrentCombatEntities * CarnivoreRatio);
    int32 NeutralCount = CurrentCombatEntities - HerbivoreCount - CarnivoreCount;
    
    // Update combat metrics
    if (CurrentCombatEntities > 0)
    {
        CombatEfficiencyScore = CalculateCombatEfficiency();
    }
}

int32 ACrowd_MassiveCombatIntegration::GetEntitiesInCombatZone()
{
    // Simulate entity counting in combat zone
    // In a real implementation, this would query the Mass Entity system
    return FMath::RandRange(1000, MaxCombatEntities);
}

void ACrowd_MassiveCombatIntegration::CheckMassiveBattleConditions()
{
    bool bShouldActivateMassiveBattle = CurrentCombatEntities >= BattleEscalationThreshold;
    
    if (bShouldActivateMassiveBattle && !bMassiveBattleMode)
    {
        ActivateMassiveBattleMode();
    }
    else if (!bShouldActivateMassiveBattle && bMassiveBattleMode)
    {
        DeactivateMassiveBattleMode();
    }
}

void ACrowd_MassiveCombatIntegration::ActivateMassiveBattleMode()
{
    bMassiveBattleMode = true;
    CombatIntensity *= 1.5f;
    
    UE_LOG(LogCrowdCombatIntegration, Warning, TEXT("MASSIVE BATTLE MODE ACTIVATED - %d entities in combat"), 
           CurrentCombatEntities);
    
    // Broadcast massive battle event
    OnMassiveBattleStarted.Broadcast(CurrentCombatEntities, CombatZoneType);
}

void ACrowd_MassiveCombatIntegration::DeactivateMassiveBattleMode()
{
    bMassiveBattleMode = false;
    CombatIntensity /= 1.5f;
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Massive battle mode deactivated"));
    
    // Broadcast battle end event
    OnMassiveBattleEnded.Broadcast(TotalCombatEvents, CombatEfficiencyScore);
}

void ACrowd_MassiveCombatIntegration::ProcessCombatBehaviors(float DeltaTime)
{
    switch (CombatBehaviorMode)
    {
        case ECrowd_CombatBehavior::TacticalEngagement:
            ProcessTacticalCombat(DeltaTime);
            break;
            
        case ECrowd_CombatBehavior::ChaoticBrawl:
            ProcessChaoticCombat(DeltaTime);
            break;
            
        case ECrowd_CombatBehavior::OrganizedWarfare:
            ProcessOrganizedCombat(DeltaTime);
            break;
            
        case ECrowd_CombatBehavior::HuntingBehavior:
            ProcessHuntingBehavior(DeltaTime);
            break;
            
        case ECrowd_CombatBehavior::DefensiveFormation:
            ProcessDefensiveBehavior(DeltaTime);
            break;
    }
}

void ACrowd_MassiveCombatIntegration::ProcessTacticalCombat(float DeltaTime)
{
    // Implement tactical combat behavior
    ActiveCombatGroups = FMath::Max(1, CurrentCombatEntities / 100);
    TotalCombatEvents += FMath::RandRange(5, 20);
}

void ACrowd_MassiveCombatIntegration::ProcessChaoticCombat(float DeltaTime)
{
    // Implement chaotic combat behavior
    ActiveCombatGroups = CurrentCombatEntities / 50;
    TotalCombatEvents += FMath::RandRange(20, 50);
}

void ACrowd_MassiveCombatIntegration::ProcessOrganizedCombat(float DeltaTime)
{
    // Implement organized combat behavior
    ActiveCombatGroups = FMath::Max(1, CurrentCombatEntities / 200);
    TotalCombatEvents += FMath::RandRange(10, 30);
}

void ACrowd_MassiveCombatIntegration::ProcessHuntingBehavior(float DeltaTime)
{
    // Implement hunting behavior
    ActiveCombatGroups = FMath::Max(1, CurrentCombatEntities / 150);
    TotalCombatEvents += FMath::RandRange(8, 25);
}

void ACrowd_MassiveCombatIntegration::ProcessDefensiveBehavior(float DeltaTime)
{
    // Implement defensive behavior
    ActiveCombatGroups = FMath::Max(1, CurrentCombatEntities / 300);
    TotalCombatEvents += FMath::RandRange(3, 15);
}

void ACrowd_MassiveCombatIntegration::UpdateCombatZones(float DeltaTime)
{
    // Update combat zone properties based on current state
    if (bMassiveBattleMode)
    {
        // Expand combat radius during massive battles
        float ExpandedRadius = CombatRadius * 1.5f;
        if (CombatZoneSphere)
        {
            CombatZoneSphere->SetSphereRadius(ExpandedRadius);
        }
    }
}

void ACrowd_MassiveCombatIntegration::ProcessCombatEvents(float DeltaTime)
{
    // Process combat events and update metrics
    if (CurrentCombatEntities > 0)
    {
        // Generate combat events based on entity density and behavior mode
        int32 NewCombatEvents = CalculateCombatEvents(DeltaTime);
        TotalCombatEvents += NewCombatEvents;
    }
}

int32 ACrowd_MassiveCombatIntegration::CalculateCombatEvents(float DeltaTime)
{
    float EventRate = CombatIntensity * (CurrentCombatEntities / 1000.0f) * DeltaTime;
    return FMath::RoundToInt(EventRate * FMath::RandRange(0.5f, 2.0f));
}

void ACrowd_MassiveCombatIntegration::UpdateCombatMetrics(float DeltaTime)
{
    // Update combat efficiency score
    CombatEfficiencyScore = CalculateCombatEfficiency();
    
    // Log metrics periodically
    static float MetricsLogTimer = 0.0f;
    MetricsLogTimer += DeltaTime;
    
    if (MetricsLogTimer >= 5.0f)
    {
        UE_LOG(LogCrowdCombatIntegration, Log, 
               TEXT("Combat Metrics - Entities: %d, Groups: %d, Events: %d, Efficiency: %f"), 
               CurrentCombatEntities, ActiveCombatGroups, TotalCombatEvents, CombatEfficiencyScore);
        MetricsLogTimer = 0.0f;
    }
}

float ACrowd_MassiveCombatIntegration::CalculateCombatEfficiency()
{
    if (CurrentCombatEntities == 0)
    {
        return 0.0f;
    }
    
    float GroupEfficiency = (float)ActiveCombatGroups / (CurrentCombatEntities / 100.0f);
    float EventEfficiency = (float)TotalCombatEvents / (CurrentCombatEntities * 0.1f);
    float IntensityFactor = FMath::Clamp(CombatIntensity, 0.1f, 3.0f);
    
    return (GroupEfficiency + EventEfficiency) * IntensityFactor * 0.5f;
}

void ACrowd_MassiveCombatIntegration::SetCombatZoneType(ECrowd_CombatZoneType NewType)
{
    CombatZoneType = NewType;
    SetupCombatZone();
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Combat zone type changed to: %d"), (int32)NewType);
}

void ACrowd_MassiveCombatIntegration::SetCombatBehaviorMode(ECrowd_CombatBehavior NewMode)
{
    CombatBehaviorMode = NewMode;
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Combat behavior mode changed to: %d"), (int32)NewMode);
}

void ACrowd_MassiveCombatIntegration::SetMaxCombatEntities(int32 NewMax)
{
    MaxCombatEntities = FMath::Max(100, NewMax);
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Max combat entities set to: %d"), MaxCombatEntities);
}

void ACrowd_MassiveCombatIntegration::SetCombatRadius(float NewRadius)
{
    CombatRadius = FMath::Max(500.0f, NewRadius);
    
    if (CombatZoneSphere)
    {
        CombatZoneSphere->SetSphereRadius(CombatRadius);
    }
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Combat radius set to: %f"), CombatRadius);
}

FCrowd_CombatMetrics ACrowd_MassiveCombatIntegration::GetCombatMetrics() const
{
    FCrowd_CombatMetrics Metrics;
    Metrics.TotalEntities = CurrentCombatEntities;
    Metrics.ActiveGroups = ActiveCombatGroups;
    Metrics.CombatEvents = TotalCombatEvents;
    Metrics.EfficiencyScore = CombatEfficiencyScore;
    Metrics.CombatIntensity = CombatIntensity;
    Metrics.bMassiveBattleActive = bMassiveBattleMode;
    
    return Metrics;
}

void ACrowd_MassiveCombatIntegration::ResetCombatMetrics()
{
    TotalCombatEvents = 0;
    ActiveCombatGroups = 0;
    CombatEfficiencyScore = 0.0f;
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Combat metrics reset"));
}

void ACrowd_MassiveCombatIntegration::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up timers
    UWorld* World = GetWorld();
    if (World && CombatUpdateTimer.IsValid())
    {
        World->GetTimerManager().ClearTimer(CombatUpdateTimer);
    }
    
    bCombatActive = false;
    
    UE_LOG(LogCrowdCombatIntegration, Log, TEXT("Massive Combat Integration Actor destroyed"));
    
    Super::EndPlay(EndPlayReason);
}