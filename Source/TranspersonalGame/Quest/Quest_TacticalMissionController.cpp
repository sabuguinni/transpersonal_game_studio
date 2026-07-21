#include "Quest_TacticalMissionController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AQuest_TacticalMissionController::AQuest_TacticalMissionController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentMissionID = TEXT("");
    CurrentMissionType = EQuest_TacticalMissionType::None;
    MissionStartTime = 0.0f;
    ThreatAssessmentRadius = 2000.0f;
    MaxSquadSize = 8;
    bMissionInProgress = false;
    
    // Initialize arrays
    ActiveObjectives.Empty();
    SquadMembers.Empty();
}

void AQuest_TacticalMissionController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_TacticalMissionController: Tactical mission system initialized"));
    
    // Set up periodic threat assessment
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &AQuest_TacticalMissionController::UpdateThreatAssessment,
        5.0f,
        true
    );
}

void AQuest_TacticalMissionController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bMissionInProgress)
    {
        MonitorSquadHealth();
        CheckObjectiveCompletion();
    }
}

bool AQuest_TacticalMissionController::StartTacticalMission(const FString& MissionID, EQuest_TacticalMissionType MissionType, const FVector& TargetLocation)
{
    if (bMissionInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_TacticalMissionController: Cannot start new mission - mission already in progress"));
        return false;
    }
    
    CurrentMissionID = MissionID;
    CurrentMissionType = MissionType;
    MissionStartTime = GetWorld()->GetTimeSeconds();
    bMissionInProgress = true;
    
    // Generate initial objectives based on mission type
    switch (MissionType)
    {
        case EQuest_TacticalMissionType::DefendPosition:
            {
                FQuest_TacticalObjective DefenseObj = CreateDefensiveObjective(TargetLocation, 1000.0f);
                ActiveObjectives.Add(DefenseObj);
            }
            break;
            
        case EQuest_TacticalMissionType::AssaultTarget:
            {
                EQuest_ThreatLevel ThreatLevel = AssessThreatLevel(TargetLocation);
                FQuest_TacticalObjective AssaultObj = CreateOffensiveObjective(TargetLocation, ThreatLevel);
                ActiveObjectives.Add(AssaultObj);
            }
            break;
            
        case EQuest_TacticalMissionType::Reconnaissance:
            {
                FQuest_TacticalObjective ReconObj = CreateReconObjective(TargetLocation, 1500.0f);
                ActiveObjectives.Add(ReconObj);
            }
            break;
            
        default:
            GenerateDynamicObjectives(TargetLocation, 2);
            break;
    }
    
    LogTacticalEvent(FString::Printf(TEXT("Mission Started: %s at location %s"), *MissionID, *TargetLocation.ToString()));
    return true;
}

void AQuest_TacticalMissionController::CompleteTacticalMission()
{
    if (!bMissionInProgress)
    {
        return;
    }
    
    float MissionDuration = GetWorld()->GetTimeSeconds() - MissionStartTime;
    float SuccessRate = CalculateMissionSuccessProbability();
    
    LogTacticalEvent(FString::Printf(TEXT("Mission Completed: %s - Duration: %.1fs - Success Rate: %.1f%%"), 
        *CurrentMissionID, MissionDuration, SuccessRate * 100.0f));
    
    // Reset mission state
    bMissionInProgress = false;
    ActiveObjectives.Empty();
    CurrentMissionID = TEXT("");
    CurrentMissionType = EQuest_TacticalMissionType::None;
}

void AQuest_TacticalMissionController::AbortTacticalMission()
{
    if (bMissionInProgress)
    {
        LogTacticalEvent(FString::Printf(TEXT("Mission Aborted: %s"), *CurrentMissionID));
        
        bMissionInProgress = false;
        ActiveObjectives.Empty();
        CurrentMissionID = TEXT("");
        CurrentMissionType = EQuest_TacticalMissionType::None;
    }
}

