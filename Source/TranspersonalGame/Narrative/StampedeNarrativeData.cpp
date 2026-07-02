#include "StampedeNarrativeData.h"

// ============================================================
// Constructor — registers the data object, no world dependencies
// ============================================================
UNarr_StampedeNarrativeData::UNarr_StampedeNarrativeData()
{
	// Lore defaults — set here so CDO is valid without PopulateDefaultData()
	LoreContext.ValleyName             = FText::FromString(TEXT("Thunderpass Valley"));
	LoreContext.HerdSpecies            = FText::FromString(TEXT("Parasaurolophus"));
	LoreContext.HerdSize               = 340;
	LoreContext.PredatorSpecies        = FText::FromString(TEXT("Tyrannosaurus Rex"));
	LoreContext.Season                 = FText::FromString(TEXT("Late Dry Season"));
	LoreContext.PredatorMigrationReason = FText::FromString(
		TEXT("A lone adult T-Rex has followed the Parasaurolophus migration south from the highland forests, "
		     "tracking the herd for three days. Weakened prey at the rear of the herd are its target. "
		     "When it charges, the entire herd panics and stampedes through the valley floor."));
	LoreContext.PlayerJournalEntry = FText::FromString(
		TEXT("Day 47. I nearly died today.\n\n"
		     "The elder warned me about the valley — I thought he was being cautious. He was not. "
		     "The herd came through like a river of muscle and bone, thousands of them, the ground shaking "
		     "so hard I could not keep my footing. I heard the predator before I saw it — a sound like "
		     "nothing I can describe, deep enough to feel in my chest.\n\n"
		     "I ran. I dropped my water skin. I dropped the wood I had spent the morning cutting. "
		     "I did not look back. The elder had pointed to the ridge on the east side — I climbed "
		     "on hands and knees, rocks cutting my palms, and watched the herd pour through below me "
		     "like a brown flood.\n\n"
		     "When it was over, the valley was torn apart. Tracks everywhere. Three animals had fallen "
		     "and been trampled. The predator took one and dragged it north.\n\n"
		     "I went back down and collected what I could from the wreckage. Bones, hide, meat still warm. "
		     "Enough for weeks. The elder nodded when I returned. He said nothing. He did not need to."));
}

// ============================================================
// GetLinesForPhase — returns all lines matching the given phase
// ============================================================
TArray<FNarr_StampedeDialogueLine> UNarr_StampedeNarrativeData::GetLinesForPhase(ENarr_StampedePhase Phase) const
{
	TArray<FNarr_StampedeDialogueLine> Result;
	for (const FNarr_StampedeDialogueLine& Line : DialogueLines)
	{
		if (Line.Phase == Phase)
		{
			Result.Add(Line);
		}
	}
	return Result;
}

// ============================================================
// GetPriorityLineForPhase — returns the single highest-priority line for the phase
// ============================================================
bool UNarr_StampedeNarrativeData::GetPriorityLineForPhase(ENarr_StampedePhase Phase, FNarr_StampedeDialogueLine& OutLine) const
{
	const FNarr_StampedeDialogueLine* Best = nullptr;
	for (const FNarr_StampedeDialogueLine& Line : DialogueLines)
	{
		if (Line.Phase == Phase)
		{
			if (!Best || Line.Priority > Best->Priority)
			{
				Best = &Line;
			}
		}
	}

	if (Best)
	{
		OutLine = *Best;
		return true;
	}
	return false;
}

