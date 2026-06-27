// RaptorDinosaur.h
// Transpersonal Game Studio — Prehistoric Survival Game
// Agent #04 Performance Optimizer — Cycle AUTO_20260627_013
// Raptor: fast pack predator with coordinated flanking AI

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "RaptorDinosaur.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorDinosaur();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === PACK BEHAVIOR ===

    /** Is this raptor part of a pack? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    bool bIsPackMember = true;

    /** Maximum pack size for this raptor group */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 PackSize = 3;

    /** Reference to pack leader (nullptr if this IS the leader) */
    UPROPERTY(BlueprintReadWrite, Category = "Raptor|Pack")
    ARaptorDinosaur* PackLeader = nullptr;

    /** Pack members this raptor leads (only set on leader) */
    UPROPERTY(BlueprintReadWrite, Category = "Raptor|Pack")
    TArray<ARaptorDinosaur*> PackMembers;

    /** Flanking offset from pack leader — each raptor attacks from a different angle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    FVector FlankingOffset = FVector(0, 300, 0);

    /** Is this raptor currently the pack leader? */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    bool bIsPackLeader = false;

    // === RAPTOR ABILITIES ===

    /** Pounce attack — leap onto target, dealing damage + knockdown */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Abilities")
    float PounceDamage = 35.0f;

    /** Pounce range — distance at which raptor leaps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Abilities")
    float PounceRange = 400.0f;

    /** Pounce cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Abilities")
    float PounceCooldown = 5.0f;

    /** Claw slash damage — fast melee attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Abilities")
    float ClawDamage = 20.0f;

    /** Claw attack range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Abilities")
    float ClawRange = 150.0f;

    /** Claw attack cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Abilities")
    float ClawCooldown = 1.5f;

    // === RAPTOR STATS ===

    /** Sprint speed — raptors are very fast */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Stats")
    float SprintSpeed = 1200.0f;

    /** Stamina for sustained sprint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Stats")
    float SprintStamina = 100.0f;

    /** Current sprint stamina */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Stats")
    float CurrentSprintStamina = 100.0f;

    /** Coordination radius — raptors within this range share target info */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Stats")
    float CoordinationRadius = 2000.0f;

    // === PACK FUNCTIONS ===

    /** Register this raptor as pack leader and assign members */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void InitializeAsPackLeader(const TArray<ARaptorDinosaur*>& Members);

    /** Register this raptor as a pack follower */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void RegisterWithPackLeader(ARaptorDinosaur* Leader);

    /** Called when pack leader detects a target — all members receive alert */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void AlertPackToTarget(AActor* Target);

    /** Get coordinated flanking position around target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    FVector GetFlankingPosition(AActor* Target, int32 FlankIndex) const;

    // === ABILITY FUNCTIONS ===

    /** Execute pounce attack on target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Abilities")
    void ExecutePounce(AActor* Target);

    /** Execute claw slash attack */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Abilities")
    void ExecuteClawAttack(AActor* Target);

    /** Check if pounce is ready */
    UFUNCTION(BlueprintPure, Category = "Raptor|Abilities")
    bool CanPounce() const;

    /** Check if claw attack is ready */
    UFUNCTION(BlueprintPure, Category = "Raptor|Abilities")
    bool CanClawAttack() const;

    // === PLAYER DETECTION OVERRIDE ===

    /** Override: raptors alert pack on player detection */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Raptor|AI")
    void OnPlayerDetected(AActor* Player);
    virtual void OnPlayerDetected_Implementation(AActor* Player) override;

protected:
    /** Timer handle for pounce cooldown */
    FTimerHandle PounceTimerHandle;

    /** Timer handle for claw cooldown */
    FTimerHandle ClawTimerHandle;

    /** Is pounce currently on cooldown? */
    bool bPounceCooldownActive = false;

    /** Is claw attack on cooldown? */
    bool bClawCooldownActive = false;

    /** Current pounce cooldown remaining */
    float CurrentPounceCooldown = 0.0f;

    /** Current claw cooldown remaining */
    float CurrentClawCooldown = 0.0f;

    /** Reset pounce cooldown */
    void ResetPounceCooldown();

    /** Reset claw cooldown */
    void ResetClawCooldown();

    /** Update sprint stamina */
    void UpdateSprintStamina(float DeltaTime);
};
