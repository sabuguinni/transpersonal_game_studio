// NarrativeDialogueManager.cpp
// Agent #15 — Narrative & Dialogue Agent
// CYCLE: PROD_CYCLE_AUTO_20260627_005

#include "NarrativeDialogueManager.h"
#include "Engine/World.h"

ANarrativeDialogueManager::ANarrativeDialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bDialogueActive = false;
}

void ANarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultProfiles();
}

void ANarrativeDialogueManager::InitializeDefaultProfiles()
{
    // --- Tribe Elder ---
    FNarr_NPCVoiceProfile ElderProfile;
    ElderProfile.Speaker = ENarr_DialogueSpeaker::TribeElder;
    ElderProfile.CharacterName = TEXT("The Elder");

    FNarr_DialogueLine ElderLine1;
    ElderLine1.Speaker = ENarr_DialogueSpeaker::TribeElder;
    ElderLine1.Trigger = ENarr_DialogueTrigger::OnApproach;
    ElderLine1.LineText = TEXT("Three winters ago, a man from the northern clan tried to cross the valley alone. We found his bones near the river bend — cracked open, marrow sucked clean. That was one raptor. There are seven now.");
    ElderLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782548764065_TribeElder.mp3");
    ElderLine1.Duration = 27.0f;
    ElderProfile.DialogueLines.Add(ElderLine1);

    FNarr_DialogueLine ElderLine2;
    ElderLine2.Speaker = ENarr_DialogueSpeaker::TribeElder;
    ElderLine2.Trigger = ENarr_DialogueTrigger::OnQuestStart;
    ElderLine2.LineText = TEXT("The old hunter crouches by the fire. His eyes do not leave the treeline. You want to hunt in that valley? Bring four spears and two good men. Or do not go at all.");
    ElderLine2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782548764065_TribeElder.mp3");
    ElderLine2.Duration = 27.0f;
    ElderProfile.DialogueLines.Add(ElderLine2);

    NPCProfiles.Add(ElderProfile);

    // --- Hunt Leader ---
    FNarr_NPCVoiceProfile HuntProfile;
    HuntProfile.Speaker = ENarr_DialogueSpeaker::HuntLeader;
    HuntProfile.CharacterName = TEXT("Hunt Leader");

    FNarr_DialogueLine HuntLine1;
    HuntLine1.Speaker = ENarr_DialogueSpeaker::HuntLeader;
    HuntLine1.Trigger = ENarr_DialogueTrigger::OnQuestStart;
    HuntLine1.LineText = TEXT("Move. Now. The herd is crossing the ridge — if we lose sight of them before the canyon narrows, we lose the hunt. Stay low. Stay quiet. One clean throw to the flank. We eat tonight.");
    HuntLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782548768754_HuntLeader.mp3");
    HuntLine1.Duration = 21.0f;
    HuntProfile.DialogueLines.Add(HuntLine1);

    NPCProfiles.Add(HuntProfile);

    // --- Scout ---
    FNarr_NPCVoiceProfile ScoutProfile;
    ScoutProfile.Speaker = ENarr_DialogueSpeaker::Scout;
    ScoutProfile.CharacterName = TEXT("Scout");

    FNarr_DialogueLine ScoutLine1;
    ScoutLine1.Speaker = ENarr_DialogueSpeaker::Scout;
    ScoutLine1.Trigger = ENarr_DialogueTrigger::OnDanger;
    ScoutLine1.LineText = TEXT("You found the cache. Good. The ones who hid it — they are gone now. Taken by the large one, the one with the scarred hide. We call it the Shadow. It hunts at dusk, always from downwind. Take what you need. Then move camp. Tonight.");
    ScoutLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782548790894_ScoutNPC.mp3");
    ScoutLine1.Duration = 23.0f;
    ScoutProfile.DialogueLines.Add(ScoutLine1);

    NPCProfiles.Add(ScoutProfile);

    // --- Tribal Warrior ---
    FNarr_NPCVoiceProfile WarriorProfile;
    WarriorProfile.Speaker = ENarr_DialogueSpeaker::TribalWarrior;
    WarriorProfile.CharacterName = TEXT("Tribal Warrior");

    FNarr_DialogueLine WarriorLine1;
    WarriorLine1.Speaker = ENarr_DialogueSpeaker::TribalWarrior;
    WarriorLine1.Trigger = ENarr_DialogueTrigger::OnDanger;
    WarriorLine1.LineText = TEXT("The child ran back to camp screaming. Said she saw it at the river — standing still, watching. Not hunting. Watching. That is worse. The pack leader does not attack until it knows the numbers. We have twelve people here. Six fighters. It knows that now. We need to move.");
    WarriorLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782548803486_TribalWarrior.mp3");
    WarriorLine1.Duration = 26.0f;
    WarriorProfile.DialogueLines.Add(WarriorLine1);

    NPCProfiles.Add(WarriorProfile);

    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Initialized %d NPC profiles"), NPCProfiles.Num());
}

void ANarrativeDialogueManager::TriggerDialogue(ENarr_DialogueSpeaker Speaker, ENarr_DialogueTrigger Trigger)
{
    if (bDialogueActive)
    {
        return;
    }

    for (const FNarr_NPCVoiceProfile& Profile : NPCProfiles)
    {
        if (Profile.Speaker == Speaker)
        {
            for (const FNarr_DialogueLine& Line : Profile.DialogueLines)
            {
                if (Line.Trigger == Trigger)
                {
                    ActiveLine = Line;
                    bDialogueActive = true;
                    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Playing dialogue for %s — %s"),
                        *Profile.CharacterName, *Line.LineText.Left(60));
                    return;
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: No dialogue found for speaker %d, trigger %d"),
        (int32)Speaker, (int32)Trigger);
}

void ANarrativeDialogueManager::EndDialogue()
{
    bDialogueActive = false;
    ActiveLine = FNarr_DialogueLine();
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Dialogue ended"));
}

TArray<FNarr_DialogueLine> ANarrativeDialogueManager::GetLinesForSpeaker(ENarr_DialogueSpeaker Speaker) const
{
    for (const FNarr_NPCVoiceProfile& Profile : NPCProfiles)
    {
        if (Profile.Speaker == Speaker)
        {
            return Profile.DialogueLines;
        }
    }
    return TArray<FNarr_DialogueLine>();
}

void ANarrativeDialogueManager::RegisterNPCProfile(const FNarr_NPCVoiceProfile& Profile)
{
    // Remove existing profile for this speaker if present
    NPCProfiles.RemoveAll([&Profile](const FNarr_NPCVoiceProfile& Existing)
    {
        return Existing.Speaker == Profile.Speaker;
    });
    NPCProfiles.Add(Profile);
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Registered profile for %s"), *Profile.CharacterName);
}

void ANarrativeDialogueManager::LoadDefaultDialogueLines()
{
    NPCProfiles.Empty();
    InitializeDefaultProfiles();
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Default dialogue lines reloaded"));
}
