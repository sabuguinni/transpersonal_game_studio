#include "Quest_CrowdDensityMissionTrigger.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "../AI/Crowd/CrowdSimulationManager.h"
#include "QuestManager.h"

AQuest_CrowdDensityMissionTrigger::AQuest_CrowdDensityMissionTrigger()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize mission properties
    bIsActive = true;
    MonitoringRadius = 2000.0f;
    UpdateInterval = 1.0f;
    
    // Initialize crowd state
    CurrentCrowdDensity = 0.0f;
    EntitiesInRange = 0;
    AverageMovementSpeed = 0.0f;
    bStampedeDetected = false;
    
    // Initialize mission progress
    bMissionTriggered = false;
    MissionTimer = 0.0f;
    bPlayerInTrigger = false;
    PlayerSurvivalTime = 0.0f;
    
    // Internal state
    LastUpdateTime = 0.0f;
    LastCrowdFlowDirection = FVector::ZeroVector;
    bStampedeWarningIssued = false;
    
    // Component references
    CrowdManager = nullptr;
    QuestManager = nullptr;
    
    // Set up collision
    if (GetCollisionComponent())
    {
        GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &AQuest_CrowdDensityMissionTrigger::OnTriggerBeginOverlap);
        GetCollisionComponent()->OnComponentEndOverlap.AddDynamic(this, &AQuest_CrowdDensityMissionTrigger::OnTriggerEndOverlap);
    }
}

void AQuest_CrowdDensityMissionTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    // Find crowd simulation manager
    CrowdManager = GetWorld()->GetGameInstance()->GetSubsystem<UCrowdSimulationManager>();
    
    // Find quest manager
    QuestManager = GetWorld()->GetGameInstance()->GetSubsystem<UQuestManager>();
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdDensityMissionTrigger initialized at %s"), *GetActorLocation().ToString());
}

void AQuest_CrowdDensityMissionTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsActive) return;
    
    // Update mission timer
    if (bMissionTriggered)
    {
        UpdateMissionProgress(DeltaTime);
    }
    
    // Update crowd monitoring at intervals
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime >= UpdateInterval)
    {
        UpdateCrowdMonitoring();
        CheckMissionConditions();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void AQuest_CrowdDensityMissionTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (Character->IsPlayerControlled())
        {
            bPlayerInTrigger = true;
            PlayerSurvivalTime = 0.0f;
            
            UE_LOG(LogTemp, Warning, TEXT("Player entered crowd density mission trigger"));
            
            // Start mission if conditions are met
            if (!bMissionTriggered && bIsActive)
            {
                StartMission();
            }
        }
    }
}

void AQuest_CrowdDensityMissionTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        if (Character->IsPlayerControlled())
        {
            bPlayerInTrigger = false;
            
            UE_LOG(LogTemp, Warning, TEXT("Player left crowd density mission trigger"));
            
            // Check if mission should be completed or failed
            if (bMissionTriggered)
            {
                if (PlayerSurvivalTime >= MissionData.TimeRequirement)
                {
                    CompleteMission();
                }
                else
                {
                    FailMission();
                }
            }
        }
    }
}

void AQuest_CrowdDensityMissionTrigger::StartMission()
{
    if (bMissionTriggered) return;
    
    bMissionTriggered = true;
    MissionTimer = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting crowd density mission: %s"), *MissionData.MissionName);
    
    // Notify quest manager
    if (QuestManager)
    {
        // QuestManager->StartMission(MissionData.MissionName);
    }
    
    BroadcastMissionUpdate();
}

void AQuest_CrowdDensityMissionTrigger::CompleteMission()
{
    if (!bMissionTriggered) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Completing crowd density mission: %s"), *MissionData.MissionName);
    
    // Notify quest manager
    if (QuestManager)
    {
        // QuestManager->CompleteMission(MissionData.MissionName, MissionData.RewardPoints);
    }
    
    ResetMission();
}

