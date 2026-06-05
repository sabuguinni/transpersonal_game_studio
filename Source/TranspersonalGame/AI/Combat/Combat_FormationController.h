#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_FormationController.generated.h"

class UCombat_TacticalAIComponent;

UENUM(BlueprintType)
enum class ECombat_FormationType : uint8
{
    None            UMETA(DisplayName = "No Formation"),
    AlphaLead       UMETA(DisplayName = "Alpha Leader Formation"),
    PackFlank       UMETA(DisplayName = "Pack Flanking Formation"),
    CircleHunt      UMETA(DisplayName = "Circle Hunt Formation"),
    AmbushTrap      UMETA(DisplayName = "Ambush Trap Formation")
};

USTRUCT(BlueprintType)
struct FCombat_FormationPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector RelativePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FRotator RelativeRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bIsLeaderPosition;

    FCombat_FormationPosition()
    {
        RelativePosition = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Priority = 1.0f;
        bIsLeaderPosition = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_FormationController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_FormationController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    ECombat_FormationType CurrentFormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    TArray<FCombat_FormationPosition> FormationPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    float FormationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    APawn* FormationTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    TArray<ACombat_FormationController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    ACombat_FormationController* AlphaLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    bool bIsAlphaLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    float CombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Formation")
    float FlankingDistance;

private:
    FTimerHandle FormationUpdateTimer;
    FVector LastTargetPosition;
    float FormationCohesion;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void SetFormationType(ECombat_FormationType NewFormationType);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void SetFormationTarget(APawn* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void AddPackMember(ACombat_FormationController* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void RemovePackMember(ACombat_FormationController* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void SetAsAlphaLeader(bool bNewAlphaStatus);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    FVector GetFormationPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    bool IsInFormation() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void ExecuteFormationCommand(const FString& Command);

protected:
    UFUNCTION()
    void UpdateFormation();

    UFUNCTION()
    void CalculateFormationPositions();

    UFUNCTION()
    void MoveToFormationPosition();

    UFUNCTION()
    bool ValidateFormationIntegrity();

    UFUNCTION()
    void HandleCombatEngagement();

    UFUNCTION()
    void CoordinatePackAttack();
};