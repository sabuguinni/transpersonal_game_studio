// QuestManagerComponent.cpp
// Agent #15 — Narrative & Dialogue Agent
// Full implementation of quest lifecycle management

#include "QuestManagerComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UQuestManagerComponent::UQuestManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UQuestManagerComponent::ActivateQuest(const FName& QuestID)
{
	// Prevent duplicate activation
	if (IsQuestActive(QuestID))
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s is already active."), *QuestID.ToString());
		return false;
	}

	// Build quest data from predefined builders
	FNarr_QuestData QuestData;
	bool bFound = false;

	if (QuestID == FName("HunterRescue"))       { QuestData = BuildQuest_HunterRescue();       bFound = true; }
	else if (QuestID == FName("RiverCrossing"))  { QuestData = BuildQuest_RiverCrossing();       bFound = true; }
	else if (QuestID == FName("StampedeWarning")){ QuestData = BuildQuest_StampedeWarning();     bFound = true; }
	else if (QuestID == FName("FirstHunt"))      { QuestData = BuildQuest_FirstHunt();           bFound = true; }
	else if (QuestID == FName("CampDefense"))    { QuestData = BuildQuest_CampDefense();         bFound = true; }

	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestManager: Unknown quest ID: %s"), *QuestID.ToString());
		return false;
	}

	// Check prerequisites
	for (const FName& PrereqID : QuestData.PrerequisiteQuestIDs)
	{
		if (!IsQuestCompleted(PrereqID))
		{
			UE_LOG(LogTemp, Warning, TEXT("QuestManager: Prerequisite %s not met for quest %s."), *PrereqID.ToString(), *QuestID.ToString());
			return false;
		}
	}

	QuestData.Status = ENarr_QuestStatus::Active;
	ActiveQuests.Add(QuestData);

	// Start timer if quest has a time limit
	if (QuestData.TimeLimitSeconds > 0.f)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUFunction(this, FName("FailQuest"), QuestID);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, QuestData.TimeLimitSeconds, false);
		ActiveQuestTimers.Add(QuestID, TimerHandle);
	}

	OnQuestActivated.Broadcast(QuestData);
	UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest activated — %s"), *QuestData.Title.ToString());
	return true;
}

bool UQuestManagerComponent::AbandonQuest(const FName& QuestID)
{
	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		if (ActiveQuests[i].QuestID == QuestID)
		{
			ActiveQuests[i].Status = ENarr_QuestStatus::Abandoned;
			ClearQuestTimer(QuestID);
			UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest abandoned — %s"), *QuestID.ToString());
			ActiveQuests.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UQuestManagerComponent::UpdateObjectiveProgress(const FName& QuestID, int32 ObjectiveIndex, int32 ProgressDelta)
{
	for (FNarr_QuestData& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID && Quest.Objectives.IsValidIndex(ObjectiveIndex))
		{
			FNarr_QuestObjective& Obj = Quest.Objectives[ObjectiveIndex];
			if (Obj.bIsCompleted) return false;

			Obj.CurrentProgress = FMath::Clamp(Obj.CurrentProgress + ProgressDelta, 0, Obj.RequiredProgress);
			OnObjectiveUpdated.Broadcast(QuestID, ObjectiveIndex, Obj.CurrentProgress);

			if (Obj.CurrentProgress >= Obj.RequiredProgress)
			{
				return CompleteObjective(QuestID, ObjectiveIndex);
			}
			return true;
		}
	}
	return false;
}

bool UQuestManagerComponent::CompleteObjective(const FName& QuestID, int32 ObjectiveIndex)
{
	for (FNarr_QuestData& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID && Quest.Objectives.IsValidIndex(ObjectiveIndex))
		{
			Quest.Objectives[ObjectiveIndex].bIsCompleted = true;
			UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective %d completed for quest %s"), ObjectiveIndex, *QuestID.ToString());

			// Check if all required objectives are done
			bool bAllDone = true;
			for (const FNarr_QuestObjective& Obj : Quest.Objectives)
			{
				if (!Obj.bIsOptional && !Obj.bIsCompleted)
				{
					bAllDone = false;
					break;
				}
			}

			if (bAllDone)
			{
				Quest.Status = ENarr_QuestStatus::Completed;
				CompletedQuests.Add(Quest);
				ClearQuestTimer(QuestID);
				OnQuestCompleted.Broadcast(Quest);
				ActiveQuests.Remove(Quest);
				UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest COMPLETED — %s"), *QuestID.ToString());
			}
			return true;
		}
	}
	return false;
}

