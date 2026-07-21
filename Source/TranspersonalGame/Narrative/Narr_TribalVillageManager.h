#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Narr_TribalVillageManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_VillageState : uint8
{
	Peaceful,
	Alert,
	UnderAttack,
	Celebrating,
	Mourning,
	Migrating
};

UENUM(BlueprintType)
enum class ENarr_TribalRole : uint8
{
	Elder,
	Warrior,
	Hunter,
	Scout,
	Crafter,
	Healer,
	Child
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_TribalMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
	ENarr_TribalRole Role;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
	int32 Age;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
	float Morale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
	bool bIsAlive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
	FVector LastKnownLocation;

	FNarr_TribalMember()
	{
		Name = TEXT("Unknown");
		Role = ENarr_TribalRole::Hunter;
		Age = 25;
		Health = 100.0f;
		Morale = 50.0f;
		bIsAlive = true;
		LastKnownLocation = FVector::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VillageEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village Event")
	FString EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village Event")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village Event")
	float EventTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village Event")
	ENarr_VillageState RequiredState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village Event")
	bool bIsActive;

	FNarr_VillageEvent()
	{
		EventName = TEXT("Default Event");
		Description = TEXT("A village event occurred");
		EventTime = 0.0f;
		RequiredState = ENarr_VillageState::Peaceful;
		bIsActive = false;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_TribalVillageManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UNarr_TribalVillageManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Village State Management
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village State")
	ENarr_VillageState CurrentVillageState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village State")
	float StateChangeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village State")
	float VillageMorale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village State")
	int32 VillagePopulation;

	// Tribal Members
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Members")
	TArray<FNarr_TribalMember> TribalMembers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Members")
	int32 MaxTribalMembers;

	// Village Events
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village Events")
	TArray<FNarr_VillageEvent> ActiveEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Village Events")
	float EventCheckInterval;

	// Player Relationship
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Relationship")
	float PlayerReputation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Relationship")
	bool bPlayerIsTribalMember;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Relationship")
	ENarr_TribalRole PlayerRole;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Village Management")
	void ChangeVillageState(ENarr_VillageState NewState);

	UFUNCTION(BlueprintCallable, Category = "Village Management")
	void AddTribalMember(const FString& Name, ENarr_TribalRole Role, int32 Age);

	UFUNCTION(BlueprintCallable, Category = "Village Management")
	void RemoveTribalMember(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Village Management")
	FNarr_TribalMember* FindTribalMember(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Village Management")
	void TriggerVillageEvent(const FString& EventName, const FString& Description);

	UFUNCTION(BlueprintCallable, Category = "Village Management")
	void UpdatePlayerReputation(float ReputationChange);

	UFUNCTION(BlueprintCallable, Category = "Village Management")
	void PromotePlayerRole();

	UFUNCTION(BlueprintCallable, Category = "Village Management")
	TArray<FNarr_TribalMember> GetMembersByRole(ENarr_TribalRole Role);

	UFUNCTION(BlueprintCallable, Category = "Village Management")
	void HandleDinosaurThreat(const FString& DinosaurType, int32 ThreatLevel);

	UFUNCTION(BlueprintCallable, Category = "Village Management")
	void CelebrateSurvival();

private:
	FTimerHandle EventCheckTimer;
	
	void CheckForRandomEvents();
	void UpdateTribalMemberStates();
	void ProcessVillageStateChanges();
	void InitializeDefaultTribalMembers();
};