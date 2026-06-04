#include "Narr_TacticalNarrativeIntegration.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarr_TacticalNarrativeIntegration::UNarr_TacticalNarrativeIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    bMissionBriefingActive = false;
    MissionNarrativeTimer = 0.0f;
    CurrentMissionContext = TEXT("Standby");
}

void UNarr_TacticalNarrativeIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMissionBriefings();
    InitializeTacticalNPCs();
    
    UE_LOG(LogTemp, Warning, TEXT("Tactical Narrative Integration initialized"));
}

void UNarr_TacticalNarrativeIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bMissionBriefingActive)
    {
        ProcessMissionNarrative(DeltaTime);
    }
}

void UNarr_TacticalNarrativeIntegration::StartMissionBriefing(const FString& MissionName, ENarr_ThreatLevel ThreatLevel)
{
    bMissionBriefingActive = true;
    MissionNarrativeTimer = 0.0f;
    CurrentMissionContext = FString::Printf(TEXT("Mission: %s - Threat Level: %d"), *MissionName, (int32)ThreatLevel);
    
    // Find and activate mission briefing
    for (const auto& Briefing : MissionBriefings)
    {
        if (Briefing.MissionName == MissionName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission Briefing Started: %s"), *Briefing.BriefingText);
            break;
        }
    }
}

void UNarr_TacticalNarrativeIntegration::EndMissionBriefing()
{
    bMissionBriefingActive = false;
    MissionNarrativeTimer = 0.0f;
    CurrentMissionContext = TEXT("Mission Complete");
    
    UE_LOG(LogTemp, Warning, TEXT("Mission briefing ended"));
}

FNarr_TacticalMissionBriefing UNarr_TacticalNarrativeIntegration::GetMissionBriefing(const FString& MissionName)
{
    for (const auto& Briefing : MissionBriefings)
    {
        if (Briefing.MissionName == MissionName)
        {
            return Briefing;
        }
    }
    
    // Return default briefing if not found
    FNarr_TacticalMissionBriefing DefaultBriefing;
    DefaultBriefing.MissionName = MissionName;
    DefaultBriefing.BriefingText = TEXT("Standard survival mission. Proceed with caution.");
    return DefaultBriefing;
}

void UNarr_TacticalNarrativeIntegration::RegisterTacticalNPC(const FNarr_TacticalNPCProfile& NPCProfile)
{
    TacticalNPCs.Add(NPCProfile);
    UE_LOG(LogTemp, Warning, TEXT("Registered tactical NPC: %s"), *NPCProfile.NPCName);
}

TArray<FNarr_TacticalNPCProfile> UNarr_TacticalNarrativeIntegration::GetAvailableNPCs(ENarr_NPCRole RequiredRole)
{
    TArray<FNarr_TacticalNPCProfile> AvailableNPCs;
    
    for (const auto& NPC : TacticalNPCs)
    {
        if (NPC.TacticalRole == RequiredRole && NPC.bIsAvailableForMissions)
        {
            AvailableNPCs.Add(NPC);
        }
    }
    
    return AvailableNPCs;
}

FString UNarr_TacticalNarrativeIntegration::GetNPCDialogue(const FString& NPCName, const FString& Context)
{
    for (const auto& NPC : TacticalNPCs)
    {
        if (NPC.NPCName == NPCName)
        {
            return GenerateContextualDialogue(NPC, Context);
        }
    }
    
    return TEXT("NPC not available for dialogue.");
}

void UNarr_TacticalNarrativeIntegration::UpdateMissionContext(const FString& NewContext)
{
    CurrentMissionContext = NewContext;
    UE_LOG(LogTemp, Warning, TEXT("Mission context updated: %s"), *NewContext);
}

void UNarr_TacticalNarrativeIntegration::TriggerMissionNarrative(const FString& NarrativeEvent)
{
    UE_LOG(LogTemp, Warning, TEXT("Mission narrative triggered: %s"), *NarrativeEvent);
    
    // Process narrative event based on context
    if (NarrativeEvent.Contains(TEXT("Combat")))
    {
        CurrentMissionContext = TEXT("Combat Engagement");
    }
    else if (NarrativeEvent.Contains(TEXT("Stealth")))
    {
        CurrentMissionContext = TEXT("Stealth Operation");
    }
    else if (NarrativeEvent.Contains(TEXT("Extraction")))
    {
        CurrentMissionContext = TEXT("Extraction Phase");
    }
}