bool AQuest_TacticalMissionController::AddSquadMember(const FString& MemberID, EQuest_SquadRole Role, const FVector& Position)
{
    if (SquadMembers.Num() >= MaxSquadSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_TacticalMissionController: Squad at maximum capacity"));
        return false;
    }
    
    // Check if member already exists
    for (const FQuest_SquadMember& Member : SquadMembers)
    {
        if (Member.MemberID == MemberID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_TacticalMissionController: Squad member %s already exists"), *MemberID);
            return false;
        }
    }
    
    FQuest_SquadMember NewMember;
    NewMember.MemberID = MemberID;
    NewMember.Role = Role;
    NewMember.Position = Position;
    NewMember.HealthPercentage = 100.0f;
    NewMember.StaminaPercentage = 100.0f;
    NewMember.bIsAlive = true;
    NewMember.bIsInCombat = false;
    
    SquadMembers.Add(NewMember);
    LogTacticalEvent(FString::Printf(TEXT("Squad Member Added: %s - Role: %d"), *MemberID, (int32)Role));
    
    return true;
}

void AQuest_TacticalMissionController::RemoveSquadMember(const FString& MemberID)
{
    for (int32 i = 0; i < SquadMembers.Num(); i++)
    {
        if (SquadMembers[i].MemberID == MemberID)
        {
            SquadMembers.RemoveAt(i);
            LogTacticalEvent(FString::Printf(TEXT("Squad Member Removed: %s"), *MemberID));
            break;
        }
    }
}

void AQuest_TacticalMissionController::UpdateSquadMemberStatus(const FString& MemberID, float Health, float Stamina, bool bInCombat)
{
    for (FQuest_SquadMember& Member : SquadMembers)
    {
        if (Member.MemberID == MemberID)
        {
            Member.HealthPercentage = FMath::Clamp(Health, 0.0f, 100.0f);
            Member.StaminaPercentage = FMath::Clamp(Stamina, 0.0f, 100.0f);
            Member.bIsInCombat = bInCombat;
            Member.bIsAlive = (Member.HealthPercentage > 0.0f);
            break;
        }
    }
}

EQuest_ThreatLevel AQuest_TacticalMissionController::AssessThreatLevel(const FVector& Location)
{
    // Simulate threat assessment based on location and environmental factors
    float DistanceFromOrigin = FVector::Dist(Location, FVector::ZeroVector);
    float ThreatScore = 0.0f;
    
    // Distance-based threat (further = more dangerous)
    ThreatScore += DistanceFromOrigin / 10000.0f;
    
    // Add random environmental factors
    ThreatScore += FMath::RandRange(0.0f, 0.5f);
    
    // Check for nearby hostile entities (simulated)
    ThreatScore += FMath::RandRange(0.0f, 0.3f);
    
    if (ThreatScore < 0.2f)
        return EQuest_ThreatLevel::Minimal;
    else if (ThreatScore < 0.4f)
        return EQuest_ThreatLevel::Low;
    else if (ThreatScore < 0.6f)
        return EQuest_ThreatLevel::Moderate;
    else if (ThreatScore < 0.8f)
        return EQuest_ThreatLevel::High;
    else if (ThreatScore < 0.9f)
        return EQuest_ThreatLevel::Extreme;
    else
        return EQuest_ThreatLevel::Lethal;
}

FQuest_TacticalObjective AQuest_TacticalMissionController::CreateDefensiveObjective(const FVector& DefensePoint, float DefenseRadius)
{
    FQuest_TacticalObjective Objective;
    Objective.ObjectiveID = GenerateObjectiveID();
    Objective.Description = TEXT("Defend the designated position against hostile forces");
    Objective.MissionType = EQuest_TacticalMissionType::DefendPosition;
    Objective.TargetLocation = DefensePoint;
    Objective.CompletionRadius = DefenseRadius;
    Objective.ThreatAssessment = AssessThreatLevel(DefensePoint);
    Objective.RequiredSquadSize = FMath::Max(3, (int32)Objective.ThreatAssessment + 1);
    Objective.TimeLimit = 900.0f; // 15 minutes
    Objective.bIsActive = true;
    Objective.bIsCompleted = false;
    
    return Objective;
}

FQuest_TacticalObjective AQuest_TacticalMissionController::CreateOffensiveObjective(const FVector& TargetPoint, EQuest_ThreatLevel ThreatLevel)
{
    FQuest_TacticalObjective Objective;
    Objective.ObjectiveID = GenerateObjectiveID();
    Objective.Description = TEXT("Assault the target position and neutralize threats");
    Objective.MissionType = EQuest_TacticalMissionType::AssaultTarget;
    Objective.TargetLocation = TargetPoint;
    Objective.CompletionRadius = 300.0f;
    Objective.ThreatAssessment = ThreatLevel;
    Objective.RequiredSquadSize = FMath::Max(2, (int32)ThreatLevel + 1);
    Objective.TimeLimit = 600.0f; // 10 minutes
    Objective.bIsActive = true;
    Objective.bIsCompleted = false;
    
    return Objective;
}

