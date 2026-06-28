// QuestCraftingSystem.cpp — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260628_010
// Implements crafting-driven quest objectives for the survival game.

#include "QuestCraftingSystem.h"
#include "Engine/World.h"

UQuestCraftingSystem::UQuestCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuestCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultMissions();
    UE_LOG(LogTemp, Log, TEXT("[QuestCraftingSystem] Initialized with %d missions in database."), MissionDatabase.Num());
}

// ============================================================
// MISSION DATABASE — 3 core crafting quests
// ============================================================

void UQuestCraftingSystem::InitializeDefaultMissions()
{
    MissionDatabase.Empty();

    // ---- MISSION 1: Forge the First Axe ----
    {
        FQuest_CraftingMission M;
        M.MissionID = FName("QUEST_FIRST_AXE");
        M.MissionTitle = FText::FromString("Forge the First Axe");
        M.MissionDescription = FText::FromString(
            "The tribe needs tools. Gather flint stones from the river bend "
            "and dry wood from the eastern fallen trees. Craft a Stone Axe "
            "and return to the elder before nightfall.");
        M.RewardItem = EQuest_CraftedItemType::StoneAxe;
        M.CompletionNarrativeLine = FText::FromString(
            "The Stone Axe is yours. With this blade, you can cut bone, "
            "strip bark, and defend against smaller predators.");

        // Objective 1: Gather 2 flint stones
        FQuest_CraftingObjective Obj1;
        Obj1.ObjectiveID = FName("OBJ_GATHER_FLINT");
        Obj1.ObjectiveType = EQuest_CraftingObjectiveType::GatherResource;
        Obj1.TargetResource = EQuest_ResourceType::FlintStone;
        Obj1.RequiredCount = 2;
        Obj1.ObjectiveDescription = FText::FromString("Gather Flint Stones (0/2)");
        M.Objectives.Add(Obj1);

        // Objective 2: Gather 1 dry wood
        FQuest_CraftingObjective Obj2;
        Obj2.ObjectiveID = FName("OBJ_GATHER_WOOD");
        Obj2.ObjectiveType = EQuest_CraftingObjectiveType::GatherResource;
        Obj2.TargetResource = EQuest_ResourceType::DryWood;
        Obj2.RequiredCount = 1;
        Obj2.ObjectiveDescription = FText::FromString("Gather Dry Wood (0/1)");
        M.Objectives.Add(Obj2);

        // Objective 3: Craft the stone axe
        FQuest_CraftingObjective Obj3;
        Obj3.ObjectiveID = FName("OBJ_CRAFT_AXE");
        Obj3.ObjectiveType = EQuest_CraftingObjectiveType::CraftItem;
        Obj3.TargetItem = EQuest_CraftedItemType::StoneAxe;
        Obj3.RequiredCount = 1;
        Obj3.ObjectiveDescription = FText::FromString("Craft a Stone Axe (0/1)");
        M.Objectives.Add(Obj3);

        MissionDatabase.Add(M);
    }

    // ---- MISSION 2: Light the First Fire ----
    {
        FQuest_CraftingMission M;
        M.MissionID = FName("QUEST_FIRST_FIRE");
        M.MissionTitle = FText::FromString("Light the First Fire");
        M.MissionDescription = FText::FromString(
            "Night falls fast and the predators grow bold in darkness. "
            "Gather three bundles of dry wood and build a campfire "
            "near the camp perimeter. Fire keeps the hunters away.");
        M.RewardItem = EQuest_CraftedItemType::Campfire;
        M.CompletionNarrativeLine = FText::FromString(
            "The fire burns. Tonight, the tribe sleeps safely. "
            "But gather more wood — fires die, and so do the careless.");

        // Objective 1: Gather 3 dry wood
        FQuest_CraftingObjective Obj1;
        Obj1.ObjectiveID = FName("OBJ_GATHER_WOOD_FIRE");
        Obj1.ObjectiveType = EQuest_CraftingObjectiveType::GatherResource;
        Obj1.TargetResource = EQuest_ResourceType::DryWood;
        Obj1.RequiredCount = 3;
        Obj1.ObjectiveDescription = FText::FromString("Gather Dry Wood (0/3)");
        M.Objectives.Add(Obj1);

        // Objective 2: Build campfire
        FQuest_CraftingObjective Obj2;
        Obj2.ObjectiveID = FName("OBJ_BUILD_CAMPFIRE");
        Obj2.ObjectiveType = EQuest_CraftingObjectiveType::CraftItem;
        Obj2.TargetItem = EQuest_CraftedItemType::Campfire;
        Obj2.RequiredCount = 1;
        Obj2.ObjectiveDescription = FText::FromString("Build a Campfire (0/1)");
        M.Objectives.Add(Obj2);

        // Objective 3: Use the campfire (sit by it)
        FQuest_CraftingObjective Obj3;
        Obj3.ObjectiveID = FName("OBJ_USE_CAMPFIRE");
        Obj3.ObjectiveType = EQuest_CraftingObjectiveType::UseItem;
        Obj3.TargetItem = EQuest_CraftedItemType::Campfire;
        Obj3.RequiredCount = 1;
        Obj3.ObjectiveDescription = FText::FromString("Rest by the Campfire (0/1)");
        M.Objectives.Add(Obj3);

        MissionDatabase.Add(M);
    }

    // ---- MISSION 3: Water from Stone ----
    {
        FQuest_CraftingMission M;
        M.MissionID = FName("QUEST_WATER_CONTAINER");
        M.MissionTitle = FText::FromString("Water from Stone");
        M.MissionDescription = FText::FromString(
            "The dry season approaches. Without a way to carry water, "
            "the tribe cannot follow the herds. Find a smooth river stone "
            "and a broad leaf. Shape a water container — it will save lives.");
        M.RewardItem = EQuest_CraftedItemType::WaterContainer;
        M.CompletionNarrativeLine = FText::FromString(
            "The container holds. Now you can carry water from the river "
            "and follow the great herds into the open plains.");

        // Objective 1: Gather 1 smooth stone (FlintStone variant)
        FQuest_CraftingObjective Obj1;
        Obj1.ObjectiveID = FName("OBJ_GATHER_STONE_WATER");
        Obj1.ObjectiveType = EQuest_CraftingObjectiveType::GatherResource;
        Obj1.TargetResource = EQuest_ResourceType::FlintStone;
        Obj1.RequiredCount = 1;
        Obj1.ObjectiveDescription = FText::FromString("Gather a Smooth Stone (0/1)");
        M.Objectives.Add(Obj1);

        // Objective 2: Gather 1 large leaf
        FQuest_CraftingObjective Obj2;
        Obj2.ObjectiveID = FName("OBJ_GATHER_LEAF");
        Obj2.ObjectiveType = EQuest_CraftingObjectiveType::GatherResource;
        Obj2.TargetResource = EQuest_ResourceType::Leaf;
        Obj2.RequiredCount = 1;
        Obj2.ObjectiveDescription = FText::FromString("Gather a Large Leaf (0/1)");
        M.Objectives.Add(Obj2);

        // Objective 3: Craft water container
        FQuest_CraftingObjective Obj3;
        Obj3.ObjectiveID = FName("OBJ_CRAFT_CONTAINER");
        Obj3.ObjectiveType = EQuest_CraftingObjectiveType::CraftItem;
        Obj3.TargetItem = EQuest_CraftedItemType::WaterContainer;
        Obj3.RequiredCount = 1;
        Obj3.ObjectiveDescription = FText::FromString("Craft a Water Container (0/1)");
        M.Objectives.Add(Obj3);

        // Objective 4: Fill it at the river (UseItem)
        FQuest_CraftingObjective Obj4;
        Obj4.ObjectiveID = FName("OBJ_FILL_CONTAINER");
        Obj4.ObjectiveType = EQuest_CraftingObjectiveType::UseItem;
        Obj4.TargetItem = EQuest_CraftedItemType::WaterContainer;
        Obj4.RequiredCount = 1;
        Obj4.ObjectiveDescription = FText::FromString("Fill the Container at the River (0/1)");
        M.Objectives.Add(Obj4);

        MissionDatabase.Add(M);
    }
}

