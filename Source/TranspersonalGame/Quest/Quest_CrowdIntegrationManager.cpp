#include "Quest_CrowdIntegrationManager.h"
#include "AI/Crowd/Crowd_MassEntityManager.h"
#include "Character/TranspersonalCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"

AQuest_CrowdIntegrationManager::AQuest_CrowdIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create quest marker mesh
    QuestMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarkerMesh"));
    QuestMarkerMesh->SetupAttachment(RootComponent);
    QuestMarkerMesh->SetRelativeLocation(FVector(0, 0, 300));
    QuestMarkerMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));

    // Initialize quest properties
    CrowdInteractionRadius = 1000.0f;
    CurrentCrowdCount = 0;
    CrowdDensityThreshold = 50.0f;
    bQuestSystemActive = true;
    ActiveQuestCount = 0;
    TotalQuestRewards = 0.0f;

    // Initialize timers
    EscortMissionTimer = 0.0f;
    CrowdControlTimer = 0.0f;
    EvacuationTimer = 0.0f;

    // Initialize internal tracking
    LastCrowdUpdateTime = 0.0f;
    CompletedQuestCount = 0;
    SystemStartTime = 0.0f;

    // Initialize default escort objectives
    FQuest_EscortObjective DefaultEscort;
    DefaultEscort.ObjectiveName = TEXT("Escort Survivors to Safety");
    DefaultEscort.RequiredCrowdSize = 15;
    DefaultEscort.EscortRadius = 600.0f;
    DefaultEscort.CompletionReward = 200.0f;
    EscortObjectives.Add(DefaultEscort);

    // Initialize default crowd control objectives
    FQuest_CrowdControlObjective DefaultControl;
    DefaultControl.ObjectiveName = TEXT("Maintain Order During Crisis");
    DefaultControl.ControlRadius = 800.0f;
    DefaultControl.MaxCrowdDensity = 30;
    DefaultControl.TimeLimit = 240.0f;
    DefaultControl.CompletionReward = 150.0f;
    CrowdControlObjectives.Add(DefaultControl);

    // Initialize default evacuation objectives
    FQuest_CrowdEvacuationObjective DefaultEvacuation;
    DefaultEvacuation.ObjectiveName = TEXT("Emergency Evacuation");
    DefaultEvacuation.DangerZoneRadius = 1200.0f;
    DefaultEvacuation.RequiredEvacuees = 20;
    DefaultEvacuation.TimeLimit = 480.0f;
    DefaultEvacuation.CompletionReward = 300.0f;
    EvacuationObjectives.Add(DefaultEvacuation);
}

void AQuest_CrowdIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    SystemStartTime = GetWorld()->GetTimeSeconds();
    
    // Find crowd manager in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), UCrowd_MassEntityManager::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        CrowdManager = Cast<UCrowd_MassEntityManager>(FoundActors[0]);
        if (CrowdManager)
        {
            UE_LOG(LogTemp, Log, TEXT("Quest system connected to crowd manager"));
        }
    }

    // Initialize progress tracking arrays
    EscortProgressTracking.SetNum(EscortObjectives.Num());
    CrowdControlProgressTracking.SetNum(CrowdControlObjectives.Num());
    EvacuationProgressTracking.SetNum(EvacuationObjectives.Num());

    for (int32 i = 0; i < EscortProgressTracking.Num(); i++)
    {
        EscortProgressTracking[i] = 0.0f;
    }
    for (int32 i = 0; i < CrowdControlProgressTracking.Num(); i++)
    {
        CrowdControlProgressTracking[i] = 0.0f;
    }
    for (int32 i = 0; i < EvacuationProgressTracking.Num(); i++)
    {
        EvacuationProgressTracking[i] = 0.0f;
    }

    UpdateQuestMarkers();
}

