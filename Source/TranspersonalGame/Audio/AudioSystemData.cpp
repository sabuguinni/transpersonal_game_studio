// AudioSystemData.cpp — Agent #16 Audio Agent
// Cycle: PROD_CYCLE_AUTO_20260629_011
// Full implementation of UAudio_QuestAudioManager with all quest dialogue audio URLs
// TTS assets generated in cycles 009-011 via ElevenLabs / Supabase

#include "AudioSystemData.h"

// ============================================================
// Supabase TTS Audio URLs (generated cycles 009-011)
// ============================================================
// Scout_MidHunt:
//   https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751285919_Scout_MidHunt.mp3
// Hunter_Brek_Wave2:
//   https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751311806_Hunter_Brek_Wave2.mp3
// Elder_QuestBriefing_Hunt (cycle 010):
//   https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751147011_Elder_QuestBriefing_Hunt.mp3
// Elder_QuestBriefing_Gather (cycle 010):
//   https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751149998_Elder_QuestBriefing_Gather.mp3
// Elder_QuestBriefing_Defend (cycle 010):
//   https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751169995_Elder_QuestBriefing_Defend.mp3
// Freesound river ambient IDs: 442478, 579427, 819768, 552485, 610726
// ============================================================

UAudio_QuestAudioManager::UAudio_QuestAudioManager()
{
    InitializeQuestAudio();
}

void UAudio_QuestAudioManager::InitializeQuestAudio()
{
    QuestAudioSets.Empty();

    // --------------------------------------------------------
    // HUNT ZONE — Raptor Pack
    // --------------------------------------------------------
    {
        FAudio_QuestAudioSet HuntSet;
        HuntSet.QuestZone = EAudio_QuestZone::Hunt;

        // Briefing
        HuntSet.BriefingLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Elder,
            TEXT("The raptor pack circles at dusk. Three of them — young males, hungry and bold. You have one chance to drive them off before they reach the camp."),
            TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751147011_Elder_QuestBriefing_Hunt.mp3"),
            6.0f
        ));

        // Progress — Scout mid-hunt
        HuntSet.ProgressLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Scout,
            TEXT("Scout report — mid-hunt. The raptors split. Two went east toward the river, one circled back behind you. Watch your back. Do not let it flank you."),
            TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751285919_Scout_MidHunt.mp3"),
            6.0f
        ));

        // Completion
        HuntSet.CompletionLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Elder,
            TEXT("You drove them back. The pack will not return tonight. The camp is safe because of what you did."),
            TEXT(""),  // TTS pending
            4.0f
        ));

        // Failure
        HuntSet.FailureLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Elder,
            TEXT("The raptors reached the camp. We lost two shelters. You did what you could — but next time, we must be faster."),
            TEXT(""),  // TTS pending
            5.0f
        ));

        // No specific freesound for jungle hunt — use general wind ambience
        HuntSet.FreesoundAmbientIDs.Add(0);

        QuestAudioSets.Add(HuntSet);
    }

    // --------------------------------------------------------
    // GATHER ZONE — Flint at River Bend
    // --------------------------------------------------------
    {
        FAudio_QuestAudioSet GatherSet;
        GatherSet.QuestZone = EAudio_QuestZone::Gather;

        // Briefing
        GatherSet.BriefingLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Elder,
            TEXT("Flint. Sharp flint from the river bend. Without it, we cannot make blades, we cannot scrape hides, we cannot survive the cold season."),
            TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751149998_Elder_QuestBriefing_Gather.mp3"),
            6.0f
        ));

        // Progress
        GatherSet.ProgressLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Player,
            TEXT("The river bend. Flint in the shallows — I can see it glinting under the water. Need to move carefully. The current is strong here."),
            TEXT(""),  // TTS pending
            4.0f
        ));

        // Completion
        GatherSet.CompletionLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Elder,
            TEXT("Good flint. Sharp edges, no cracks. This will make three blades, maybe four. You did well. Now go back — there is more where that came from."),
            TEXT(""),  // TTS pending
            5.0f
        ));

        // Failure
        GatherSet.FailureLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Elder,
            TEXT("No flint. We will have to use bone tools for now. It will slow us down, but we survive. Try again when the river drops."),
            TEXT(""),  // TTS pending
            4.0f
        ));

        // Freesound river ambient IDs from search results
        GatherSet.FreesoundAmbientIDs.Add(442478);   // Forest river flows over rocks
        GatherSet.FreesoundAmbientIDs.Add(579427);   // Rocky stream calm
        GatherSet.FreesoundAmbientIDs.Add(819768);   // Small brook flowing around rocks

        QuestAudioSets.Add(GatherSet);
    }

    // --------------------------------------------------------
    // DEFEND ZONE — Camp Night Raid
    // --------------------------------------------------------
    {
        FAudio_QuestAudioSet DefendSet;
        DefendSet.QuestZone = EAudio_QuestZone::Defend;

        // Briefing
        DefendSet.BriefingLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Elder,
            TEXT("They came in the night. Twelve of them — armed with bone clubs and fire. We held the eastern wall but lost two hunters. They will come back."),
            TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751169995_Elder_QuestBriefing_Defend.mp3"),
            6.0f
        ));

        // Progress — Hunter Brek second wave
        DefendSet.ProgressLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::HunterBrek,
            TEXT("Hunter Brek calling — second wave incoming from the north tree line! At least eight of them, maybe more. Hold the wall! Do not break formation!"),
            TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782751311806_Hunter_Brek_Wave2.mp3"),
            5.0f
        ));

        // Completion
        DefendSet.CompletionLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Elder,
            TEXT("The camp is secured. They will not return — not after what we showed them tonight. Rest. Tomorrow we rebuild."),
            TEXT(""),  // TTS pending
            4.0f
        ));

        // Failure
        DefendSet.FailureLines.Add(FAudio_DialogueCue(
            EAudio_DialogueSpeaker::Elder,
            TEXT("The wall broke. We retreat to the caves — all of us. Take only what you can carry. We will rebuild somewhere safer."),
            TEXT(""),  // TTS pending
            5.0f
        ));

        // Camp ambience — no specific freesound found, placeholder
        DefendSet.FreesoundAmbientIDs.Add(0);

        QuestAudioSets.Add(DefendSet);
    }
}

FAudio_QuestAudioSet UAudio_QuestAudioManager::GetQuestAudioSet(EAudio_QuestZone Zone) const
{
    for (const FAudio_QuestAudioSet& Set : QuestAudioSets)
    {
        if (Set.QuestZone == Zone)
        {
            return Set;
        }
    }
    return FAudio_QuestAudioSet();
}

TArray<FAudio_DialogueCue> UAudio_QuestAudioManager::GetBriefingLines(EAudio_QuestZone Zone) const
{
    return GetQuestAudioSet(Zone).BriefingLines;
}

TArray<FAudio_DialogueCue> UAudio_QuestAudioManager::GetProgressLines(EAudio_QuestZone Zone) const
{
    return GetQuestAudioSet(Zone).ProgressLines;
}

TArray<FAudio_DialogueCue> UAudio_QuestAudioManager::GetCompletionLines(EAudio_QuestZone Zone) const
{
    return GetQuestAudioSet(Zone).CompletionLines;
}

TArray<FAudio_DialogueCue> UAudio_QuestAudioManager::GetFailureLines(EAudio_QuestZone Zone) const
{
    return GetQuestAudioSet(Zone).FailureLines;
}