bool UQuestManagerComponent::FailQuest(const FName& QuestID)
{
	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		if (ActiveQuests[i].QuestID == QuestID)
		{
			ActiveQuests[i].Status = ENarr_QuestStatus::Failed;
			FailedQuests.Add(ActiveQuests[i]);
			ClearQuestTimer(QuestID);
			OnQuestFailed.Broadcast(ActiveQuests[i]);
			UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest FAILED — %s"), *QuestID.ToString());
			ActiveQuests.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UQuestManagerComponent::IsQuestActive(const FName& QuestID) const
{
	for (const FNarr_QuestData& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID) return true;
	}
	return false;
}

bool UQuestManagerComponent::IsQuestCompleted(const FName& QuestID) const
{
	for (const FNarr_QuestData& Quest : CompletedQuests)
	{
		if (Quest.QuestID == QuestID) return true;
	}
	return false;
}

bool UQuestManagerComponent::GetQuestData(const FName& QuestID, FNarr_QuestData& OutData) const
{
	for (const FNarr_QuestData& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID) { OutData = Quest; return true; }
	}
	for (const FNarr_QuestData& Quest : CompletedQuests)
	{
		if (Quest.QuestID == QuestID) { OutData = Quest; return true; }
	}
	for (const FNarr_QuestData& Quest : FailedQuests)
	{
		if (Quest.QuestID == QuestID) { OutData = Quest; return true; }
	}
	return false;
}

TArray<FNarr_QuestData> UQuestManagerComponent::GetActiveQuestsByCategory(ENarr_QuestCategory Category) const
{
	TArray<FNarr_QuestData> Result;
	for (const FNarr_QuestData& Quest : ActiveQuests)
	{
		if (Quest.Category == Category) Result.Add(Quest);
	}
	return Result;
}

void UQuestManagerComponent::ClearQuestTimer(const FName& QuestID)
{
	if (FTimerHandle* Handle = ActiveQuestTimers.Find(QuestID))
	{
		if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(*Handle);
		ActiveQuestTimers.Remove(QuestID);
	}
}

// ─── Quest Builders ────────────────────────────────────────────────────────

FNarr_QuestData UQuestManagerComponent::BuildQuest_HunterRescue()
{
	FNarr_QuestData Q;
	Q.QuestID = FName("HunterRescue");
	Q.Title = FText::FromString("Lost in the Canyon");
	Q.Description = FText::FromString("Three hunters entered the canyon at dawn and have not returned. Find them before the Allosaurus does.");
	Q.Category = ENarr_QuestCategory::Rescue;
	Q.Difficulty = ENarr_QuestDifficulty::Hunter;
	Q.QuestGiverNPCID = FName("ElderHunter_01");
	Q.TimeLimitSeconds = 0.f; // No hard time limit — tension through narrative

	FNarr_QuestObjective Obj1;
	Obj1.ObjectiveID = FName("FindCanyonEntrance");
	Obj1.Description = FText::FromString("Reach the canyon entrance");
	Obj1.Type = ENarr_QuestObjectiveType::ReachLocation;
	Obj1.TargetLocation = FVector(1200.f, -800.f, 100.f);
	Obj1.LocationRadius = 300.f;
	Obj1.RequiredProgress = 1;
	Q.Objectives.Add(Obj1);

	FNarr_QuestObjective Obj2;
	Obj2.ObjectiveID = FName("TrackHunters");
	Obj2.Description = FText::FromString("Follow the hunters' tracks deeper into the canyon");
	Obj2.Type = ENarr_QuestObjectiveType::TrackAnimal; // reused for tracking humans
	Obj2.RequiredProgress = 3;
	Q.Objectives.Add(Obj2);

	FNarr_QuestObjective Obj3;
	Obj3.ObjectiveID = FName("RescueHunters");
	Obj3.Description = FText::FromString("Find and escort the surviving hunters out");
	Obj3.Type = ENarr_QuestObjectiveType::ProtectNPC;
	Obj3.TargetActorTag = FName("Hunter_Survivor");
	Obj3.RequiredProgress = 2; // at least 2 of 3 must survive
	Q.Objectives.Add(Obj3);

	FNarr_QuestObjective Obj4;
	Obj4.ObjectiveID = FName("KillAllosaurus");
	Obj4.Description = FText::FromString("Kill the Allosaurus blocking the exit (optional)");
	Obj4.Type = ENarr_QuestObjectiveType::KillTarget;
	Obj4.TargetActorTag = FName("Allosaurus_Canyon");
	Obj4.RequiredProgress = 1;
	Obj4.bIsOptional = true;
	Q.Objectives.Add(Obj4);

	Q.Reward.ExperiencePoints = 350;
	Q.Reward.UnlockedRecipeID = FName("Recipe_BoneTippedSpear");
	return Q;
}