void AQuest_CrowdIntegrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bQuestSystemActive) return;

    // Update crowd count
    CurrentCrowdCount = GetNearbyCrowdCount();
    
    // Process active objectives
    ProcessEscortObjectives(DeltaTime);
    ProcessCrowdControlObjectives(DeltaTime);
    ProcessEvacuationObjectives(DeltaTime);

    // Update timers
    LastCrowdUpdateTime += DeltaTime;
    if (LastCrowdUpdateTime >= 1.0f) // Update every second
    {
        LastCrowdUpdateTime = 0.0f;
        float CurrentDensity = CalculateCrowdDensity();
        OnCrowdDensityChanged(CurrentDensity);
    }
}

void AQuest_CrowdIntegrationManager::StartEscortMission(int32 ObjectiveIndex)
{
    if (ObjectiveIndex >= 0 && ObjectiveIndex < EscortObjectives.Num())
    {
        EscortObjectives[ObjectiveIndex].bIsActive = true;
        EscortMissionTimer = 0.0f;
        EscortProgressTracking[ObjectiveIndex] = 0.0f;
        ActiveQuestCount++;
        
        UE_LOG(LogTemp, Log, TEXT("Started escort mission: %s"), *EscortObjectives[ObjectiveIndex].ObjectiveName);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                FString::Printf(TEXT("Quest Started: %s"), *EscortObjectives[ObjectiveIndex].ObjectiveName));
        }
    }
}

void AQuest_CrowdIntegrationManager::StartCrowdControlMission(int32 ObjectiveIndex)
{
    if (ObjectiveIndex >= 0 && ObjectiveIndex < CrowdControlObjectives.Num())
    {
        CrowdControlObjectives[ObjectiveIndex].bIsActive = true;
        CrowdControlTimer = 0.0f;
        CrowdControlProgressTracking[ObjectiveIndex] = 0.0f;
        ActiveQuestCount++;
        
        UE_LOG(LogTemp, Log, TEXT("Started crowd control mission: %s"), *CrowdControlObjectives[ObjectiveIndex].ObjectiveName);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, 
                FString::Printf(TEXT("Quest Started: %s"), *CrowdControlObjectives[ObjectiveIndex].ObjectiveName));
        }
    }
}

void AQuest_CrowdIntegrationManager::StartEvacuationMission(int32 ObjectiveIndex)
{
    if (ObjectiveIndex >= 0 && ObjectiveIndex < EvacuationObjectives.Num())
    {
        EvacuationObjectives[ObjectiveIndex].bIsActive = true;
        EvacuationTimer = 0.0f;
        EvacuationProgressTracking[ObjectiveIndex] = 0.0f;
        ActiveQuestCount++;
        
        UE_LOG(LogTemp, Log, TEXT("Started evacuation mission: %s"), *EvacuationObjectives[ObjectiveIndex].ObjectiveName);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("Emergency Quest: %s"), *EvacuationObjectives[ObjectiveIndex].ObjectiveName));
        }
    }
}

void AQuest_CrowdIntegrationManager::CompleteQuest(const FString& QuestName, float RewardAmount)
{
    TotalQuestRewards += RewardAmount;
    CompletedQuestCount++;
    ActiveQuestCount = FMath::Max(0, ActiveQuestCount - 1);
    
    UE_LOG(LogTemp, Log, TEXT("Quest completed: %s - Reward: %.2f"), *QuestName, RewardAmount);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
            FString::Printf(TEXT("Quest Complete! %s - Reward: %.0f"), *QuestName, RewardAmount));
    }
}

void AQuest_CrowdIntegrationManager::FailQuest(const FString& QuestName)
{
    ActiveQuestCount = FMath::Max(0, ActiveQuestCount - 1);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest failed: %s"), *QuestName);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
            FString::Printf(TEXT("Quest Failed: %s"), *QuestName));
    }
}

int32 AQuest_CrowdIntegrationManager::GetNearbyCrowdCount()
{
    int32 Count = 0;
    
    if (CrowdManager)
    {
        // This would interface with the crowd manager to get actual count
        // For now, simulate based on detection sphere overlaps
        TArray<AActor*> OverlappingActors;
        DetectionSphere->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
        
        // Filter for crowd entities (not player)
        for (AActor* Actor : OverlappingActors)
        {
            if (Actor && !Actor->IsA<ATranspersonalCharacter>())
            {
                Count++;
            }
        }
    }
    
    return Count;
}

