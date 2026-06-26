// DinosaurRaptor.h — Velociraptor subclass of ADinosaurBase
// Agent #4 Performance Optimizer — Cycle 012
// Pack-hunting raptor with coordinated attack AI

#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "DinosaurRaptor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Pack coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackCoordinationRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MaxPackSize = 6;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raptor|Pack")
    int32 CurrentPackCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    bool bIsPackLeader = false;

    // Raptor-specific movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Movement")
    float SprintSpeedMultiplier = 1.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Movement")
    float JumpStrength = 650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Movement")
    bool bCanLeapAtTarget = true;

    // Leap attack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapAttackRange = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapAttackDamage = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float ClawSlashDamage = 25.0f;

    // Stealth / flanking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Behavior")
    bool bUseFlankingBehavior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Behavior")
    float FlankAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Behavior")
    float StalkingDistance = 800.0f;

    // LOD performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Performance")
    float LOD0_Distance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Performance")
    float LOD1_Distance = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Performance")
    float LOD2_Distance = 7000.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void ScanForPackMembers();

    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    bool TryLeapAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformClawSlash();

    UFUNCTION(BlueprintCallable, Category = "Raptor|Behavior")
    FVector CalculateFlankPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Raptor|Performance")
    void ApplyLODSettings();

protected:
    float LeapCooldownRemaining = 0.0f;
    bool bIsLeaping = false;
    TArray<ARaptorCharacter*> PackMembers;
};