FQuest_TacticalObjective AQuest_TacticalMissionController::CreateReconObjective(const FVector& ScoutLocation, float IntelRadius)
{
    FQuest_TacticalObjective Objective;
    Objective.ObjectiveID = GenerateObjectiveID();
    Objective.Description = TEXT("Gather intelligence on the target area");
    Objective.MissionType = EQuest_TacticalMissionType::Reconnaissance;
    Objective.TargetLocation = ScoutLocation;
    Objective.CompletionRadius = IntelRadius;
    Objective.ThreatAssessment = AssessThreatLevel(ScoutLocation);
    Objective.RequiredSquadSize = 1;
    Objective.TimeLimit = 1200.0f; // 20 minutes
    Objective.bIsActive = true;
    Objective.bIsCompleted = false;
    
    return Objective;
}

void AQuest_TacticalMissionController::UpdateObjectiveProgress(const FString& ObjectiveID, float Progress)
{
    for (FQuest_TacticalObjective& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            if (Progress >= 1.0f)
            {
                Objective.bIsCompleted = true;
                LogTacticalEvent(FString::Printf(TEXT("Objective Completed: %s"), *ObjectiveID));
            }
            break;
        }
    }
}

bool AQuest_TacticalMissionController::IsObjectiveComplete(const FString& ObjectiveID)
{
    for (const FQuest_TacticalObjective& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.bIsCompleted;
        }
    }
    return false;
}

TArray<FQuest_SquadMember> AQuest_TacticalMissionController::GetSquadMembersInRadius(const FVector& Center, float Radius)
{
    TArray<FQuest_SquadMember> MembersInRadius;
    
    for (const FQuest_SquadMember& Member : SquadMembers)
    {
        if (Member.bIsAlive && FVector::Dist(Member.Position, Center) <= Radius)
        {
            MembersInRadius.Add(Member);
        }
    }
    
    return MembersInRadius;
}

FVector AQuest_TacticalMissionController::CalculateOptimalSquadFormation(const FVector& TargetLocation)
{
    // Calculate center of mass for current squad
    FVector SquadCenter = FVector::ZeroVector;
    int32 AliveMembers = 0;
    
    for (const FQuest_SquadMember& Member : SquadMembers)
    {
        if (Member.bIsAlive)
        {
            SquadCenter += Member.Position;
            AliveMembers++;
        }
    }
    
    if (AliveMembers > 0)
    {
        SquadCenter /= AliveMembers;
    }
    
    // Calculate optimal formation point between current position and target
    FVector OptimalPosition = FMath::Lerp(SquadCenter, TargetLocation, 0.6f);
    
    return OptimalPosition;
}

void AQuest_TacticalMissionController::IssueSquadCommand(const FString& Command, const FVector& TargetLocation)
{
    LogTacticalEvent(FString::Printf(TEXT("Squad Command Issued: %s at %s"), *Command, *TargetLocation.ToString()));
    
    // Update squad member positions based on command
    for (FQuest_SquadMember& Member : SquadMembers)
    {
        if (Member.bIsAlive)
        {
            // Simulate squad movement towards target with formation spacing
            FVector Offset = FVector(
                FMath::RandRange(-200.0f, 200.0f),
                FMath::RandRange(-200.0f, 200.0f),
                0.0f
            );
            Member.Position = TargetLocation + Offset;
        }
    }
}

float AQuest_TacticalMissionController::CalculateMissionSuccessProbability()
{
    if (!bMissionInProgress || SquadMembers.Num() == 0)
    {
        return 0.0f;
    }
    
    float SuccessProbability = 0.5f; // Base 50%
    
    // Factor in squad health
    float AverageHealth = 0.0f;
    int32 AliveMembers = 0;
    
    for (const FQuest_SquadMember& Member : SquadMembers)
    {
        if (Member.bIsAlive)
        {
            AverageHealth += Member.HealthPercentage;
            AliveMembers++;
        }
    }
    
    if (AliveMembers > 0)
    {
        AverageHealth /= AliveMembers;
        SuccessProbability += (AverageHealth / 100.0f) * 0.3f;
    }
    
    // Factor in squad size vs required size
    for (const FQuest_TacticalObjective& Objective : ActiveObjectives)
    {
        if (Objective.bIsActive && !Objective.bIsCompleted)
        {
            float SizeRatio = (float)AliveMembers / (float)Objective.RequiredSquadSize;
            SuccessProbability += FMath::Clamp(SizeRatio - 1.0f, -0.2f, 0.2f);
        }
    }
    
    return FMath::Clamp(SuccessProbability, 0.0f, 1.0f);
}

