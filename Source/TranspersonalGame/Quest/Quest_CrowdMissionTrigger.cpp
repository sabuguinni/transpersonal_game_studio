#include "Quest_CrowdMissionTrigger.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/TranspersonalCharacter.h"

AQuest_CrowdMissionTrigger::AQuest_CrowdMissionTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger box component
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
    TriggerBox->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AQuest_CrowdMissionTrigger::OnTriggerBeginOverlap);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AQuest_CrowdMissionTrigger::OnTriggerEndOverlap);

    // Initialize mission data
    MissionData.MissionType = EQuest_CrowdMissionType::NavigateCrowd;
    MissionData.MissionTitle = TEXT("Navigate the Crowd");
    MissionData.MissionDescription = TEXT("Move through the crowd without causing panic");
    MissionData.RequiredCrowdDensity = 10;
    MissionData.TimeLimit = 300.0f;
    MissionData.ExperienceReward = 100;
    MissionData.bRequiresStealthMode = false;
    MissionData.CrowdPanicThreshold = 0.7f;

    // Initialize state
    bMissionActive = false;
    bMissionCompleted = false;
    CrowdDetectionRadius = 1000.0f;
    CurrentCrowdCount = 0;
    CurrentCrowdPanicLevel = 0.0f;
    MissionTimer = 0.0f;
}

void AQuest_CrowdMissionTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize crowd detection
    UpdateCrowdStatus();
}

void AQuest_CrowdMissionTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bMissionActive && !bMissionCompleted)
    {
        MissionTimer += DeltaTime;
        
        // Update crowd status
        UpdateCrowdStatus();
        
        // Process mission logic
        ProcessMissionLogic();
        
        // Check for mission completion or failure
        CheckMissionCompletion();
        
        // Check time limit
        if (MissionData.TimeLimit > 0.0f && MissionTimer >= MissionData.TimeLimit)
        {
            FailMission();
        }
    }
}

void AQuest_CrowdMissionTrigger::StartMission()
{
    if (!bMissionActive && !bMissionCompleted)
    {
        bMissionActive = true;
        MissionTimer = 0.0f;
        CurrentCrowdPanicLevel = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Mission Started: %s"), *MissionData.MissionTitle);
        OnMissionStarted();
    }
}

void AQuest_CrowdMissionTrigger::CompleteMission()
{
    if (bMissionActive && !bMissionCompleted)
    {
        bMissionActive = false;
        bMissionCompleted = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Mission Completed: %s - Reward: %d XP"), 
               *MissionData.MissionTitle, MissionData.ExperienceReward);
        
        OnMissionCompleted();
    }
}

void AQuest_CrowdMissionTrigger::FailMission()
{
    if (bMissionActive && !bMissionCompleted)
    {
        bMissionActive = false;
        
        UE_LOG(LogTemp, Warning, TEXT("Mission Failed: %s"), *MissionData.MissionTitle);
        OnMissionFailed();
    }
}

void AQuest_CrowdMissionTrigger::UpdateCrowdStatus()
{
    CurrentCrowdCount = GetCrowdCountInRadius();
    float NewPanicLevel = CalculateCrowdPanicLevel();
    
    if (FMath::Abs(NewPanicLevel - CurrentCrowdPanicLevel) > 0.1f)
    {
        CurrentCrowdPanicLevel = NewPanicLevel;
        OnCrowdPanicLevelChanged(CurrentCrowdPanicLevel);
    }
}

int32 AQuest_CrowdMissionTrigger::GetCrowdCountInRadius()
{
    if (!GetWorld())
    {
        return 0;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::class, FoundActors);
    
    int32 CrowdCount = 0;
    FVector MyLocation = GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
        {
            float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
            if (Distance <= CrowdDetectionRadius)
            {
                CrowdCount++;
            }
        }
    }
    
    return CrowdCount;
}

