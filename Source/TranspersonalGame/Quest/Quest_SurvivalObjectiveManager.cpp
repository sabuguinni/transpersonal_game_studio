#include "Quest_SurvivalObjectiveManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

AQuest_SurvivalObjectiveManager::AQuest_SurvivalObjectiveManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxActiveObjectives = 5;
    ObjectiveUpdateInterval = 5.0f;
    ThreatDetectionRadius = 2000.0f;
    bNightSurvivalMode = false;
    CurrentTimeOfDay = 12.0f;
}

void AQuest_SurvivalObjectiveManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize survival objectives system
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalObjectiveManager: System initialized"));
    
    // Set up timers for objective monitoring
    GetWorld()->GetTimerManager().SetTimer(ObjectiveTimerHandle, 
        [this]() { UpdateObjectivePriorities(); }, 
        ObjectiveUpdateInterval, true);
    
    GetWorld()->GetTimerManager().SetTimer(ThreatMonitorHandle, 
        [this]() { MonitorEnvironmentalThreats(); }, 
        2.0f, true);
    
    GetWorld()->GetTimerManager().SetTimer(EnvironmentCheckHandle, 
        [this]() { HandleWeatherChange(); }, 
        10.0f, true);
    
    // Create initial survival objectives
    CreateSurvivalObjective(EQuest_SurvivalObjectiveType::FindWater, FVector(1000, 1000, 100), 600.0f);
    CreateSurvivalObjective(EQuest_SurvivalObjectiveType::GatherFood, FVector(500, -500, 100), 900.0f);
}

void AQuest_SurvivalObjectiveManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateObjectiveTimers(DeltaTime);
    CheckForExpiredObjectives();
}

void AQuest_SurvivalObjectiveManager::CreateSurvivalObjective(EQuest_SurvivalObjectiveType ObjectiveType, FVector Location, float TimeLimit)
{
    if (ActiveObjectives.Num() >= MaxActiveObjectives)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalObjectiveManager: Maximum objectives reached"));
        return;
    }
    
    FQuest_SurvivalObjective NewObjective;
    NewObjective.ObjectiveType = ObjectiveType;
    NewObjective.TargetLocation = Location;
    NewObjective.TimeLimit = TimeLimit;
    NewObjective.RemainingTime = TimeLimit;
    NewObjective.bIsActive = true;
    NewObjective.bIsCompleted = false;
    
    // Set objective description and priority based on type
    switch (ObjectiveType)
    {
        case EQuest_SurvivalObjectiveType::FindWater:
            NewObjective.ObjectiveDescription = TEXT("Find a clean water source to survive");
            NewObjective.Priority = EQuest_SurvivalPriority::Critical;
            NewObjective.CompletionRadius = 300.0f;
            break;
            
        case EQuest_SurvivalObjectiveType::BuildShelter:
            NewObjective.ObjectiveDescription = TEXT("Build shelter before nightfall");
            NewObjective.Priority = EQuest_SurvivalPriority::High;
            NewObjective.CompletionRadius = 500.0f;
            break;
            
        case EQuest_SurvivalObjectiveType::GatherFood:
            NewObjective.ObjectiveDescription = TEXT("Gather food to maintain strength");
            NewObjective.Priority = EQuest_SurvivalPriority::High;
            NewObjective.CompletionRadius = 200.0f;
            break;
            
        case EQuest_SurvivalObjectiveType::CreateFire:
            NewObjective.ObjectiveDescription = TEXT("Create fire for warmth and protection");
            NewObjective.Priority = EQuest_SurvivalPriority::Medium;
            NewObjective.CompletionRadius = 400.0f;
            break;
            
        case EQuest_SurvivalObjectiveType::CraftTool:
            NewObjective.ObjectiveDescription = TEXT("Craft essential survival tools");
            NewObjective.Priority = EQuest_SurvivalPriority::Medium;
            NewObjective.CompletionRadius = 100.0f;
            break;
            
        case EQuest_SurvivalObjectiveType::EscapePredator:
            NewObjective.ObjectiveDescription = TEXT("Escape from dangerous predator");
            NewObjective.Priority = EQuest_SurvivalPriority::Critical;
            NewObjective.CompletionRadius = 1000.0f;
            break;
            
        case EQuest_SurvivalObjectiveType::ExploreArea:
            NewObjective.ObjectiveDescription = TEXT("Explore new territory for resources");
            NewObjective.Priority = EQuest_SurvivalPriority::Low;
            NewObjective.CompletionRadius = 800.0f;
            break;
            
        case EQuest_SurvivalObjectiveType::SurviveNight:
            NewObjective.ObjectiveDescription = TEXT("Survive the dangerous night");
            NewObjective.Priority = EQuest_SurvivalPriority::Critical;
            NewObjective.CompletionRadius = 0.0f;
            break;
    }
    
    ActiveObjectives.Add(NewObjective);
    SpawnObjectiveMarker(Location, ObjectiveType);
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalObjectiveManager: Created objective: %s"), *NewObjective.ObjectiveDescription);
}

