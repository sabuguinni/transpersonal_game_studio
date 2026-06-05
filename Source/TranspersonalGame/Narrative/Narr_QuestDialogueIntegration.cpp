#include "Narr_QuestDialogueIntegration.h"
#include "Engine/Engine.h"

UNarr_QuestDialogueIntegration::UNarr_QuestDialogueIntegration()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UNarr_QuestDialogueIntegration::BeginPlay()
{
    Super::BeginPlay();
    InitializeQuestDialogues();
}

void UNarr_QuestDialogueIntegration::InitializeQuestDialogues()
{
    QuestDialogueNodes.Empty();
    MissionBriefings.Empty();

    SetupHuntMissionDialogues();
    SetupGatherMissionDialogues();
    SetupExploreMissionDialogues();
    SetupCraftMissionDialogues();
    SetupRescueMissionDialogues();
    SetupDefenseMissionDialogues();
    SetupDiscoveryMissionDialogues();

    CreateMissionBriefings();
    SetupCharacterVoices();

    UE_LOG(LogTemp, Warning, TEXT("Quest Dialogue Integration initialized with %d dialogue nodes and %d mission briefings"), 
           QuestDialogueNodes.Num(), MissionBriefings.Num());
}

FNarr_QuestDialogueNode UNarr_QuestDialogueIntegration::GetQuestDialogue(EQuest_MissionType MissionType, bool bIsComplete)
{
    for (const FNarr_QuestDialogueNode& Node : QuestDialogueNodes)
    {
        if (Node.AssociatedMissionType == MissionType && Node.bIsQuestComplete == bIsComplete)
        {
            return Node;
        }
    }

    // Return default dialogue if not found
    FNarr_QuestDialogueNode DefaultNode;
    DefaultNode.DialogueText = TEXT("The hunt continues, survivor. Stay vigilant.");
    DefaultNode.CharacterName = TEXT("Elder Kava");
    DefaultNode.EmotionalTone = ENarr_EmotionalState::Determined;
    return DefaultNode;
}

FNarr_MissionBriefing UNarr_QuestDialogueIntegration::GetMissionBriefing(EQuest_MissionType MissionType, EQuest_DifficultyLevel Difficulty)
{
    for (const FNarr_MissionBriefing& Briefing : MissionBriefings)
    {
        if (Briefing.MissionType == MissionType && Briefing.DifficultyLevel == Difficulty)
        {
            return Briefing;
        }
    }

    // Return default briefing
    FNarr_MissionBriefing DefaultBriefing;
    DefaultBriefing.BriefingTitle = TEXT("Survival Mission");
    DefaultBriefing.BriefingDescription = TEXT("Complete the assigned task to ensure tribal survival.");
    return DefaultBriefing;
}

