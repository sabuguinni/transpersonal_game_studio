#include "Crowd_MassAISubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassArchetypeTypes.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UCrowd_MassAISubsystem::UCrowd_MassAISubsystem()
{
    MaxSimulatedEntities = 1000;
    bLODSystemEnabled = true;
    MassEntitySubsystem = nullptr;
    MassSimulationSubsystem = nullptr;
}

void UCrowd_MassAISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassAISubsystem::Initialize - Starting Mass AI system"));
    
    // Get Mass subsystems
    UWorld* World = GetWorld();
    if (World)
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
        
        if (MassEntitySubsystem && MassSimulationSubsystem)
        {
            InitializeMassAI();
            UE_LOG(LogTemp, Warning, TEXT("Mass AI subsystems initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get Mass subsystems"));
        }
    }
}

void UCrowd_MassAISubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassAISubsystem::Deinitialize - Shutting down Mass AI"));
    
    ActiveHerds.Empty();
    HerdBehaviors.Empty();
    
    Super::Deinitialize();
}

void UCrowd_MassAISubsystem::InitializeMassAI()
{
    // Initialize Mass AI framework
    CreateHerdArchetype();
    
    UE_LOG(LogTemp, Warning, TEXT("Mass AI framework initialized with max entities: %d"), MaxSimulatedEntities);
}

void UCrowd_MassAISubsystem::CreateHerdArchetype()
{
    // Create archetype for herd entities
    if (MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Creating herd archetype for Mass entities"));
        // Archetype creation would be done here with proper Mass framework setup
    }
}

void UCrowd_MassAISubsystem::SpawnDinosaurHerd(const FCrowd_DinosaurHerdData& HerdData)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn herd - MassEntitySubsystem not available"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawning dinosaur herd: %s, Size: %d, Center: %s"), 
           *HerdData.DinosaurSpecies, 
           HerdData.HerdSize, 
           *HerdData.HerdCenter.ToString());
    
    // Add to active herds
    int32 HerdIndex = ActiveHerds.Add(HerdData);
    HerdBehaviors.Add(HerdIndex, ECrowd_HerdBehavior::Grazing);
    
    // Spawn individual herd members
    for (int32 i = 0; i < HerdData.HerdSize; i++)
    {
        // Calculate random position within herd radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, HerdData.HerdRadius);
        
        FVector MemberLocation = HerdData.HerdCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        FMassEntityHandle EntityHandle = SpawnHerdMember(MemberLocation, HerdData.DinosaurSpecies);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned herd member %d at location %s"), i, *MemberLocation.ToString());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Herd spawned successfully. Total active herds: %d"), ActiveHerds.Num());
}

FMassEntityHandle UCrowd_MassAISubsystem::SpawnHerdMember(const FVector& Location, const FString& Species)
{
    if (!MassEntitySubsystem)
    {
        return FMassEntityHandle();
    }
    
    // Create Mass entity for herd member
    FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
    
    UE_LOG(LogTemp, Log, TEXT("Created Mass entity for %s at %s"), *Species, *Location.ToString());
    
    return EntityHandle;
}

void UCrowd_MassAISubsystem::SetHerdBehavior(int32 HerdIndex, ECrowd_HerdBehavior NewBehavior)
{
    if (ActiveHerds.IsValidIndex(HerdIndex))
    {
        HerdBehaviors[HerdIndex] = NewBehavior;
        
        FString BehaviorName;
        switch (NewBehavior)
        {
            case ECrowd_HerdBehavior::Grazing: BehaviorName = TEXT("Grazing"); break;
            case ECrowd_HerdBehavior::Migrating: BehaviorName = TEXT("Migrating"); break;
            case ECrowd_HerdBehavior::Fleeing: BehaviorName = TEXT("Fleeing"); break;
            case ECrowd_HerdBehavior::Drinking: BehaviorName = TEXT("Drinking"); break;
            case ECrowd_HerdBehavior::Resting: BehaviorName = TEXT("Resting"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Herd %d behavior changed to: %s"), HerdIndex, *BehaviorName);
    }
}

void UCrowd_MassAISubsystem::UpdateHerdMigration(int32 HerdIndex, const FVector& TargetLocation)
{
    if (ActiveHerds.IsValidIndex(HerdIndex))
    {
        ActiveHerds[HerdIndex].HerdCenter = TargetLocation;
        SetHerdBehavior(HerdIndex, ECrowd_HerdBehavior::Migrating);
        
        UE_LOG(LogTemp, Warning, TEXT("Herd %d migration target set to: %s"), HerdIndex, *TargetLocation.ToString());
    }
}

void UCrowd_MassAISubsystem::TriggerPredatorAlert(const FVector& PredatorLocation, float AlertRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("PREDATOR ALERT! Location: %s, Radius: %f"), 
           *PredatorLocation.ToString(), AlertRadius);
    
    // Check all herds within alert radius
    for (int32 i = 0; i < ActiveHerds.Num(); i++)
    {
        float DistanceToHerd = FVector::Dist(ActiveHerds[i].HerdCenter, PredatorLocation);
        
        if (DistanceToHerd <= AlertRadius)
        {
            // Set herd to fleeing behavior
            SetHerdBehavior(i, ECrowd_HerdBehavior::Fleeing);
            
            // Calculate flee direction (away from predator)
            FVector FleeDirection = (ActiveHerds[i].HerdCenter - PredatorLocation).GetSafeNormal();
            FVector FleeTarget = ActiveHerds[i].HerdCenter + (FleeDirection * 5000.0f);
            
            UpdateHerdMigration(i, FleeTarget);
            
            UE_LOG(LogTemp, Warning, TEXT("Herd %d fleeing from predator to: %s"), i, *FleeTarget.ToString());
        }
    }
}

void UCrowd_MassAISubsystem::SetMaxSimulatedEntities(int32 MaxEntities)
{
    MaxSimulatedEntities = FMath::Clamp(MaxEntities, 100, 10000);
    UE_LOG(LogTemp, Warning, TEXT("Max simulated entities set to: %d"), MaxSimulatedEntities);
}

void UCrowd_MassAISubsystem::EnableLODSystem(bool bEnable)
{
    bLODSystemEnabled = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("LOD system %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}