#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"),
    Brachio     UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Stego       UMETA(DisplayName = "Stegosaurus")
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
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxStamina = 100.0f;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        Hunger = 50.0f;
        Aggression = 30.0f;
        Fear = 0.0f;
        Stamina = 100.0f;
        MaxStamina = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RunSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsHerbivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritorialRadius = 5000.0f;

    FNPC_DinosaurBehaviorData()
    {
        PatrolRadius = 2000.0f;
        DetectionRange = 1500.0f;
        AttackRange = 300.0f;
        MovementSpeed = 400.0f;
        RunSpeed = 800.0f;
        bIsPackHunter = false;
        bIsHerbivore = false;
        TerritorialRadius = 5000.0f;
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

    // Configuração da espécie
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Setup")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    // Estado atual
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    // Stats do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FNPC_DinosaurStats Stats;

    // Dados de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurBehaviorData BehaviorData;

    // Posição inicial para patrulha
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation")
    FVector HomeLocation;

    // Alvo atual (jogador ou presa)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
    AActor* CurrentTarget = nullptr;

    // Última posição conhecida do alvo
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
    FVector LastKnownTargetLocation;

    // Timer para mudanças de estado
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float StateTimer = 0.0f;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetSpecies(ENPC_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ChangeState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    float GetDistanceToTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void TakeDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StopHunting();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FVector GetRandomPatrolPoint() const;

protected:
    // Funções de comportamento
    void UpdateIdleState(float DeltaTime);
    void UpdatePatrollingState(float DeltaTime);
    void UpdateHuntingState(float DeltaTime);
    void UpdateFleeingState(float DeltaTime);
    void UpdateFeedingState(float DeltaTime);
    void UpdateSleepingState(float DeltaTime);

    // Funções de detecção
    void CheckForThreats();
    void CheckForPrey();
    AActor* FindNearestPlayer() const;
    
    // Funções de movimento
    void MoveToLocation(const FVector& TargetLocation, float Speed);
    void RotateTowards(const FVector& TargetLocation, float DeltaTime);

    // Configuração por espécie
    void ConfigureSpeciesDefaults();
    void ConfigureTRexBehavior();
    void ConfigureRaptorBehavior();
    void ConfigureBrachioBehavior();

    // Variáveis internas
    FVector CurrentDestination;
    bool bHasDestination = false;
    float LastDetectionCheck = 0.0f;
    float DetectionCheckInterval = 0.5f;
};

#include "DinosaurBehaviorComponent.generated.h"