#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Combat_TacticalCombatManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

UENUM(BlueprintType)
enum class ECombat_Formation : uint8
{
    None        UMETA(DisplayName = "No Formation"),
    Circle      UMETA(DisplayName = "Defensive Circle"),
    Line        UMETA(DisplayName = "Battle Line"),
    Ambush      UMETA(DisplayName = "Ambush Formation"),
    Scatter     UMETA(DisplayName = "Scatter Formation"),
    PackHunt    UMETA(DisplayName = "Pack Hunt Formation")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalSituation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_Formation RecommendedFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector ThreatDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> HostileActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> AlliedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatStartTime;

    FCombat_TacticalSituation()
    {
        ThreatLevel = ECombat_ThreatLevel::None;
        RecommendedFormation = ECombat_Formation::None;
        ThreatDirection = FVector::ZeroVector;
        ThreatDistance = 0.0f;
        bInCombat = false;
        CombatStartTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_CombatZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    TArray<FString> TacticalAdvantages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    bool bIsActive;

    FCombat_CombatZone()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        ZoneName = TEXT("Unknown Zone");
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_TacticalCombatManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalCombatManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat Analysis
    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    FCombat_TacticalSituation AnalyzeTacticalSituation(AActor* CentralActor, float ScanRadius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    ECombat_Formation DetermineOptimalFormation(const FCombat_TacticalSituation& Situation);

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    void InitiateCombatResponse(const FCombat_TacticalSituation& Situation);

    // Combat Zone Management
    UFUNCTION(BlueprintCallable, Category = "Combat Zone")
    void RegisterCombatZone(const FCombat_CombatZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Combat Zone")
    FCombat_CombatZone GetNearestCombatZone(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Combat Zone")
    TArray<FVector> GetEscapeRoutes(FVector CurrentLocation, float MaxDistance = 1500.0f);

    // NPC Integration
    UFUNCTION(BlueprintCallable, Category = "NPC Integration")
    void UpdateNPCCombatAwareness(AActor* NPCActor, const FCombat_TacticalSituation& Situation);

    UFUNCTION(BlueprintCallable, Category = "NPC Integration")
    void TriggerGroupDefenseBehavior(const TArray<AActor*>& NPCGroup, FVector ThreatLocation);

    // Dinosaur AI Integration
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ConfigureDinosaurCombatBehavior(AActor* DinosaurActor, bool bIsPackHunter = false);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void UpdatePackHuntingCoordination(const TArray<AActor*>& PackMembers, AActor* Target);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    FCombat_TacticalSituation CurrentSituation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zones")
    TArray<FCombat_CombatZone> RegisteredCombatZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    bool bAutoManageCombat;

private:
    float LastUpdateTime;
    
    // Internal helper methods
    ECombat_ThreatLevel CalculateThreatLevel(const TArray<AActor*>& Hostiles, float Distance);
    TArray<AActor*> FindNearbyActorsWithTag(FVector Location, float Radius, const FString& Tag);
    void BroadcastCombatAlert(const FCombat_TacticalSituation& Situation);
};