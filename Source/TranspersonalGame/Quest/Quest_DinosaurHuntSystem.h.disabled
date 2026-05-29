#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Quest_DinosaurHuntSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Velociraptor   UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus   UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Unknown        UMETA(DisplayName = "Unknown Species")
};

UENUM(BlueprintType)
enum class EQuest_HuntDifficulty : uint8
{
    Scavenge       UMETA(DisplayName = "Scavenge (Find Dead)"),
    Ambush         UMETA(DisplayName = "Ambush (Sleeping)"),
    Hunt           UMETA(DisplayName = "Active Hunt"),
    PackHunt       UMETA(DisplayName = "Pack Hunt"),
    ApexPredator   UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    EQuest_DinosaurSpecies Species = EQuest_DinosaurSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float ThreatLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    bool bIsAlive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float TrackingRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 PackSize = 1;

    FQuest_HuntTarget()
    {
        Species = EQuest_DinosaurSpecies::Unknown;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 1.0f;
        bIsAlive = true;
        TrackingRadius = 5000.0f;
        PackSize = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    FString MissionName = TEXT("Hunt Mission");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    FQuest_HuntTarget Target;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    EQuest_HuntDifficulty Difficulty = EQuest_HuntDifficulty::Hunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    float TimeLimit = 1800.0f; // 30 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    float RemainingTime = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    TArray<FString> RequiredTools;

    FQuest_HuntMission()
    {
        MissionName = TEXT("Hunt Mission");
        Difficulty = EQuest_HuntDifficulty::Hunt;
        TimeLimit = 1800.0f;
        RemainingTime = 1800.0f;
        bIsActive = false;
        bIsCompleted = false;
        StartLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_DinosaurHuntSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DinosaurHuntSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Hunt Mission Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    void StartHuntMission(EQuest_DinosaurSpecies Species, EQuest_HuntDifficulty Difficulty, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    void CompleteHuntMission();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    void CancelHuntMission();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    bool IsHuntActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    float GetRemainingTime() const;

    // Target Tracking
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    void UpdateTargetLocation(FVector NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    FVector GetTargetLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    float GetDistanceToTarget() const;

    // Hunt Validation
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    bool ValidateHuntSuccess(AActor* KilledDinosaur);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    bool CheckRequiredTools();

    // Hunt Difficulty Scaling
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    void ScaleDifficultyBySpecies(EQuest_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    FString GetHuntInstructions() const;

    // Hunt Rewards
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    void CalculateHuntRewards();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    int32 GetMeatReward() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    int32 GetBoneReward() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Hunt")
    int32 GetHideReward() const;

protected:
    // Current Hunt Mission
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hunt State")
    FQuest_HuntMission CurrentMission;

    // Hunt Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Config")
    float MaxHuntDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Config")
    float TrackingUpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Config")
    bool bShowHuntMarkers = true;

    // Reward Multipliers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Rewards")
    float MeatMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Rewards")
    float BoneMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Rewards")
    float HideMultiplier = 1.0f;

    // Calculated Rewards
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hunt Rewards")
    int32 CalculatedMeatReward = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hunt Rewards")
    int32 CalculatedBoneReward = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hunt Rewards")
    int32 CalculatedHideReward = 0;

private:
    float TrackingTimer = 0.0f;
    void UpdateHuntTimer(float DeltaTime);
    void CheckHuntTimeout();
    FString GetSpeciesDisplayName(EQuest_DinosaurSpecies Species) const;
    FString GetDifficultyDisplayName(EQuest_HuntDifficulty Difficulty) const;
};