// ============================================================
// PopulateDefaultData — seeds all dialogue lines and lore
// Called from editor via CallInEditor button
// ============================================================
#if WITH_EDITOR
void UNarr_StampedeNarrativeData::PopulateDefaultData()
{
	DialogueLines.Empty();

	// ---- PRE-QUEST BRIEFING — Elder warns player before they approach the herd ----

	// Line 1: Main warning (highest priority — plays first)
	{
		FNarr_StampedeDialogueLine Line;
		Line.Phase           = ENarr_StampedePhase::PreQuest;
		Line.SpeakerID       = FName("Elder_Tribesman");
		Line.SpeakerDisplayName = FText::FromString(TEXT("Elder"));
		Line.LineText        = FText::FromString(
			TEXT("Listen carefully, young hunter. The great herd moves through this valley every season — "
			     "thousands of hooves, shaking the earth itself. But something has changed. A lone predator, "
			     "massive, has followed them south. When it strikes, the herd panics. Everything in its path "
			     "dies. You must reach the high ground before the stampede begins. Watch the birds — when "
			     "they scatter, you have moments, not minutes."));
		Line.AudioURL        = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782952575401_Elder_Tribesman_Warning.mp3");
		Line.DisplayDuration = 0.0f; // auto
		Line.Priority        = 10;
		DialogueLines.Add(Line);
	}

	// Line 2: Lore context — why the valley is dangerous (medium priority)
	{
		FNarr_StampedeDialogueLine Line;
		Line.Phase           = ENarr_StampedePhase::PreQuest;
		Line.SpeakerID       = FName("Elder_Tribesman");
		Line.SpeakerDisplayName = FText::FromString(TEXT("Elder"));
		Line.LineText        = FText::FromString(
			TEXT("The valley has been silent for three days. That is not peace — that is a warning. "
			     "The herd knows something we do not. I have seen this before, when I was young. "
			     "The ground trembles before you hear anything. If you feel that trembling, drop "
			     "everything and run. Do not look back. Do not try to save your tools. Just run."));
		Line.AudioURL        = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782952599155_Elder_Tribesman_Lore.mp3");
		Line.DisplayDuration = 0.0f;
		Line.Priority        = 8;
		DialogueLines.Add(Line);
	}

	// ---- WATCHING HERD — Player is in the herd zone, observing ----
	{
		FNarr_StampedeDialogueLine Line;
		Line.Phase           = ENarr_StampedePhase::WatchingHerd;
		Line.SpeakerID       = FName("Elder_Tribesman");
		Line.SpeakerDisplayName = FText::FromString(TEXT("Elder"));
		Line.LineText        = FText::FromString(
			TEXT("Stay still. Do not startle them. Watch the edge of the herd — see how the animals "
			     "on the south side keep looking toward the tree line? They already sense it. "
			     "When they start moving north, that is your signal."));
		Line.AudioURL        = TEXT("");
		Line.DisplayDuration = 0.0f;
		Line.Priority        = 7;
		DialogueLines.Add(Line);
	}

	// ---- ESCAPE WINDOW — Player has 3 seconds to start running ----
	{
		FNarr_StampedeDialogueLine Line;
		Line.Phase           = ENarr_StampedePhase::EscapeWindow;
		Line.SpeakerID       = FName("Elder_Tribesman");
		Line.SpeakerDisplayName = FText::FromString(TEXT("Elder"));
		Line.LineText        = FText::FromString(
			TEXT("Now! Run — the ridge to the east! Do not stop!"));
		Line.AudioURL        = TEXT("");
		Line.DisplayDuration = 3.0f;
		Line.Priority        = 10;
		DialogueLines.Add(Line);
	}

	// ---- STAMPEDE ACTIVE — Player is running, herd is moving ----
	{
		FNarr_StampedeDialogueLine Line;
		Line.Phase           = ENarr_StampedePhase::StampedeActive;
		Line.SpeakerID       = FName("Elder_Tribesman");
		Line.SpeakerDisplayName = FText::FromString(TEXT("Elder"));
		Line.LineText        = FText::FromString(
			TEXT("The ridge! Keep climbing! Do not stop!"));
		Line.AudioURL        = TEXT("");
		Line.DisplayDuration = 2.5f;
		Line.Priority        = 10;
		DialogueLines.Add(Line);
	}

	// ---- COMPLETED — Player reached safe zone ----
	{
		FNarr_StampedeDialogueLine Line;
		Line.Phase           = ENarr_StampedePhase::Completed;
		Line.SpeakerID       = FName("Elder_Tribesman");
		Line.SpeakerDisplayName = FText::FromString(TEXT("Elder"));
		Line.LineText        = FText::FromString(
			TEXT("You made it. I watched from the ridge — I thought you were finished when the lead "
			     "animals turned. But you read the terrain well. The old hunters say the valley "
			     "remembers those who survive it. Now you are one of them. Rest. Tomorrow we follow "
			     "the herd's trail and see what the predator left behind. There will be meat, bones, "
			     "hide — enough to last the cold season."));
		Line.AudioURL        = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782952631956_Elder_Tribesman_Completion.mp3");
		Line.DisplayDuration = 0.0f;
		Line.Priority        = 10;
		DialogueLines.Add(Line);
	}

	// ---- FAILED — Player was caught in the stampede ----
	{
		FNarr_StampedeDialogueLine Line;
		Line.Phase           = ENarr_StampedePhase::Failed;
		Line.SpeakerID       = FName("Elder_Tribesman");
		Line.SpeakerDisplayName = FText::FromString(TEXT("Elder"));
		Line.LineText        = FText::FromString(
			TEXT("You are alive. That is more than I expected. The herd does not forgive hesitation. "
			     "Rest now. When you are ready, we will try again — but next time, trust your legs "
			     "before your eyes."));
		Line.AudioURL        = TEXT("");
		Line.DisplayDuration = 0.0f;
		Line.Priority        = 10;
		DialogueLines.Add(Line);
	}

	UE_LOG(LogTemp, Log, TEXT("UNarr_StampedeNarrativeData: Populated %d dialogue lines"), DialogueLines.Num());
}
#endif
