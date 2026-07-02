#include "DialogueSystem.h"
#include "Engine/World.h"
#include "Math/Vector.h"

// ============================================================
// UNarr_DialogueComponent
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    InteractionRadius = 300.0f;
    bHasBeenVisited = false;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UNarr_DialogueComponent::ConditionMet(
    ENarr_DialogueCondition Cond,
    bool bHasCampfire, bool bHasSpear, bool bHasAxe, bool bHasWrap,
    bool bHerdStamping, bool bNearNest, bool bIsNight, float HealthPct) const
{
    switch (Cond)
    {
    case ENarr_DialogueCondition::None:            return true;
    case ENarr_DialogueCondition::HasCampfire:     return bHasCampfire;
    case ENarr_DialogueCondition::HasSpear:        return bHasSpear;
    case ENarr_DialogueCondition::HasStoneAxe:     return bHasAxe;
    case ENarr_DialogueCondition::HasLeatherWrap:  return bHasWrap;
    case ENarr_DialogueCondition::HerdStampeding:  return bHerdStamping;
    case ENarr_DialogueCondition::NearRaptorNest:  return bNearNest;
    case ENarr_DialogueCondition::NightTime:       return bIsNight;
    case ENarr_DialogueCondition::FirstVisit:      return !bHasBeenVisited;
    case ENarr_DialogueCondition::PlayerHealthLow: return HealthPct < 0.30f;
    default: return false;
    }
}

FNarr_DialogueLine UNarr_DialogueComponent::GetBestLine(
    bool bPlayerHasCampfire,
    bool bPlayerHasSpear,
    bool bPlayerHasStoneAxe,
    bool bPlayerHasLeatherWrap,
    bool bHerdStampeding,
    bool bNearRaptorNest,
    bool bIsNight,
    float PlayerHealthPct)
{
    // Priority: first-visit-only lines first, then condition-specific, then unconditional
    FNarr_DialogueLine BestLine;
    bool bFoundConditioned = false;

    for (const FNarr_DialogueLine& Line : DialogueTree.Lines)
    {
        if (Line.bIsFirstVisitOnly && bHasBeenVisited)
            continue;

        bool bMet = ConditionMet(
            Line.RequiredCondition,
            bPlayerHasCampfire, bPlayerHasSpear, bPlayerHasStoneAxe, bPlayerHasLeatherWrap,
            bHerdStampeding, bNearRaptorNest, bIsNight, PlayerHealthPct
        );

        if (bMet)
        {
            // Prefer conditioned lines over None-condition lines
            if (Line.RequiredCondition != ENarr_DialogueCondition::None || !bFoundConditioned)
            {
                BestLine = Line;
                if (Line.RequiredCondition != ENarr_DialogueCondition::None)
                    bFoundConditioned = true;
            }
        }
    }

    // Fallback to default text if nothing matched
    if (BestLine.LineText.IsEmpty())
    {
        BestLine.LineText = DialogueTree.DefaultLine;
        BestLine.DisplayDurationSeconds = 4.0f;
    }

    return BestLine;
}

void UNarr_DialogueComponent::MarkVisited()
{
    bHasBeenVisited = true;
}

bool UNarr_DialogueComponent::IsPlayerInRange(FVector PlayerLocation) const
{
    if (!GetOwner()) return false;
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerLocation);
    return Dist <= InteractionRadius;
}

// ============================================================
// ANarr_DialogueNPC
// ============================================================

ANarr_DialogueNPC::ANarr_DialogueNPC()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    DialogueComp = CreateDefaultSubobject<UNarr_DialogueComponent>(TEXT("DialogueComp"));
    AddOwnedComponent(DialogueComp);
}

void ANarr_DialogueNPC::BeginPlay()
{
    Super::BeginPlay();
    InitialiseDialogueTree();
}