void AQuest_SurvivalObjectiveManager::CompleteObjective(int32 ObjectiveIndex)
{
    if (ActiveObjectives.IsValidIndex(ObjectiveIndex))
    {
        FQuest_SurvivalObjective CompletedObjective = ActiveObjectives[ObjectiveIndex];
        CompletedObjective.bIsCompleted = true;
        CompletedObjective.bIsActive = false;
        
        CompletedObjectives.Add(CompletedObjective);
        ActiveObjectives.RemoveAt(ObjectiveIndex);
        
        UE_LOG(LogTemp, Warning, TEXT("SurvivalObjectiveManager: Objective completed: %s"), *CompletedObjective.ObjectiveDescription);
    }
}

void AQuest_SurvivalObjectiveManager::FailObjective(int32 ObjectiveIndex)
{
    if (ActiveObjectives.IsValidIndex(ObjectiveIndex))
    {
        FQuest_SurvivalObjective FailedObjective = ActiveObjectives[ObjectiveIndex];
        ActiveObjectives.RemoveAt(ObjectiveIndex);
        
        UE_LOG(LogTemp, Warning, TEXT("SurvivalObjectiveManager: Objective failed: %s"), *FailedObjective.ObjectiveDescription);
        
        // Create emergency replacement objective if critical
        if (FailedObjective.Priority == EQuest_SurvivalPriority::Critical)
        {
            CreateEmergencyObjective(FailedObjective.ObjectiveType, FailedObjective.TargetLocation);
        }
    }
}

bool AQuest_SurvivalObjectiveManager::CheckObjectiveCompletion(int32 ObjectiveIndex, FVector PlayerLocation)
{
    if (!ActiveObjectives.IsValidIndex(ObjectiveIndex))
        return false;
    
    FQuest_SurvivalObjective& Objective = ActiveObjectives[ObjectiveIndex];
    
    if (Objective.CompletionRadius > 0.0f)
    {
        float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
        if (Distance <= Objective.CompletionRadius)
        {
            CompleteObjective(ObjectiveIndex);
            return true;
        }
    }
    
    return false;
}

void AQuest_SurvivalObjectiveManager::UpdateObjectivePriorities()
{
    // Adjust priorities based on current conditions
    for (FQuest_SurvivalObjective& Objective : ActiveObjectives)
    {
        if (bNightSurvivalMode)
        {
            // Increase priority of shelter and fire during night
            if (Objective.ObjectiveType == EQuest_SurvivalObjectiveType::BuildShelter ||
                Objective.ObjectiveType == EQuest_SurvivalObjectiveType::CreateFire)
            {
                Objective.Priority = EQuest_SurvivalPriority::Critical;
            }
        }
        
        // Increase priority as time runs out
        if (Objective.RemainingTime < 60.0f && Objective.Priority != EQuest_SurvivalPriority::Critical)
        {
            Objective.Priority = EQuest_SurvivalPriority::High;
        }
    }
    
    SortObjectivesByPriority();
}

FQuest_SurvivalObjective AQuest_SurvivalObjectiveManager::GetHighestPriorityObjective()
{
    if (ActiveObjectives.Num() == 0)
    {
        return FQuest_SurvivalObjective();
    }
    
    SortObjectivesByPriority();
    return ActiveObjectives[0];
}

TArray<FQuest_SurvivalObjective> AQuest_SurvivalObjectiveManager::GetActiveObjectives()
{
    return ActiveObjectives;
}

void AQuest_SurvivalObjectiveManager::MonitorEnvironmentalThreats()
{
    // Check for predators in the area
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance < ThreatDetectionRadius)
            {
                CreateEmergencyObjective(EQuest_SurvivalObjectiveType::EscapePredator, Actor->GetActorLocation());
                break;
            }
        }
    }
}

void AQuest_SurvivalObjectiveManager::CreateEmergencyObjective(EQuest_SurvivalObjectiveType ObjectiveType, FVector ThreatLocation)
{
    // Remove lower priority objectives to make room
    for (int32 i = ActiveObjectives.Num() - 1; i >= 0; i--)
    {
        if (ActiveObjectives[i].Priority == EQuest_SurvivalPriority::Low)
        {
            ActiveObjectives.RemoveAt(i);
            break;
        }
    }
    
    FVector SafeLocation = ThreatLocation + FVector(1500, 1500, 0);
    CreateSurvivalObjective(ObjectiveType, SafeLocation, 120.0f);
}

void AQuest_SurvivalObjectiveManager::HandleNightfall()
{
    bNightSurvivalMode = true;
    
    // Create night survival objective
    CreateSurvivalObjective(EQuest_SurvivalObjectiveType::SurviveNight, GetActorLocation(), 480.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalObjectiveManager: Night survival mode activated"));
}

void AQuest_SurvivalObjectiveManager::HandleWeatherChange()
{
    // Simulate weather changes affecting objectives
    if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        // Bad weather - prioritize shelter
        for (FQuest_SurvivalObjective& Objective : ActiveObjectives)
        {
            if (Objective.ObjectiveType == EQuest_SurvivalObjectiveType::BuildShelter)
            {
                Objective.Priority = EQuest_SurvivalPriority::Critical;
            }
        }
    }
}

