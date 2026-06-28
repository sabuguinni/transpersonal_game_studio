// Velociraptor.h — Velociraptor dinosaur implementation
// Agent #4 Performance Optimizer — Transpersonal Game Studio
// Pack predator with coordinated hunting behavior

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

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === PACK BEHAVIOR ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Pack")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Pack")
    int32 PackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Pack")
    float PackCommunicationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Pack")
    TArray<AVelociraptor*> PackMembers;

    // === ATTACK BEHAVIOR ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float LeapAttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float LeapAttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float LeapCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float ClawSlashDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    bool bIsFlankingTarget;

    // === MOVEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Movement")
    float SprintSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Movement")
    float StalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Movement")
    bool bIsStalking;

    // === INTELLIGENCE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|AI")
    float FlankingAngleOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|AI")
    bool bCanOpenDoors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|AI")
    float ProblemSolvingScore;

    // === FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Pack")
    void SignalPackToAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Pack")
    void JoinPack(AVelociraptor* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Pack")
    void LeavePackFormation();

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Combat")
    void PerformLeapAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Combat")
    void PerformClawSlash(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Combat")
    void BeginFlankingManeuver(AActor* Target, float AngleOffset);

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Movement")
    void EnterStalkMode();

    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Movement")
    void ExitStalkMode();

    UFUNCTION(BlueprintPure, Category = "Velociraptor|Pack")
    bool IsPackLeader() const { return bIsPackLeader; }

    UFUNCTION(BlueprintPure, Category = "Velociraptor|Pack")
    int32 GetPackSize() const { return PackMembers.Num(); }

protected:
    virtual void OnTargetDetected(AActor* Target) override;
    virtual void OnDeath() override;

private:
    float LeapCooldownTimer;
    bool bLeapOnCooldown;
    FVector FlankingDestination;

    void UpdatePackCoordination(float DeltaTime);
    void CalculateFlankingPosition(AActor* Target);
};
