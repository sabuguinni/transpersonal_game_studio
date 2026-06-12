#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NPCDailyRoutine.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_RoutineActivity : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    ENPC_BehaviorType ActivityType = ENPC_BehaviorType::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float StartTime = 6.0f; // Hour of day (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float Duration = 2.0f; // Hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    FVector ActivityLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    bool bCanBeInterrupted = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* RelatedNPC = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ENPC_RelationType RelationType = ENPC_RelationType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationStrength = 0.5f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DailyRoutineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DailyRoutineComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    UDataTable* RoutineDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FNPC_RoutineActivity> DailyActivities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FNPC_SocialRelation> SocialRelations;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNPC_RoutineActivity CurrentActivity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentGameTime = 6.0f; // Start at 6 AM

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TimeScale = 1.0f; // Game hours per real hour

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FNPC_RoutineActivity GetCurrentActivity();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void SetCurrentActivity(const FNPC_RoutineActivity& NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateSocialRelation(AActor* OtherNPC, ENPC_RelationType NewRelation, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FNPC_SocialRelation GetSocialRelation(AActor* OtherNPC);

private:
    void UpdateCurrentActivity();
    void ProcessSocialInteractions();
};