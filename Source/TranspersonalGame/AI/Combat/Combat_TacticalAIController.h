#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Combat_TacticalAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
	Patrol		UMETA(DisplayName = "Patrol"),
	Hunt		UMETA(DisplayName = "Hunt"),
	Attack		UMETA(DisplayName = "Attack"),
	Retreat		UMETA(DisplayName = "Retreat"),
	Flank		UMETA(DisplayName = "Flank"),
	Ambush		UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalFormation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	FVector LeaderPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	TArray<FVector> MemberPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	float FormationRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	bool bMaintainFormation;

	FCombat_TacticalFormation()
	{
		LeaderPosition = FVector::ZeroVector;
		FormationRadius = 500.0f;
		bMaintainFormation = true;
	}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACombat_TacticalAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// AI Perception
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAISightConfig* SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIHearingConfig* HearingConfig;

	// Tactical State
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	ECombat_TacticalState CurrentTacticalState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FCombat_TacticalFormation CurrentFormation;

	// Combat Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "3000.0"))
	float DetectionRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Aggression;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Intelligence;

	// Pack Behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
	TArray<ACombat_TacticalAIController*> PackMembers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
	ACombat_TacticalAIController* PackLeader;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
	bool bIsPackLeader;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
	float PackCohesion;

public:
	// Tactical Functions
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetTacticalState(ECombat_TacticalState NewState);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void InitiateAttack(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExecuteFlankingManeuver(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetupAmbush(FVector AmbushLocation);

	// Pack Functions
	UFUNCTION(BlueprintCallable, Category = "Pack")
	void JoinPack(ACombat_TacticalAIController* Leader);

	UFUNCTION(BlueprintCallable, Category = "Pack")
	void FormPack(const TArray<ACombat_TacticalAIController*>& Members);

	UFUNCTION(BlueprintCallable, Category = "Pack")
	void UpdateFormation(const FCombat_TacticalFormation& NewFormation);

	// Perception Callbacks
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

protected:
	// Internal Functions
	void UpdateTacticalBehavior(float DeltaTime);
	void ProcessPackCommunication();
	FVector CalculateFlankingPosition(AActor* Target);
	bool IsInAttackRange(AActor* Target) const;
	void BroadcastTacticalState();
};