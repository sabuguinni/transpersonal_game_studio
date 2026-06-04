#include "Narr_TribalFactionSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UNarr_TribalFactionSystem::UNarr_TribalFactionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxTrustLevel = 100;
    MinTrustLevel = -100;
    RelationshipUpdateInterval = 30.0f;
}

void UNarr_TribalFactionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalFactions();
    
    // Set up periodic relationship updates
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            RelationshipUpdateTimer,
            this,
            &UNarr_TribalFactionSystem::UpdateFactionDynamics,
            RelationshipUpdateInterval,
            true
        );
    }
}

void UNarr_TribalFactionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UNarr_TribalFactionSystem::InitializeTribalFactions()
{
    CreateDefaultFactions();
    CreateTribalCharacters();
    SetupFactionRelationships();
    
    UE_LOG(LogTemp, Warning, TEXT("Tribal Faction System initialized with %d factions"), TribalFactions.Num());
}

void UNarr_TribalFactionSystem::CreateDefaultFactions()
{
    // Main Player Tribe - Stormwatcher Clan
    FNarr_FactionData StormwatcherClan;
    StormwatcherClan.FactionName = TEXT("Stormwatcher Clan");
    StormwatcherClan.Territory = TEXT("Central Valley near the Great River");
    StormwatcherClan.Resources = {TEXT("Fresh Water"), TEXT("Stone Tools"), TEXT("Fire Knowledge"), TEXT("Shelter Caves")};
    StormwatcherClan.PlayerRelationship = ENarr_FactionRelationship::Allied;
    StormwatcherClan.TrustLevel = 75;
    TribalFactions.Add(TEXT("Stormwatcher"), StormwatcherClan);

    // Rival Tribe - Bonecrusher Pack
    FNarr_FactionData BonecrusherPack;
    BonecrusherPack.FactionName = TEXT("Bonecrusher Pack");
    BonecrusherPack.Territory = TEXT("Northern Highlands near the Bone Fields");
    BonecrusherPack.Resources = {TEXT("Hunting Grounds"), TEXT("Bone Weapons"), TEXT("Predator Knowledge"), TEXT("Mountain Paths")};
    BonecrusherPack.PlayerRelationship = ENarr_FactionRelationship::Suspicious;
    BonecrusherPack.TrustLevel = -25;
    TribalFactions.Add(TEXT("Bonecrusher"), BonecrusherPack);

    // Neutral Tribe - Gatherer Circle
    FNarr_FactionData GathererCircle;
    GathererCircle.FactionName = TEXT("Gatherer Circle");
    GathererCircle.Territory = TEXT("Southern Forest near the Berry Groves");
    GathererCircle.Resources = {TEXT("Plant Knowledge"), TEXT("Healing Herbs"), TEXT("Food Preservation"), TEXT("Weather Prediction")};
    GathererCircle.PlayerRelationship = ENarr_FactionRelationship::Neutral;
    GathererCircle.TrustLevel = 10;
    TribalFactions.Add(TEXT("Gatherer"), GathererCircle);
}

