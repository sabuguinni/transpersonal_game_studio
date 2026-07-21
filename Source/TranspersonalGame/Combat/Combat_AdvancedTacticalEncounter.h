#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Combat_AdvancedTacticalEncounter.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"), 
    High        UMETA(DisplayName = "High Threat"),
    Extreme     UMETA(DisplayName = "Extreme Threat"),
    Lethal      UMETA(DisplayName = "Lethal Threat")
};

UENUM(BlueprintType)
enum class ECombat_EncounterPhase : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Detection   UMETA(DisplayName = "Detection Phase"),
    Stalking    UMETA(DisplayName = "Stalking Phase"),
    Positioning UMETA(DisplayName = "Positioning Phase"),
    Engagement  UMETA(DisplayName = "Active Engagement"),
    Retreat     UMETA(DisplayName = "Retreat Phase"),
    Victory     UMETA(DisplayName = "Victory Phase")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    float AdvantageScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    bool bIsHighGround = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    bool bHasCover = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    bool bIsFlankingPosition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    float DistanceToTarget = 0.0f;
};

USTRUCT(BlueprintType)
struct FCombat_EncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_EncounterPhase CurrentPhase = ECombat_EncounterPhase::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 ParticipantCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float EncounterDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bIsPackEncounter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float ThreatEscalationRate = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AdvancedTacticalEncounter : public AActor
{
    GENERATED_BODY()

public:
    ACombat_AdvancedTacticalEncounter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Encounter")
    FCombat_EncounterData EncounterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Encounter")
    TArray<FCombat_TacticalPosition> TacticalPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Encounter")
    float EncounterRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Encounter")
    float ThreatDetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Encounter")
    float EngagementRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Encounter")
    bool bAutoEscalateThreat = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Encounter")
    float PhaseTransitionDelay = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    bool bEnablePackCoordination = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    int32 MaxPackSize = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    float PackCoordinationRange = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    float FlankingDistance = 600.0f;

private:
    UPROPERTY()
    TArray<AActor*> EncounterParticipants;

    UPROPERTY()
    AActor* PrimaryTarget;

    UPROPERTY()
    float LastPhaseTransition;

    UPROPERTY()
    bool bEncounterActive;

public:
    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    void InitializeEncounter(ECombat_ThreatLevel InitialThreat, bool bIsPackBased = false);

    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    void AddParticipant(AActor* Participant);

    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    void RemoveParticipant(AActor* Participant);

    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    void SetPrimaryTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    void EscalateThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    void TransitionToPhase(ECombat_EncounterPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    FCombat_TacticalPosition FindBestTacticalPosition(AActor* ForActor, bool bPreferFlanking = false);

    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    void CalculateTacticalPositions();

    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    bool IsPlayerInThreatRange() const;

    UFUNCTION(BlueprintCallable, Category = "Tactical Encounter")
    float GetThreatIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void CoordinatePackMovement();

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void AssignPackRoles();

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    TArray<FVector> GenerateFlankingPositions(const FVector& TargetLocation);

    UFUNCTION(BlueprintPure, Category = "Tactical Encounter")
    ECombat_EncounterPhase GetCurrentPhase() const { return EncounterData.CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Tactical Encounter")
    ECombat_ThreatLevel GetThreatLevel() const { return EncounterData.ThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Tactical Encounter")
    int32 GetParticipantCount() const { return EncounterParticipants.Num(); }

    UFUNCTION(BlueprintPure, Category = "Tactical Encounter")
    bool IsEncounterActive() const { return bEncounterActive; }

protected:
    void UpdateEncounterLogic(float DeltaTime);
    void ProcessPhaseTransitions(float DeltaTime);
    void UpdateThreatEscalation(float DeltaTime);
    void ManagePackCoordination();
    bool ShouldTransitionPhase() const;
    float CalculatePositionAdvantage(const FVector& Position, const FVector& TargetPosition) const;
};