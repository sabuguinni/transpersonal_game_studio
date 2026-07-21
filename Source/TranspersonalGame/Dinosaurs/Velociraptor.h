// Velociraptor.h
// Performance Optimizer Agent #4 — Cycle AUTO_20260701_010
// Pack hunter dinosaur subclass — high speed, flanking AI, coordinated attacks
#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "Velociraptor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVelociraptor : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptor();

    // Pack coordination — shared target across pack members
    UPROPERTY(BlueprintReadWrite, Category = "Velociraptor|Pack")
    AActor* PackTargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Pack")
    float PackCoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Pack")
    int32 MaxPackSize;

    // Flanking behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float FlankAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float JumpAttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float JumpAttackCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Velociraptor|Combat",
        meta = (AllowPrivateAccess = "true"))
    bool bIsJumpAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Velociraptor|Combat",
        meta = (AllowPrivateAccess = "true"))
    float LastJumpAttackTime;

    // Pack behavior methods
    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Pack")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Pack")
    TArray<AVelociraptor*> FindNearbyPackMembers() const;

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Pack")
    void SetPackTarget(AActor* NewTarget);

    // Combat methods
    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Combat")
    void PerformJumpAttack();

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Combat")
    FVector CalculateFlankPosition(AActor* Target) const;

    // Override base class
    virtual void ApplySpeciesDefaults() override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle JumpAttackCooldownTimer;
    void ResetJumpAttackCooldown();
};
