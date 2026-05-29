#include "Quest_PrehistoricQuestRewardSystem.h"
#include "TranspersonalCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UQuest_PrehistoricQuestRewardSystem::UQuest_PrehistoricQuestRewardSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
}

void UQuest_PrehistoricQuestRewardSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeRewardTemplates();
    InitializeExperienceMultipliers();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Reward System initialized with %d reward templates"), RewardTemplates.Num());
}

void UQuest_PrehistoricQuestRewardSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UQuest_PrehistoricQuestRewardSystem::GrantReward(ATranspersonalCharacter* Player, const FQuest_RewardDefinition& Reward)
{
    if (!Player)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot grant reward: Player is null"));
        return false;
    }

    if (!CanGrantReward(Player, Reward))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot grant reward %s to player"), *Reward.RewardID);
        return false;
    }

    bool bSuccess = false;

    switch (Reward.RewardType)
    {
        case EQuest_RewardType::Experience:
            GrantExperience(Player, Reward.ExperiencePoints, Reward.RewardID);
            bSuccess = true;
            break;

        case EQuest_RewardType::Resources:
            bSuccess = GrantResourceReward(Player, Reward.RewardID, Reward.Quantity);
            break;

        case EQuest_RewardType::Tools:
            bSuccess = GrantToolReward(Player, Reward.RewardID, Reward.Quantity);
            break;

        case EQuest_RewardType::Knowledge:
            bSuccess = GrantKnowledgeReward(Player, Reward.RewardID);
            break;

        case EQuest_RewardType::Territory:
            bSuccess = GrantTerritoryAccess(Player, Reward.RewardID);
            break;

        case EQuest_RewardType::Skills:
            bSuccess = GrantSkillImprovement(Player, Reward.RewardID, Reward.ExperiencePoints);
            break;

        case EQuest_RewardType::Crafting_Recipe:
            bSuccess = GrantKnowledgeReward(Player, FString::Printf(TEXT("Recipe_%s"), *Reward.RewardID));
            break;

        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown reward type for reward %s"), *Reward.RewardID);
            break;
    }

    if (bSuccess)
    {
        LogRewardGrant(Player, Reward);
    }

    return bSuccess;
}

bool UQuest_PrehistoricQuestRewardSystem::GrantRewardBundle(ATranspersonalCharacter* Player, const FQuest_RewardBundle& Bundle)
{
    if (!Player || !ValidateRewardBundle(Bundle))
    {
        return false;
    }

    bool bAllSuccessful = true;

    // Grant individual rewards
    for (const FQuest_RewardDefinition& Reward : Bundle.Rewards)
    {
        if (!GrantReward(Player, Reward))
        {
            bAllSuccessful = false;
            UE_LOG(LogTemp, Warning, TEXT("Failed to grant reward %s in bundle %s"), *Reward.RewardID, *Bundle.BundleName);
        }
    }

    // Grant bonus experience
    if (Bundle.TotalExperience > 0.0f)
    {
        GrantExperience(Player, Bundle.TotalExperience, Bundle.BundleName);
    }

    if (bAllSuccessful)
    {
        OnRewardGranted.Broadcast(Bundle, Player);
        UE_LOG(LogTemp, Log, TEXT("Successfully granted reward bundle %s to player"), *Bundle.BundleName);
    }

    return bAllSuccessful;
}

FQuest_RewardBundle UQuest_PrehistoricQuestRewardSystem::CreateBasicSurvivalReward(int32 ExperienceAmount)
{
    FQuest_RewardBundle Bundle;
    Bundle.BundleName = TEXT("Basic Survival");
    Bundle.TotalExperience = ExperienceAmount;

    FQuest_RewardDefinition ExpReward;
    ExpReward.RewardType = EQuest_RewardType::Experience;
    ExpReward.ExperiencePoints = ExperienceAmount;
    ExpReward.Description = TEXT("Experience for basic survival tasks");
    Bundle.Rewards.Add(ExpReward);

    FQuest_RewardDefinition ResourceReward;
    ResourceReward.RewardType = EQuest_RewardType::Resources;
    ResourceReward.RewardID = TEXT("Stone");
    ResourceReward.Quantity = 3;
    ResourceReward.Description = TEXT("Basic crafting stones");
    Bundle.Rewards.Add(ResourceReward);

    return Bundle;
}

