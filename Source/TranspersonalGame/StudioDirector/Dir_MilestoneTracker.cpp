#include "Dir_MilestoneTracker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDir_MilestoneTracker::UDir_MilestoneTracker()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentCycle = 2; // Starting from cycle 002
    CurrentPhase = TEXT("WALK_AROUND_PROTOTYPE");
}

void UDir_MilestoneTracker::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize Walk Around milestones on first run
    if (Milestones.Num() == 0)
    {
        InitializeWalkAroundMilestones();
    }
}

void UDir_MilestoneTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update cycle counter periodically
    UpdateCurrentCycle();
}

void UDir_MilestoneTracker::AddMilestone(const FString& MilestoneName, const FString& AssignedAgent)
{
    FDir_MilestoneData NewMilestone;
    NewMilestone.MilestoneName = MilestoneName;
    NewMilestone.AssignedAgent = AssignedAgent;
    NewMilestone.Status = EDir_MilestoneStatus::NotStarted;
    NewMilestone.CycleStarted = CurrentCycle;
    
    Milestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone added: %s assigned to %s"), *MilestoneName, *AssignedAgent);
}

void UDir_MilestoneTracker::UpdateMilestoneStatus(const FString& MilestoneName, EDir_MilestoneStatus NewStatus, float CompletionPercentage)
{
    int32 Index = FindMilestoneIndex(MilestoneName);
    if (Index != INDEX_NONE)
    {
        Milestones[Index].Status = NewStatus;
        Milestones[Index].CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
        
        if (NewStatus == EDir_MilestoneStatus::InProgress && Milestones[Index].CycleStarted == -1)
        {
            Milestones[Index].CycleStarted = CurrentCycle;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Milestone %s updated to status %d with %.1f%% completion"), 
               *MilestoneName, (int32)NewStatus, CompletionPercentage);
    }
}

void UDir_MilestoneTracker::CompleteMilestone(const FString& MilestoneName)
{
    int32 Index = FindMilestoneIndex(MilestoneName);
    if (Index != INDEX_NONE)
    {
        Milestones[Index].Status = EDir_MilestoneStatus::Completed;
        Milestones[Index].CompletionPercentage = 100.0f;
        Milestones[Index].CycleCompleted = CurrentCycle;
        
        UE_LOG(LogTemp, Warning, TEXT("Milestone COMPLETED: %s in cycle %d"), *MilestoneName, CurrentCycle);
    }
}

void UDir_MilestoneTracker::BlockMilestone(const FString& MilestoneName, const FString& BlockReason)
{
    int32 Index = FindMilestoneIndex(MilestoneName);
    if (Index != INDEX_NONE)
    {
        Milestones[Index].Status = EDir_MilestoneStatus::Blocked;
        
        UE_LOG(LogTemp, Error, TEXT("Milestone BLOCKED: %s - Reason: %s"), *MilestoneName, *BlockReason);
    }
}

TArray<FDir_MilestoneData> UDir_MilestoneTracker::GetAllMilestones() const
{
    return Milestones;
}

FDir_MilestoneData UDir_MilestoneTracker::GetMilestone(const FString& MilestoneName) const
{
    int32 Index = FindMilestoneIndex(MilestoneName);
    if (Index != INDEX_NONE)
    {
        return Milestones[Index];
    }
    
    return FDir_MilestoneData();
}

TArray<FDir_MilestoneData> UDir_MilestoneTracker::GetMilestonesByAgent(const FString& AgentName) const
{
    TArray<FDir_MilestoneData> AgentMilestones;
    
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.AssignedAgent == AgentName)
        {
            AgentMilestones.Add(Milestone);
        }
    }
    
    return AgentMilestones;
}

TArray<FDir_MilestoneData> UDir_MilestoneTracker::GetMilestonesByStatus(EDir_MilestoneStatus Status) const
{
    TArray<FDir_MilestoneData> StatusMilestones;
    
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.Status == Status)
        {
            StatusMilestones.Add(Milestone);
        }
    }
    
    return StatusMilestones;
}

