// NarrativeDialogueData.cpp
// Agent #15 — Narrative & Dialogue Agent
// Full dialogue tree for NPC_HerdTracker_QuestGiver + waypoint flavour lines + death beats
// Cycle: PROD_CYCLE_AUTO_20260629_006

#include "NarrativeDialogueData.h"

UNarr_HerdTrackerDialogueAsset::UNarr_HerdTrackerDialogueAsset()
{
    // Populate default content on construction
    PopulateDefaultContent();
}

void UNarr_HerdTrackerDialogueAsset::PopulateDefaultContent()
{
    DialogueNodes.Empty();
    WaypointLines.Empty();
    DeathBeats.Empty();

    // ─── DIALOGUE TREE: NPC_HerdTracker_QuestGiver ────────────────────────────
    // Node IDs: HT_Greet, HT_QuestOffer, HT_Accept, HT_Decline, HT_InProgress_1..3, HT_Complete, HT_Failed

    // --- GREETING ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_Greet");
        Node.NodeType = ENarr_DialogueNodeType::NPC_Line;
        Node.SpeakerName = FText::FromString(TEXT("Kael"));
        Node.DialogueText = FText::FromString(
            TEXT("You look like you know how to move quietly. Good. I need someone who can track without being tracked.")
        );
        Node.AudioURL = TEXT(""); // Placeholder — record in next cycle
        Node.NextNodeIDs = { FName("HT_QuestOffer") };
        DialogueNodes.Add(Node);
    }

    // --- QUEST OFFER ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_QuestOffer");
        Node.NodeType = ENarr_DialogueNodeType::NPC_Line;
        Node.SpeakerName = FText::FromString(TEXT("Kael"));
        Node.DialogueText = FText::FromString(
            TEXT("The Triceratops herd is moving through the valley. Three bulls at the front — they are nervous. "
                 "Something spooked them two days ago and they have not settled since. "
                 "I need you to follow their migration route to the southern pass and tell me what is hunting them. "
                 "Will you do it?")
        );
        Node.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782722202064_QuestGiver_HerdTracker.mp3");
        Node.NextNodeIDs = { FName("HT_Accept"), FName("HT_Decline") };
        DialogueNodes.Add(Node);
    }

    // --- PLAYER CHOICE: ACCEPT ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_Accept");
        Node.NodeType = ENarr_DialogueNodeType::Player_Choice;
        Node.SpeakerName = FText::FromString(TEXT("Player"));
        Node.DialogueText = FText::FromString(TEXT("I will track the herd."));
        Node.NextNodeIDs = { FName("HT_Accept_Response") };
        DialogueNodes.Add(Node);
    }

    // --- PLAYER CHOICE: DECLINE ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_Decline");
        Node.NodeType = ENarr_DialogueNodeType::Player_Choice;
        Node.SpeakerName = FText::FromString(TEXT("Player"));
        Node.DialogueText = FText::FromString(TEXT("That sounds like a good way to get killed."));
        Node.NextNodeIDs = { FName("HT_Decline_Response") };
        DialogueNodes.Add(Node);
    }

    // --- ACCEPT RESPONSE ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_Accept_Response");
        Node.NodeType = ENarr_DialogueNodeType::NPC_Line;
        Node.SpeakerName = FText::FromString(TEXT("Kael"));
        Node.DialogueText = FText::FromString(
            TEXT("Good. Follow the valley east. You will find their tracks near the river. "
                 "Do not get between the bulls — they are unpredictable right now. "
                 "And if you hear the ground shake, climb. Immediately.")
        );
        Node.NextNodeIDs = { FName("HT_End_Accept") };
        DialogueNodes.Add(Node);
    }

    // --- DECLINE RESPONSE ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_Decline_Response");
        Node.NodeType = ENarr_DialogueNodeType::NPC_Line;
        Node.SpeakerName = FText::FromString(TEXT("Kael"));
        Node.DialogueText = FText::FromString(
            TEXT("You are right. It probably is. Come back when you are ready — "
                 "the herd will not reach the southern pass for another two days.")
        );
        Node.NextNodeIDs = { FName("HT_End_Decline") };
        DialogueNodes.Add(Node);
    }

    // --- IN-PROGRESS CHECK-IN 1 (player returns mid-quest) ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_InProgress_1");
        Node.NodeType = ENarr_DialogueNodeType::NPC_Line;
        Node.SpeakerName = FText::FromString(TEXT("Kael"));
        Node.DialogueText = FText::FromString(
            TEXT("You are still alive. Good sign. Have you reached the river crossing yet? "
                 "That is where the predator usually makes its move.")
        );
        Node.ConditionTag = FName("Quest_InProgress");
        Node.NextNodeIDs = { FName("HT_End_InProgress") };
        DialogueNodes.Add(Node);
    }

    // --- IN-PROGRESS CHECK-IN 2 (past river, approaching resting ground) ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_InProgress_2");
        Node.NodeType = ENarr_DialogueNodeType::NPC_Line;
        Node.SpeakerName = FText::FromString(TEXT("Kael"));
        Node.DialogueText = FText::FromString(
            TEXT("Past the river. You are doing well. The resting ground is ahead — "
                 "that is where I lost the last tracker I sent. He did not come back. "
                 "Watch the tree line.")
        );
        Node.ConditionTag = FName("Quest_PastRiver");
        Node.NextNodeIDs = { FName("HT_End_InProgress") };
        DialogueNodes.Add(Node);
    }

    // --- IN-PROGRESS CHECK-IN 3 (stampede active) ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_InProgress_3");
        Node.NodeType = ENarr_DialogueNodeType::NPC_Line;
        Node.SpeakerName = FText::FromString(TEXT("Kael"));
        Node.DialogueText = FText::FromString(
            TEXT("If you can hear me — get to high ground. The herd is running. "
                 "Do not try to outrun them. You cannot. Find a rock, a tree, anything above the ground. Go.")
        );
        Node.ConditionTag = FName("Quest_StampedeActive");
        Node.NextNodeIDs = { FName("HT_End_InProgress") };
        DialogueNodes.Add(Node);
    }

    // --- QUEST COMPLETE ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_Complete");
        Node.NodeType = ENarr_DialogueNodeType::NPC_Line;
        Node.SpeakerName = FText::FromString(TEXT("Kael"));
        Node.DialogueText = FText::FromString(
            TEXT("You tracked the herd all the way to the southern pass. Good work. "
                 "Now we know their route — and we know what hunts them. "
                 "The Shadow Bull. I have heard stories. I hoped they were just stories. "
                 "Take this. You earned it.")
        );
        Node.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782722220879_QuestGiver_HerdTracker_Complet.mp3");
        Node.ConditionTag = FName("Quest_Completed");
        Node.NextNodeIDs = { FName("HT_End_Complete") };
        DialogueNodes.Add(Node);
    }

    // --- QUEST FAILED ---
    {
        FNarr_DialogueNode Node;
        Node.NodeID = FName("HT_Failed");
        Node.NodeType = ENarr_DialogueNodeType::NPC_Line;
        Node.SpeakerName = FText::FromString(TEXT("Kael"));
        Node.DialogueText = FText::FromString(
            TEXT("You made it back. That is more than the last one managed. "
                 "Rest. Eat something. When you are ready, the herd will come through again in three days. "
                 "The migration does not stop just because we fail.")
        );
        Node.ConditionTag = FName("Quest_Failed");
        Node.NextNodeIDs = { FName("HT_End_Failed") };
        DialogueNodes.Add(Node);
    }

    // ─── WAYPOINT FLAVOUR LINES ────────────────────────────────────────────────

    // Valley Entrance
    {
        FNarr_WaypointFlavourLine Line;
        Line.WaypointID = ENarr_WaypointID::ValleyEntrance;
        Line.SpeakerName = FText::FromString(TEXT("Kael"));
        Line.FlavourText = FText::FromString(
            TEXT("You have reached the valley entrance. The herd passed through here this morning — "
                 "look at the ground. Those deep prints, the broken shrubs. "
                 "Three tonnes of Triceratops, moving fast. Something scared them.")
        );
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782722363792_Waypoint_ValleyEntrance.mp3");
        WaypointLines.Add(Line);
    }

    // River Crossing
    {
        FNarr_WaypointFlavourLine Line;
        Line.WaypointID = ENarr_WaypointID::RiverCrossing;
        Line.SpeakerName = FText::FromString(TEXT("Kael"));
        Line.FlavourText = FText::FromString(
            TEXT("The river crossing. This is where the herd is most vulnerable — "
                 "they slow down in the water. That is when the predator strikes. "
                 "Stay on the high bank. Do not go into the water.")
        );
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782722366239_Waypoint_RiverCrossing.mp3");
        WaypointLines.Add(Line);
    }

    // Herd Resting Ground
    {
        FNarr_WaypointFlavourLine Line;
        Line.WaypointID = ENarr_WaypointID::HerdRestingGround;
        Line.SpeakerName = FText::FromString(TEXT("Kael"));
        Line.FlavourText = FText::FromString(
            TEXT("The herd rested here last night. You can still smell them. "
                 "But look — over there, at the tree line. Claw marks on the bark. High up. "
                 "That is not a raptor. Something bigger has been watching this place.")
        );
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782722377371_Waypoint_HerdRestingGround.mp3");
        WaypointLines.Add(Line);
    }

    // Predator Territory (no audio recorded yet — placeholder)
    {
        FNarr_WaypointFlavourLine Line;
        Line.WaypointID = ENarr_WaypointID::PredatorTerritory;
        Line.SpeakerName = FText::FromString(TEXT("Kael"));
        Line.FlavourText = FText::FromString(
            TEXT("Predator territory. The herd knows it — see how they bunched together here, "
                 "the tracks closer, the calves pushed to the centre. "
                 "Whatever hunts them, it lives in this stretch of ground. Move carefully.")
        );
        Line.AudioURL = TEXT(""); // To be recorded next cycle
        WaypointLines.Add(Line);
    }

    // Southern Pass
    {
        FNarr_WaypointFlavourLine Line;
        Line.WaypointID = ENarr_WaypointID::SouthernPass;
        Line.SpeakerName = FText::FromString(TEXT("Kael"));
        Line.FlavourText = FText::FromString(
            TEXT("You made it to the southern pass. The herd is through. "
                 "But I need you to understand something — that predator that follows the migration, "
                 "the one we call the Shadow Bull, it is not hunting the Triceratops. "
                 "It is hunting whatever follows the Triceratops. It is hunting us.")
        );
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782722380394_Waypoint_SouthernPass_Reveal.mp3");
        WaypointLines.Add(Line);
    }

    // ─── DEATH NARRATIVE BEATS ────────────────────────────────────────────────

    // Stampede death
    {
        FNarr_DeathNarrativeBeat Beat;
        Beat.CauseTag = FName("Stampede");
        Beat.NarrativeText = FText::FromString(
            TEXT("The ground took you. Not the predator — the herd itself. "
                 "Three tonnes of Triceratops do not see what is beneath their feet. "
                 "You were in the wrong place when the panic started. "
                 "The valley remembers nothing. The herd moves on.")
        );
        Beat.DisplayDuration = 6.0f;
        DeathBeats.Add(Beat);
    }

    // Raptor death
    {
        FNarr_DeathNarrativeBeat Beat;
        Beat.CauseTag = FName("Raptor");
        Beat.NarrativeText = FText::FromString(
            TEXT("They came from three directions at once. "
                 "You knew the theory — raptors flank, they coordinate, they wait for the moment you commit to one. "
                 "Knowing it and surviving it are different things. "
                 "The valley is quieter now.")
        );
        Beat.DisplayDuration = 6.0f;
        DeathBeats.Add(Beat);
    }

    // TRex death
    {
        FNarr_DeathNarrativeBeat Beat;
        Beat.CauseTag = FName("TRex");
        Beat.NarrativeText = FText::FromString(
            TEXT("You froze. That was the mistake. "
                 "Movement triggers the hunt — but freezing, in the open, with nowhere to go, "
                 "that is not survival. That is waiting. "
                 "The valley does not wait with you.")
        );
        Beat.DisplayDuration = 6.0f;
        DeathBeats.Add(Beat);
    }

    // ─── SHADOW BULL LORE ─────────────────────────────────────────────────────

    ShadowBullLoreEntry = FText::FromString(
        TEXT("The Shadow Bull. That is what the trackers call it. "
             "We do not know the species — it is not a Tyrannosaurus, not a Spinosaurus. "
             "It is something that follows the Triceratops migrations, year after year, "
             "learning their routes, learning their vulnerabilities. "
             "It does not hunt the herd. It hunts the hunters. "
             "Every tracker who has followed this migration and not come back — "
             "we think it was the Shadow Bull.")
    );

    ShadowBullSpeciesNote = FText::FromString(
        TEXT("Species: Unknown large theropod. Estimated length 12-14 metres. "
             "Behaviour: Opportunistic ambush predator. Follows large herbivore migrations. "
             "Targets secondary predators and scavengers that trail the herd. "
             "High intelligence indicated by territorial marking patterns and adaptive hunting routes. "
             "Avoid at all costs. Do not engage. Do not follow.")
    );
}

FNarr_DialogueNode UNarr_HerdTrackerDialogueAsset::GetNodeByID(FName NodeID) const
{
    for (const FNarr_DialogueNode& Node : DialogueNodes)
    {
        if (Node.NodeID == NodeID)
        {
            return Node;
        }
    }
    // Return empty node if not found
    return FNarr_DialogueNode();
}

FNarr_WaypointFlavourLine UNarr_HerdTrackerDialogueAsset::GetWaypointLine(ENarr_WaypointID WaypointID) const
{
    for (const FNarr_WaypointFlavourLine& Line : WaypointLines)
    {
        if (Line.WaypointID == WaypointID)
        {
            return Line;
        }
    }
    return FNarr_WaypointFlavourLine();
}

FNarr_DeathNarrativeBeat UNarr_HerdTrackerDialogueAsset::GetDeathBeat(FName CauseTag) const
{
    for (const FNarr_DeathNarrativeBeat& Beat : DeathBeats)
    {
        if (Beat.CauseTag == CauseTag)
        {
            return Beat;
        }
    }
    return FNarr_DeathNarrativeBeat();
}
