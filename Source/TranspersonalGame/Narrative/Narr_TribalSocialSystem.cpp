#include "Narr_TribalSocialSystem.h"
#include "Engine/Engine.h"

ANarr_TribalSocialSystem::ANarr_TribalSocialSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default tribal values
    TribeName = TEXT("Stone Spear Tribe");
    TotalPopulation = 0;
    TribalMorale = 50.0f;
    FoodSupply = 75.0f;
    WaterSupply = 80.0f;
    SafetyLevel = 60.0f;
}

void ANarr_TribalSocialSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the tribe with default members
    InitializeDefaultTribe();
    
    UE_LOG(LogTemp, Warning, TEXT("Tribal Social System initialized for %s with %d members"), 
           *TribeName, TribeMembers.Num());
}

void ANarr_TribalSocialSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update tribal dynamics over time
    static float TimeAccumulator = 0.0f;
    TimeAccumulator += DeltaTime;
    
    // Process tribal activities every 30 seconds
    if (TimeAccumulator >= 30.0f)
    {
        ProcessDailyTribalActivities();
        TimeAccumulator = 0.0f;
    }
}

void ANarr_TribalSocialSystem::AddTribeMember(const FNarr_TribalMember& NewMember)
{
    TribeMembers.Add(NewMember);
    TotalPopulation = TribeMembers.Num();
    
    UE_LOG(LogTemp, Log, TEXT("Added new tribe member: %s (Rank: %d)"), 
           *NewMember.Name, (int32)NewMember.Rank);
    
    // Adding new members slightly increases morale
    UpdateTribalMorale(5.0f);
}

void ANarr_TribalSocialSystem::RemoveTribeMember(const FString& MemberName)
{
    for (int32 i = TribeMembers.Num() - 1; i >= 0; i--)
    {
        if (TribeMembers[i].Name == MemberName)
        {
            TribeMembers.RemoveAt(i);
            TotalPopulation = TribeMembers.Num();
            
            UE_LOG(LogTemp, Warning, TEXT("Removed tribe member: %s"), *MemberName);
            
            // Losing members decreases morale
            UpdateTribalMorale(-10.0f);
            break;
        }
    }
}

FNarr_TribalMember ANarr_TribalSocialSystem::GetTribeMember(const FString& MemberName)
{
    for (const FNarr_TribalMember& Member : TribeMembers)
    {
        if (Member.Name == MemberName)
        {
            return Member;
        }
    }
    
    // Return default member if not found
    return FNarr_TribalMember();
}

void ANarr_TribalSocialSystem::UpdateMemberRank(const FString& MemberName, ENarr_TribalRank NewRank)
{
    for (FNarr_TribalMember& Member : TribeMembers)
    {
        if (Member.Name == MemberName)
        {
            ENarr_TribalRank OldRank = Member.Rank;
            Member.Rank = NewRank;
            
            UE_LOG(LogTemp, Log, TEXT("Updated %s rank from %d to %d"), 
                   *MemberName, (int32)OldRank, (int32)NewRank);
            
            // Rank promotions increase morale
            if (NewRank > OldRank)
            {
                UpdateTribalMorale(3.0f);
            }
            break;
        }
    }
}

void ANarr_TribalSocialSystem::CreateSocialBond(const FString& MemberA, const FString& MemberB, ENarr_SocialRelation RelationType)
{
    FNarr_SocialBond NewBond;
    NewBond.MemberA = MemberA;
    NewBond.MemberB = MemberB;
    NewBond.RelationType = RelationType;
    NewBond.BondStrength = 25.0f; // Starting bond strength
    NewBond.SharedExperience = TEXT("First meeting");
    
    SocialBonds.Add(NewBond);
    
    UE_LOG(LogTemp, Log, TEXT("Created social bond between %s and %s (Type: %d)"), 
           *MemberA, *MemberB, (int32)RelationType);
}

void ANarr_TribalSocialSystem::UpdateSocialBond(const FString& MemberA, const FString& MemberB, float BondChange)
{
    for (FNarr_SocialBond& Bond : SocialBonds)
    {
        if ((Bond.MemberA == MemberA && Bond.MemberB == MemberB) ||
            (Bond.MemberA == MemberB && Bond.MemberB == MemberA))
        {
            Bond.BondStrength = FMath::Clamp(Bond.BondStrength + BondChange, 0.0f, 100.0f);
            
            // Update relation type based on bond strength
            if (Bond.BondStrength >= 80.0f)
                Bond.RelationType = ENarr_SocialRelation::Family;
            else if (Bond.BondStrength >= 60.0f)
                Bond.RelationType = ENarr_SocialRelation::Bonded;
            else if (Bond.BondStrength >= 40.0f)
                Bond.RelationType = ENarr_SocialRelation::Trusted;
            else if (Bond.BondStrength >= 20.0f)
                Bond.RelationType = ENarr_SocialRelation::Friendly;
            else if (Bond.BondStrength >= 10.0f)
                Bond.RelationType = ENarr_SocialRelation::Neutral;
            else
                Bond.RelationType = ENarr_SocialRelation::Suspicious;
            
            break;
        }
    }
}