float AQuest_CrowdIntegrationManager::CalculateCrowdDensity()
{
    if (CrowdInteractionRadius <= 0.0f) return 0.0f;
    
    float Area = PI * CrowdInteractionRadius * CrowdInteractionRadius;
    return (float)CurrentCrowdCount / (Area / 10000.0f); // Normalize to per 100x100 unit area
}

void AQuest_CrowdIntegrationManager::TriggerCrowdPanic()
{
    if (CrowdManager)
    {
        // Interface with crowd manager to trigger panic behavior
        UE_LOG(LogTemp, Warning, TEXT("Crowd panic triggered at quest location"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("CROWD PANIC!"));
        }
    }
}

void AQuest_CrowdIntegrationManager::DirectCrowdToLocation(const FVector& TargetLocation)
{
    if (CrowdManager)
    {
        // Interface with crowd manager to direct crowd movement
        UE_LOG(LogTemp, Log, TEXT("Directing crowd to location: %s"), *TargetLocation.ToString());
    }
}

void AQuest_CrowdIntegrationManager::SetCrowdBehaviorZone(const FVector& Center, float Radius, int32 BehaviorType)
{
    if (CrowdManager)
    {
        // Interface with crowd manager to set behavior zones
        UE_LOG(LogTemp, Log, TEXT("Setting crowd behavior zone at %s with radius %.2f"), *Center.ToString(), Radius);
    }
}

bool AQuest_CrowdIntegrationManager::ValidateEscortProgress(int32 ObjectiveIndex)
{
    if (ObjectiveIndex < 0 || ObjectiveIndex >= EscortObjectives.Num()) return false;
    
    const FQuest_EscortObjective& Objective = EscortObjectives[ObjectiveIndex];
    if (!Objective.bIsActive) return false;
    
    // Check if required crowd size is being escorted
    int32 EscortedCount = GetNearbyCrowdCount();
    bool bSufficientCrowd = EscortedCount >= Objective.RequiredCrowdSize;
    
    // Update progress
    if (bSufficientCrowd)
    {
        EscortProgressTracking[ObjectiveIndex] += 0.1f; // Increment progress
        if (EscortProgressTracking[ObjectiveIndex] >= 1.0f)
        {
            OnEscortMissionComplete(ObjectiveIndex);
            return true;
        }
    }
    
    return false;
}

bool AQuest_CrowdIntegrationManager::ValidateCrowdControlProgress(int32 ObjectiveIndex)
{
    if (ObjectiveIndex < 0 || ObjectiveIndex >= CrowdControlObjectives.Num()) return false;
    
    const FQuest_CrowdControlObjective& Objective = CrowdControlObjectives[ObjectiveIndex];
    if (!Objective.bIsActive) return false;
    
    // Check crowd density within control radius
    float CurrentDensity = CalculateCrowdDensity();
    bool bDensityControlled = CurrentDensity <= (float)Objective.MaxCrowdDensity;
    
    // Update progress based on control maintenance
    if (bDensityControlled)
    {
        CrowdControlProgressTracking[ObjectiveIndex] += 0.05f;
        if (CrowdControlProgressTracking[ObjectiveIndex] >= 1.0f)
        {
            OnCrowdControlMissionComplete(ObjectiveIndex);
            return true;
        }
    }
    else
    {
        CrowdControlProgressTracking[ObjectiveIndex] = FMath::Max(0.0f, CrowdControlProgressTracking[ObjectiveIndex] - 0.02f);
    }
    
    return false;
}

bool AQuest_CrowdIntegrationManager::ValidateEvacuationProgress(int32 ObjectiveIndex)
{
    if (ObjectiveIndex < 0 || ObjectiveIndex >= EvacuationObjectives.Num()) return false;
    
    const FQuest_CrowdEvacuationObjective& Objective = EvacuationObjectives[ObjectiveIndex];
    if (!Objective.bIsActive) return false;
    
    // Check evacuation progress (simplified - count crowd near safe zone)
    FVector SafeZoneLocation = Objective.SafeZoneCenter;
    int32 EvacuatedCount = 0;
    
    // This would check actual crowd positions relative to safe zone
    // For now, simulate based on time and crowd management
    float EvacuationRate = (float)CurrentCrowdCount / Objective.RequiredEvacuees;
    EvacuationProgressTracking[ObjectiveIndex] += EvacuationRate * 0.01f;
    
    if (EvacuationProgressTracking[ObjectiveIndex] >= 1.0f)
    {
        OnEvacuationMissionComplete(ObjectiveIndex);
        return true;
    }
    
    return false;
}

