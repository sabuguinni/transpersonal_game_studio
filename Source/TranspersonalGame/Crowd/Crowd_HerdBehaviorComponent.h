#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Crowd_HerdBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha Leader"),
    Beta        UMETA(DisplayName = "Beta Lieutenant"),
    Member      UMETA(DisplayName = "Herd Member"),
    Scout       UMETA(DisplayName = "Scout"),
    Guardian    UMETA(DisplayName = "Guardian")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    TWeakObjectPtr<APawn> MemberPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    ECrowd_HerdRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float DistanceFromAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    bool bIsAlive;

    FCrowd_HerdMember()
    {
        MemberPawn = nullptr;
        Role = ECrowd_HerdRole::Member;
        DistanceFromAlpha = 0.0f;
        ThreatLevel = 0.0f;
        bIsAlive = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_HerdBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_HerdBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Herd Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    int32 MaxHerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float CohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float SeparationStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float AlignmentStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    bool bIsAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Herd State")
    TArray<FCrowd_HerdMember> HerdMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Herd State")
    TWeakObjectPtr<APawn> AlphaPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float GrazingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float NormalSpeed;

    // Herd Functions
    UFUNCTION(BlueprintCallable, Category = "Herd")
    void JoinHerd(APawn* NewMember, ECrowd_HerdRole Role = ECrowd_HerdRole::Member);

    UFUNCTION(BlueprintCallable, Category = "Herd")
    void LeaveHerd(APawn* LeavingMember);

    UFUNCTION(BlueprintCallable, Category = "Herd")
    void SetAlpha(APawn* NewAlpha);

    UFUNCTION(BlueprintCallable, Category = "Herd")
    FVector CalculateHerdMovement();

    UFUNCTION(BlueprintCallable, Category = "Herd")
    void DetectThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Herd")
    void StartFleeingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Herd")
    void StopFleeingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Herd")
    bool IsInDanger() const;

private:
    FVector CalculateCohesion();
    FVector CalculateSeparation();
    FVector CalculateAlignment();
    void UpdateHerdPositions();
    void CheckForThreats();

    bool bIsFleeingFromThreat;
    float LastThreatDetectionTime;
    FVector ThreatLocation;
};