void AQuest_CrowdDensityMissionTrigger::FailMission()
{
    if (!bMissionTriggered) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Failing crowd density mission: %s"), *MissionData.MissionName);
    
    // Notify quest manager
    if (QuestManager)
    {
        // QuestManager->FailMission(MissionData.MissionName);
    }
    
    ResetMission();
}

void AQuest_CrowdDensityMissionTrigger::ResetMission()
{
    bMissionTriggered = false;
    MissionTimer = 0.0f;
    PlayerSurvivalTime = 0.0f;
    bStampedeWarningIssued = false;
    
    UE_LOG(LogTemp, Log, TEXT("Reset crowd density mission: %s"), *MissionData.MissionName);
}

float AQuest_CrowdDensityMissionTrigger::CalculateCrowdDensity()
{
    if (!GetWorld()) return 0.0f;
    
    // Get all actors in monitoring radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    int32 EntitiesCount = 0;
    float TotalSpeed = 0.0f;
    FVector TriggerLocation = GetActorLocation();
    
    NearbyEntities.Empty();
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == this) continue;
        
        float Distance = FVector::Distance(Actor->GetActorLocation(), TriggerLocation);
        if (Distance <= MonitoringRadius)
        {
            // Check if this is a crowd entity (has movement component or specific tags)
            if (Actor->GetVelocity().Size() > 0.1f || Actor->Tags.Contains("CrowdEntity"))
            {
                NearbyEntities.Add(Actor);
                EntitiesCount++;
                TotalSpeed += Actor->GetVelocity().Size();
            }
        }
    }
    
    EntitiesInRange = EntitiesCount;
    AverageMovementSpeed = EntitiesCount > 0 ? TotalSpeed / EntitiesCount : 0.0f;
    
    // Calculate density as entities per square kilometer
    float Area = PI * MonitoringRadius * MonitoringRadius / 1000000.0f; // Convert to km²
    CurrentCrowdDensity = Area > 0.0f ? EntitiesCount / Area : 0.0f;
    
    return CurrentCrowdDensity;
}

bool AQuest_CrowdDensityMissionTrigger::DetectStampede()
{
    // Stampede conditions: high density + high average speed + directional movement
    bool bHighDensity = CurrentCrowdDensity > 50.0f;
    bool bHighSpeed = AverageMovementSpeed > 800.0f;
    bool bDirectionalMovement = GetCrowdFlowDirection().Size() > 0.7f;
    
    bStampedeDetected = bHighDensity && bHighSpeed && bDirectionalMovement;
    
    if (bStampedeDetected && !bStampedeWarningIssued)
    {
        HandleStampedeEvent();
        bStampedeWarningIssued = true;
    }
    
    return bStampedeDetected;
}

FVector AQuest_CrowdDensityMissionTrigger::GetCrowdFlowDirection()
{
    if (NearbyEntities.Num() == 0) return FVector::ZeroVector;
    
    FVector AverageDirection = FVector::ZeroVector;
    int32 ValidEntities = 0;
    
    for (AActor* Entity : NearbyEntities)
    {
        if (Entity && Entity->GetVelocity().Size() > 50.0f)
        {
            AverageDirection += Entity->GetVelocity().GetSafeNormal();
            ValidEntities++;
        }
    }
    
    if (ValidEntities > 0)
    {
        LastCrowdFlowDirection = (AverageDirection / ValidEntities).GetSafeNormal();
    }
    
    return LastCrowdFlowDirection;
}

bool AQuest_CrowdDensityMissionTrigger::IsSafeNavigationPath(const FVector& StartLocation, const FVector& EndLocation)
{
    // Check if path intersects with high-density crowd areas
    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
    float Distance = FVector::Distance(StartLocation, EndLocation);
    
    // Sample points along the path
    int32 SampleCount = FMath::Max(1, FMath::FloorToInt(Distance / 500.0f));
    
    for (int32 i = 0; i <= SampleCount; i++)
    {
        float Alpha = SampleCount > 0 ? static_cast<float>(i) / SampleCount : 0.0f;
        FVector SamplePoint = FMath::Lerp(StartLocation, EndLocation, Alpha);
        
        // Check crowd density at sample point
        int32 LocalEntities = 0;
        for (AActor* Entity : NearbyEntities)
        {
            if (Entity && FVector::Distance(Entity->GetActorLocation(), SamplePoint) < 200.0f)
            {
                LocalEntities++;
            }
        }
        
        // If local density is too high, path is unsafe
        if (LocalEntities > 10)
        {
            return false;
        }
    }
    
    return true;
}