float AQuest_CrowdMissionTrigger::CalculateCrowdPanicLevel()
{
    // Base panic level calculation
    float BasePanic = FMath::Clamp(CurrentCrowdCount / 50.0f, 0.0f, 1.0f);
    
    // Add player movement influence
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerChar)
    {
        float PlayerSpeed = PlayerChar->GetVelocity().Size();
        float SpeedInfluence = FMath::Clamp(PlayerSpeed / 1000.0f, 0.0f, 0.5f);
        BasePanic += SpeedInfluence;
    }
    
    return FMath::Clamp(BasePanic, 0.0f, 1.0f);
}

void AQuest_CrowdMissionTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ATranspersonalCharacter* PlayerChar = Cast<ATranspersonalCharacter>(OtherActor);
    if (PlayerChar && !bMissionActive && !bMissionCompleted)
    {
        // Check if crowd density requirements are met
        if (CurrentCrowdCount >= MissionData.RequiredCrowdDensity)
        {
            StartMission();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission requires %d crowd members, current: %d"), 
                   MissionData.RequiredCrowdDensity, CurrentCrowdCount);
        }
    }
}

void AQuest_CrowdMissionTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    ATranspersonalCharacter* PlayerChar = Cast<ATranspersonalCharacter>(OtherActor);
    if (PlayerChar && bMissionActive)
    {
        // Player left the mission area - check mission type
        switch (MissionData.MissionType)
        {
            case EQuest_CrowdMissionType::NavigateCrowd:
                // Navigation missions complete when player exits successfully
                if (ValidatePlayerBehavior())
                {
                    CompleteMission();
                }
                else
                {
                    FailMission();
                }
                break;
                
            default:
                // Other mission types may require staying in area
                break;
        }
    }
}

void AQuest_CrowdMissionTrigger::ProcessMissionLogic()
{
    switch (MissionData.MissionType)
    {
        case EQuest_CrowdMissionType::NavigateCrowd:
            // Check if player is moving too aggressively
            if (CurrentCrowdPanicLevel > MissionData.CrowdPanicThreshold)
            {
                FailMission();
            }
            break;
            
        case EQuest_CrowdMissionType::AvoidCrowdDanger:
            // Check if player is maintaining safe distance from dangerous crowd areas
            break;
            
        case EQuest_CrowdMissionType::LeadCrowd:
            // Check if crowd is following player to designated area
            break;
            
        case EQuest_CrowdMissionType::EscortNPC:
            // Check if NPC is safe and following
            break;
            
        case EQuest_CrowdMissionType::CrowdHunt:
            // Check if target has been eliminated without causing panic
            break;
            
        case EQuest_CrowdMissionType::CrowdDefense:
            // Check if crowd is being protected from threats
            break;
            
        case EQuest_CrowdMissionType::CrowdGathering:
            // Check if resources are being collected efficiently
            break;
    }
}

void AQuest_CrowdMissionTrigger::CheckMissionCompletion()
{
    // Mission-specific completion checks
    bool bShouldComplete = false;
    
    switch (MissionData.MissionType)
    {
        case EQuest_CrowdMissionType::NavigateCrowd:
            // Completion handled in OnTriggerEndOverlap
            break;
            
        case EQuest_CrowdMissionType::CrowdGathering:
            // Example: Complete when enough resources gathered
            // bShouldComplete = (GatheredResources >= RequiredResources);
            break;
            
        default:
            break;
    }
    
    if (bShouldComplete)
    {
        CompleteMission();
    }
}

bool AQuest_CrowdMissionTrigger::ValidatePlayerBehavior()
{
    // Validate that player completed mission objectives properly
    
    // Check panic level threshold
    if (CurrentCrowdPanicLevel > MissionData.CrowdPanicThreshold)
    {
        return false;
    }
    
    // Check stealth requirement
    if (MissionData.bRequiresStealthMode)
    {
        ATranspersonalCharacter* PlayerChar = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        if (PlayerChar)
        {
            // Check if player maintained stealth (example logic)
            float PlayerSpeed = PlayerChar->GetVelocity().Size();
            if (PlayerSpeed > 300.0f) // Too fast for stealth
            {
                return false;
            }
        }
    }
    
    return true;
}