float UDir_MilestoneTracker::GetOverallCompletionPercentage() const
{
    if (Milestones.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalCompletion = 0.0f;
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        TotalCompletion += Milestone.CompletionPercentage;
    }
    
    return TotalCompletion / Milestones.Num();
}

int32 UDir_MilestoneTracker::GetCompletedMilestonesCount() const
{
    int32 CompletedCount = 0;
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.Status == EDir_MilestoneStatus::Completed)
        {
            CompletedCount++;
        }
    }
    
    return CompletedCount;
}

int32 UDir_MilestoneTracker::GetBlockedMilestonesCount() const
{
    int32 BlockedCount = 0;
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.Status == EDir_MilestoneStatus::Blocked)
        {
            BlockedCount++;
        }
    }
    
    return BlockedCount;
}

void UDir_MilestoneTracker::LogMilestoneStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MILESTONE STATUS REPORT - CYCLE %d ==="), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Overall Completion: %.1f%%"), GetOverallCompletionPercentage());
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d | Blocked: %d | Total: %d"), 
           GetCompletedMilestonesCount(), GetBlockedMilestonesCount(), Milestones.Num());
    
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        FString StatusText;
        switch (Milestone.Status)
        {
            case EDir_MilestoneStatus::NotStarted: StatusText = TEXT("NOT_STARTED"); break;
            case EDir_MilestoneStatus::InProgress: StatusText = TEXT("IN_PROGRESS"); break;
            case EDir_MilestoneStatus::Completed: StatusText = TEXT("COMPLETED"); break;
            case EDir_MilestoneStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("  %s [%s] %.1f%% - Agent: %s"), 
               *Milestone.MilestoneName, *StatusText, Milestone.CompletionPercentage, *Milestone.AssignedAgent);
    }
}

void UDir_MilestoneTracker::InitializeWalkAroundMilestones()
{
    // Clear existing milestones
    Milestones.Empty();
    
    // WALK AROUND PROTOTYPE milestones
    AddMilestone(TEXT("ThirdPersonCharacter_Movement"), TEXT("Agent_09_Character"));
    AddMilestone(TEXT("Camera_FollowSystem"), TEXT("Agent_10_Animation"));
    AddMilestone(TEXT("Landscape_BasicTerrain"), TEXT("Agent_05_ProceduralWorld"));
    AddMilestone(TEXT("PlayerInput_WASD"), TEXT("Agent_03_CoreSystems"));
    AddMilestone(TEXT("Dinosaur_StaticMeshes"), TEXT("Agent_06_Environment"));
    AddMilestone(TEXT("Lighting_DirectionalSun"), TEXT("Agent_08_Lighting"));
    AddMilestone(TEXT("Atmosphere_SkyFog"), TEXT("Agent_08_Lighting"));
    AddMilestone(TEXT("Survival_HUD"), TEXT("Agent_12_Combat"));
    
    // Set initial statuses based on current state
    UpdateMilestoneStatus(TEXT("ThirdPersonCharacter_Movement"), EDir_MilestoneStatus::InProgress, 60.0f);
    UpdateMilestoneStatus(TEXT("Landscape_BasicTerrain"), EDir_MilestoneStatus::InProgress, 40.0f);
    UpdateMilestoneStatus(TEXT("Lighting_DirectionalSun"), EDir_MilestoneStatus::Completed, 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Walk Around milestones initialized - %d total milestones"), Milestones.Num());
}

void UDir_MilestoneTracker::UpdateCurrentCycle()
{
    // Simple cycle increment logic - could be enhanced with real-time tracking
    static float CycleTimer = 0.0f;
    CycleTimer += GetWorld()->GetDeltaSeconds();
    
    // Increment cycle every 10 minutes of real time (for testing)
    if (CycleTimer > 600.0f)
    {
        CurrentCycle++;
        CycleTimer = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("Cycle advanced to: %d"), CurrentCycle);
    }
}

int32 UDir_MilestoneTracker::FindMilestoneIndex(const FString& MilestoneName) const
{
    for (int32 i = 0; i < Milestones.Num(); i++)
    {
        if (Milestones[i].MilestoneName == MilestoneName)
        {
            return i;
        }
    }
    
    return INDEX_NONE;
}