#include "Narr_TribalVillageManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_TribalVillageManager::UNarr_TribalVillageManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	CurrentVillageState = ENarr_VillageState::Peaceful;
	StateChangeTime = 0.0f;
	VillageMorale = 75.0f;
	VillagePopulation = 0;
	MaxTribalMembers = 50;
	EventCheckInterval = 30.0f;
	PlayerReputation = 0.0f;
	bPlayerIsTribalMember = false;
	PlayerRole = ENarr_TribalRole::Hunter;
}

void UNarr_TribalVillageManager::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeDefaultTribalMembers();
	
	// Start event checking timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(EventCheckTimer, this, &UNarr_TribalVillageManager::CheckForRandomEvents, EventCheckInterval, true);
	}
}

void UNarr_TribalVillageManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	StateChangeTime += DeltaTime;
	UpdateTribalMemberStates();
	ProcessVillageStateChanges();
}

void UNarr_TribalVillageManager::ChangeVillageState(ENarr_VillageState NewState)
{
	if (CurrentVillageState != NewState)
	{
		ENarr_VillageState PreviousState = CurrentVillageState;
		CurrentVillageState = NewState;
		StateChangeTime = 0.0f;
		
		// Log state change
		UE_LOG(LogTemp, Warning, TEXT("Village state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
		
		// Trigger appropriate events based on new state
		switch (NewState)
		{
		case ENarr_VillageState::Alert:
			TriggerVillageEvent(TEXT("Village Alert"), TEXT("The village is on high alert due to nearby threats"));
			VillageMorale -= 10.0f;
			break;
		case ENarr_VillageState::UnderAttack:
			TriggerVillageEvent(TEXT("Under Attack"), TEXT("The village is under attack by dangerous creatures"));
			VillageMorale -= 25.0f;
			break;
		case ENarr_VillageState::Celebrating:
			TriggerVillageEvent(TEXT("Celebration"), TEXT("The village celebrates a great victory or achievement"));
			VillageMorale += 20.0f;
			break;
		case ENarr_VillageState::Mourning:
			TriggerVillageEvent(TEXT("Mourning"), TEXT("The village mourns the loss of tribal members"));
			VillageMorale -= 15.0f;
			break;
		case ENarr_VillageState::Peaceful:
			VillageMorale += 5.0f;
			break;
		}
		
		// Clamp morale
		VillageMorale = FMath::Clamp(VillageMorale, 0.0f, 100.0f);
	}
}

void UNarr_TribalVillageManager::AddTribalMember(const FString& Name, ENarr_TribalRole Role, int32 Age)
{
	if (TribalMembers.Num() < MaxTribalMembers)
	{
		FNarr_TribalMember NewMember;
		NewMember.Name = Name;
		NewMember.Role = Role;
		NewMember.Age = Age;
		NewMember.Health = 100.0f;
		NewMember.Morale = VillageMorale;
		NewMember.bIsAlive = true;
		NewMember.LastKnownLocation = FVector::ZeroVector;
		
		TribalMembers.Add(NewMember);
		VillagePopulation = TribalMembers.Num();
		
		UE_LOG(LogTemp, Log, TEXT("Added tribal member: %s, Role: %d"), *Name, (int32)Role);
	}
}

void UNarr_TribalVillageManager::RemoveTribalMember(const FString& Name)
{
	for (int32 i = TribalMembers.Num() - 1; i >= 0; i--)
	{
		if (TribalMembers[i].Name == Name)
		{
			TribalMembers.RemoveAt(i);
			VillagePopulation = TribalMembers.Num();
			UE_LOG(LogTemp, Warning, TEXT("Removed tribal member: %s"), *Name);
			break;
		}
	}
}

FNarr_TribalMember* UNarr_TribalVillageManager::FindTribalMember(const FString& Name)
{
	for (FNarr_TribalMember& Member : TribalMembers)
	{
		if (Member.Name == Name)
		{
			return &Member;
		}
	}
	return nullptr;
}

void UNarr_TribalVillageManager::TriggerVillageEvent(const FString& EventName, const FString& Description)
{
	FNarr_VillageEvent NewEvent;
	NewEvent.EventName = EventName;
	NewEvent.Description = Description;
	NewEvent.EventTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	NewEvent.RequiredState = CurrentVillageState;
	NewEvent.bIsActive = true;
	
	ActiveEvents.Add(NewEvent);
	
	UE_LOG(LogTemp, Warning, TEXT("Village Event: %s - %s"), *EventName, *Description);
}

void UNarr_TribalVillageManager::UpdatePlayerReputation(float ReputationChange)
{
	PlayerReputation += ReputationChange;
	PlayerReputation = FMath::Clamp(PlayerReputation, -100.0f, 100.0f);
	
	// Check for role promotion
	if (PlayerReputation > 50.0f && !bPlayerIsTribalMember)
	{
		bPlayerIsTribalMember = true;
		PlayerRole = ENarr_TribalRole::Hunter;
		TriggerVillageEvent(TEXT("Player Accepted"), TEXT("The player has been accepted into the tribe as a hunter"));
	}
	else if (PlayerReputation > 80.0f && PlayerRole == ENarr_TribalRole::Hunter)
	{
		PromotePlayerRole();
	}
}

void UNarr_TribalVillageManager::PromotePlayerRole()
{
	switch (PlayerRole)
	{
	case ENarr_TribalRole::Hunter:
		PlayerRole = ENarr_TribalRole::Warrior;
		TriggerVillageEvent(TEXT("Player Promoted"), TEXT("The player has been promoted to Warrior"));
		break;
	case ENarr_TribalRole::Warrior:
		PlayerRole = ENarr_TribalRole::Elder;
		TriggerVillageEvent(TEXT("Player Promoted"), TEXT("The player has been promoted to Elder"));
		break;
	default:
		break;
	}
}

TArray<FNarr_TribalMember> UNarr_TribalVillageManager::GetMembersByRole(ENarr_TribalRole Role)
{
	TArray<FNarr_TribalMember> MembersWithRole;
	
	for (const FNarr_TribalMember& Member : TribalMembers)
	{
		if (Member.Role == Role && Member.bIsAlive)
		{
			MembersWithRole.Add(Member);
		}
	}
	
	return MembersWithRole;
}

void UNarr_TribalVillageManager::HandleDinosaurThreat(const FString& DinosaurType, int32 ThreatLevel)
{
	if (ThreatLevel >= 8)
	{
		ChangeVillageState(ENarr_VillageState::UnderAttack);
		TriggerVillageEvent(TEXT("Dinosaur Attack"), FString::Printf(TEXT("A %s is attacking the village!"), *DinosaurType));
	}
	else if (ThreatLevel >= 5)
	{
		ChangeVillageState(ENarr_VillageState::Alert);
		TriggerVillageEvent(TEXT("Dinosaur Threat"), FString::Printf(TEXT("A %s has been spotted near the village"), *DinosaurType));
	}
	
	// Reduce morale based on threat level
	VillageMorale -= ThreatLevel * 2.0f;
	VillageMorale = FMath::Clamp(VillageMorale, 0.0f, 100.0f);
}

void UNarr_TribalVillageManager::CelebrateSurvival()
{
	ChangeVillageState(ENarr_VillageState::Celebrating);
	VillageMorale += 15.0f;
	VillageMorale = FMath::Clamp(VillageMorale, 0.0f, 100.0f);
	
	// Increase player reputation if they're a member
	if (bPlayerIsTribalMember)
	{
		UpdatePlayerReputation(10.0f);
	}
}

void UNarr_TribalVillageManager::CheckForRandomEvents()
{
	float RandomValue = FMath::RandRange(0.0f, 1.0f);
	
	// Random event chances based on village state
	switch (CurrentVillageState)
	{
	case ENarr_VillageState::Peaceful:
		if (RandomValue < 0.1f)
		{
			TriggerVillageEvent(TEXT("Resource Discovery"), TEXT("Scouts have found a new source of materials"));
			VillageMorale += 5.0f;
		}
		else if (RandomValue < 0.15f)
		{
			TriggerVillageEvent(TEXT("Dinosaur Sighting"), TEXT("A herbivore herd has been spotted in the distance"));
		}
		break;
		
	case ENarr_VillageState::Alert:
		if (RandomValue < 0.2f)
		{
			TriggerVillageEvent(TEXT("False Alarm"), TEXT("The threat has passed, the village can relax"));
			ChangeVillageState(ENarr_VillageState::Peaceful);
		}
		break;
		
	case ENarr_VillageState::UnderAttack:
		if (RandomValue < 0.3f)
		{
			TriggerVillageEvent(TEXT("Successful Defense"), TEXT("The village has successfully defended against the attack"));
			ChangeVillageState(ENarr_VillageState::Celebrating);
		}
		break;
	}
}

void UNarr_TribalVillageManager::UpdateTribalMemberStates()
{
	for (FNarr_TribalMember& Member : TribalMembers)
	{
		if (Member.bIsAlive)
		{
			// Update morale based on village state
			switch (CurrentVillageState)
			{
			case ENarr_VillageState::Peaceful:
				Member.Morale += 0.1f;
				break;
			case ENarr_VillageState::Celebrating:
				Member.Morale += 0.5f;
				break;
			case ENarr_VillageState::UnderAttack:
				Member.Morale -= 0.3f;
				break;
			case ENarr_VillageState::Mourning:
				Member.Morale -= 0.2f;
				break;
			}
			
			Member.Morale = FMath::Clamp(Member.Morale, 0.0f, 100.0f);
		}
	}
}

void UNarr_TribalVillageManager::ProcessVillageStateChanges()
{
	// Auto-transition from temporary states
	if (StateChangeTime > 60.0f) // 1 minute
	{
		switch (CurrentVillageState)
		{
		case ENarr_VillageState::Celebrating:
		case ENarr_VillageState::Mourning:
			ChangeVillageState(ENarr_VillageState::Peaceful);
			break;
		case ENarr_VillageState::Alert:
			if (StateChangeTime > 120.0f) // 2 minutes
			{
				ChangeVillageState(ENarr_VillageState::Peaceful);
			}
			break;
		}
	}
}

void UNarr_TribalVillageManager::InitializeDefaultTribalMembers()
{
	// Add default tribal members
	AddTribalMember(TEXT("Grok the Elder"), ENarr_TribalRole::Elder, 65);
	AddTribalMember(TEXT("Thara the Wise"), ENarr_TribalRole::Elder, 58);
	AddTribalMember(TEXT("Korg the Brave"), ENarr_TribalRole::Warrior, 35);
	AddTribalMember(TEXT("Vera the Swift"), ENarr_TribalRole::Scout, 28);
	AddTribalMember(TEXT("Brak the Strong"), ENarr_TribalRole::Warrior, 42);
	AddTribalMember(TEXT("Nala the Hunter"), ENarr_TribalRole::Hunter, 30);
	AddTribalMember(TEXT("Rok the Crafter"), ENarr_TribalRole::Crafter, 45);
	AddTribalMember(TEXT("Zara the Healer"), ENarr_TribalRole::Healer, 38);
	AddTribalMember(TEXT("Pip the Young"), ENarr_TribalRole::Child, 12);
	AddTribalMember(TEXT("Mira the Child"), ENarr_TribalRole::Child, 8);
}