void AQuest_TacticalMissionController::GenerateDynamicObjectives(const FVector& MissionArea, int32 ObjectiveCount)
{
    for (int32 i = 0; i < ObjectiveCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(-1000.0f, 1000.0f),
            0.0f
        );
        
        FVector ObjectiveLocation = MissionArea + RandomOffset;
        EQuest_TacticalMissionType RandomType = (EQuest_TacticalMissionType)FMath::RandRange(1, 7);
        
        FQuest_TacticalObjective DynamicObjective;
        DynamicObjective.ObjectiveID = GenerateObjectiveID();
        DynamicObjective.Description = TEXT("Dynamic tactical objective");
        DynamicObjective.MissionType = RandomType;
        DynamicObjective.TargetLocation = ObjectiveLocation;
        DynamicObjective.CompletionRadius = FMath::RandRange(200.0f, 800.0f);
        DynamicObjective.ThreatAssessment = AssessThreatLevel(ObjectiveLocation);
        DynamicObjective.RequiredSquadSize = FMath::RandRange(1, 4);
        DynamicObjective.TimeLimit = FMath::RandRange(300.0f, 1800.0f);
        DynamicObjective.bIsActive = true;
        DynamicObjective.bIsCompleted = false;
        
        ActiveObjectives.Add(DynamicObjective);
    }
}

void AQuest_TacticalMissionController::UpdateThreatAssessment()
{
    if (!bMissionInProgress)
    {
        return;
    }
    
    // Update threat levels for all active objectives
    for (FQuest_TacticalObjective& Objective : ActiveObjectives)
    {
        if (Objective.bIsActive && !Objective.bIsCompleted)
        {
            Objective.ThreatAssessment = AssessThreatLevel(Objective.TargetLocation);
        }
    }
}

void AQuest_TacticalMissionController::MonitorSquadHealth()
{
    int32 CriticalHealthMembers = 0;
    int32 AliveMembers = 0;
    
    for (const FQuest_SquadMember& Member : SquadMembers)
    {
        if (Member.bIsAlive)
        {
            AliveMembers++;
            if (Member.HealthPercentage < 25.0f)
            {
                CriticalHealthMembers++;
            }
        }
    }
    
    // Log warnings for critical situations
    if (CriticalHealthMembers > 0)
    {
        LogTacticalEvent(FString::Printf(TEXT("Warning: %d squad members in critical health"), CriticalHealthMembers));
    }
    
    if (AliveMembers < 2 && SquadMembers.Num() > 2)
    {
        LogTacticalEvent(TEXT("Critical: Squad severely depleted"));
    }
}

void AQuest_TacticalMissionController::CheckObjectiveCompletion()
{
    bool AllObjectivesComplete = true;
    
    for (const FQuest_TacticalObjective& Objective : ActiveObjectives)
    {
        if (Objective.bIsActive && !Objective.bIsCompleted)
        {
            AllObjectivesComplete = false;
            
            // Check for time limit expiration
            float ElapsedTime = GetWorld()->GetTimeSeconds() - MissionStartTime;
            if (ElapsedTime > Objective.TimeLimit)
            {
                LogTacticalEvent(FString::Printf(TEXT("Objective Failed: %s - Time limit exceeded"), *Objective.ObjectiveID));
            }
        }
    }
    
    if (AllObjectivesComplete && ActiveObjectives.Num() > 0)
    {
        CompleteTacticalMission();
    }
}

FString AQuest_TacticalMissionController::GenerateObjectiveID()
{
    static int32 ObjectiveCounter = 0;
    ObjectiveCounter++;
    return FString::Printf(TEXT("TACT_OBJ_%d_%d"), FMath::RandRange(1000, 9999), ObjectiveCounter);
}

void AQuest_TacticalMissionController::LogTacticalEvent(const FString& Event)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_TacticalMissionController: %s"), *Event);
}