FQuest_RewardBundle UQuest_PrehistoricQuestRewardSystem::CreateHuntingReward(const FString& PreyType, int32 PreySize)
{
    FQuest_RewardBundle Bundle;
    Bundle.BundleName = FString::Printf(TEXT("Hunt %s"), *PreyType);
    Bundle.TotalExperience = PreySize * 25.0f; // Larger prey = more experience

    FQuest_RewardDefinition MeatReward;
    MeatReward.RewardType = EQuest_RewardType::Resources;
    MeatReward.RewardID = TEXT("Meat");
    MeatReward.Quantity = FMath::Max(1, PreySize / 2);
    MeatReward.Description = FString::Printf(TEXT("Meat from %s"), *PreyType);
    Bundle.Rewards.Add(MeatReward);

    FQuest_RewardDefinition HideReward;
    HideReward.RewardType = EQuest_RewardType::Resources;
    HideReward.RewardID = TEXT("Hide");
    HideReward.Quantity = 1;
    HideReward.Description = FString::Printf(TEXT("Hide from %s"), *PreyType);
    Bundle.Rewards.Add(HideReward);

    if (PreySize >= 5) // Large prey
    {
        FQuest_RewardDefinition SkillReward;
        SkillReward.RewardType = EQuest_RewardType::Skills;
        SkillReward.RewardID = TEXT("Hunting");
        SkillReward.ExperiencePoints = 10.0f;
        SkillReward.Description = TEXT("Improved hunting skills");
        Bundle.Rewards.Add(SkillReward);
    }

    return Bundle;
}

FQuest_RewardBundle UQuest_PrehistoricQuestRewardSystem::CreateCraftingReward(const FString& ItemCrafted)
{
    FQuest_RewardBundle Bundle;
    Bundle.BundleName = FString::Printf(TEXT("Craft %s"), *ItemCrafted);
    Bundle.TotalExperience = 15.0f;

    FQuest_RewardDefinition SkillReward;
    SkillReward.RewardType = EQuest_RewardType::Skills;
    SkillReward.RewardID = TEXT("Crafting");
    SkillReward.ExperiencePoints = 5.0f;
    SkillReward.Description = FString::Printf(TEXT("Crafting experience from making %s"), *ItemCrafted);
    Bundle.Rewards.Add(SkillReward);

    // Unlock related recipes
    if (ItemCrafted == TEXT("Stone_Axe"))
    {
        FQuest_RewardDefinition RecipeReward;
        RecipeReward.RewardType = EQuest_RewardType::Crafting_Recipe;
        RecipeReward.RewardID = TEXT("Stone_Spear");
        RecipeReward.Description = TEXT("Learned to craft stone spears");
        Bundle.Rewards.Add(RecipeReward);
    }

    return Bundle;
}

FQuest_RewardBundle UQuest_PrehistoricQuestRewardSystem::CreateExplorationReward(const FString& LocationDiscovered)
{
    FQuest_RewardBundle Bundle;
    Bundle.BundleName = FString::Printf(TEXT("Discover %s"), *LocationDiscovered);
    Bundle.TotalExperience = 30.0f;

    FQuest_RewardDefinition ExpReward;
    ExpReward.RewardType = EQuest_RewardType::Experience;
    ExpReward.ExperiencePoints = 30.0f;
    ExpReward.Description = FString::Printf(TEXT("Discovery of %s"), *LocationDiscovered);
    Bundle.Rewards.Add(ExpReward);

    FQuest_RewardDefinition KnowledgeReward;
    KnowledgeReward.RewardType = EQuest_RewardType::Knowledge;
    KnowledgeReward.RewardID = FString::Printf(TEXT("Location_%s"), *LocationDiscovered);
    KnowledgeReward.Description = FString::Printf(TEXT("Knowledge of %s location"), *LocationDiscovered);
    Bundle.Rewards.Add(KnowledgeReward);

    return Bundle;
}