void UNarr_TribalFactionSystem::CreateTribalCharacters()
{
    // Elder Kava Stormwatcher - Wise Leader
    FNarr_CharacterBackstory Kava;
    Kava.CharacterName = TEXT("Kava Stormwatcher");
    Kava.Role = ENarr_FactionRole::Elder;
    Kava.Age = 58;
    Kava.SurvivalExperience = 40;
    Kava.BackstoryText = TEXT("Eldest of the Stormwatcher Clan, Kava has survived more harsh winters and predator attacks than anyone alive. She earned her name by predicting the great storm that saved the tribe from a massive predator migration. Her wisdom comes from understanding the patterns of nature and the behavior of the great beasts.");
    Kava.SpecialSkills = {TEXT("Weather Prediction"), TEXT("Beast Behavior Analysis"), TEXT("Tribal Leadership"), TEXT("Ancient Knowledge")};
    Kava.KnownLocations = {TEXT("Sacred Cave of Echoes"), TEXT("Hidden Water Springs"), TEXT("Ancient Bone Yards"), TEXT("Safe Migration Routes")};
    Kava.PersonalityTraits = TEXT("Wise, Patient, Protective, Observant");
    CharacterDatabase.Add(TEXT("Kava"), Kava);

    // Scout Thane Pathfinder - Master Tracker
    FNarr_CharacterBackstory Thane;
    Thane.CharacterName = TEXT("Thane Pathfinder");
    Thane.Role = ENarr_FactionRole::Scout;
    Thane.Age = 34;
    Thane.SurvivalExperience = 18;
    Thane.BackstoryText = TEXT("The tribe's most skilled tracker and pathfinder. Thane has mapped every game trail from the salt marshes to the fire mountains. He once spent three days stalking a pack of raptors to learn their hunting patterns, knowledge that has saved countless lives. His feet know every safe path through dangerous territory.");
    Thane.SpecialSkills = {TEXT("Tracking"), TEXT("Stealth Movement"), TEXT("Terrain Navigation"), TEXT("Predator Avoidance")};
    Thane.KnownLocations = {TEXT("Raptor Nesting Grounds"), TEXT("Great Herd Migration Routes"), TEXT("Hidden Valley Passages"), TEXT("Predator Territory Boundaries")};
    Thane.PersonalityTraits = TEXT("Alert, Cautious, Brave, Independent");
    CharacterDatabase.Add(TEXT("Thane"), Thane);

    // Craft Master Nira Stonehand - Tool Expert
    FNarr_CharacterBackstory Nira;
    Nira.CharacterName = TEXT("Nira Stonehand");
    Nira.Role = ENarr_FactionRole::CraftMaster;
    Nira.Age = 42;
    Nira.SurvivalExperience = 25;
    Nira.BackstoryText = TEXT("Master of stone tools and shelter construction. Nira's hands can shape flint into razor-sharp spear points and build shelters that withstand the fiercest storms. She discovered the technique for fire-hardening wooden spears, revolutionizing the tribe's hunting capabilities. Her innovations have kept the tribe alive through the harshest seasons.");
    Nira.SpecialSkills = {TEXT("Stone Tool Crafting"), TEXT("Shelter Construction"), TEXT("Fire Techniques"), TEXT("Weapon Making")};
    Nira.KnownLocations = {TEXT("Quality Flint Deposits"), TEXT("Clay Sources"), TEXT("Hardwood Groves"), TEXT("Obsidian Outcrops")};
    Nira.PersonalityTraits = TEXT("Methodical, Innovative, Practical, Determined");
    CharacterDatabase.Add(TEXT("Nira"), Nira);

    // Add characters to their respective factions
    if (TribalFactions.Contains(TEXT("Stormwatcher")))
    {
        TribalFactions[TEXT("Stormwatcher")].Members.Add(Kava);
        TribalFactions[TEXT("Stormwatcher")].Members.Add(Thane);
        TribalFactions[TEXT("Stormwatcher")].Members.Add(Nira);
    }
}

void UNarr_TribalFactionSystem::SetupFactionRelationships()
{
    // Stormwatcher Clan relationships are already set in CreateDefaultFactions
    UE_LOG(LogTemp, Warning, TEXT("Faction relationships initialized"));
}

FNarr_FactionData UNarr_TribalFactionSystem::GetFactionData(const FString& FactionName)
{
    if (TribalFactions.Contains(FactionName))
    {
        return TribalFactions[FactionName];
    }
    
    return FNarr_FactionData();
}

void UNarr_TribalFactionSystem::UpdateFactionRelationship(const FString& FactionName, ENarr_FactionRelationship NewRelationship)
{
    if (TribalFactions.Contains(FactionName))
    {
        TribalFactions[FactionName].PlayerRelationship = NewRelationship;
        UE_LOG(LogTemp, Warning, TEXT("Updated relationship with %s"), *FactionName);
    }
}

void UNarr_TribalFactionSystem::ModifyTrustLevel(const FString& FactionName, int32 TrustChange)
{
    if (TribalFactions.Contains(FactionName))
    {
        int32 NewTrust = TribalFactions[FactionName].TrustLevel + TrustChange;
        TribalFactions[FactionName].TrustLevel = FMath::Clamp(NewTrust, MinTrustLevel, MaxTrustLevel);
        
        // Update relationship based on trust level
        if (TribalFactions[FactionName].TrustLevel >= 60)
        {
            TribalFactions[FactionName].PlayerRelationship = ENarr_FactionRelationship::Allied;
        }
        else if (TribalFactions[FactionName].TrustLevel >= 20)
        {
            TribalFactions[FactionName].PlayerRelationship = ENarr_FactionRelationship::Neutral;
        }
        else if (TribalFactions[FactionName].TrustLevel >= -20)
        {
            TribalFactions[FactionName].PlayerRelationship = ENarr_FactionRelationship::Suspicious;
        }
        else
        {
            TribalFactions[FactionName].PlayerRelationship = ENarr_FactionRelationship::Hostile;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Trust with %s changed by %d, new level: %d"), 
               *FactionName, TrustChange, TribalFactions[FactionName].TrustLevel);
    }
}

