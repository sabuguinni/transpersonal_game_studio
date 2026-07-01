#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Running     UMETA(DisplayName = "Running"),
    Dispersing  UMETA(DisplayName = "Dispersing"),
    Settled     UMETA(DisplayName = "Settled")
};

UENUM(BlueprintType)
enum class ECrowd_StampedeTrigger : uint8
{
    None            UMETA(DisplayName = "None"),
    PredatorNearby  UMETA(DisplayName = "PredatorNearby"),
    LoudNoise       UMETA(DisplayName = "LoudNoise"),
    PlayerProximity UMETA(DisplayName = "PlayerProximity"),
    Fire            UMETA(DisplayName = "Fire"),
    Earthquake      UMETA(DisplayName = "Earthquake")
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float Speed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float Mass = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 AgentID = -1;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_StampedeState State = ECrowd_StampedeState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_StampedeTrigger Trigger = ECrowd_StampedeTrigger::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector RunDirection = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float TriggerRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 AgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float ElapsedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeDuration = 30.0f;
};

/**
 * UCrowd_StampedeControllerComponent
 * Manages stampede behavior for prehistoric herds.
 * Drives up to 200 agents in a panic-flee simulation.
 */
UCLASS(ClassGroup = "Crowd", meta = (BlueprintSpawnableComponent), DisplayName = "Stampede Controller Component")
class TRANSPERSONALGAME_API UCrowd_StampedeControllerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_StampedeControllerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Stampede API ---

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(ECrowd_StampedeTrigger InTrigger, FVector InTriggerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void StopStampede();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void RegisterAgent(int32 AgentID, FVector InitialLocation, float AgentMass = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void UnregisterAgent(int32 AgentID);

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    ECrowd_StampedeState GetStampedeState() const { return StampedeData.State; }

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    FVector GetFleeDirection(FVector AgentLocation) const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    float GetPanicIntensity(FVector AgentLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    int32 GetActiveAgentCount() const { return Agents.Num(); }

protected:
    void UpdateStampedePhysics(float DeltaTime);
    void PropagateStampedePanic(float DeltaTime);
    FVector ComputeFlockingForce(const FCrowd_StampedeAgent& Agent) const;
    FVector ComputeSeparationForce(const FCrowd_StampedeAgent& Agent) const;
    FVector ComputeAlignmentForce(const FCrowd_StampedeAgent& Agent) const;
    FVector ComputeCohesionForce(const FCrowd_StampedeAgent& Agent) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FCrowd_StampedeData StampedeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    TArray<FCrowd_StampedeAgent> Agents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float SeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float AlignmentRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float CohesionRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float MaxStampedeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float CurrentPanicLevel = 0.0f;
};

/**
 * ACrowd_StampedeActor
 * World actor that manages a stampede zone.
 * Place in level to define stampede-capable herd areas.
 */
UCLASS(BlueprintType, Blueprintable, DisplayName = "Stampede Zone Actor")
class TRANSPERSONALGAME_API ACrowd_StampedeActor : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_StampedeActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd|Stampede")
    void InitializeHerd();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void OnPredatorDetected(AActor* Predator);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 HerdSize = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float HerdRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FString HerdSpecies = TEXT("Triceratops");

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Stampede", meta = (AllowPrivateAccess = "true"))
    UCrowd_StampedeControllerComponent* StampedeController;
};