// ============================================================
// PUBLIC API
// ============================================================

void UQuestCraftingSystem::StartMission(FName MissionID)
{
    // Check if already active or completed
    for (const FQuest_CraftingMission& M : ActiveMissions)
    {
        if (M.MissionID == MissionID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[QuestCraftingSystem] Mission %s already active."), *MissionID.ToString());
            return;
        }
    }

    // Find in database
    for (const FQuest_CraftingMission& DBMission : MissionDatabase)
    {
        if (DBMission.MissionID == MissionID)
        {
            FQuest_CraftingMission NewMission = DBMission;
            NewMission.bActive = true;
            NewMission.bCompleted = false;
            ActiveMissions.Add(NewMission);
            UE_LOG(LogTemp, Log, TEXT("[QuestCraftingSystem] Started mission: %s"), *MissionID.ToString());
            return;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("[QuestCraftingSystem] Mission %s not found in database."), *MissionID.ToString());
}

void UQuestCraftingSystem::ReportResourceGathered(EQuest_ResourceType ResourceType, int32 Count)
{
    for (FQuest_CraftingMission& Mission : ActiveMissions)
    {
        if (!Mission.bActive || Mission.bCompleted) continue;
        UpdateObjectiveProgress(Mission,
            EQuest_CraftingObjectiveType::GatherResource,
            ResourceType,
            EQuest_CraftedItemType::StoneAxe, // unused for resource gather
            Count);
    }
}

void UQuestCraftingSystem::ReportItemCrafted(EQuest_CraftedItemType ItemType)
{
    for (FQuest_CraftingMission& Mission : ActiveMissions)
    {
        if (!Mission.bActive || Mission.bCompleted) continue;
        UpdateObjectiveProgress(Mission,
            EQuest_CraftingObjectiveType::CraftItem,
            EQuest_ResourceType::FlintStone, // unused for craft
            ItemType,
            1);
    }
}

void UQuestCraftingSystem::ReportItemUsed(EQuest_CraftedItemType ItemType)
{
    for (FQuest_CraftingMission& Mission : ActiveMissions)
    {
        if (!Mission.bActive || Mission.bCompleted) continue;
        UpdateObjectiveProgress(Mission,
            EQuest_CraftingObjectiveType::UseItem,
            EQuest_ResourceType::FlintStone, // unused for use
            ItemType,
            1);
    }
}

bool UQuestCraftingSystem::IsMissionActive(FName MissionID) const
{
    for (const FQuest_CraftingMission& M : ActiveMissions)
    {
        if (M.MissionID == MissionID) return M.bActive && !M.bCompleted;
    }
    return false;
}

bool UQuestCraftingSystem::IsMissionCompleted(FName MissionID) const
{
    for (const FQuest_CraftingMission& M : ActiveMissions)
    {
        if (M.MissionID == MissionID) return M.bCompleted;
    }
    return false;
}

float UQuestCraftingSystem::GetMissionProgress(FName MissionID) const
{
    for (const FQuest_CraftingMission& M : ActiveMissions)
    {
        if (M.MissionID != MissionID) continue;
        if (M.Objectives.Num() == 0) return 0.0f;

        int32 CompletedCount = 0;
        for (const FQuest_CraftingObjective& Obj : M.Objectives)
        {
            if (Obj.bCompleted) CompletedCount++;
        }
        return static_cast<float>(CompletedCount) / static_cast<float>(M.Objectives.Num());
    }
    return 0.0f;
}

FQuest_CraftingMission UQuestCraftingSystem::GetMissionData(FName MissionID) const
{
    for (const FQuest_CraftingMission& M : ActiveMissions)
    {
        if (M.MissionID == MissionID) return M;
    }
    return FQuest_CraftingMission();
}

// ============================================================
// PRIVATE HELPERS
// ============================================================

void UQuestCraftingSystem::UpdateObjectiveProgress(
    FQuest_CraftingMission& Mission,
    EQuest_CraftingObjectiveType ObjType,
    EQuest_ResourceType Resource,
    EQuest_CraftedItemType Item,
    int32 Count)
{
    for (FQuest_CraftingObjective& Obj : Mission.Objectives)
    {
        if (Obj.bCompleted) continue;
        if (Obj.ObjectiveType != ObjType) continue;

        bool bMatches = false;
        if (ObjType == EQuest_CraftingObjectiveType::GatherResource && Obj.TargetResource == Resource)
            bMatches = true;
        if ((ObjType == EQuest_CraftingObjectiveType::CraftItem ||
             ObjType == EQuest_CraftingObjectiveType::UseItem) && Obj.TargetItem == Item)
            bMatches = true;

        if (!bMatches) continue;

        Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Count, Obj.RequiredCount);
        if (Obj.CurrentCount >= Obj.RequiredCount)
        {
            Obj.bCompleted = true;
            UE_LOG(LogTemp, Log, TEXT("[QuestCraftingSystem] Objective %s COMPLETED in mission %s"),
                *Obj.ObjectiveID.ToString(), *Mission.MissionID.ToString());
            OnObjectiveUpdated.Broadcast(Mission.MissionID, Obj.ObjectiveID);
        }
    }

    CheckMissionCompletion(Mission);
}

void UQuestCraftingSystem::CheckMissionCompletion(FQuest_CraftingMission& Mission)
{
    if (Mission.bCompleted) return;

    bool bAllDone = true;
    for (const FQuest_CraftingObjective& Obj : Mission.Objectives)
    {
        if (!Obj.bCompleted) { bAllDone = false; break; }
    }

    if (bAllDone)
    {
        Mission.bCompleted = true;
        Mission.bActive = false;
        UE_LOG(LogTemp, Log, TEXT("[QuestCraftingSystem] MISSION COMPLETED: %s"), *Mission.MissionID.ToString());
        OnMissionCompleted.Broadcast(Mission.MissionID);
    }
}