FNarr_CharacterBackstory UNarr_TribalFactionSystem::GetCharacterBackstory(const FString& CharacterName)
{
    if (CharacterDatabase.Contains(CharacterName))
    {
        return CharacterDatabase[CharacterName];
    }
    
    return FNarr_CharacterBackstory();
}

void UNarr_TribalFactionSystem::AddCharacterToFaction(const FString& FactionName, const FNarr_CharacterBackstory& Character)
{
    if (TribalFactions.Contains(FactionName))
    {
        TribalFactions[FactionName].Members.Add(Character);
        CharacterDatabase.Add(Character.CharacterName, Character);
        UE_LOG(LogTemp, Warning, TEXT("Added character %s to faction %s"), *Character.CharacterName, *FactionName);
    }
}

TArray<FNarr_CharacterBackstory> UNarr_TribalFactionSystem::GetFactionMembers(const FString& FactionName)
{
    if (TribalFactions.Contains(FactionName))
    {
        return TribalFactions[FactionName].Members;
    }
    
    return TArray<FNarr_CharacterBackstory>();
}

bool UNarr_TribalFactionSystem::CanPlayerInteractWithCharacter(const FString& CharacterName)
{
    if (CharacterDatabase.Contains(CharacterName))
    {
        // Find which faction this character belongs to
        for (const auto& FactionPair : TribalFactions)
        {
            for (const auto& Member : FactionPair.Value.Members)
            {
                if (Member.CharacterName == CharacterName)
                {
                    // Allow interaction unless hostile
                    return FactionPair.Value.PlayerRelationship != ENarr_FactionRelationship::Hostile;
                }
            }
        }
    }
    
    return false;
}

FString UNarr_TribalFactionSystem::GetRelationshipStatusText(const FString& FactionName)
{
    if (TribalFactions.Contains(FactionName))
    {
        const FNarr_FactionData& Faction = TribalFactions[FactionName];
        FString StatusText;
        
        switch (Faction.PlayerRelationship)
        {
            case ENarr_FactionRelationship::Allied:
                StatusText = FString::Printf(TEXT("Allied (Trust: %d) - They welcome you as family"), Faction.TrustLevel);
                break;
            case ENarr_FactionRelationship::Neutral:
                StatusText = FString::Printf(TEXT("Neutral (Trust: %d) - They are cautiously friendly"), Faction.TrustLevel);
                break;
            case ENarr_FactionRelationship::Suspicious:
                StatusText = FString::Printf(TEXT("Suspicious (Trust: %d) - They watch you carefully"), Faction.TrustLevel);
                break;
            case ENarr_FactionRelationship::Hostile:
                StatusText = FString::Printf(TEXT("Hostile (Trust: %d) - They see you as a threat"), Faction.TrustLevel);
                break;
            default:
                StatusText = TEXT("Unknown relationship");
                break;
        }
        
        return StatusText;
    }
    
    return TEXT("Faction not found");
}

void UNarr_TribalFactionSystem::ProcessPlayerAction(const FString& ActionType, const FString& TargetFaction, int32 ImpactValue)
{
    if (ActionType == TEXT("Help") || ActionType == TEXT("Gift") || ActionType == TEXT("Protect"))
    {
        ModifyTrustLevel(TargetFaction, FMath::Abs(ImpactValue));
    }
    else if (ActionType == TEXT("Steal") || ActionType == TEXT("Attack") || ActionType == TEXT("Betray"))
    {
        ModifyTrustLevel(TargetFaction, -FMath::Abs(ImpactValue));
    }
    else if (ActionType == TEXT("Trade"))
    {
        ModifyTrustLevel(TargetFaction, ImpactValue / 2); // Trading has moderate positive impact
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Player action '%s' processed for faction '%s' with impact %d"), 
           *ActionType, *TargetFaction, ImpactValue);
}

void UNarr_TribalFactionSystem::UpdateFactionDynamics()
{
    // Periodic relationship decay/improvement based on time and circumstances
    for (auto& FactionPair : TribalFactions)
    {
        FNarr_FactionData& Faction = FactionPair.Value;
        
        // Slight trust decay over time if no interaction (relationships require maintenance)
        if (Faction.TrustLevel > 0)
        {
            Faction.TrustLevel = FMath::Max(0, Faction.TrustLevel - 1);
        }
        else if (Faction.TrustLevel < 0)
        {
            Faction.TrustLevel = FMath::Min(0, Faction.TrustLevel + 1);
        }
    }
}