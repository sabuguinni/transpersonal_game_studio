#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SharedTypes.h"
#include "NPC_TerritorialBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_TerritorialState : uint8
{
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Chasing         UMETA(DisplayName = "Chasing"),
    Defending       UMETA(DisplayName = "Defending"),
    Returning       UMETA(DisplayName = "Returning")
};

UENUM(BlueprintType)
enum class ENPC_TerritorialResponse : uint8
{
    Ignore          UMETA(DisplayName = "Ignore"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Warn            UMETA(DisplayName = "Warn"),
    Chase           UMETA(DisplayName = "Chase"),
    Attack          UMETA(DisplayName = "Attack")
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float WaitTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bIsRestPoint;

    FNPC_PatrolPoint()
    {
        Location = FVector::ZeroVector;
        WaitTime = 3.0f;
        bIsRestPoint = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_TerritoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float AggressionLevel;

    FNPC_TerritoryData()
    {
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
        BiomeType = EBiomeType::Savanna;
        AggressionLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_IntruderData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    AActor* IntruderActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    float TimeFirstDetected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intruder")
    bool bIsHostile;

    FNPC_IntruderData()
    {
        IntruderActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        TimeFirstDetected = 0.0f;
        bIsHostile = false;
    }
};

/**
 * Sistema de comportamento territorial para NPCs e dinossauros
 * Gerencia patrulhas, detecção de intrusos e respostas territoriais
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TerritorialBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TerritorialBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURAÇÃO DO TERRITÓRIO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Setup", meta = (AllowPrivateAccess = "true"))
    FNPC_TerritoryData TerritoryData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Setup", meta = (AllowPrivateAccess = "true"))
    ENPC_TerritorialState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Setup", meta = (AllowPrivateAccess = "true"))
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Setup", meta = (AllowPrivateAccess = "true"))
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Setup", meta = (AllowPrivateAccess = "true"))
    float DetectionRadius;

    // === SISTEMA DE DETECÇÃO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_IntruderData> DetectedIntruders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float VisionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float VisionAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    float HearingRange;

    // === CONFIGURAÇÃO DE RESPOSTA ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response", meta = (AllowPrivateAccess = "true"))
    ENPC_TerritorialResponse DefaultResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response", meta = (AllowPrivateAccess = "true"))
    float ChaseDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response", meta = (AllowPrivateAccess = "true"))
    float ChaseTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response", meta = (AllowPrivateAccess = "true"))
    float ReturnToPatrolDelay;

    // === MÉTODOS PÚBLICOS ===
    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void InitializeTerritory(const FVector& Center, float Radius, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void AddPatrolPoint(const FVector& Location, float WaitTime = 3.0f, bool bIsRestPoint = false);

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void StopPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void OnIntruderDetected(AActor* Intruder, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void OnIntruderLost(AActor* Intruder);

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    FVector GetNextPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    ENPC_TerritorialResponse DetermineResponse(AActor* Intruder) const;

    // === EVENTOS BLUEPRINT ===
    UFUNCTION(BlueprintImplementableEvent, Category = "Territorial Events")
    void OnTerritoryEntered(AActor* Intruder);

    UFUNCTION(BlueprintImplementableEvent, Category = "Territorial Events")
    void OnTerritoryExited(AActor* Intruder);

    UFUNCTION(BlueprintImplementableEvent, Category = "Territorial Events")
    void OnPatrolPointReached(int32 PointIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Territorial Events")
    void OnStateChanged(ENPC_TerritorialState NewState);

private:
    // === MÉTODOS PRIVADOS ===
    void UpdatePatrolling(float DeltaTime);
    void UpdateInvestigating(float DeltaTime);
    void UpdateChasing(float DeltaTime);
    void UpdateDefending(float DeltaTime);
    void UpdateReturning(float DeltaTime);

    void ScanForIntruders();
    bool CanSeeActor(AActor* Actor) const;
    bool CanHearActor(AActor* Actor) const;
    void ProcessIntruder(AActor* Intruder);

    void ChangeState(ENPC_TerritorialState NewState);
    void MoveToLocation(const FVector& TargetLocation);

    // === VARIÁVEIS PRIVADAS ===
    UPROPERTY()
    AAIController* OwnerAIController;

    UPROPERTY()
    APawn* OwnerPawn;

    float StateTimer;
    float LastDetectionTime;
    FVector LastTargetLocation;
    AActor* CurrentTarget;

    // Timers para comportamentos
    float PatrolWaitTimer;
    float ChaseTimer;
    float ReturnTimer;
};