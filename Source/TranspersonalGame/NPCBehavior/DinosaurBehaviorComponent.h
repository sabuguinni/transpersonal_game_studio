#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Flee        UMETA(DisplayName = "Flee"),
    Feed        UMETA(DisplayName = "Feed"),
    Rest        UMETA(DisplayName = "Rest")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurType : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pteranodon      UMETA(DisplayName = "Pteranodon")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Energy = 100.0f;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Hunger = 50.0f;
        Aggression = 50.0f;
        Fear = 0.0f;
        Energy = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DetectionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SmellRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float AlertLevel = 0.0f;

    FNPC_DetectionSettings()
    {
        SightRange = 2000.0f;
        HearingRange = 1500.0f;
        SmellRange = 1000.0f;
        SightAngle = 90.0f;
        AlertLevel = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Estado comportamental
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurBehaviorState CurrentState = ENPC_DinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurType DinosaurType = ENPC_DinosaurType::TRex;

    // Estatísticas do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FNPC_DinosaurStats Stats;

    // Configurações de detecção
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    FNPC_DetectionSettings Detection;

    // Referências de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* CurrentWaypoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 CurrentWaypointIndex = 0;

    // Configurações de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float PatrolRadius = 2000.0f;

    // Timers comportamentais
    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float IdleTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertTime = 10.0f;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void DetectPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartChase(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartFlee(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsPlayerInRange(AActor* Player, float Range);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdatePatrol();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void FindNearestWaypoint();

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void TakeDamage(float Damage);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void RestoreHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ModifyHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ModifyFear(float Amount);

private:
    // Funções internas
    void UpdateBehavior(float DeltaTime);
    void ProcessIdleState(float DeltaTime);
    void ProcessPatrolState(float DeltaTime);
    void ProcessAlertState(float DeltaTime);
    void ProcessChaseState(float DeltaTime);
    void ProcessFleeState(float DeltaTime);
    void ProcessFeedState(float DeltaTime);
    void ProcessRestState(float DeltaTime);

    void InitializeDinosaurType();
    void UpdateStats(float DeltaTime);
    AActor* FindNearestPlayer();
    FVector GetRandomPatrolPoint();
    bool CanSeeTarget(AActor* Target);
};