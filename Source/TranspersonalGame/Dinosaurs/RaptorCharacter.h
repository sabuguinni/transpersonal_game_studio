// RaptorCharacter.h
// Transpersonal Game Studio — Agent #4 Performance Optimizer
// Cycle: AUTO_20260701_001
// Raptor pack predator — fast, flanking, coordinated pack AI

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "RaptorCharacter.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // --- Combat ---
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformClawAttack();

    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformLeapAttack();

    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void CallPackMembers();

    // --- Pack AI ---
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void SetPackRole(int32 RoleIndex);

    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void CoordinateFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Raptor|Pack")
    bool IsPackLeader() const { return bIsPackLeader; }

    // --- Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Stats")
    float ClawDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Stats")
    float LeapDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Stats")
    float PackCallRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 PackRole; // 0=Leader, 1=Left Flanker, 2=Right Flanker

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float FlankingOffset; // lateral offset from target in flanking maneuver

private:
    bool bIsPackLeader;
    float ClawCooldown;
    float LeapCooldown;
    float PackCallCooldown;

    FTimerHandle ClawCooldownTimer;
    FTimerHandle LeapCooldownTimer;
    FTimerHandle PackCallTimer;

    void ResetClawCooldown();
    void ResetLeapCooldown();
    void ResetPackCallCooldown();
};
