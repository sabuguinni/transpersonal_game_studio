#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Combat_EncounterManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_EncounterType : uint8
{
    Ambush          UMETA(DisplayName = "Ambush"),
    DirectAssault   UMETA(DisplayName = "Direct Assault"),
    PackHunt        UMETA(DisplayName = "Pack Hunt"),
    TerritorialDefense UMETA(DisplayName = "Territorial Defense"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Retreat         UMETA(DisplayName = "Retreat")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme"),
    Lethal      UMETA(DisplayName = "Lethal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_EncounterType EncounterType = ECombat_EncounterType::DirectAssault;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    TArray<AActor*> ParticipatingActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FVector EncounterCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float EncounterRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bIsActive = false;

    FCombat_EncounterData()
    {
        ParticipatingActors.Empty();
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_EncounterManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_EncounterManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core encounter management
    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void InitiateEncounter(ECombat_EncounterType Type, const TArray<AActor*>& Participants, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void EndEncounter(bool bPlayerVictory);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void UpdateEncounter(float DeltaTime);

    // Encounter queries
    UFUNCTION(BlueprintPure, Category = "Combat Encounter")
    bool IsEncounterActive() const { return CurrentEncounter.bIsActive; }

    UFUNCTION(BlueprintPure, Category = "Combat Encounter")
    ECombat_ThreatLevel GetCurrentThreatLevel() const { return CurrentEncounter.ThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Combat Encounter")
    float GetEncounterDuration() const { return CurrentEncounter.Duration; }

    // Tactical analysis
    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    ECombat_EncounterType DetermineOptimalEncounterType(const TArray<AActor*>& Predators, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    ECombat_ThreatLevel CalculateThreatLevel(const TArray<AActor*>& Threats, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    TArray<FVector> GenerateTacticalPositions(FVector Center, int32 NumPositions, float Radius);

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void CoordinatePackBehavior(const TArray<AActor*>& PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    FVector GetOptimalFlankingPosition(AActor* Predator, AActor* Target, const TArray<AActor*>& PackMembers);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    FCombat_EncounterData CurrentEncounter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float EncounterUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float MaxEncounterDuration = 300.0f; // 5 minutes max

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float ThreatDecayRate = 1.0f;

private:
    float LastUpdateTime = 0.0f;
    
    // Internal encounter logic
    void ProcessAmbushEncounter(float DeltaTime);
    void ProcessDirectAssaultEncounter(float DeltaTime);
    void ProcessPackHuntEncounter(float DeltaTime);
    void ProcessTerritorialDefenseEncounter(float DeltaTime);
    void ProcessStalkingEncounter(float DeltaTime);
    void ProcessRetreatEncounter(float DeltaTime);

    // Utility functions
    float CalculateDistanceToTarget(AActor* Predator, AActor* Target);
    bool IsPlayerInDanger(AActor* Player, const TArray<AActor*>& Threats);
    void BroadcastEncounterEvent(const FString& EventName, const FCombat_EncounterData& Data);
};

#include "Combat_EncounterManager.generated.h"