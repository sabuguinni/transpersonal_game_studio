#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NPC_BehaviorTreeManager.h"
#include "NPC_DinosaurController.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Velociraptor   UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus   UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops  UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus  UMETA(DisplayName = "Tsintaosaurus")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float FearThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    bool bIsApexPredator = false;
};

UCLASS()
class TRANSPERSONALGAME_API ANPC_DinosaurController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_DinosaurController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Species configuration
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetDinosaurSpecies(ENPC_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    ENPC_DinosaurSpecies GetDinosaurSpecies() const { return DinosaurSpecies; }

    // Behavior management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartHunting(AActor* Prey);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartFleeing(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartFeeding();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartResting();

    // Pack behavior (for pack animals like Velociraptors)
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void JoinPack(ANPC_DinosaurController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInPack() const { return PackMembers.Num() > 1 || PackLeader != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void CallPackForHelp(AActor* Threat);

    // Territory management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetTerritoryCenter(const FVector& Center);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void DefendTerritory(AActor* Intruder);

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ReactToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ReactToDinosaur(ANPC_DinosaurController* OtherDinosaur);

    // Combat system
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void TakeDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool ShouldFlee() const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ENPC_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    UBehaviorTree* DinosaurBehaviorTree;

protected:
    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UNPC_BehaviorTreeManager* BehaviorManager;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur State")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur State")
    AActor* CurrentPrey;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur State")
    AActor* CurrentThreat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur State")
    FVector TerritoryCenter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur State")
    float LastFeedTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur State")
    float LastRestTime;

    // Pack system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack System")
    ANPC_DinosaurController* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack System")
    TArray<ANPC_DinosaurController*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack System")
    bool bIsPackLeader;

    // Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float HungerTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float RestTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float AggressionCooldown;

private:
    void InitializePerception();
    void InitializeBehaviorTree();
    void ConfigureSpeciesStats();
    void UpdateNeeds(float DeltaTime);
    void UpdatePackBehavior(float DeltaTime);
    bool CanSeeActor(AActor* Actor) const;
    bool IsHostileTowards(AActor* Actor) const;
    void BroadcastToPackMembers(const FString& Message, AActor* RelevantActor = nullptr);
};