void UNarr_QuestDialogueIntegration::UpdateQuestProgress(EQuest_MissionType MissionType, bool bCompleted)
{
    for (FNarr_QuestDialogueNode& Node : QuestDialogueNodes)
    {
        if (Node.AssociatedMissionType == MissionType)
        {
            Node.bIsQuestComplete = bCompleted;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Quest progress updated for mission type %d: %s"), 
           (int32)MissionType, bCompleted ? TEXT("COMPLETED") : TEXT("IN PROGRESS"));
}

FString UNarr_QuestDialogueIntegration::GetCharacterVoiceID(const FString& CharacterName)
{
    if (CharacterVoiceMapping.Contains(CharacterName))
    {
        return CharacterVoiceMapping[CharacterName];
    }
    return TEXT("pNInz6obpgDQGcFmaJgB"); // Default voice
}

void UNarr_QuestDialogueIntegration::SetupHuntMissionDialogues()
{
    // Hunt Mission Start
    FNarr_QuestDialogueNode HuntStart;
    HuntStart.DialogueText = TEXT("The great beasts roam the eastern territories. Bring down a predator and prove your worth as a hunter. Remember - aim for the heart, strike with precision, and never hunt alone.");
    HuntStart.CharacterName = TEXT("Hunter Grok");
    HuntStart.EmotionalTone = ENarr_EmotionalState::Determined;
    HuntStart.PlayerResponses.Add(TEXT("I accept this hunt."));
    HuntStart.PlayerResponses.Add(TEXT("What weapons should I bring?"));
    HuntStart.PlayerResponses.Add(TEXT("I need more preparation time."));
    HuntStart.AssociatedMissionType = EQuest_MissionType::Hunt;
    HuntStart.bIsQuestComplete = false;
    QuestDialogueNodes.Add(HuntStart);

    // Hunt Mission Complete
    FNarr_QuestDialogueNode HuntComplete;
    HuntComplete.DialogueText = TEXT("Excellent work, hunter! The beast's roar will echo no more in our territories. Your courage has earned you respect among the tribe. Take these crafted spears as your reward.");
    HuntComplete.CharacterName = TEXT("Hunter Grok");
    HuntComplete.EmotionalTone = ENarr_EmotionalState::Proud;
    HuntComplete.PlayerResponses.Add(TEXT("The hunt was challenging but rewarding."));
    HuntComplete.PlayerResponses.Add(TEXT("What's our next target?"));
    HuntComplete.AssociatedMissionType = EQuest_MissionType::Hunt;
    HuntComplete.bIsQuestComplete = true;
    QuestDialogueNodes.Add(HuntComplete);
}

void UNarr_QuestDialogueIntegration::SetupGatherMissionDialogues()
{
    // Gather Mission Start
    FNarr_QuestDialogueNode GatherStart;
    GatherStart.DialogueText = TEXT("The tribe's food stores run low. Venture to the fertile valleys and gather berries, roots, and medicinal herbs. Beware of the creatures that also seek these resources.");
    GatherStart.CharacterName = TEXT("Elder Kava");
    GatherStart.EmotionalTone = ENarr_EmotionalState::Concerned;
    GatherStart.PlayerResponses.Add(TEXT("I will gather what the tribe needs."));
    GatherStart.PlayerResponses.Add(TEXT("Which plants are safe to harvest?"));
    GatherStart.PlayerResponses.Add(TEXT("How much should I collect?"));
    GatherStart.AssociatedMissionType = EQuest_MissionType::Gather;
    GatherStart.bIsQuestComplete = false;
    QuestDialogueNodes.Add(GatherStart);

    // Gather Mission Complete
    FNarr_QuestDialogueNode GatherComplete;
    GatherComplete.DialogueText = TEXT("Well done! These resources will sustain us through the harsh season. Your knowledge of the land grows stronger with each expedition. The tribe is grateful for your dedication.");
    GatherComplete.CharacterName = TEXT("Elder Kava");
    GatherComplete.EmotionalTone = ENarr_EmotionalState::Grateful;
    GatherComplete.PlayerResponses.Add(TEXT("The valley was rich with resources."));
    GatherComplete.PlayerResponses.Add(TEXT("I learned much about the plants."));
    GatherComplete.AssociatedMissionType = EQuest_MissionType::Gather;
    GatherComplete.bIsQuestComplete = true;
    QuestDialogueNodes.Add(GatherComplete);
}

void UNarr_QuestDialogueIntegration::SetupExploreMissionDialogues()
{
    // Explore Mission Start
    FNarr_QuestDialogueNode ExploreStart;
    ExploreStart.DialogueText = TEXT("Unknown territories lie beyond the great ridge. Scout these lands, map safe passages, and report any threats or opportunities. The future of our tribe may depend on what you discover.");
    ExploreStart.CharacterName = TEXT("Scout Thane");
    ExploreStart.EmotionalTone = ENarr_EmotionalState::Focused;
    ExploreStart.PlayerResponses.Add(TEXT("I will explore the unknown lands."));
    ExploreStart.PlayerResponses.Add(TEXT("What should I look for specifically?"));
    ExploreStart.PlayerResponses.Add(TEXT("How far should I venture?"));
    ExploreStart.AssociatedMissionType = EQuest_MissionType::Explore;
    ExploreStart.bIsQuestComplete = false;
    QuestDialogueNodes.Add(ExploreStart);

    // Explore Mission Complete
    FNarr_QuestDialogueNode ExploreComplete;
    ExploreComplete.DialogueText = TEXT("Your scouting report is invaluable! The paths you've mapped will serve our people well. Your courage to venture into the unknown shows the spirit of a true pathfinder.");
    ExploreComplete.CharacterName = TEXT("Scout Thane");
    ExploreComplete.EmotionalTone = ENarr_EmotionalState::Impressed;
    ExploreComplete.PlayerResponses.Add(TEXT("The new territories hold promise."));
    ExploreComplete.PlayerResponses.Add(TEXT("I marked several safe camping spots."));
    ExploreComplete.AssociatedMissionType = EQuest_MissionType::Explore;
    ExploreComplete.bIsQuestComplete = true;
    QuestDialogueNodes.Add(ExploreComplete);
}

void UNarr_QuestDialogueIntegration::SetupCraftMissionDialogues()
{
    // Craft Mission Start
    FNarr_QuestDialogueNode CraftStart;
    CraftStart.DialogueText = TEXT("Our tools grow dull and our shelters need reinforcement. Craft the implements we need for survival - sharp blades, sturdy spears, and protective barriers. Quality craftsmanship saves lives.");
    CraftStart.CharacterName = TEXT("Craft Master Nira");
    CraftStart.EmotionalTone = ENarr_EmotionalState::Instructive;
    CraftStart.PlayerResponses.Add(TEXT("I will craft what the tribe needs."));
    CraftStart.PlayerResponses.Add(TEXT("What materials should I use?"));
    CraftStart.PlayerResponses.Add(TEXT("Can you teach me advanced techniques?"));
    CraftStart.AssociatedMissionType = EQuest_MissionType::Craft;
    CraftStart.bIsQuestComplete = false;
    QuestDialogueNodes.Add(CraftStart);

    // Craft Mission Complete
    FNarr_QuestDialogueNode CraftComplete;
    CraftComplete.DialogueText = TEXT("Exceptional craftsmanship! These tools will serve our hunters and gatherers well. Your skill with stone and wood rivals the masters of old. The tribe's survival is more secure with your contributions.");
    CraftComplete.CharacterName = TEXT("Craft Master Nira");
    CraftComplete.EmotionalTone = ENarr_EmotionalState::Proud;
    CraftComplete.PlayerResponses.Add(TEXT("The materials shaped well under my hands."));
    CraftComplete.PlayerResponses.Add(TEXT("I've learned new crafting techniques."));
    CraftComplete.AssociatedMissionType = EQuest_MissionType::Craft;
    CraftComplete.bIsQuestComplete = true;
    QuestDialogueNodes.Add(CraftComplete);
}

void UNarr_QuestDialogueIntegration::SetupRescueMissionDialogues()
{
    // Rescue Mission Start
    FNarr_QuestDialogueNode RescueStart;
    RescueStart.DialogueText = TEXT("One of our own is trapped in the canyon caves. Time is running short and dangerous predators prowl the area. Bring them home safely - no one gets left behind in our tribe.");
    RescueStart.CharacterName = TEXT("Elder Kava");
    RescueStart.EmotionalTone = ENarr_EmotionalState::Urgent;
    RescueStart.PlayerResponses.Add(TEXT("I will bring them home."));
    RescueStart.PlayerResponses.Add(TEXT("What dangers should I expect?"));
    RescueStart.PlayerResponses.Add(TEXT("Do we know their exact location?"));
    RescueStart.AssociatedMissionType = EQuest_MissionType::Rescue;
    RescueStart.bIsQuestComplete = false;
    QuestDialogueNodes.Add(RescueStart);

    // Rescue Mission Complete
    FNarr_QuestDialogueNode RescueComplete;
    RescueComplete.DialogueText = TEXT("You have saved a life today! Your bravery and quick thinking prevented a tragedy. The rescued tribesman speaks highly of your courage. You are truly a guardian of our people.");
    RescueComplete.CharacterName = TEXT("Elder Kava");
    RescueComplete.EmotionalTone = ENarr_EmotionalState::Grateful;
    RescueComplete.PlayerResponses.Add(TEXT("I couldn't leave them behind."));
    RescueComplete.PlayerResponses.Add(TEXT("The caves were treacherous but navigable."));
    RescueComplete.AssociatedMissionType = EQuest_MissionType::Rescue;
    RescueComplete.bIsQuestComplete = true;
    QuestDialogueNodes.Add(RescueComplete);
}

void UNarr_QuestDialogueIntegration::SetupDefenseMissionDialogues()
{
    // Defense Mission Start
    FNarr_QuestDialogueNode DefenseStart;
    DefenseStart.DialogueText = TEXT("A pack of predators approaches our settlement! Fortify our defenses and stand ready to protect the tribe. Position yourself at the perimeter and drive back any threat that dares approach our home.");
    DefenseStart.CharacterName = TEXT("Hunter Grok");
    DefenseStart.EmotionalTone = ENarr_EmotionalState::Alert;
    DefenseStart.PlayerResponses.Add(TEXT("I will defend our home."));
    DefenseStart.PlayerResponses.Add(TEXT("How many predators are coming?"));
    DefenseStart.PlayerResponses.Add(TEXT("What weapons are available?"));
    DefenseStart.AssociatedMissionType = EQuest_MissionType::Defense;
    DefenseStart.bIsQuestComplete = false;
    QuestDialogueNodes.Add(DefenseStart);

    // Defense Mission Complete
    FNarr_QuestDialogueNode DefenseComplete;
    DefenseComplete.DialogueText = TEXT("Outstanding defense! The predators have been driven off and our settlement remains secure. Your vigilance and combat skill protected every member of the tribe. We sleep safely because of your courage.");
    DefenseComplete.CharacterName = TEXT("Hunter Grok");
    DefenseComplete.EmotionalTone = ENarr_EmotionalState::Relieved;
    DefenseComplete.PlayerResponses.Add(TEXT("The tribe's safety comes first."));
    DefenseComplete.PlayerResponses.Add(TEXT("We should strengthen our perimeter."));
    DefenseComplete.AssociatedMissionType = EQuest_MissionType::Defense;
    DefenseComplete.bIsQuestComplete = true;
    QuestDialogueNodes.Add(DefenseComplete);
}

void UNarr_QuestDialogueIntegration::SetupDiscoveryMissionDialogues()
{
    // Discovery Mission Start
    FNarr_QuestDialogueNode DiscoveryStart;
    DiscoveryStart.DialogueText = TEXT("Ancient markings have been spotted in the northern cliffs. Investigate these mysterious symbols and uncover their meaning. Such discoveries could reveal secrets of those who came before us.");
    DiscoveryStart.CharacterName = TEXT("Scout Thane");
    DiscoveryStart.EmotionalTone = ENarr_EmotionalState::Curious;
    DiscoveryStart.PlayerResponses.Add(TEXT("I will investigate the ancient markings."));
    DiscoveryStart.PlayerResponses.Add(TEXT("What do the symbols look like?"));
    DiscoveryStart.PlayerResponses.Add(TEXT("Are there dangers in that area?"));
    DiscoveryStart.AssociatedMissionType = EQuest_MissionType::Discovery;
    DiscoveryStart.bIsQuestComplete = false;
    QuestDialogueNodes.Add(DiscoveryStart);

    // Discovery Mission Complete
    FNarr_QuestDialogueNode DiscoveryComplete;
    DiscoveryComplete.DialogueText = TEXT("Remarkable discovery! These ancient markings tell stories of the old ones who survived in these lands before us. Your findings will be preserved in our tribal memory. Knowledge is our greatest treasure.");
    DiscoveryComplete.CharacterName = TEXT("Scout Thane");
    DiscoveryComplete.EmotionalTone = ENarr_EmotionalState::Amazed;
    DiscoveryComplete.PlayerResponses.Add(TEXT("The ancients were wise survivors."));
    DiscoveryComplete.PlayerResponses.Add(TEXT("There may be more secrets to uncover."));
    DiscoveryComplete.AssociatedMissionType = EQuest_MissionType::Discovery;
    DiscoveryComplete.bIsQuestComplete = true;
    QuestDialogueNodes.Add(DiscoveryComplete);
}

void UNarr_QuestDialogueIntegration::CreateMissionBriefings()
{
    // Hunt Mission Briefings
    FNarr_MissionBriefing HuntEasy;
    HuntEasy.BriefingTitle = TEXT("Predator Hunt - Small Game");
    HuntEasy.BriefingDescription = TEXT("Track and eliminate a small predator threatening our food sources. Target: Compsognathus pack in the southern grasslands.");
    HuntEasy.ObjectiveDetails = TEXT("Eliminate 3-5 Compsognathus. Collect their hide for crafting materials.");
    HuntEasy.WarningText = TEXT("Small but fast - use ranged weapons and maintain distance.");
    HuntEasy.MissionType = EQuest_MissionType::Hunt;
    HuntEasy.DifficultyLevel = EQuest_DifficultyLevel::Easy;
    HuntEasy.EstimatedDuration = 300.0f;
    MissionBriefings.Add(HuntEasy);

    FNarr_MissionBriefing HuntHard;
    HuntHard.BriefingTitle = TEXT("Apex Predator Hunt - Carnotaurus");
    HuntHard.BriefingDescription = TEXT("Face the ultimate challenge - a massive Carnotaurus has claimed territory near our water source. Only the most skilled hunters should attempt this mission.");
    HuntHard.ObjectiveDetails = TEXT("Eliminate the Carnotaurus. Bring back its heart as proof of victory.");
    HuntHard.WarningText = TEXT("EXTREME DANGER - Massive size, powerful bite, incredible speed. Coordinate with other hunters.");
    HuntHard.MissionType = EQuest_MissionType::Hunt;
    HuntHard.DifficultyLevel = EQuest_DifficultyLevel::Hard;
    HuntHard.EstimatedDuration = 900.0f;
    MissionBriefings.Add(HuntHard);

    // Gather Mission Briefings
    FNarr_MissionBriefing GatherMedium;
    GatherMedium.BriefingTitle = TEXT("Resource Gathering - Medicinal Herbs");
    GatherMedium.BriefingDescription = TEXT("Collect rare healing herbs from the dangerous swamplands. These plants are essential for treating injuries from predator encounters.");
    GatherMedium.ObjectiveDetails = TEXT("Gather 10 Bloodroot plants, 15 Healing Moss clusters, 5 Pain-Relief Bark strips.");
    GatherMedium.WarningText = TEXT("Swampland predators and poisonous plants pose significant threats.");
    GatherMedium.MissionType = EQuest_MissionType::Gather;
    GatherMedium.DifficultyLevel = EQuest_DifficultyLevel::Medium;
    GatherMedium.EstimatedDuration = 450.0f;
    MissionBriefings.Add(GatherMedium);

    // Explore Mission Briefings
    FNarr_MissionBriefing ExploreHard;
    ExploreHard.BriefingTitle = TEXT("Deep Territory Exploration - Volcanic Region");
    ExploreHard.BriefingDescription = TEXT("Scout the treacherous volcanic territories for new hunting grounds and resources. Map safe passages through this dangerous but resource-rich region.");
    ExploreHard.ObjectiveDetails = TEXT("Map 5 safe camping locations, identify 3 water sources, catalog dangerous predator territories.");
    ExploreHard.WarningText = TEXT("Extreme heat, toxic gases, and aggressive territorial predators. Bring heat protection.");
    ExploreHard.MissionType = EQuest_MissionType::Explore;
    ExploreHard.DifficultyLevel = EQuest_DifficultyLevel::Hard;
    ExploreHard.EstimatedDuration = 720.0f;
    MissionBriefings.Add(ExploreHard);
}

void UNarr_QuestDialogueIntegration::SetupCharacterVoices()
{
    CharacterVoiceMapping.Empty();
    
    // Map character names to ElevenLabs voice IDs
    CharacterVoiceMapping.Add(TEXT("Elder Kava"), TEXT("EXAVITQu4vr4xnSDxMaL"));
    CharacterVoiceMapping.Add(TEXT("Hunter Grok"), TEXT("pNInz6obpgDQGcFmaJgB"));
    CharacterVoiceMapping.Add(TEXT("Scout Thane"), TEXT("ErXwobaYiN019PkySvjV"));
    CharacterVoiceMapping.Add(TEXT("Craft Master Nira"), TEXT("MF3mGyEYCl7XYWbV9V6O"));
    
    UE_LOG(LogTemp, Warning, TEXT("Character voice mapping initialized with %d voices"), CharacterVoiceMapping.Num());
}