FQuest_RewardBundle UQuest_PrehistoricQuestRewardSystem::CreateTribalReward(int32 TribeMembers)
{
    FQuest_RewardBundle Bundle;
    Bundle.BundleName = TEXT("Tribal Achievement");
    Bundle.TotalExperience = TribeMembers * 20.0f;

    FQuest_RewardDefinition RepReward;
    RepReward.RewardType = EQuest_RewardType::Reputation;
    RepReward.RewardID = TEXT("Tribe");
    RepReward.ExperiencePoints = TribeMembers * 5.0f;
    RepReward.Description = TEXT("Increased tribal standing");
    Bundle.Rewards.Add(RepReward);

    FQuest_RewardDefinition SkillReward;
    SkillReward.RewardType = EQuest_RewardType::Skills;
    SkillReward.RewardID = TEXT("Leadership");
    SkillReward.ExperiencePoints = 15.0f;
    SkillReward.Description = TEXT("Leadership experience");
    Bundle.Rewards.Add(SkillReward);

    return Bundle;
}

bool UQuest_PrehistoricQuestRewardSystem::CanGrantReward(ATranspersonalCharacter* Player, const FQuest_RewardDefinition& Reward)
{
    if (!Player)
    {
        return false;
    }

    // Check inventory space for physical rewards
    if (Reward.RewardType == EQuest_RewardType::Resources || Reward.RewardType == EQuest_RewardType::Tools)
    {
        return HasInventorySpace(Player, Reward);
    }

    // Check skill caps
    if (Reward.RewardType == EQuest_RewardType::Skills)
    {
        if (SkillCaps.Contains(Reward.RewardID))
        {
            // Would need to check player's current skill level
            // For now, assume it's valid
            return true;
        }
    }

    return true;
}

bool UQuest_PrehistoricQuestRewardSystem::ValidateRewardBundle(const FQuest_RewardBundle& Bundle)
{
    if (Bundle.Rewards.Num() == 0)
    {
        return false;
    }

    for (const FQuest_RewardDefinition& Reward : Bundle.Rewards)
    {
        if (Reward.RewardType == EQuest_RewardType::None || Reward.RewardID.IsEmpty())
        {
            return false;
        }
    }

    return true;
}

void UQuest_PrehistoricQuestRewardSystem::GrantExperience(ATranspersonalCharacter* Player, float Amount, const FString& Source)
{
    if (!Player || Amount <= 0.0f)
    {
        return;
    }

    float Multiplier = 1.0f;
    if (ExperienceMultipliers.Contains(Source))
    {
        Multiplier = ExperienceMultipliers[Source];
    }

    float FinalAmount = Amount * Multiplier;
    
    // Add experience to player (would need actual player experience system)
    UE_LOG(LogTemp, Log, TEXT("Granted %f experience to player from %s (multiplier: %f)"), FinalAmount, *Source, Multiplier);
}

float UQuest_PrehistoricQuestRewardSystem::CalculateExperienceBonus(ATranspersonalCharacter* Player, const FString& ActivityType)
{
    if (!Player)
    {
        return 1.0f;
    }

    // Base multiplier
    float Bonus = 1.0f;

    // Activity-specific bonuses
    if (ActivityType == TEXT("Hunting"))
    {
        Bonus += 0.2f; // 20% bonus for hunting
    }
    else if (ActivityType == TEXT("Exploration"))
    {
        Bonus += 0.3f; // 30% bonus for exploration
    }
    else if (ActivityType == TEXT("Crafting"))
    {
        Bonus += 0.1f; // 10% bonus for crafting
    }

    return Bonus;
}

bool UQuest_PrehistoricQuestRewardSystem::GrantResourceReward(ATranspersonalCharacter* Player, const FString& ResourceType, int32 Amount)
{
    if (!Player || Amount <= 0)
    {
        return false;
    }

    // Check max resource limits
    if (MaxResourceAmounts.Contains(ResourceType))
    {
        int32 MaxAmount = MaxResourceAmounts[ResourceType];
        if (Amount > MaxAmount)
        {
            UE_LOG(LogTemp, Warning, TEXT("Resource reward %s amount %d exceeds maximum %d"), *ResourceType, Amount, MaxAmount);
            Amount = MaxAmount;
        }
    }

    // Grant resource to player inventory (would need actual inventory system)
    UE_LOG(LogTemp, Log, TEXT("Granted %d %s to player"), Amount, *ResourceType);
    return true;
}

bool UQuest_PrehistoricQuestRewardSystem::GrantToolReward(ATranspersonalCharacter* Player, const FString& ToolType, int32 Quality)
{
    if (!Player)
    {
        return false;
    }

    // Grant tool to player inventory (would need actual inventory system)
    UE_LOG(LogTemp, Log, TEXT("Granted %s (quality %d) to player"), *ToolType, Quality);
    return true;
}

