#pragma once
// RaptorDinosaur.h
// Performance Optimizer — Agent #04 | PROD_CYCLE_AUTO_20260620_008
// Raptor: fast pack hunter — 80 HP, 800 speed, 40 damage, flanking behaviour

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "RaptorDinosaur.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorDinosaur();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Pack behaviour — reference to pack leader
    UPROPERTY(BlueprintReadWrite, Category = "Raptor|Pack")
    ARaptorDinosaur* PackLeader;

    // Flanking offset from leader (set by pack leader assignment)
    UPROPERTY(BlueprintReadWrite, Category = "Raptor|Pack")
    FVector FlankOffset;

    // Max pack size this raptor belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MaxPackSize;

    // Leap attack — burst of speed toward target
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapImpulse;

    // Leap cooldown in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Combat",
        meta = (AllowPrivateAccess = "true"))
    bool bLeapOnCooldown;

    // Perform leap attack toward target
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void LeapAttack(AActor* Target);

    // Assign this raptor to a pack with a leader
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void JoinPack(ARaptorDinosaur* Leader, FVector Offset);

    // Override base attack — faster, lower damage, no bleed
    virtual void Attack_Implementation(AActor* Target) override;

    // Override death — notify pack leader
    virtual void OnDeath_Implementation() override;

private:
    FTimerHandle LeapCooldownHandle;

    void ResetLeapCooldown();
};
