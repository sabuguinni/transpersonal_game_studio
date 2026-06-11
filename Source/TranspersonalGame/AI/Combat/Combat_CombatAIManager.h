#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "../../SharedTypes.h"
#include "Combat_CombatAIManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_CombatZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zone")
    TArray<AActor*> ActiveCombatants;

    FCombat_CombatZone()
    {
        ZoneName = TEXT("DefaultZone");
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        ThreatLevel = ECombat_ThreatLevel::Low;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EngagementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* Attacker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* Target;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float EngagementStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_CombatState CombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DamageDealt;

    FCombat_EngagementData()
    {
        Attacker = nullptr;
        Target = nullptr;
        EngagementStartTime = 0.0f;
        CombatState = ECombat_CombatState::Idle;
        DamageDealt = 0.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACombat_CombatAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_CombatAIManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Combat zones management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zones")
    TArray<FCombat_CombatZone> CombatZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zones")
    float ZoneUpdateInterval;

    // Active combat tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tracking")
    TArray<FCombat_EngagementData> ActiveEngagements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tracking")
    int32 MaxSimultaneousEngagements;

    // AI behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float AggressionMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float FleeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float PackCoordinationRadius;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAIUpdatesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AIUpdateFrequency;

private:
    float LastZoneUpdate;
    float LastAIUpdate;
    int32 CurrentAIUpdateIndex;

public:
    // Combat zone management
    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    void CreateCombatZone(const FString& ZoneName, const FVector& Location, float Radius, ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    void RemoveCombatZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    FCombat_CombatZone* GetCombatZoneAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    void UpdateCombatZones();

    // Combat engagement management
    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    void StartCombatEngagement(AActor* Attacker, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    void EndCombatEngagement(AActor* Attacker, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    bool IsInCombat(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat Management")
    TArray<AActor*> GetNearbyThreats(AActor* Actor, float SearchRadius);

    // AI coordination
    UFUNCTION(BlueprintCallable, Category = "AI Coordination")
    void UpdateAIBehaviors();

    UFUNCTION(BlueprintCallable, Category = "AI Coordination")
    void AssignPackBehavior(const TArray<AActor*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "AI Coordination")
    void TriggerFleeResponse(AActor* FleeingActor, AActor* Threat);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float CalculateThreatLevel(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool ShouldEngageInCombat(AActor* Potential Attacker, AActor* PotentialTarget);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LogCombatEvent(const FString& EventDescription);
};