bool UQuest_PrehistoricQuestRewardSystem::GrantKnowledgeReward(ATranspersonalCharacter* Player, const FString& KnowledgeType)
{
    if (!Player)
    {
        return false;
    }

    // Add knowledge to player's knowledge base (would need actual knowledge system)
    UE_LOG(LogTemp, Log, TEXT("Granted knowledge %s to player"), *KnowledgeType);
    return true;
}

bool UQuest_PrehistoricQuestRewardSystem::GrantSkillImprovement(ATranspersonalCharacter* Player, const FString& SkillName, float Improvement)
{
    if (!Player || Improvement <= 0.0f)
    {
        return false;
    }

    // Check skill cap
    if (SkillCaps.Contains(SkillName))
    {
        float Cap = SkillCaps[SkillName];
        // Would need to check player's current skill level and cap improvement
    }

    // Improve player skill (would need actual skill system)
    UE_LOG(LogTemp, Log, TEXT("Improved %s skill by %f for player"), *SkillName, Improvement);
    return true;
}

bool UQuest_PrehistoricQuestRewardSystem::GrantTerritoryAccess(ATranspersonalCharacter* Player, const FString& TerritoryName)
{
    if (!Player)
    {
        return false;
    }

    // Grant territory access to player (would need actual territory system)
    UE_LOG(LogTemp, Log, TEXT("Granted access to territory %s for player"), *TerritoryName);
    return true;
}

void UQuest_PrehistoricQuestRewardSystem::ModifyReputation(ATranspersonalCharacter* Player, const FString& Faction, float Change)
{
    if (!Player)
    {
        return;
    }

    // Modify player reputation with faction (would need actual reputation system)
    UE_LOG(LogTemp, Log, TEXT("Modified reputation with %s by %f for player"), *Faction, Change);
}

void UQuest_PrehistoricQuestRewardSystem::InitializeRewardTemplates()
{
    // Initialize common reward templates
    RewardTemplates.Add(TEXT("BasicSurvival"), CreateBasicSurvivalReward(10));
    RewardTemplates.Add(TEXT("SmallHunt"), CreateHuntingReward(TEXT("Rabbit"), 1));
    RewardTemplates.Add(TEXT("LargeHunt"), CreateHuntingReward(TEXT("Mammoth"), 10));
    RewardTemplates.Add(TEXT("FirstCraft"), CreateCraftingReward(TEXT("Stone_Axe")));
    RewardTemplates.Add(TEXT("CaveDiscovery"), CreateExplorationReward(TEXT("Hidden_Cave")));
    RewardTemplates.Add(TEXT("TribalLeader"), CreateTribalReward(5));
}

void UQuest_PrehistoricQuestRewardSystem::InitializeExperienceMultipliers()
{
    ExperienceMultipliers.Add(TEXT("Hunting"), 1.2f);
    ExperienceMultipliers.Add(TEXT("Exploration"), 1.3f);
    ExperienceMultipliers.Add(TEXT("Crafting"), 1.1f);
    ExperienceMultipliers.Add(TEXT("Survival"), 1.0f);
    ExperienceMultipliers.Add(TEXT("Combat"), 1.4f);
    ExperienceMultipliers.Add(TEXT("Social"), 1.1f);
}

bool UQuest_PrehistoricQuestRewardSystem::HasInventorySpace(ATranspersonalCharacter* Player, const FQuest_RewardDefinition& Reward)
{
    if (!Player)
    {
        return false;
    }

    // For now, assume player always has space
    // In a real implementation, this would check the player's inventory component
    return true;
}

void UQuest_PrehistoricQuestRewardSystem::LogRewardGrant(ATranspersonalCharacter* Player, const FQuest_RewardDefinition& Reward)
{
    if (!Player)
    {
        return;
    }

    FString PlayerName = Player->GetName();
    FString RewardTypeStr = UEnum::GetValueAsString(Reward.RewardType);
    
    UE_LOG(LogTemp, Log, TEXT("REWARD GRANTED - Player: %s, Type: %s, ID: %s, Quantity: %d, Description: %s"), 
           *PlayerName, *RewardTypeStr, *Reward.RewardID, Reward.Quantity, *Reward.Description);
}