TArray<FNarr_TribalMember> ANarr_TribalSocialSystem::GetMembersByRank(ENarr_TribalRank Rank)
{
    TArray<FNarr_TribalMember> FilteredMembers;
    
    for (const FNarr_TribalMember& Member : TribeMembers)
    {
        if (Member.Rank == Rank)
        {
            FilteredMembers.Add(Member);
        }
    }
    
    return FilteredMembers;
}

TArray<FNarr_TribalMember> ANarr_TribalSocialSystem::GetMembersByActivity(ENarr_TribalActivity Activity)
{
    TArray<FNarr_TribalMember> FilteredMembers;
    
    for (const FNarr_TribalMember& Member : TribeMembers)
    {
        if (Member.CurrentActivity == Activity)
        {
            FilteredMembers.Add(Member);
        }
    }
    
    return FilteredMembers;
}

void ANarr_TribalSocialSystem::UpdateTribalMorale(float MoraleChange)
{
    TribalMorale = FMath::Clamp(TribalMorale + MoraleChange, 0.0f, 100.0f);
    
    if (MoraleChange > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Tribal morale increased to %.1f"), TribalMorale);
    }
    else if (MoraleChange < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tribal morale decreased to %.1f"), TribalMorale);
    }
}

void ANarr_TribalSocialSystem::UpdateSupplies(float FoodChange, float WaterChange)
{
    FoodSupply = FMath::Clamp(FoodSupply + FoodChange, 0.0f, 100.0f);
    WaterSupply = FMath::Clamp(WaterSupply + WaterChange, 0.0f, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Supplies updated - Food: %.1f, Water: %.1f"), FoodSupply, WaterSupply);
    
    // Low supplies affect morale
    if (FoodSupply < 20.0f || WaterSupply < 20.0f)
    {
        UpdateTribalMorale(-2.0f);
    }
}

void ANarr_TribalSocialSystem::UpdateSafetyLevel(float SafetyChange)
{
    SafetyLevel = FMath::Clamp(SafetyLevel + SafetyChange, 0.0f, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Safety level updated to %.1f"), SafetyLevel);
    
    // Safety affects morale
    if (SafetyLevel < 30.0f)
    {
        UpdateTribalMorale(-1.0f);
    }
    else if (SafetyLevel > 70.0f)
    {
        UpdateTribalMorale(1.0f);
    }
}

FString ANarr_TribalSocialSystem::GetTribalStatus()
{
    FString Status = FString::Printf(TEXT("Tribe: %s | Population: %d | Morale: %.1f%% | Food: %.1f%% | Water: %.1f%% | Safety: %.1f%%"),
                                     *TribeName, TotalPopulation, TribalMorale, FoodSupply, WaterSupply, SafetyLevel);
    return Status;
}

void ANarr_TribalSocialSystem::InitializeDefaultTribe()
{
    TribeMembers.Empty();
    SocialBonds.Empty();
    
    // Create tribal leader
    FNarr_TribalMember Leader;
    Leader.Name = TEXT("Kael the Elder");
    Leader.Rank = ENarr_TribalRank::TribalLeader;
    Leader.Age = 55;
    Leader.ExperiencePoints = 1000;
    Leader.HuntingSkill = 70.0f;
    Leader.CraftingSkill = 80.0f;
    Leader.SurvivalKnowledge = 95.0f;
    Leader.Leadership = 90.0f;
    Leader.CurrentActivity = ENarr_TribalActivity::Teaching;
    Leader.KnownStories.Add(TEXT("The Great Hunt of the Ancient Beast"));
    Leader.KnownStories.Add(TEXT("How Fire Was Tamed"));
    Leader.KnownStories.Add(TEXT("The First Shelter"));
    AddTribeMember(Leader);
    
    // Create chief hunter
    FNarr_TribalMember ChiefHunter;
    ChiefHunter.Name = TEXT("Grok Spearmaster");
    ChiefHunter.Rank = ENarr_TribalRank::ChiefHunter;
    ChiefHunter.Age = 35;
    ChiefHunter.ExperiencePoints = 750;
    ChiefHunter.HuntingSkill = 95.0f;
    ChiefHunter.CraftingSkill = 60.0f;
    ChiefHunter.SurvivalKnowledge = 80.0f;
    ChiefHunter.Leadership = 70.0f;
    ChiefHunter.CurrentActivity = ENarr_TribalActivity::Hunting;
    ChiefHunter.KnownStories.Add(TEXT("The Raptor Pack Ambush"));
    AddTribeMember(ChiefHunter);
    
    // Create scout
    FNarr_TribalMember Scout;
    Scout.Name = TEXT("Vex Pathfinder");
    Scout.Rank = ENarr_TribalRank::Scout;
    Scout.Age = 28;
    Scout.ExperiencePoints = 500;
    Scout.HuntingSkill = 75.0f;
    Scout.CraftingSkill = 45.0f;
    Scout.SurvivalKnowledge = 85.0f;
    Scout.Leadership = 50.0f;
    Scout.CurrentActivity = ENarr_TribalActivity::Patrolling;
    Scout.KnownStories.Add(TEXT("The Hidden Water Source"));
    AddTribeMember(Scout);
    
    // Create craftsman
    FNarr_TribalMember Craftsman;
    Craftsman.Name = TEXT("Thane Stonecarver");
    Craftsman.Rank = ENarr_TribalRank::Craftsman;
    Craftsman.Age = 42;
    Craftsman.ExperiencePoints = 600;
    Craftsman.HuntingSkill = 40.0f;
    Craftsman.CraftingSkill = 90.0f;
    Craftsman.SurvivalKnowledge = 70.0f;
    Craftsman.Leadership = 45.0f;
    Craftsman.CurrentActivity = ENarr_TribalActivity::Crafting;
    Craftsman.KnownStories.Add(TEXT("The Perfect Spear Point"));
    AddTribeMember(Craftsman);
    
    // Create some social bonds
    CreateSocialBond(TEXT("Kael the Elder"), TEXT("Grok Spearmaster"), ENarr_SocialRelation::Trusted);
    CreateSocialBond(TEXT("Grok Spearmaster"), TEXT("Vex Pathfinder"), ENarr_SocialRelation::Bonded);
    CreateSocialBond(TEXT("Thane Stonecarver"), TEXT("Kael the Elder"), ENarr_SocialRelation::Trusted);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %s with %d members"), *TribeName, TribeMembers.Num());
}

void ANarr_TribalSocialSystem::ProcessDailyTribalActivities()
{
    // Simulate daily tribal life
    for (FNarr_TribalMember& Member : TribeMembers)
    {
        // Randomly assign activities based on rank and skills
        switch (Member.Rank)
        {
        case ENarr_TribalRank::TribalLeader:
        case ENarr_TribalRank::Elder:
            Member.CurrentActivity = (FMath::RandRange(0, 1) == 0) ? ENarr_TribalActivity::Teaching : ENarr_TribalActivity::Storytelling;
            break;
            
        case ENarr_TribalRank::ChiefHunter:
        case ENarr_TribalRank::Warrior:
        case ENarr_TribalRank::Hunter:
            Member.CurrentActivity = (FMath::RandRange(0, 2) == 0) ? ENarr_TribalActivity::Hunting : ENarr_TribalActivity::Patrolling;
            break;
            
        case ENarr_TribalRank::Scout:
            Member.CurrentActivity = ENarr_TribalActivity::Patrolling;
            break;
            
        case ENarr_TribalRank::Craftsman:
            Member.CurrentActivity = ENarr_TribalActivity::Crafting;
            break;
            
        default:
            Member.CurrentActivity = (FMath::RandRange(0, 1) == 0) ? ENarr_TribalActivity::Gathering : ENarr_TribalActivity::Resting;
            break;
        }
        
        // Gain experience based on activity
        Member.ExperiencePoints += FMath::RandRange(1, 5);
    }
    
    // Update supplies based on activities
    int32 Hunters = GetMembersByActivity(ENarr_TribalActivity::Hunting).Num();
    int32 Gatherers = GetMembersByActivity(ENarr_TribalActivity::Gathering).Num();
    
    float FoodGain = (Hunters * 3.0f) + (Gatherers * 2.0f);
    float WaterGain = Gatherers * 1.5f;
    
    // Natural consumption
    float FoodConsumption = TotalPopulation * 1.0f;
    float WaterConsumption = TotalPopulation * 1.2f;
    
    UpdateSupplies(FoodGain - FoodConsumption, WaterGain - WaterConsumption);
}

TArray<FString> ANarr_TribalSocialSystem::GetAvailableStories()
{
    TArray<FString> AllStories;
    
    for (const FNarr_TribalMember& Member : TribeMembers)
    {
        for (const FString& Story : Member.KnownStories)
        {
            AllStories.AddUnique(Story);
        }
    }
    
    return AllStories;
}

void ANarr_TribalSocialSystem::ShareStoryWithTribe(const FString& StoryTeller, const FString& Story)
{
    // Find the storyteller
    for (FNarr_TribalMember& Member : TribeMembers)
    {
        if (Member.Name == StoryTeller)
        {
            // Add story to other members who don't know it
            for (FNarr_TribalMember& Listener : TribeMembers)
            {
                if (Listener.Name != StoryTeller && !Listener.KnownStories.Contains(Story))
                {
                    Listener.KnownStories.Add(Story);
                }
            }
            
            // Storytelling increases morale
            UpdateTribalMorale(2.0f);
            
            UE_LOG(LogTemp, Log, TEXT("%s shared the story: %s"), *StoryTeller, *Story);
            break;
        }
    }
}