FNarr_QuestData UQuestManagerComponent::BuildQuest_RiverCrossing()
{
	FNarr_QuestData Q;
	Q.QuestID = FName("RiverCrossing");
	Q.Title = FText::FromString("Before the Flood");
	Q.Description = FText::FromString("The river is rising fast. Retrieve the tribe's stored food cache from the far bank before it floods.");
	Q.Category = ENarr_QuestCategory::Survival;
	Q.Difficulty = ENarr_QuestDifficulty::Scavenger;
	Q.QuestGiverNPCID = FName("RiverWatcher_01");
	Q.TimeLimitSeconds = 600.f; // 10 minutes — the flood is real

	FNarr_QuestObjective Obj1;
	Obj1.ObjectiveID = FName("CrossRiver");
	Obj1.Description = FText::FromString("Cross the river before water level rises");
	Obj1.Type = ENarr_QuestObjectiveType::ReachLocation;
	Obj1.TargetLocation = FVector(-800.f, 1500.f, 80.f);
	Obj1.LocationRadius = 200.f;
	Obj1.RequiredProgress = 1;
	Q.Objectives.Add(Obj1);

	FNarr_QuestObjective Obj2;
	Obj2.ObjectiveID = FName("CollectFoodCache");
	Obj2.Description = FText::FromString("Collect the food cache bundles (4 of 6 minimum)");
	Obj2.Type = ENarr_QuestObjectiveType::CollectItem;
	Obj2.TargetItemID = FName("Item_FoodBundle");
	Obj2.RequiredProgress = 4;
	Q.Objectives.Add(Obj2);

	FNarr_QuestObjective Obj3;
	Obj3.ObjectiveID = FName("ReturnToSafety");
	Obj3.Description = FText::FromString("Return to high ground before the flood");
	Obj3.Type = ENarr_QuestObjectiveType::EscapeArea;
	Obj3.TargetLocation = FVector(-200.f, 400.f, 250.f);
	Obj3.LocationRadius = 400.f;
	Obj3.RequiredProgress = 1;
	Q.Objectives.Add(Obj3);

	Q.Reward.ExperiencePoints = 200;
	Q.Reward.ItemRewards.Add(FName("Item_DriedMeat_x5"));
	return Q;
}

FNarr_QuestData UQuestManagerComponent::BuildQuest_StampedeWarning()
{
	FNarr_QuestData Q;
	Q.QuestID = FName("StampedeWarning");
	Q.Title = FText::FromString("The Ground Shakes");
	Q.Description = FText::FromString("A massive Triceratops stampede is heading for camp. Help evacuate the tribe to high ground before they arrive.");
	Q.Category = ENarr_QuestCategory::Defense;
	Q.Difficulty = ENarr_QuestDifficulty::Hunter;
	Q.QuestGiverNPCID = FName("ScoutRunner_01");
	Q.TimeLimitSeconds = 480.f; // 8 minutes

	FNarr_QuestObjective Obj1;
	Obj1.ObjectiveID = FName("WarnCamp");
	Obj1.Description = FText::FromString("Alert the camp — reach the central fire");
	Obj1.Type = ENarr_QuestObjectiveType::ReachLocation;
	Obj1.TargetLocation = FVector(0.f, 0.f, 120.f);
	Obj1.LocationRadius = 150.f;
	Obj1.RequiredProgress = 1;
	Q.Objectives.Add(Obj1);

	FNarr_QuestObjective Obj2;
	Obj2.ObjectiveID = FName("EscortElders");
	Obj2.Description = FText::FromString("Escort the elders and children to the ridge (protect 5 NPCs)");
	Obj2.Type = ENarr_QuestObjectiveType::ProtectNPC;
	Obj2.TargetActorTag = FName("Civilian_Evacuee");
	Obj2.RequiredProgress = 5;
	Q.Objectives.Add(Obj2);

	FNarr_QuestObjective Obj3;
	Obj3.ObjectiveID = FName("SurviveStampede");
	Obj3.Description = FText::FromString("Survive until the herd passes");
	Obj3.Type = ENarr_QuestObjectiveType::SurviveDuration;
	Obj3.RequiredProgress = 120; // 2 minutes of survival after herd arrives
	Q.Objectives.Add(Obj3);

	Q.Reward.ExperiencePoints = 500;
	Q.Reward.UnlockedAreaID = FName("Area_HighRidgeCamp");
	return Q;
}

