// CrowdNPCSpawner.h
// Agent #13 — Crowd & Traffic Simulation
// Spawns and manages prehistoric NPC crowds (hunter-gatherer groups, migrating herds, tribal camps)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "CrowdNPCSpawner.generated.h"

UENUM(BlueprintType)
enum class ECrowd_NPCRole : uint8
{
    Hunter        UMETA(DisplayName = "Hunter"),
    Gatherer      UMETA(DisplayName = "Gatherer"),
    Scout         UMETA(DisplayName = "Scout"),
    TribalElder   UMETA(DisplayName = "Tribal Elder"),
    Child         UMETA(DisplayName = "Child"),
    Guard         UMETA(DisplayName = "Guard")
};

UENUM(BlueprintType)
enum class ECrowd_GroupState : uint8
{
    Idle          UMETA(DisplayName = "Idle"),
    Foraging      UMETA(DisplayName = "Foraging"),
    Migrating     UMETA(DisplayName = "Migrating"),
    Fleeing       UMETA(DisplayName = "Fleeing"),
    Camping       UMETA(DisplayName = "Camping"),
    Hunting       UMETA(DisplayName = "Hunting")
};

USTRUCT(BlueprintType)
struct FCrowd_NPCData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|NPC")
    ECrowd_NPCRole Role = ECrowd_NPCRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|NPC")
    float MoveSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|NPC")
    float FleeRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|NPC")
    float AwarenessRadius = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|NPC")
    bool bCarriesWeapon = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|NPC")
    FString SpeciesName = TEXT("Homo sapiens");
};

USTRUCT(BlueprintType)
struct FCrowd_GroupConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    int32 GroupSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    float SpawnRadius = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    ECrowd_GroupState InitialState = ECrowd_GroupState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    float WanderRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    bool bFleeFromDinosaurs = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Group")
    bool bReactToPlayerPresence = true;
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACrowdNPCSpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowdNPCSpawner();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawner")
    void SpawnGroup(const FCrowd_GroupConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawner")
    void DespawnAllGroups();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawner")
    void SetGroupState(ECrowd_GroupState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawner")
    int32 GetActiveNPCCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawner")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawner")
    void UpdateLOD(float DistanceFromPlayer);

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FCrowd_GroupConfig DefaultGroupConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxNPCsPerSpawner = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD_FullDetailDistance = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LOD_CullDistance = 8000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float UpdateTickRate = 0.25f;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedNPCs;

    UPROPERTY()
    ECrowd_GroupState CurrentGroupState;

    float TimeSinceLastTick = 0.f;

    void UpdateGroupBehavior(float DeltaTime);
    void HandleFleeState(float DeltaTime);
    FVector GetRandomPointInRadius(FVector Center, float Radius) const;
};