void AQuest_CrowdIntegrationManager::UpdateQuestMarkers()
{
    // Update visual markers based on active quests
    if (QuestMarkerMesh)
    {
        bool bHasActiveQuests = false;
        
        // Check for any active objectives
        for (const FQuest_EscortObjective& Escort : EscortObjectives)
        {
            if (Escort.bIsActive) { bHasActiveQuests = true; break; }
        }
        for (const FQuest_CrowdControlObjective& Control : CrowdControlObjectives)
        {
            if (Control.bIsActive) { bHasActiveQuests = true; break; }
        }
        for (const FQuest_CrowdEvacuationObjective& Evacuation : EvacuationObjectives)
        {
            if (Evacuation.bIsActive) { bHasActiveQuests = true; break; }
        }
        
        QuestMarkerMesh->SetVisibility(bHasActiveQuests);
    }
}

void AQuest_CrowdIntegrationManager::ResetAllQuests()
{
    // Reset all objectives
    for (FQuest_EscortObjective& Escort : EscortObjectives)
    {
        Escort.bIsActive = false;
    }
    for (FQuest_CrowdControlObjective& Control : CrowdControlObjectives)
    {
        Control.bIsActive = false;
    }
    for (FQuest_CrowdEvacuationObjective& Evacuation : EvacuationObjectives)
    {
        Evacuation.bIsActive = false;
    }
    
    // Reset counters
    ActiveQuestCount = 0;
    EscortMissionTimer = 0.0f;
    CrowdControlTimer = 0.0f;
    EvacuationTimer = 0.0f;
    
    // Reset progress tracking
    for (int32 i = 0; i < EscortProgressTracking.Num(); i++)
    {
        EscortProgressTracking[i] = 0.0f;
    }
    for (int32 i = 0; i < CrowdControlProgressTracking.Num(); i++)
    {
        CrowdControlProgressTracking[i] = 0.0f;
    }
    for (int32 i = 0; i < EvacuationProgressTracking.Num(); i++)
    {
        EvacuationProgressTracking[i] = 0.0f;
    }
    
    UpdateQuestMarkers();
    
    UE_LOG(LogTemp, Log, TEXT("All quests reset"));
}

FString AQuest_CrowdIntegrationManager::GetQuestStatusReport()
{
    FString Report = FString::Printf(TEXT("=== Quest System Status ===\n"));
    Report += FString::Printf(TEXT("Active Quests: %d\n"), ActiveQuestCount);
    Report += FString::Printf(TEXT("Completed Quests: %d\n"), CompletedQuestCount);
    Report += FString::Printf(TEXT("Total Rewards: %.2f\n"), TotalQuestRewards);
    Report += FString::Printf(TEXT("Crowd Count: %d\n"), CurrentCrowdCount);
    Report += FString::Printf(TEXT("Crowd Density: %.2f\n"), CalculateCrowdDensity());
    Report += FString::Printf(TEXT("System Uptime: %.2f seconds\n"), GetWorld()->GetTimeSeconds() - SystemStartTime);
    
    return Report;
}

void AQuest_CrowdIntegrationManager::ProcessEscortObjectives(float DeltaTime)
{
    EscortMissionTimer += DeltaTime;
    
    for (int32 i = 0; i < EscortObjectives.Num(); i++)
    {
        if (EscortObjectives[i].bIsActive)
        {
            ValidateEscortProgress(i);
        }
    }
}