void AQuest_SurvivalObjectiveManager::MonitorPlayerNeeds(float Health, float Hunger, float Thirst, float Temperature)
{
    // Create objectives based on player's current needs
    if (Health < 30.0f)
    {
        CreateSurvivalObjective(EQuest_SurvivalObjectiveType::BuildShelter, FindSafeShelterLocation(), 300.0f);
    }
    
    if (Thirst < 20.0f)
    {
        CreateSurvivalObjective(EQuest_SurvivalObjectiveType::FindWater, FindNearestWaterSource(), 180.0f);
    }
    
    if (Hunger < 25.0f)
    {
        CreateSurvivalObjective(EQuest_SurvivalObjectiveType::GatherFood, GetActorLocation() + FVector(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), 0), 240.0f);
    }
    
    if (Temperature < 10.0f)
    {
        CreateSurvivalObjective(EQuest_SurvivalObjectiveType::CreateFire, GetActorLocation(), 200.0f);
    }
}

void AQuest_SurvivalObjectiveManager::CreateNeedBasedObjective(float Health, float Hunger, float Thirst)
{
    // Determine most urgent need
    if (Thirst < Health && Thirst < Hunger)
    {
        CreateSurvivalObjective(EQuest_SurvivalObjectiveType::FindWater, FindNearestWaterSource(), 300.0f);
    }
    else if (Hunger < Health)
    {
        CreateSurvivalObjective(EQuest_SurvivalObjectiveType::GatherFood, GetActorLocation() + FVector(800, 800, 0), 400.0f);
    }
    else
    {
        CreateSurvivalObjective(EQuest_SurvivalObjectiveType::BuildShelter, FindSafeShelterLocation(), 500.0f);
    }
}

void AQuest_SurvivalObjectiveManager::UpdateObjectiveTimers(float DeltaTime)
{
    for (FQuest_SurvivalObjective& Objective : ActiveObjectives)
    {
        if (Objective.bIsActive && Objective.TimeLimit > 0.0f)
        {
            Objective.RemainingTime -= DeltaTime;
        }
    }
}

void AQuest_SurvivalObjectiveManager::CheckForExpiredObjectives()
{
    for (int32 i = ActiveObjectives.Num() - 1; i >= 0; i--)
    {
        if (ActiveObjectives[i].RemainingTime <= 0.0f)
        {
            FailObjective(i);
        }
    }
}

void AQuest_SurvivalObjectiveManager::SortObjectivesByPriority()
{
    ActiveObjectives.Sort([](const FQuest_SurvivalObjective& A, const FQuest_SurvivalObjective& B) {
        return (int32)A.Priority < (int32)B.Priority;
    });
}

FVector AQuest_SurvivalObjectiveManager::FindNearestWaterSource()
{
    // Simple water source location logic
    TArray<FVector> WaterLocations = {
        FVector(2000, 1500, 50),
        FVector(-1500, 2500, 30),
        FVector(3000, -1000, 40)
    };
    
    FVector ClosestWater = WaterLocations[0];
    float ClosestDistance = FVector::Dist(GetActorLocation(), ClosestWater);
    
    for (const FVector& WaterLoc : WaterLocations)
    {
        float Distance = FVector::Dist(GetActorLocation(), WaterLoc);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestWater = WaterLoc;
        }
    }
    
    return ClosestWater;
}

FVector AQuest_SurvivalObjectiveManager::FindSafeShelterLocation()
{
    FVector SafeLocation = GetActorLocation();
    
    // Find elevated, protected location
    for (int32 i = 0; i < 10; i++)
    {
        FVector TestLocation = GetActorLocation() + FVector(FMath::RandRange(-2000, 2000), FMath::RandRange(-2000, 2000), 0);
        
        if (IsLocationSafe(TestLocation))
        {
            SafeLocation = TestLocation;
            break;
        }
    }
    
    return SafeLocation;
}

bool AQuest_SurvivalObjectiveManager::IsLocationSafe(FVector Location)
{
    // Basic safety check - avoid water and steep slopes
    FHitResult HitResult;
    FVector StartTrace = Location + FVector(0, 0, 1000);
    FVector EndTrace = Location - FVector(0, 0, 1000);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic))
    {
        FVector Normal = HitResult.Normal;
        float SlopeAngle = FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)) * 180.0f / PI;
        
        return SlopeAngle < 30.0f; // Safe if slope is less than 30 degrees
    }
    
    return false;
}

void AQuest_SurvivalObjectiveManager::SpawnObjectiveMarker(FVector Location, EQuest_SurvivalObjectiveType ObjectiveType)
{
    // Spawn a visual marker for the objective
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* Marker = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (Marker)
    {
        FString MarkerName = FString::Printf(TEXT("QuestMarker_%s"), *UEnum::GetValueAsString(ObjectiveType));
        Marker->SetActorLabel(MarkerName);
        
        UE_LOG(LogTemp, Warning, TEXT("SurvivalObjectiveManager: Spawned objective marker at %s"), *Location.ToString());
    }
}