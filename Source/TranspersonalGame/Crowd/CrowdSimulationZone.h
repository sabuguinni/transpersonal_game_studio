#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationZone.generated.h"

UENUM(BlueprintType)
enum class ECrowd_ZoneType : uint8
{
    HerbivoreGrazing    UMETA(DisplayName = "Herbivore Grazing"),
    PredatorPatrol      UMETA(DisplayName = "Predator Patrol"),
    WateringHole        UMETA(DisplayName = "Watering Hole"),
    MigrationWaypoint   UMETA(DisplayName = "Migration Waypoint"),
    NeutralRoaming      UMETA(DisplayName = "Neutral Roaming")
};

USTRUCT(BlueprintType)
struct FCrowd_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    ECrowd_ZoneType ZoneType = ECrowd_ZoneType::NeutralRoaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    int32 MaxAgents = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float AgentSpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float LOD0Distance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float LOD1Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float LOD2Distance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    bool bIsActive = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationZone : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationZone();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    FCrowd_ZoneConfig ZoneConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Zone")
    int32 CurrentAgentCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Zone")
    bool bZoneActive = false;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    void DeactivateZone();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    int32 GetAvailableSlots() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    bool CanAcceptAgent() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    FVector GetRandomSpawnLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    void RegisterAgent();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zone")
    void UnregisterAgent();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    class UBoxComponent* ZoneBounds;
};
