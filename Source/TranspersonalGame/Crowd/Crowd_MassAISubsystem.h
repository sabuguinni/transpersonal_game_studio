#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassProcessingTypes.h"
#include "Engine/World.h"
#include "Crowd_MassAISubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSimulationSubsystem;

USTRUCT(BlueprintType)
struct FCrowd_DinosaurHerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector HerdCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FString DinosaurSpecies;

    FCrowd_DinosaurHerdData()
    {
        HerdCenter = FVector::ZeroVector;
        HerdRadius = 2000.0f;
        HerdSize = 5;
        DinosaurSpecies = TEXT("Triceratops");
    }
};

UENUM(BlueprintType)
enum class ECrowd_HerdBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Resting     UMETA(DisplayName = "Resting")
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassAISubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassAISubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mass AI management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnDinosaurHerd(const FCrowd_DinosaurHerdData& HerdData);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetHerdBehavior(int32 HerdIndex, ECrowd_HerdBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateHerdMigration(int32 HerdIndex, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveHerdCount() const { return ActiveHerds.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPredatorAlert(const FVector& PredatorLocation, float AlertRadius);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxSimulatedEntities(int32 MaxEntities);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableLODSystem(bool bEnable);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Simulation")
    TArray<FCrowd_DinosaurHerdData> ActiveHerds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Simulation")
    TMap<int32, ECrowd_HerdBehavior> HerdBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimulatedEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bLODSystemEnabled;

private:
    UMassEntitySubsystem* MassEntitySubsystem;
    UMassSimulationSubsystem* MassSimulationSubsystem;

    void InitializeMassAI();
    void CreateHerdArchetype();
    FMassEntityHandle SpawnHerdMember(const FVector& Location, const FString& Species);
};