FNarr_QuestData UQuestManagerComponent::BuildQuest_FirstHunt()
{
	FNarr_QuestData Q;
	Q.QuestID = FName("FirstHunt");
	Q.Title = FText::FromString("Earn Your Place");
	Q.Description = FText::FromString("Prove yourself to the tribe. Join the hunting party and bring down a Triceratops to earn your place among the hunters.");
	Q.Category = ENarr_QuestCategory::Hunting;
	Q.Difficulty = ENarr_QuestDifficulty::Scavenger;
	Q.QuestGiverNPCID = FName("HuntLeader_01");
	Q.TimeLimitSeconds = 0.f;

	FNarr_QuestObjective Obj1;
	Obj1.ObjectiveID = FName("JoinHuntingParty");
	Obj1.Description = FText::FromString("Meet the hunting party at the eastern tree line");
	Obj1.Type = ENarr_QuestObjectiveType::ReachLocation;
	Obj1.TargetLocation = FVector(900.f, 200.f, 100.f);
	Obj1.LocationRadius = 250.f;
	Obj1.RequiredProgress = 1;
	Q.Objectives.Add(Obj1);

	FNarr_QuestObjective Obj2;
	Obj2.ObjectiveID = FName("TrackTriceratops");
	Obj2.Description = FText::FromString("Track the wounded Triceratops");
	Obj2.Type = ENarr_QuestObjectiveType::TrackAnimal;
	Obj2.TargetActorTag = FName("Triceratops_WoundedTarget");
	Obj2.RequiredProgress = 1;
	Q.Objectives.Add(Obj2);

	FNarr_QuestObjective Obj3;
	Obj3.ObjectiveID = FName("KillTriceratops");
	Obj3.Description = FText::FromString("Kill the Triceratops (coordinate with the hunting party)");
	Obj3.Type = ENarr_QuestObjectiveType::KillTarget;
	Obj3.TargetActorTag = FName("Triceratops_WoundedTarget");
	Obj3.RequiredProgress = 1;
	Q.Objectives.Add(Obj3);

	FNarr_QuestObjective Obj4;
	Obj4.ObjectiveID = FName("HarvestMeat");
	Obj4.Description = FText::FromString("Harvest meat from the carcass");
	Obj4.Type = ENarr_QuestObjectiveType::CollectItem;
	Obj4.TargetItemID = FName("Item_TriceratopsMeat");
	Obj4.RequiredProgress = 3;
	Q.Objectives.Add(Obj4);

	Q.Reward.ExperiencePoints = 250;
	Q.Reward.UnlockedRecipeID = FName("Recipe_HuntingSpear");
	Q.Reward.ItemRewards.Add(FName("Item_TriceratopsHide"));
	return Q;
}

FNarr_QuestData UQuestManagerComponent::BuildQuest_CampDefense()
{
	FNarr_QuestData Q;
	Q.QuestID = FName("CampDefense");
	Q.Title = FText::FromString("Hold the Line");
	Q.Description = FText::FromString("Raptors attacked the camp at night. Two hunters are dead. Guard the eastern perimeter until dawn while the survivors reinforce the barriers.");
	Q.Category = ENarr_QuestCategory::Defense;
	Q.Difficulty = ENarr_QuestDifficulty::Apex;
	Q.QuestGiverNPCID = FName("CampGuard_01");
	Q.TimeLimitSeconds = 0.f; // Survive until dawn — triggered by day/night cycle

	FNarr_QuestObjective Obj1;
	Obj1.ObjectiveID = FName("TakeEasternPost");
	Obj1.Description = FText::FromString("Reach the eastern perimeter post");
	Obj1.Type = ENarr_QuestObjectiveType::ReachLocation;
	Obj1.TargetLocation = FVector(600.f, -300.f, 100.f);
	Obj1.LocationRadius = 200.f;
	Obj1.RequiredProgress = 1;
	Q.Objectives.Add(Obj1);

	FNarr_QuestObjective Obj2;
	Obj2.ObjectiveID = FName("KillRaptors");
	Obj2.Description = FText::FromString("Kill the raptors attacking the camp");
	Obj2.Type = ENarr_QuestObjectiveType::KillTarget;
	Obj2.TargetActorTag = FName("Raptor_NightRaider");
	Obj2.RequiredProgress = 3;
	Q.Objectives.Add(Obj2);

	FNarr_QuestObjective Obj3;
	Obj3.ObjectiveID = FName("SurviveUntilDawn");
	Obj3.Description = FText::FromString("Hold the perimeter until dawn (survive 3 waves)");
	Obj3.Type = ENarr_QuestObjectiveType::SurviveDuration;
	Obj3.RequiredProgress = 3; // 3 waves
	Q.Objectives.Add(Obj3);

	FNarr_QuestObjective Obj4;
	Obj4.ObjectiveID = FName("ScoutLocation");
	Obj4.Description = FText::FromString("Scout the raptor nest location at first light (optional)");
	Obj4.Type = ENarr_QuestObjectiveType::ScoutLocation;
	Obj4.TargetLocation = FVector(1500.f, -600.f, 80.f);
	Obj4.LocationRadius = 300.f;
	Obj4.RequiredProgress = 1;
	Obj4.bIsOptional = true;
	Q.Objectives.Add(Obj4);

	Q.Reward.ExperiencePoints = 600;
	Q.Reward.UnlockedRecipeID = FName("Recipe_WoodenBarricade");
	Q.Reward.UnlockedAreaID = FName("Area_RaptorNestZone");
	return Q;
}
