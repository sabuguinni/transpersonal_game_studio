#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "Velociraptor.generated.h"

/**
 * AVelociraptor — Pack hunter dinosaur
 * Fast, low-HP flanking predator. Hunts in coordinated packs of 3-6.
 * Species stats: 400 HP, 45 dmg, 800 cm/s sprint, 8000 kg detection radius
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVelociraptor : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptor();

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void SetPackLeader(AVelociraptor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void CallPackToHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    bool IsPackLeader() const { return bIsPackLeader; }

    // Leap attack
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformLeapAttack(AActor* Target);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Pack state
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Pack", meta = (AllowPrivateAccess = "true"))
    AVelociraptor* PackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Pack", meta = (AllowPrivateAccess = "true"))
    TArray<AVelociraptor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    float PackDetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    float FlankingAngle = 120.0f; // degrees offset from pack leader

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float LeapAttackRange = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float LeapAttackDamage = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float LeapAttackCooldown = 4.0f;

private:
    bool bIsPackLeader = false;
    float LeapCooldownRemaining = 0.0f;

    FTimerHandle PackScanTimer;

    void ScanForPackMembers();
    void CoordinateFlankPosition(AActor* Target);
};
