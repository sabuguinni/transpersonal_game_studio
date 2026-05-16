#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NPC_BehaviorTreeComponent.h"
#include "NPC_DinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurDiet : uint8
{
    Carnivore UMETA(DisplayName = "Carnivore"),
    Herbivore UMETA(DisplayName = "Herbivore"),
    Omnivore UMETA(DisplayName = "Omnivore")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    ENPC_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    ENPC_DinosaurDiet Diet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float HuntingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float PackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    bool bIsApexPredator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    bool bCanFormPacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    TArray<ENPC_DinosaurSpecies> PreySpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    TArray<ENPC_DinosaurSpecies> PredatorSpecies;

    FNPC_DinosaurTraits()
    {
        Species = ENPC_DinosaurSpecies::TRex;
        Diet = ENPC_DinosaurDiet::Carnivore;
        MaxHealth = 1000.0f;
        AttackDamage = 100.0f;
        MovementSpeed = 400.0f;
        HuntingRange = 5000.0f;
        PackSize = 1.0f;
        bIsApexPredator = false;
        bCanFormPacks = false;
        PreySpecies.Empty();
        PredatorSpecies.Empty();
    }
};

USTRUCT(BlueprintType)
struct FNPC_PackBehavior
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    FVector PackCenterLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    float PackCohesionRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    bool bIsHunting;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    AActor* CurrentTarget;

    FNPC_PackBehavior()
    {
        PackMembers.Empty();
        PackLeader = nullptr;
        PackCenterLocation = FVector::ZeroVector;
        PackCohesionRadius = 2000.0f;
        bIsHunting = false;
        CurrentTarget = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dinosaur Traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Traits")
    FNPC_DinosaurTraits DinosaurTraits;

    // Pack Behavior
    UPROPERTY(BlueprintReadWrite, Category = "Pack Behavior")
    FNPC_PackBehavior PackBehavior;

    // Current Stats
    UPROPERTY(BlueprintReadOnly, Category = "Current Stats")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Current Stats")
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Current Stats")
    float StaminaLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Current Stats")
    float FearLevel;

    // Behavior Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    bool bEnablePackBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    bool bEnableHuntingBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
    bool bEnableTerritorialBehavior;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void InitializeDinosaurTraits(ENPC_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void JoinPack(AActor* PackLeaderActor);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInPack() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetHuntingTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    AActor* FindNearestPrey();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    AActor* FindNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool ShouldFleeFromTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool ShouldAttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void UpdateHungerAndStamina(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void PlayDinosaurSound(const FString& SoundType);

private:
    UPROPERTY()
    UNPC_BehaviorTreeComponent* BehaviorTreeComponent;

    float LastUpdateTime;
    float LastSoundTime;
    float HungerDecayRate;
    float StaminaRecoveryRate;

    void UpdatePackBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateTerritorialBehavior(float DeltaTime);
    void UpdateDinosaurStats(float DeltaTime);
    
    TArray<AActor*> FindNearbyDinosaurs(float SearchRadius);
    bool IsSpeciesCompatibleForPack(ENPC_DinosaurSpecies OtherSpecies);
    void BroadcastPackAlert(const FString& AlertType, AActor* Target);
};

#include "NPC_DinosaurAI.generated.h"