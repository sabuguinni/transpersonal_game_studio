#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Combat_DinosaurBehavior.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackPower = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float Aggression = 0.5f; // 0.0 = Passive, 1.0 = Highly Aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float FearLevel = 0.0f; // 0.0 = Fearless, 1.0 = Very Fearful

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    bool bIsTerritorial = true;

    FCombat_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        AttackPower = 30.0f;
        MovementSpeed = 400.0f;
        Aggression = 0.5f;
        FearLevel = 0.0f;
        bIsPackHunter = false;
        bIsTerritorial = true;
    }
};

UENUM(BlueprintType)
enum class ECombat_DinosaurBehavior : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Defensive   UMETA(DisplayName = "Defensive"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Territorial UMETA(DisplayName = "Territorial"),
    PackHunt    UMETA(DisplayName = "Pack Hunt")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_DinosaurBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ECombat_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FCombat_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ECombat_DinosaurBehavior CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    TArray<AActor*> PackMembers;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetSpecies(ECombat_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehavior(ECombat_DinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeSpeciesStats();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool ShouldAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool ShouldFleeFromTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateBehaviorLogic();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    TArray<AActor*> FindNearbyThreats() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    TArray<AActor*> FindNearbyPrey() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void AddPackMember(AActor* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void RemovePackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsAlive() const;

private:
    FTimerHandle BehaviorUpdateTimer;
    float LastDamageTime;
    AActor* LastAttacker;

    void UpdatePassiveBehavior();
    void UpdateDefensiveBehavior();
    void UpdateAggressiveBehavior();
    void UpdateHuntingBehavior();
    void UpdateFleeingBehavior();
    void UpdateTerritorialBehavior();
    void UpdatePackHuntBehavior();

    bool IsTargetThreat(AActor* Target) const;
    bool IsTargetPrey(AActor* Target) const;
    float CalculateThreatLevel(AActor* Target) const;
    void InitializeTRexStats();
    void InitializeVelociraptorStats();
    void InitializeTriceratopsStats();
    void InitializeBrachiosaurusStats();
    void InitializeAnkylosaurusStats();
    void InitializeParasaurolophusStats();
    void InitializePachycephalosaurusStats();
    void InitializeProtoceratopsStats();
    void InitializeTsintaosaurusStats();
};