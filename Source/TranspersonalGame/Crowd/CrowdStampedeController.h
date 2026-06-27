// CrowdStampedeController.h
// Agent #13 — Crowd & Traffic Simulation
// Stampede and mass-panic crowd behavior for prehistoric herds

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Alert       UMETA(DisplayName = "Alert"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Dispersing  UMETA(DisplayName = "Dispersing")
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UCrowdStampedeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdStampedeController();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerPanic(AActor* ThreatActor, float ThreatRadius = 800.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void PropagatePanicToNeighbors(float PropagationRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void FindHerdLeader();

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    ECrowd_StampedeState GetStampedeState() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    float GetStampedeSpeed() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_StampedeState StampedeState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector StampedeDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float MaxStampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float HerdCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    AActor* LeaderActor;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    AActor* PanicTriggerActor;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    float TimeSincePanic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicDecayTime;

private:
    void UpdateGrazing(float DeltaTime);
    void UpdateAlert(float DeltaTime);
    void UpdateStampeding(float DeltaTime);
    void UpdateDispersing(float DeltaTime);
    void ApplySeparationForce(float DeltaTime);
};
