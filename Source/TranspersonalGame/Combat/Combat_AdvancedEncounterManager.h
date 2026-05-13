#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameModeBase.h"
#include "SharedTypes.h"
#include "Combat_AdvancedEncounterManager.generated.h"

// Forward declarations
class ACombat_TacticalAIController;
class UCombat_ThreatAssessmentSystem;

UENUM(BlueprintType)
enum class ECombat_EncounterType : uint8
{
    None UMETA(DisplayName = "None"),
    SoloAmbush UMETA(DisplayName = "Solo Ambush"),
    PackHunting UMETA(DisplayName = "Pack Hunting"),
    TerritorialDefense UMETA(DisplayName = "Territorial Defense"),
    ApexPredatorEncounter UMETA(DisplayName = "Apex Predator"),
    ScavengerSwarm UMETA(DisplayName = "Scavenger Swarm"),
    MigrationConflict UMETA(DisplayName = "Migration Conflict")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    Minimal UMETA(DisplayName = "Minimal"),
    Low UMETA(DisplayName = "Low"),
    Moderate UMETA(DisplayName = "Moderate"),
    High UMETA(DisplayName = "High"),
    Extreme UMETA(DisplayName = "Extreme"),
    Lethal UMETA(DisplayName = "Lethal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EncounterZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Zone")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Zone")
    ECombat_ThreatLevel BaseThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Zone")
    TArray<ECombat_EncounterType> PossibleEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Zone")
    float EncounterCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Zone")
    bool bIsActive;

    FCombat_EncounterZone()
    {
        ZoneName = TEXT("DefaultZone");
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        BaseThreatLevel = ECombat_ThreatLevel::Low;
        EncounterCooldown = 300.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ActiveEncounter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Encounter")
    FString EncounterId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Encounter")
    ECombat_EncounterType EncounterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Encounter")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Encounter")
    TArray<AActor*> ParticipatingActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Encounter")
    FVector EncounterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Encounter")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Encounter")
    float EscalationTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Encounter")
    bool bIsEscalating;

    FCombat_ActiveEncounter()
    {
        EncounterId = TEXT("");
        EncounterType = ECombat_EncounterType::None;
        CurrentThreatLevel = ECombat_ThreatLevel::Minimal;
        EncounterLocation = FVector::ZeroVector;
        StartTime = 0.0f;
        EscalationTimer = 0.0f;
        bIsEscalating = false;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AdvancedEncounterManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AdvancedEncounterManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Encounter zone management
    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void RegisterEncounterZone(const FCombat_EncounterZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void RemoveEncounterZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    FCombat_EncounterZone* FindEncounterZone(const FVector& Location);

    // Encounter triggering
    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    bool TriggerEncounter(const FVector& Location, ECombat_EncounterType EncounterType);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void EndEncounter(const FString& EncounterId);

    // Threat escalation
    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void EscalateThreat(const FString& EncounterId, ECombat_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void ProcessThreatEscalation(float DeltaTime);

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void CoordinatePackBehavior(const TArray<AActor*>& PackMembers, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void UpdatePackFormation(const TArray<AActor*>& PackMembers, const FString& FormationType);

    // Territory management
    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    bool IsInTerritorialZone(const FVector& Location, AActor* TerritorialActor);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    void DefendTerritory(AActor* TerritorialActor, AActor* Intruder);

    // Encounter analytics
    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    ECombat_ThreatLevel CalculateOptimalThreatLevel(const FVector& PlayerLocation, float PlayerSurvivalScore);

    UFUNCTION(BlueprintCallable, Category = "Combat Encounter")
    ECombat_EncounterType SelectEncounterType(const FCombat_EncounterZone& Zone, float TimeOfDay);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Zones")
    TArray<FCombat_EncounterZone> EncounterZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Encounters")
    TArray<FCombat_ActiveEncounter> ActiveEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Settings")
    float BaseEscalationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Settings")
    float MaxEscalationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    float PackCoordinationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    float FormationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Management")
    float TerritorialDefenseRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Management")
    float TerritorialAggressionMultiplier;

private:
    // Internal encounter management
    void UpdateActiveEncounters(float DeltaTime);
    void CleanupExpiredEncounters();
    FString GenerateEncounterId();
    
    // Threat calculation helpers
    float CalculatePlayerThreatScore(const FVector& PlayerLocation);
    float CalculateEnvironmentalThreatModifier(const FVector& Location);
    
    // Pack behavior helpers
    void CalculatePackFormation(const TArray<AActor*>& PackMembers, const FString& FormationType, TArray<FVector>& OutPositions);
    void AssignPackRoles(const TArray<AActor*>& PackMembers);

    float LastFormationUpdate;
    int32 NextEncounterId;
};