bool UNarr_TacticalNarrativeIntegration::IsMissionNarrativeActive() const
{
    return bMissionBriefingActive;
}

void UNarr_TacticalNarrativeIntegration::BroadcastSquadMessage(const FString& Message, ENarr_NPCRole SenderRole)
{
    FString RoleName;
    switch (SenderRole)
    {
        case ENarr_NPCRole::Leader:
            RoleName = TEXT("Squad Leader");
            break;
        case ENarr_NPCRole::Scout:
            RoleName = TEXT("Scout");
            break;
        case ENarr_NPCRole::Hunter:
            RoleName = TEXT("Hunter");
            break;
        case ENarr_NPCRole::Guardian:
            RoleName = TEXT("Guardian");
            break;
        case ENarr_NPCRole::Specialist:
            RoleName = TEXT("Specialist");
            break;
        default:
            RoleName = TEXT("Squad Member");
            break;
    }
    
    FString BroadcastMessage = FString::Printf(TEXT("[%s]: %s"), *RoleName, *Message);
    UE_LOG(LogTemp, Warning, TEXT("Squad Broadcast: %s"), *BroadcastMessage);
}

void UNarr_TacticalNarrativeIntegration::HandleSquadStatusUpdate(const FString& StatusMessage)
{
    UE_LOG(LogTemp, Warning, TEXT("Squad Status Update: %s"), *StatusMessage);
    
    // Update mission context based on status
    if (StatusMessage.Contains(TEXT("Injured")))
    {
        CurrentMissionContext = TEXT("Medical Emergency");
    }
    else if (StatusMessage.Contains(TEXT("Enemy")))
    {
        CurrentMissionContext = TEXT("Hostile Contact");
    }
    else if (StatusMessage.Contains(TEXT("Complete")))
    {
        CurrentMissionContext = TEXT("Objective Complete");
    }
}

void UNarr_TacticalNarrativeIntegration::InitializeMissionBriefings()
{
    // Initialize standard mission briefings
    FNarr_TacticalMissionBriefing DefendBriefing;
    DefendBriefing.MissionName = TEXT("Defend Position");
    DefendBriefing.BriefingText = TEXT("Hostile dinosaurs approaching our territory. Establish defensive perimeter and protect the settlement.");
    DefendBriefing.ObjectiveDescription = TEXT("Hold position for 10 minutes against dinosaur attacks");
    DefendBriefing.ThreatLevel = ENarr_ThreatLevel::High;
    DefendBriefing.SquadRoleRequirements = {TEXT("Guardian"), TEXT("Hunter"), TEXT("Scout")};
    DefendBriefing.EstimatedDuration = 600.0f;
    MissionBriefings.Add(DefendBriefing);
    
    FNarr_TacticalMissionBriefing HuntBriefing;
    HuntBriefing.MissionName = TEXT("Hunt Target");
    HuntBriefing.BriefingText = TEXT("Large predator threatening our hunting grounds. Coordinate attack to eliminate the threat.");
    HuntBriefing.ObjectiveDescription = TEXT("Track and eliminate the target dinosaur");
    HuntBriefing.ThreatLevel = ENarr_ThreatLevel::Extreme;
    HuntBriefing.SquadRoleRequirements = {TEXT("Hunter"), TEXT("Scout"), TEXT("Leader")};
    HuntBriefing.EstimatedDuration = 900.0f;
    MissionBriefings.Add(HuntBriefing);
    
    FNarr_TacticalMissionBriefing ScoutBriefing;
    ScoutBriefing.MissionName = TEXT("Reconnaissance");
    ScoutBriefing.BriefingText = TEXT("Unknown territory detected. Gather intelligence on dinosaur populations and resource availability.");
    ScoutBriefing.ObjectiveDescription = TEXT("Survey the area and report back with findings");
    ScoutBriefing.ThreatLevel = ENarr_ThreatLevel::Moderate;
    ScoutBriefing.SquadRoleRequirements = {TEXT("Scout"), TEXT("Specialist")};
    ScoutBriefing.EstimatedDuration = 450.0f;
    MissionBriefings.Add(ScoutBriefing);
}

