#include "NarrativeDialogueData.h"
#include "GameFramework/Actor.h"

// ============================================================
// Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260620_007
// All 8 quest zone voice briefs with ElevenLabs TTS URLs.
// ============================================================

// TTS Audio URLs generated this cycle:
// WaterHole:     https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781948990646_ElderNPC_WaterHoleWarning.mp3
// Cave:          https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781948992627_ElderNPC_CaveQuestBrief.mp3
// Clearing:      https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949076484_ElderNPC_ClearingExplore.mp3
// RockShelter:   https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949078496_ElderNPC_RockShelterDefend.mp3
// Hilltop:       https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949100818_ElderNPC_HilltopSurvive.mp3
// RiverBank:     https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949103097_ElderNPC_RiverFish.mp3
// ForestEdge:    https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949111136_ElderNPC_ForestEdgeGather.mp3
// HuntingGround: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949113762_ElderNPC_HuntingGroundHunt.mp3

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitialiseAllQuestBriefs();
}

void ANarr_DialogueManager::RegisterBrief(ENarr_QuestZoneID Zone, const FString& Speaker,
                                           const FString& Text, const FString& AudioURL, float Duration)
{
    FNarr_DialogueLine Line;
    Line.SpeakerID = Speaker;
    Line.DialogueText = Text;
    Line.VoiceAudioURL = AudioURL;
    Line.LinkedQuestZone = Zone;
    Line.EstimatedDurationSeconds = Duration;
    QuestZoneDialogueMap.Add(static_cast<uint8>(Zone), Line);
}

void ANarr_DialogueManager::InitialiseAllQuestBriefs()
{
    QuestZoneDialogueMap.Empty();

    RegisterBrief(
        ENarr_QuestZoneID::WaterHole,
        TEXT("ElderNPC"),
        TEXT("Stranger. The water hole is not safe. Three raptors have claimed it at dusk. If you must drink, go before the sun moves two hands. After that — do not go near it."),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781948990646_ElderNPC_WaterHoleWarning.mp3"),
        10.0f
    );

    RegisterBrief(
        ENarr_QuestZoneID::ForestEdge,
        TEXT("ElderNPC"),
        TEXT("The forest edge gives wood and cover both. Take only dry branches — green wood smokes too much and draws attention. Five armloads should last three nights. Move fast and stay low."),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949111136_ElderNPC_ForestEdgeGather.mp3"),
        12.0f
    );

    RegisterBrief(
        ENarr_QuestZoneID::RockShelter,
        TEXT("ElderNPC"),
        TEXT("The rock shelter holds. But not for long. Something large circles it at night — I heard it breathing. You need to drive it off before the others return at dusk."),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949078496_ElderNPC_RockShelterDefend.mp3"),
        11.0f
    );

    RegisterBrief(
        ENarr_QuestZoneID::Clearing,
        TEXT("ElderNPC"),
        TEXT("You see that clearing? Last season, my father found three flint cores there, half buried under the roots. Go before the rains come. Bring back what you can carry."),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949076484_ElderNPC_ClearingExplore.mp3"),
        11.0f
    );

    RegisterBrief(
        ENarr_QuestZoneID::Hilltop,
        TEXT("ElderNPC"),
        TEXT("The hilltop is exposed. No cover, no shelter. But you can see everything from there — every direction. Stay until dawn. Count what moves in the dark. That knowledge is worth more than meat."),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949100818_ElderNPC_HilltopSurvive.mp3"),
        13.0f
    );

    RegisterBrief(
        ENarr_QuestZoneID::RiverBank,
        TEXT("ElderNPC"),
        TEXT("River runs fast this time of year. The fish are there — you can see them from the bank. But do not wade in past your knees. The current will take you. Use a sharpened stick and patience."),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949103097_ElderNPC_RiverFish.mp3"),
        13.0f
    );

    RegisterBrief(
        ENarr_QuestZoneID::Cave,
        TEXT("ElderNPC"),
        TEXT("The cave to the north holds bones of the old hunters. Flint. Sinew. Tools we have forgotten how to make. Go in. Take what you find. But do not go deeper than the second bend — something lives further in."),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781948992627_ElderNPC_CaveQuestBrief.mp3"),
        11.0f
    );

    RegisterBrief(
        ENarr_QuestZoneID::HuntingGround,
        TEXT("ElderNPC"),
        TEXT("The big ones move slow but they do not stop. Track it from downwind. When it pauses to feed, that is your moment. One clean throw to the neck. Do not chase it — follow the blood trail and wait."),
        TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781949113762_ElderNPC_HuntingGroundHunt.mp3"),
        13.0f
    );

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Initialised %d quest zone briefs."), QuestZoneDialogueMap.Num());
}

FNarr_DialogueLine ANarr_DialogueManager::GetDialogueForZone(ENarr_QuestZoneID ZoneID) const
{
    const FNarr_DialogueLine* Found = QuestZoneDialogueMap.Find(static_cast<uint8>(ZoneID));
    if (Found)
    {
        return *Found;
    }
    return FNarr_DialogueLine();
}

void ANarr_DialogueManager::PlayDialogueForZone(ENarr_QuestZoneID ZoneID)
{
    FNarr_DialogueLine Line = GetDialogueForZone(ZoneID);
    if (!Line.VoiceAudioURL.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogue: Playing zone %d — Speaker: %s — URL: %s"),
            static_cast<int32>(ZoneID), *Line.SpeakerID, *Line.VoiceAudioURL);
        // Audio Agent #16 picks up VoiceAudioURL via MetaSounds integration
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogue: No audio URL for zone %d"), static_cast<int32>(ZoneID));
    }
}
