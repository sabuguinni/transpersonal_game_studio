#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Pteranodon UMETA(DisplayName = "Pteranodon")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting UMETA(DisplayName = "Hunting"),
    Feeding UMETA(DisplayName = "Feeding"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial"),
    PackHunting UMETA(DisplayName = "Pack Hunting")
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
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 50.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Propriedades do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats Stats;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Referências
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AAIController* DinosaurAIController;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class APawn* OwnerPawn;

    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InitializeBehavior();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartFeeding();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartFleeing(AActor* Threat);

    // Detecção e sensores
    UFUNCTION(BlueprintCallable, Category = "Detection")
    TArray<AActor*> DetectNearbyActors(float Range, TSubclassOf<AActor> ActorClass);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    AActor* FindNearestPlayer();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool IsPlayerInRange(float Range);

    // Comportamentos específicos por espécie
    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteTRexBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteRaptorBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void ExecuteBrachiosaurusBehavior(float DeltaTime);

    // Sistema de stats
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void TakeDamage(float Damage);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void RestoreHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ModifyHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    bool IsAlive() const;

private:
    // Timers e contadores
    float StateTimer = 0.0f;
    float LastPlayerDetectionTime = 0.0f;
    float PatrolTimer = 0.0f;
    
    // Posições e alvos
    FVector HomeLocation;
    FVector PatrolTarget;
    AActor* CurrentTarget = nullptr;
    
    // Funções internas
    void UpdateBehaviorBySpecies(float DeltaTime);
    void UpdateBlackboard();
    FVector GeneratePatrolPoint();
    void HandleSpeciesSpecificLogic(float DeltaTime);
};