void UNarr_TacticalNarrativeIntegration::InitializeTacticalNPCs()
{
    // Initialize tactical NPCs with survival-focused backgrounds
    FNarr_TacticalNPCProfile ElderProfile;
    ElderProfile.NPCName = TEXT("Elder Kael");
    ElderProfile.TacticalRole = ENarr_NPCRole::Leader;
    ElderProfile.BackgroundStory = TEXT("Veteran survivor who has led the tribe through countless dangers. Expert in dinosaur behavior and tactical planning.");
    ElderProfile.SpecializedDialogue = {
        TEXT("Remember, patience is the hunter's greatest weapon."),
        TEXT("The great beasts follow patterns. Learn them, and you control the hunt."),
        TEXT("Every scar tells a story of survival. What story will yours tell?")
    };
    ElderProfile.ExperienceLevel = 10.0f;
    TacticalNPCs.Add(ElderProfile);
    
    FNarr_TacticalNPCProfile HunterProfile;
    HunterProfile.NPCName = TEXT("Hunter Zara");
    HunterProfile.TacticalRole = ENarr_NPCRole::Hunter;
    HunterProfile.BackgroundStory = TEXT("Master tracker and weapon specialist. Has taken down some of the largest predators in the region.");
    HunterProfile.SpecializedDialogue = {
        TEXT("Strike fast, strike true. Hesitation means death."),
        TEXT("I've tracked this species before. Stay downwind and watch for the tail."),
        TEXT("Your spear is only as good as your aim. Practice until it's perfect.")
    };
    HunterProfile.ExperienceLevel = 8.0f;
    TacticalNPCs.Add(HunterProfile);
    
    FNarr_TacticalNPCProfile ScoutProfile;
    ScoutProfile.NPCName = TEXT("Scout Finn");
    ScoutProfile.TacticalRole = ENarr_NPCRole::Scout;
    ScoutProfile.BackgroundStory = TEXT("Swift and silent, expert in reconnaissance and early warning systems. Eyes and ears of the tribe.");
    ScoutProfile.SpecializedDialogue = {
        TEXT("I see movement in the treeline. Three, maybe four raptors."),
        TEXT("The wind carries their scent. We have maybe ten minutes before they find us."),
        TEXT("High ground gives us the advantage. Follow me, stay low.")
    };
    ScoutProfile.ExperienceLevel = 7.0f;
    TacticalNPCs.Add(ScoutProfile);
}

void UNarr_TacticalNarrativeIntegration::ProcessMissionNarrative(float DeltaTime)
{
    MissionNarrativeTimer += DeltaTime;
    
    // Trigger narrative events based on mission progress
    if (MissionNarrativeTimer > 30.0f && CurrentMissionContext.Contains(TEXT("Mission:")))
    {
        TriggerMissionNarrative(TEXT("Mission Progress Update"));
        MissionNarrativeTimer = 0.0f; // Reset timer
    }
}

FString UNarr_TacticalNarrativeIntegration::GenerateContextualDialogue(const FNarr_TacticalNPCProfile& NPC, const FString& Context)
{
    // Generate dialogue based on NPC role and context
    if (Context.Contains(TEXT("Combat")))
    {
        switch (NPC.TacticalRole)
        {
            case ENarr_NPCRole::Leader:
                return TEXT("Hold formation! Watch your flanks!");
            case ENarr_NPCRole::Hunter:
                return TEXT("Target the weak spots! Aim for the neck!");
            case ENarr_NPCRole::Scout:
                return TEXT("More incoming from the east! Adjust positions!");
            case ENarr_NPCRole::Guardian:
                return TEXT("Protect the wounded! Form a defensive line!");
            default:
                return TEXT("Stay alert! Follow your training!");
        }
    }
    else if (Context.Contains(TEXT("Stealth")))
    {
        switch (NPC.TacticalRole)
        {
            case ENarr_NPCRole::Scout:
                return TEXT("Movement ahead. Stay low and follow my lead.");
            case ENarr_NPCRole::Hunter:
                return TEXT("Perfect position for an ambush. Wait for my signal.");
            default:
                return TEXT("Keep quiet. One sound could give us away.");
        }
    }
    
    // Return default dialogue from specialized dialogue array
    if (NPC.SpecializedDialogue.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, NPC.SpecializedDialogue.Num() - 1);
        return NPC.SpecializedDialogue[RandomIndex];
    }
    
    return TEXT("Stay focused. Survival depends on it.");
}