void ANarr_DialogueNPC::InitialiseDialogueTree()
{
    // Set NPC name and role on the dialogue component
    DialogueComp->DialogueTree.Role = NPCRole;

    switch (NPCRole)
    {
    case ENarr_NPCRole::TribalElder:
    {
        DialogueComp->DialogueTree.NPCName = TEXT("Tribal Elder");
        DialogueComp->DialogueTree.DefaultLine =
            TEXT("Sit. Listen. The land speaks if you are quiet enough to hear it.");

        // First visit — intro line
        FNarr_DialogueLine IntroLine;
        IntroLine.LineText = TEXT("Three winters ago, a pack of raptors took half our hunters. We survived because we listened to the land. You want to craft a spear? Good. But first — learn to read the tracks before you touch the flint.");
        IntroLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782986328227_TribalElder_NarrativeIntro.mp3");
        IntroLine.RequiredCondition = ENarr_DialogueCondition::FirstVisit;
        IntroLine.bIsFirstVisitOnly = true;
        IntroLine.DisplayDurationSeconds = 14.0f;
        DialogueComp->DialogueTree.Lines.Add(IntroLine);

        // Herd stampede warning
        FNarr_DialogueLine HerdLine;
        HerdLine.LineText = TEXT("The herd moves south when the rains come. Three hundred Brachiosaurus — the ground shakes for half a day before you see them. If you are in the valley when they stampede, there is no running. Climb. Always climb.");
        HerdLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782986354795_TribalElder_HerdMigration.mp3");
        HerdLine.RequiredCondition = ENarr_DialogueCondition::HerdStampeding;
        HerdLine.DisplayDurationSeconds = 15.0f;
        DialogueComp->DialogueTree.Lines.Add(HerdLine);

        // Night warning
        FNarr_DialogueLine NightLine;
        NightLine.LineText = TEXT("At night the raptors hunt in packs. They use the dark. You must use fire. A torch in each hand — they will not come close.");
        NightLine.RequiredCondition = ENarr_DialogueCondition::NightTime;
        NightLine.DisplayDurationSeconds = 10.0f;
        DialogueComp->DialogueTree.Lines.Add(NightLine);

        // Player has spear — progression acknowledgement
        FNarr_DialogueLine SpearLine;
        SpearLine.LineText = TEXT("You made a spear. Good. Now you are a hunter, not just prey. But a spear means nothing if your arm shakes. Practice on the trees before you face the raptors.");
        SpearLine.RequiredCondition = ENarr_DialogueCondition::HasSpear;
        SpearLine.DisplayDurationSeconds = 12.0f;
        DialogueComp->DialogueTree.Lines.Add(SpearLine);

        break;
    }

    case ENarr_NPCRole::ChiefHunter:
    {
        DialogueComp->DialogueTree.NPCName = TEXT("Chief Hunter");
        DialogueComp->DialogueTree.DefaultLine =
            TEXT("Hunting grounds are ours. Move before dawn or do not move at all.");

        // Fire warning — crafting-aware
        FNarr_DialogueLine FireLine;
        FireLine.LineText = TEXT("You made fire. Good. Now the cold cannot kill you. But fire draws predators — T-Rex can smell smoke from five miles. Keep it small. Keep it low. And never sleep without someone watching.");
        FireLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782986352417_ChiefHunter_FireWarning.mp3");
        FireLine.RequiredCondition = ENarr_DialogueCondition::HasCampfire;
        FireLine.DisplayDurationSeconds = 13.0f;
        DialogueComp->DialogueTree.Lines.Add(FireLine);

        // Low health — tactical advice
        FNarr_DialogueLine WoundedLine;
        WoundedLine.LineText = TEXT("You are bleeding. Stop moving. Find shelter. A wounded hunter who keeps running becomes a dead hunter. The prey can wait — your life cannot.");
        WoundedLine.RequiredCondition = ENarr_DialogueCondition::PlayerHealthLow;
        WoundedLine.DisplayDurationSeconds = 10.0f;
        DialogueComp->DialogueTree.Lines.Add(WoundedLine);

        // Leather wrap — armour progression
        FNarr_DialogueLine ArmourLine;
        ArmourLine.LineText = TEXT("Hide armour. Smart. A raptor claw through leather still hurts — but it does not kill. You are learning.");
        ArmourLine.RequiredCondition = ENarr_DialogueCondition::HasLeatherWrap;
        ArmourLine.DisplayDurationSeconds = 8.0f;
        DialogueComp->DialogueTree.Lines.Add(ArmourLine);

        break;
    }

    case ENarr_NPCRole::ScoutRunner:
    {
        DialogueComp->DialogueTree.NPCName = TEXT("Scout Runner");
        DialogueComp->DialogueTree.DefaultLine =
            TEXT("I have been watching the ridge for two days. Nothing moves without me seeing it first.");

        // Raptor nest warning
        FNarr_DialogueLine NestLine;
        NestLine.LineText = TEXT("The raptor nest is two ridges east. Twelve eggs — maybe more hidden under the ferns. Move fast. They will be back before the sun drops below the canyon wall. Take what you need and run.");
        NestLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782986344569_ScoutRunner_NestWarning.mp3");
        NestLine.RequiredCondition = ENarr_DialogueCondition::NearRaptorNest;
        NestLine.DisplayDurationSeconds = 13.0f;
        DialogueComp->DialogueTree.Lines.Add(NestLine);

        // Night scouting
        FNarr_DialogueLine NightScoutLine;
        NightScoutLine.LineText = TEXT("At night I move by sound, not sight. The T-Rex breathes loud — you can hear it two hundred paces before you see it. Trust your ears.");
        NightScoutLine.RequiredCondition = ENarr_DialogueCondition::NightTime;
        NightScoutLine.DisplayDurationSeconds = 10.0f;
        DialogueComp->DialogueTree.Lines.Add(NightScoutLine);

        break;
    }

    case ENarr_NPCRole::Tracker:
    {
        DialogueComp->DialogueTree.NPCName = TEXT("Tracker");
        DialogueComp->DialogueTree.DefaultLine =
            TEXT("The herd passed through here three days ago. See these tracks? Deep — a full-grown Brachiosaurus.");

        // Herd stampede — tracker reacts
        FNarr_DialogueLine StampedeTrackLine;
        StampedeTrackLine.LineText = TEXT("The herd is moving south — three hundred strong. If you want the mammoth tusks, you need to get ahead of them. But if they panic, nothing stops them. Nothing.");
        StampedeTrackLine.RequiredCondition = ENarr_DialogueCondition::HerdStampeding;
        StampedeTrackLine.DisplayDurationSeconds = 12.0f;
        DialogueComp->DialogueTree.Lines.Add(StampedeTrackLine);

        // Stone axe — resource gathering
        FNarr_DialogueLine AxeLine;
        AxeLine.LineText = TEXT("A stone axe. Now you can cut through the thick brush without losing half a day. The flint ridge to the north has better stone — sharper, harder. Worth the climb.");
        AxeLine.RequiredCondition = ENarr_DialogueCondition::HasStoneAxe;
        AxeLine.DisplayDurationSeconds = 10.0f;
        DialogueComp->DialogueTree.Lines.Add(AxeLine);

        break;
    }

    default:
        DialogueComp->DialogueTree.NPCName = TEXT("Tribe Member");
        break;
    }
}
