#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurBehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Hunt UMETA(DisplayName = "Hunt"),
    Flee UMETA(DisplayName = "Flee"),
    Feed UMETA(DisplayName = "Feed"),
    Sleep UMETA(DisplayName = "Sleep"),
    Aggressive UMETA(DisplayName = "Aggressive")
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
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 0.5f;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        AttackDamage = 25.0f;
        MovementSpeed = 300.0f;
        DetectionRange = 2000.0f;
        AttackRange = 500.0f;
        Hunger = 50.0f;
        Aggression = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public APawn
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Componentes principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DinosaurMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* AttackSphere;

    // Propriedades do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    // Pontos de patrulha
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    int32 CurrentPatrolIndex;

    // Target atual
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    // Timers
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AttackCooldown = 2.0f;

    // Funções de comportamento
    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetPatrolPoints(const TArray<FVector>& NewPatrolPoints);

    // Detecção
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    // Configuração por espécie
    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void ConfigureForSpecies(ENPC_DinosaurSpecies NewSpecies);

private:
    void UpdateHunger(float DeltaTime);
    void UpdateBehavior(float DeltaTime);
};