void AQuest_CrowdDensityMissionTrigger::UpdateCrowdMonitoring()
{
    CalculateCrowdDensity();
    DetectStampede();
    GetCrowdFlowDirection();
}

void AQuest_CrowdDensityMissionTrigger::CheckMissionConditions()
{
    if (!bIsActive || bMissionTriggered) return;
    
    // Check if trigger conditions are met based on mission type
    bool bConditionMet = false;
    
    switch (MissionData.TriggerType)
    {
        case EQuest_CrowdDensityTriggerType::HighDensity:
            bConditionMet = CurrentCrowdDensity >= MissionData.DensityThreshold;
            break;
            
        case EQuest_CrowdDensityTriggerType::LowDensity:
            bConditionMet = CurrentCrowdDensity <= MissionData.DensityThreshold;
            break;
            
        case EQuest_CrowdDensityTriggerType::FlowChange:
            bConditionMet = AverageMovementSpeed >= MissionData.DensityThreshold;
            break;
            
        case EQuest_CrowdDensityTriggerType::Stampede:
            bConditionMet = bStampedeDetected;
            break;
            
        case EQuest_CrowdDensityTriggerType::Gathering:
            bConditionMet = EntitiesInRange >= static_cast<int32>(MissionData.DensityThreshold);
            break;
    }
    
    // Start mission if player is in trigger and conditions are met
    if (bConditionMet && bPlayerInTrigger && !bMissionTriggered)
    {
        StartMission();
    }
}

void AQuest_CrowdDensityMissionTrigger::ProcessPlayerInTrigger()
{
    if (!bPlayerInTrigger || !bMissionTriggered) return;
    
    // Update survival time
    PlayerSurvivalTime += UpdateInterval;
    
    // Check for mission completion
    if (PlayerSurvivalTime >= MissionData.TimeRequirement)
    {
        CompleteMission();
    }
}

void AQuest_CrowdDensityMissionTrigger::HandleStampedeEvent()
{
    UE_LOG(LogTemp, Warning, TEXT("STAMPEDE DETECTED at %s! Density: %.1f, Speed: %.1f"), 
        *GetActorLocation().ToString(), CurrentCrowdDensity, AverageMovementSpeed);
    
    // Trigger emergency mission if player is nearby
    if (bPlayerInTrigger && !bMissionTriggered)
    {
        MissionData.TriggerType = EQuest_CrowdDensityTriggerType::Stampede;
        MissionData.MissionName = TEXT("Survive the Stampede");
        MissionData.MissionDescription = TEXT("Survive the massive stampede without being trampled!");
        MissionData.TimeRequirement = 30.0f; // Shorter time for stampede survival
        MissionData.RewardPoints = 200; // Higher reward for danger
        
        StartMission();
    }
}

void AQuest_CrowdDensityMissionTrigger::UpdateMissionProgress(float DeltaTime)
{
    if (!bMissionTriggered) return;
    
    MissionTimer += DeltaTime;
    
    if (bPlayerInTrigger)
    {
        PlayerSurvivalTime += DeltaTime;
        ProcessPlayerInTrigger();
    }
    
    // Check for mission timeout
    if (MissionTimer > MissionData.TimeRequirement * 2.0f)
    {
        FailMission();
    }
}

void AQuest_CrowdDensityMissionTrigger::BroadcastMissionUpdate()
{
    // Broadcast mission status to UI and other systems
    if (GEngine)
    {
        FString StatusMessage = FString::Printf(TEXT("Mission: %s - Progress: %.1f/%.1f seconds"), 
            *MissionData.MissionName, PlayerSurvivalTime, MissionData.TimeRequirement);
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, StatusMessage);
    }
}