void AQuest_CrowdIntegrationManager::ProcessCrowdControlObjectives(float DeltaTime)
{
    CrowdControlTimer += DeltaTime;
    
    for (int32 i = 0; i < CrowdControlObjectives.Num(); i++)
    {
        if (CrowdControlObjectives[i].bIsActive)
        {
            // Check time limit
            if (CrowdControlTimer >= CrowdControlObjectives[i].TimeLimit)
            {
                if (CrowdControlProgressTracking[i] >= 1.0f)
                {
                    OnCrowdControlMissionComplete(i);
                }
                else
                {
                    FailQuest(CrowdControlObjectives[i].ObjectiveName);
                    CrowdControlObjectives[i].bIsActive = false;
                }
            }
            else
            {
                ValidateCrowdControlProgress(i);
            }
        }
    }
}

void AQuest_CrowdIntegrationManager::ProcessEvacuationObjectives(float DeltaTime)
{
    EvacuationTimer += DeltaTime;
    
    for (int32 i = 0; i < EvacuationObjectives.Num(); i++)
    {
        if (EvacuationObjectives[i].bIsActive)
        {
            // Check time limit
            if (EvacuationTimer >= EvacuationObjectives[i].TimeLimit)
            {
                if (EvacuationProgressTracking[i] >= 1.0f)
                {
                    OnEvacuationMissionComplete(i);
                }
                else
                {
                    FailQuest(EvacuationObjectives[i].ObjectiveName);
                    EvacuationObjectives[i].bIsActive = false;
                }
            }
            else
            {
                ValidateEvacuationProgress(i);
            }
        }
    }
}

void AQuest_CrowdIntegrationManager::OnEscortMissionComplete(int32 ObjectiveIndex)
{
    if (ObjectiveIndex >= 0 && ObjectiveIndex < EscortObjectives.Num())
    {
        FQuest_EscortObjective& Objective = EscortObjectives[ObjectiveIndex];
        CompleteQuest(Objective.ObjectiveName, Objective.CompletionReward);
        Objective.bIsActive = false;
        EscortProgressTracking[ObjectiveIndex] = 0.0f;
    }
}

void AQuest_CrowdIntegrationManager::OnCrowdControlMissionComplete(int32 ObjectiveIndex)
{
    if (ObjectiveIndex >= 0 && ObjectiveIndex < CrowdControlObjectives.Num())
    {
        FQuest_CrowdControlObjective& Objective = CrowdControlObjectives[ObjectiveIndex];
        CompleteQuest(Objective.ObjectiveName, Objective.CompletionReward);
        Objective.bIsActive = false;
        CrowdControlProgressTracking[ObjectiveIndex] = 0.0f;
    }
}

void AQuest_CrowdIntegrationManager::OnEvacuationMissionComplete(int32 ObjectiveIndex)
{
    if (ObjectiveIndex >= 0 && ObjectiveIndex < EvacuationObjectives.Num())
    {
        FQuest_CrowdEvacuationObjective& Objective = EvacuationObjectives[ObjectiveIndex];
        CompleteQuest(Objective.ObjectiveName, Objective.CompletionReward);
        Objective.bIsActive = false;
        EvacuationProgressTracking[ObjectiveIndex] = 0.0f;
    }
}

void AQuest_CrowdIntegrationManager::OnCrowdDensityChanged(float NewDensity)
{
    if (NewDensity > CrowdDensityThreshold * 1.5f)
    {
        // High density - might trigger panic or control missions
        UE_LOG(LogTemp, Warning, TEXT("High crowd density detected: %.2f"), NewDensity);
    }
}

void AQuest_CrowdIntegrationManager::OnCrowdPanicTriggered()
{
    // Handle panic events - might auto-start evacuation missions
    for (int32 i = 0; i < EvacuationObjectives.Num(); i++)
    {
        if (!EvacuationObjectives[i].bIsActive)
        {
            StartEvacuationMission(i);
            break; // Start only one evacuation mission
        }
    }
}

void AQuest_CrowdIntegrationManager::OnCrowdReachedWaypoint(int32 WaypointIndex)
{
    // Handle waypoint events for escort missions
    UE_LOG(LogTemp, Log, TEXT("Crowd reached waypoint %d"), WaypointIndex);
}